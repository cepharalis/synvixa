#define _GNU_SOURCE
#define _DARWIN_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _FILE_OFFSET_BITS 64

#include "fileSR.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#define REQUEST_GROUPS_PER_BATCH 32U
#define CHUNKS_PER_BATCH (WINDOW_BITS * REQUEST_GROUPS_PER_BATCH)
#define REQUEST_BITMAP_BYTES 4U

#ifndef LOCAL_PORT
#define LOCAL_PORT 8888
#endif

#define MAX_PACKET_SIZE 1500U
#define HEADER_SIZE 24U
#define MAX_PAYLOAD (MAX_PACKET_SIZE - HEADER_SIZE)

#define SOCKET_TIMEOUT_MS 500
#define START_MAX_RETRIES 10
#define START_WAIT_ATTEMPTS 120
#define DATA_MAX_RETRIES 10
#define SENDER_IDLE_TIMEOUTS 120
#define PERSIST_CHECKPOINT_CHUNKS CHUNKS_PER_BATCH
#define HASH_REPAIR_PASSES 2
#define COMPLETION_LINGER_ATTEMPTS 10

#define START_METADATA_SIZE 40U
#define STATE_HEADER_SIZE 56U

_Static_assert(CHUNK_SIZE <= MAX_PAYLOAD,
               "CHUNK_SIZE must fit inside a UDP packet payload");

static const unsigned char STATE_MAGIC[8] = {
    'F', 'S', 'R', 'M', 'A', 'P', '0', '1'
};

static void put_u32(unsigned char *dst, uint32_t value)
{
    value = htonl(value);
    memcpy(dst, &value, sizeof(value));
}

static uint32_t get_u32(const unsigned char *src)
{
    uint32_t value;
    memcpy(&value, src, sizeof(value));
    return ntohl(value);
}

static void put_u64(unsigned char *dst, uint64_t value)
{
    put_u32(dst, (uint32_t)(value >> 32));
    put_u32(dst + 4, (uint32_t)value);
}

static uint64_t get_u64(const unsigned char *src)
{
    return ((uint64_t)get_u32(src) << 32) | get_u32(src + 4);
}

static void encode_header(unsigned char *out, const PacketHeader *header)
{
    put_u32(out + 0, header->transfer_id);
    put_u32(out + 4, header->type);
    put_u32(out + 8, header->chunk_index);
    put_u32(out + 12, header->total_chunks);
    put_u32(out + 16, header->payload_length);
    put_u32(out + 20, header->reserved);
}

static int decode_header(const unsigned char *data,
                         size_t length,
                         PacketHeader *header)
{
    if (length < HEADER_SIZE)
        return -1;

    header->transfer_id = get_u32(data + 0);
    header->type = get_u32(data + 4);
    header->chunk_index = get_u32(data + 8);
    header->total_chunks = get_u32(data + 12);
    header->payload_length = get_u32(data + 16);
    header->reserved = get_u32(data + 20);

    if (header->payload_length != length - HEADER_SIZE)
        return -1;

    if (header->type < PACKET_START || header->type > PACKET_REQUEST)
        return -1;

    return 0;
}


static int pread_all(int fd, void *buffer, size_t length, off_t offset)
{
    unsigned char *bytes = buffer;
    size_t done = 0;

    while (done < length) {
        ssize_t count = pread(fd, bytes + done, length - done,
                              offset + (off_t)done);

        if (count < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (count == 0)
            return -1;

        done += (size_t)count;
    }

    return 0;
}

static int pwrite_all(int fd, const void *buffer, size_t length, off_t offset)
{
    const unsigned char *bytes = buffer;
    size_t done = 0;

    while (done < length) {
        ssize_t count = pwrite(fd, bytes + done, length - done,
                               offset + (off_t)done);

        if (count < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (count == 0)
            return -1;

        done += (size_t)count;
    }

    return 0;
}


typedef struct {
    uint32_t state[8];
    uint64_t bit_count;
    unsigned char block[64];
    size_t block_used;
} SHA256Context;

static uint32_t sha256_rotr(uint32_t value, unsigned int count)
{
    return (value >> count) | (value << (32U - count));
}

static void sha256_transform(SHA256Context *ctx,
                             const unsigned char block[64])
{
    static const uint32_t k[64] = {
        0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
        0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
        0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
        0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
        0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
        0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
        0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
        0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
        0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
        0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
        0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
        0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
        0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
        0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
        0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
        0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
    };

    uint32_t w[64];

    for (unsigned int i = 0; i < 16; ++i) {
        size_t j = (size_t)i * 4U;

        w[i] = ((uint32_t)block[j] << 24) |
               ((uint32_t)block[j + 1] << 16) |
               ((uint32_t)block[j + 2] << 8) |
               (uint32_t)block[j + 3];
    }

    for (unsigned int i = 16; i < 64; ++i) {
        uint32_t s0 =
            sha256_rotr(w[i - 15], 7) ^
            sha256_rotr(w[i - 15], 18) ^
            (w[i - 15] >> 3);

        uint32_t s1 =
            sha256_rotr(w[i - 2], 17) ^
            sha256_rotr(w[i - 2], 19) ^
            (w[i - 2] >> 10);

        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];
    uint32_t f = ctx->state[5];
    uint32_t g = ctx->state[6];
    uint32_t h = ctx->state[7];

    for (unsigned int i = 0; i < 64; ++i) {
        uint32_t s1 =
            sha256_rotr(e, 6) ^
            sha256_rotr(e, 11) ^
            sha256_rotr(e, 25);

        uint32_t choice = (e & f) ^ (~e & g);
        uint32_t temp1 = h + s1 + choice + k[i] + w[i];

        uint32_t s0 =
            sha256_rotr(a, 2) ^
            sha256_rotr(a, 13) ^
            sha256_rotr(a, 22);

        uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = s0 + majority;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void sha256_init(SHA256Context *ctx)
{
    static const uint32_t initial[8] = {
        0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
        0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U
    };

    memcpy(ctx->state, initial, sizeof(initial));
    ctx->bit_count = 0;
    ctx->block_used = 0;
}

static void sha256_update(SHA256Context *ctx,
                          const unsigned char *data,
                          size_t length)
{
    ctx->bit_count += (uint64_t)length * 8U;

    while (length > 0) {
        size_t available = sizeof(ctx->block) - ctx->block_used;
        size_t amount = length < available ? length : available;

        memcpy(ctx->block + ctx->block_used, data, amount);

        ctx->block_used += amount;
        data += amount;
        length -= amount;

        if (ctx->block_used == sizeof(ctx->block)) {
            sha256_transform(ctx, ctx->block);
            ctx->block_used = 0;
        }
    }
}

static void sha256_final(SHA256Context *ctx,
                         unsigned char digest[SHA256_DIGEST_SIZE])
{
    uint64_t original_bit_count = ctx->bit_count;

    ctx->block[ctx->block_used++] = 0x80U;

    if (ctx->block_used > 56U) {
        memset(ctx->block + ctx->block_used, 0,
               sizeof(ctx->block) - ctx->block_used);

        sha256_transform(ctx, ctx->block);
        ctx->block_used = 0;
    }

    memset(ctx->block + ctx->block_used, 0, 56U - ctx->block_used);

    for (unsigned int i = 0; i < 8; ++i) {
        ctx->block[63U - i] =
            (unsigned char)(original_bit_count >> (i * 8U));
    }

    sha256_transform(ctx, ctx->block);

    for (unsigned int i = 0; i < 8; ++i) {
        digest[i * 4U] = (unsigned char)(ctx->state[i] >> 24);
        digest[i * 4U + 1U] = (unsigned char)(ctx->state[i] >> 16);
        digest[i * 4U + 2U] = (unsigned char)(ctx->state[i] >> 8);
        digest[i * 4U + 3U] = (unsigned char)ctx->state[i];
    }

    memset(ctx, 0, sizeof(*ctx));
}

int sha256_fd(int fd,
              uint64_t file_size,
              unsigned char digest[SHA256_DIGEST_SIZE])
{
    SHA256Context context;
    unsigned char buffer[64U * 1024U];
    uint64_t offset = 0;

    sha256_init(&context);

    while (offset < file_size) {
        uint64_t remaining = file_size - offset;

        size_t amount = remaining > sizeof(buffer)
                            ? sizeof(buffer)
                            : (size_t)remaining;

        if (offset > (uint64_t)INT64_MAX ||
            pread_all(fd, buffer, amount, (off_t)offset) < 0) {
            perror("sha256_fd: pread");
            return -1;
        }

        sha256_update(&context, buffer, amount);
        offset += amount;
    }

    sha256_final(&context, digest);
    return 0;
}



static int open_connection(Connection *connection)
{
    connection->fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (connection->fd < 0) {
        perror("socket");
        return -1;
    }

    int socket_buffer_size = 4 * 1024 * 1024;

    (void)setsockopt(connection->fd, SOL_SOCKET, SO_RCVBUF,
                     &socket_buffer_size, sizeof(socket_buffer_size));

    (void)setsockopt(connection->fd, SOL_SOCKET, SO_SNDBUF,
                     &socket_buffer_size, sizeof(socket_buffer_size));

    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));

    local.sin_family = AF_INET;

#ifdef __APPLE__
    local.sin_len = (uint8_t)sizeof(local);
#endif

    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(LOCAL_PORT);

    if (bind(connection->fd, (struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("bind");
        close(connection->fd);
        connection->fd = -1;
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = SOCKET_TIMEOUT_MS / 1000;
    timeout.tv_usec = (SOCKET_TIMEOUT_MS % 1000) * 1000;

    if (setsockopt(connection->fd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt SO_RCVTIMEO");
        close(connection->fd);
        connection->fd = -1;
        return -1;
    }

    return 0;
}

static int set_peer(Connection *connection, const char *ip, int port)
{
    if (ip == NULL || port < 1 || port > 65535) {
        fprintf(stderr, "Invalid peer address or port.\n");
        return -1;
    }

    memset(&connection->peer, 0, sizeof(connection->peer));

    connection->peer.sin_family = AF_INET;

#ifdef __APPLE__
    connection->peer.sin_len = (uint8_t)sizeof(connection->peer);
#endif

    connection->peer.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, ip, &connection->peer.sin_addr) != 1) {
        fprintf(stderr, "Invalid IPv4 address: %s\n", ip);
        return -1;
    }

    return 0;
}

int prepare_connection(Connection *connection,
                       const char *ip,
                       int port)
{
    connection->fd = -1;

    if (open_connection(connection) < 0)
        return -1;

    if (set_peer(connection, ip, port) < 0) {
        close(connection->fd);
        connection->fd = -1;
        return -1;
    }

    return 0;
}



static int send_packet(Connection *connection,
                       const PacketHeader *header,
                       const void *payload)
{
    if (header->payload_length > MAX_PAYLOAD) {
        fprintf(stderr, "Packet payload is too large.\n");
        return -1;
    }

    if (header->payload_length > 0 && payload == NULL) {
        fprintf(stderr, "Missing packet payload.\n");
        return -1;
    }

    unsigned char packet[MAX_PACKET_SIZE];
    encode_header(packet, header);

    if (header->payload_length > 0) {
        memcpy(packet + HEADER_SIZE, payload, header->payload_length);
    }

    size_t packet_length = HEADER_SIZE + header->payload_length;

    ssize_t sent = sendto(connection->fd,
                          packet,
                          packet_length,
                          0,
                          (struct sockaddr *)&connection->peer,
                          sizeof(connection->peer));

    if (sent < 0) {
        perror("sendto");
        return -1;
    }

    if ((size_t)sent != packet_length) {
        fprintf(stderr, "Unexpected short datagram send.\n");
        return -1;
    }

    return 0;
}

static int receive_packet(Connection *connection,
                          PacketHeader *header,
                          unsigned char *payload,
                          size_t payload_capacity,
                          struct sockaddr_in *sender)
{
    unsigned char packet[MAX_PACKET_SIZE];
    struct sockaddr_in source;
    socklen_t source_length = sizeof(source);

    ssize_t received = recvfrom(connection->fd,
                                packet,
                                sizeof(packet),
                                0,
                                (struct sockaddr *)&source,
                                &source_length);

    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;

        if (errno == EINTR)
            return 0;

        perror("recvfrom");
        return -1;
    }

    if (decode_header(packet, (size_t)received, header) < 0)
        return 2;

    if (header->payload_length > payload_capacity)
        return 2;

    if (header->payload_length > 0 && payload == NULL)
        return 2;

    if (header->payload_length > 0) {
        memcpy(payload,
               packet + HEADER_SIZE,
               header->payload_length);
    }

    if (sender != NULL)
        *sender = source;

    return 1;
}

static int same_peer(const struct sockaddr_in *a,
                     const struct sockaddr_in *b)
{
    return a->sin_family == b->sin_family &&
           a->sin_addr.s_addr == b->sin_addr.s_addr &&
           a->sin_port == b->sin_port;
}

static int send_start_ack(Connection *connection,
                          uint32_t transfer_id,
                          uint32_t total_chunks)
{
    PacketHeader ack;
    memset(&ack, 0, sizeof(ack));

    ack.transfer_id = transfer_id;
    ack.type = PACKET_ACK;
    ack.total_chunks = total_chunks;

    return send_packet(connection, &ack, NULL);
}


static int send_requested_chunks(Connection *connection,
                                 int source_fd,
                                 uint64_t file_size,
                                 uint32_t total_chunks,
                                 uint32_t transfer_id,
                                 uint32_t first_chunk,
                                 uint32_t bitmap)
{
    unsigned char buffer[CHUNK_SIZE];

    for (unsigned int bit = 0; bit < WINDOW_BITS; ++bit) {
        if ((bitmap & (UINT32_C(1) << bit)) == 0)
            continue;

        uint64_t index = (uint64_t)first_chunk + bit;

        if (index >= total_chunks) {
            fprintf(stderr, "REQUEST contains an out-of-range chunk.\n");
            return -1;
        }

        uint64_t offset = index * CHUNK_SIZE;

        if (offset >= file_size)
            return -1;

        uint64_t remaining = file_size - offset;

        size_t wanted = remaining > CHUNK_SIZE
                            ? CHUNK_SIZE
                            : (size_t)remaining;

        if (offset > (uint64_t)INT64_MAX ||
            pread_all(source_fd, buffer, wanted, (off_t)offset) < 0) {
            perror("send_requested_chunks: pread");
            return -1;
        }

        PacketHeader data;
        memset(&data, 0, sizeof(data));

        data.transfer_id = transfer_id;
        data.type = PACKET_DATA;
        data.chunk_index = (uint32_t)index;
        data.total_chunks = total_chunks;
        data.payload_length = (uint32_t)wanted;

        if (send_packet(connection, &data, buffer) < 0)
            return -1;
    }

    return 0;
}

int serve_requests(Connection *connection,
                   int source_fd,
                   uint64_t file_size,
                   uint32_t total_chunks,
                   uint32_t transfer_id,
                   const unsigned char source_hash[SHA256_DIGEST_SIZE])
{
    int idle_timeouts = 0;

    for (;;) {
        PacketHeader request;
        unsigned char payload[MAX_PAYLOAD];
        struct sockaddr_in sender;

        int result = receive_packet(connection,
                                    &request,
                                    payload,
                                    sizeof(payload),
                                    &sender);

        if (result == 0) {
            if (++idle_timeouts >= SENDER_IDLE_TIMEOUTS) {
                fprintf(stderr,
                        "Sender timed out waiting for receiver requests.\n");
                return -1;
            }

            continue;
        }

        if (result == -1)
            return -1;

        if (result == 2)
            continue;

        if (!same_peer(&sender, &connection->peer))
            continue;

        if (request.transfer_id != transfer_id ||
            request.total_chunks != total_chunks)
            continue;

        idle_timeouts = 0;

        if (request.type == PACKET_START &&
            request.chunk_index == 0 &&
            request.payload_length == START_METADATA_SIZE &&
            get_u64(payload) == file_size &&
            memcmp(payload + 8,
                   source_hash,
                   SHA256_DIGEST_SIZE) == 0) {

            if (send_start_ack(connection, transfer_id, total_chunks) < 0)
                return -1;

            continue;
        }

        if (request.type == PACKET_REQUEST) {
            if (request.payload_length != REQUEST_BITMAP_BYTES ||
                request.chunk_index % WINDOW_BITS != 0 ||
                request.chunk_index >= total_chunks)
                continue;

            uint32_t bitmap = get_u32(payload);

            if (bitmap == 0)
                continue;

            int valid = 1;

            for (unsigned int bit = 0; bit < WINDOW_BITS; ++bit) {
                if ((bitmap & (UINT32_C(1) << bit)) == 0)
                    continue;

                if ((uint64_t)request.chunk_index + bit >= total_chunks) {
                    valid = 0;
                    break;
                }
            }

            if (!valid)
                continue;

            if (send_requested_chunks(connection,
                                      source_fd,
                                      file_size,
                                      total_chunks,
                                      transfer_id,
                                      request.chunk_index,
                                      bitmap) < 0)
                return -1;

            continue;
        }

        if (request.type == PACKET_FIN && request.payload_length == 0) {
            PacketHeader complete;
            memset(&complete, 0, sizeof(complete));

            complete.transfer_id = transfer_id;
            complete.type = PACKET_COMPLETE;
            complete.total_chunks = total_chunks;

            for (int attempt = 0; attempt < START_MAX_RETRIES; ++attempt) {
                if (send_packet(connection, &complete, NULL) < 0)
                    return -1;

                for (;;) {
                    PacketHeader reply;
                    unsigned char reply_payload[MAX_PAYLOAD];
                    struct sockaddr_in reply_sender;

                    int reply_result = receive_packet(connection,
                                                      &reply,
                                                      reply_payload,
                                                      sizeof(reply_payload),
                                                      &reply_sender);

                    if (reply_result == 0)
                        break;

                    if (reply_result == -1)
                        return -1;

                    if (reply_result == 2)
                        continue;

                    if (!same_peer(&reply_sender, &connection->peer))
                        continue;

                    if (reply.transfer_id != transfer_id ||
                        reply.total_chunks != total_chunks ||
                        reply.payload_length != 0)
                        continue;

                    if (reply.type == PACKET_ACK &&
                        reply.chunk_index == total_chunks)
                        return 0;

                    if (reply.type == PACKET_FIN)
                        break;
                }
            }

            fprintf(stderr, "Final acknowledgement was not received.\n");
            return -1;
        }
    }
}



static char *make_sidecar_path(const char *output, const char *suffix)
{
    size_t output_len = strlen(output);
    size_t suffix_len = strlen(suffix);

    if (output_len > SIZE_MAX - suffix_len - 1)
        return NULL;

    char *path = malloc(output_len + suffix_len + 1);

    if (path == NULL)
        return NULL;

    memcpy(path, output, output_len);
    memcpy(path + output_len, suffix, suffix_len + 1);

    return path;
}

static void build_state_header(
    unsigned char header[STATE_HEADER_SIZE],
    uint64_t file_size,
    uint32_t total_chunks,
    const unsigned char hash[SHA256_DIGEST_SIZE])
{
    memcpy(header, STATE_MAGIC, sizeof(STATE_MAGIC));

    put_u64(header + 8, file_size);
    put_u32(header + 16, total_chunks);
    put_u32(header + 20, CHUNK_SIZE);

    memcpy(header + 24, hash, SHA256_DIGEST_SIZE);
}

void close_resume_store(ResumeStore *store)
{
    if (store->part_fd >= 0)
        close(store->part_fd);

    if (store->state_fd >= 0)
        close(store->state_fd);

    free(store->bitmap);
    free(store->part_path);
    free(store->state_path);

    store->part_fd = -1;
    store->state_fd = -1;
    store->bitmap = NULL;
    store->part_path = NULL;
    store->state_path = NULL;
    store->bitmap_size = 0;
}

int open_resume_store(
    const char *output_filename,
    uint64_t file_size,
    uint32_t total_chunks,
    const unsigned char hash[SHA256_DIGEST_SIZE],
    ResumeStore *store)
{
    memset(store, 0, sizeof(*store));

    store->part_fd = -1;
    store->state_fd = -1;

    struct stat output_stat;

    if (lstat(output_filename, &output_stat) == 0) {
        fprintf(stderr,
                "Output already exists; refusing to overwrite it.\n");
        return -1;
    }

    if (errno != ENOENT) {
        perror("lstat output");
        return -1;
    }

    uint64_t bitmap_size_64 = ((uint64_t)total_chunks + 7U) / 8U;

    if (bitmap_size_64 > SIZE_MAX ||
        bitmap_size_64 > (uint64_t)INT64_MAX - STATE_HEADER_SIZE) {
        fprintf(stderr, "Resume bitmap is too large for this platform.\n");
        return -1;
    }

    store->bitmap_size = (size_t)bitmap_size_64;

    store->part_path =
        make_sidecar_path(output_filename, ".filesr.part");

    store->state_path =
        make_sidecar_path(output_filename, ".filesr.state");

    store->bitmap = calloc(
        store->bitmap_size > 0 ? store->bitmap_size : 1,
        1
    );

    if (store->part_path == NULL ||
        store->state_path == NULL ||
        store->bitmap == NULL) {
        fprintf(stderr, "Could not allocate resume state.\n");
        close_resume_store(store);
        return -1;
    }

    store->part_fd = open(
        store->part_path,
        O_CREAT | O_RDWR | O_NOFOLLOW,
        0600
    );

    if (store->part_fd < 0) {
        perror("open partial file");
        close_resume_store(store);
        return -1;
    }

    store->state_fd = open(
        store->state_path,
        O_CREAT | O_RDWR | O_NOFOLLOW,
        0600
    );

    if (store->state_fd < 0) {
        perror("open state file");
        close_resume_store(store);
        return -1;
    }

    if (flock(store->state_fd, LOCK_EX | LOCK_NB) < 0) {
        perror("lock resume state (another receiver may be using it)");
        close_resume_store(store);
        return -1;
    }

    unsigned char expected_header[STATE_HEADER_SIZE];
    unsigned char actual_header[STATE_HEADER_SIZE];

    build_state_header(expected_header, file_size, total_chunks, hash);

    struct stat part_stat;
    struct stat state_stat;
    int can_resume = 0;

    if (fstat(store->part_fd, &part_stat) == 0 &&
        fstat(store->state_fd, &state_stat) == 0 &&
        S_ISREG(part_stat.st_mode) &&
        S_ISREG(state_stat.st_mode) &&
        part_stat.st_size == (off_t)file_size &&
        state_stat.st_size ==
            (off_t)(STATE_HEADER_SIZE + store->bitmap_size) &&
        pread_all(store->state_fd,
                  actual_header,
                  sizeof(actual_header),
                  0) == 0 &&
        memcmp(actual_header,
               expected_header,
               sizeof(expected_header)) == 0) {

        if (store->bitmap_size == 0 ||
            pread_all(store->state_fd,
                      store->bitmap,
                      store->bitmap_size,
                      STATE_HEADER_SIZE) == 0) {
            can_resume = 1;
        }
    }

    if (can_resume) {
        fprintf(stderr, "Found matching partial transfer; resuming.\n");
        return 0;
    }

    if (fstat(store->part_fd, &part_stat) < 0 ||
        fstat(store->state_fd, &state_stat) < 0) {
        perror("fstat resume state");
        close_resume_store(store);
        return -1;
    }

    if (!S_ISREG(part_stat.st_mode) || !S_ISREG(state_stat.st_mode)) {
        fprintf(stderr, "Resume paths must refer to regular files.\n");
        close_resume_store(store);
        return -1;
    }

    if (part_stat.st_size != 0 || state_stat.st_size != 0) {
        fprintf(stderr,
                "Existing partial state does not match this file. "
                "Choose another output name or remove the old .filesr.part "
                "and .filesr.state files to start over.\n");

        close_resume_store(store);
        return -1;
    }

    fprintf(stderr, "Creating fresh partial transfer state.\n");

    if (ftruncate(store->part_fd, (off_t)file_size) < 0 ||
        ftruncate(store->state_fd, 0) < 0 ||
        ftruncate(store->state_fd,
                  (off_t)(STATE_HEADER_SIZE + store->bitmap_size)) < 0 ||
        pwrite_all(store->state_fd,
                   expected_header,
                   sizeof(expected_header),
                   0) < 0 ||
        (store->bitmap_size > 0 &&
         pwrite_all(store->state_fd,
                    store->bitmap,
                    store->bitmap_size,
                    STATE_HEADER_SIZE) < 0) ||
        fsync(store->part_fd) < 0 ||
        fsync(store->state_fd) < 0) {

        perror("initialise resume state");
        close_resume_store(store);
        return -1;
    }

    return 0;
}


static int chunk_is_received(const unsigned char *bitmap, uint32_t index)
{
    return (bitmap[index / 8U] &
            (unsigned char)(1U << (index % 8U))) != 0;
}

static void mark_chunk_received(unsigned char *bitmap, uint32_t index)
{
    bitmap[index / 8U] |=
        (unsigned char)(1U << (index % 8U));
}

static uint32_t missing_chunk_mask(
    const unsigned char *bitmap,
    uint32_t total_chunks,
    uint32_t first_chunk)
{
    uint32_t missing = 0;

    for (unsigned int bit = 0; bit < WINDOW_BITS; ++bit) {
        uint64_t index = (uint64_t)first_chunk + bit;

        if (index >= total_chunks)
            continue;

        if (!chunk_is_received(bitmap, (uint32_t)index))
            missing |= UINT32_C(1) << bit;
    }

    return missing;
}



static int persist_bitmap_range(
    ResumeStore *store,
    uint32_t first_chunk,
    uint32_t end_chunk)
{
    if (fsync(store->part_fd) < 0) {
        perror("fsync partial file");
        return -1;
    }

    size_t first_byte = first_chunk / 8U;
    size_t end_byte = ((size_t)end_chunk + 7U) / 8U;

    if (end_byte > store->bitmap_size || first_byte > end_byte)
        return -1;

    size_t length = end_byte - first_byte;

    if (length > 0 &&
        pwrite_all(store->state_fd,
                   store->bitmap + first_byte,
                   length,
                   (off_t)(STATE_HEADER_SIZE + first_byte)) < 0) {
        perror("persist resume bitmap");
        return -1;
    }

    if (fsync(store->state_fd) < 0) {
        perror("fsync resume state");
        return -1;
    }

    return 0;
}