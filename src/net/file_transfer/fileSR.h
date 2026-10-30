#ifndef FILESR_H
#define FILESR_H

#include <stdint.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

#define CHUNK_SIZE 1200U
#define WINDOW_BITS 32U
#define SHA256_DIGEST_SIZE 32U

typedef struct {
    uint32_t transfer_id;
    uint32_t type;
    uint32_t chunk_index;
    uint32_t total_chunks;
    uint32_t payload_length;
    uint32_t reserved;
} PacketHeader;

enum PacketType {
    PACKET_START = 1,
    PACKET_DATA,
    PACKET_ACK,
    PACKET_NACK,
    PACKET_FIN,
    PACKET_COMPLETE,
    PACKET_PING,
    PACKET_REQUEST
};

typedef struct {
    int fd;
    struct sockaddr_in peer;
} Connection;

typedef struct {
    int part_fd;
    int state_fd;
    unsigned char *bitmap;
    size_t bitmap_size;
    char *part_path;
    char *state_path;
} ResumeStore;

/* Public API declarations */
int prepare_connection(Connection *connection, const char *ip, int port);
int sha256_fd(int fd, uint64_t file_size, unsigned char digest[SHA256_DIGEST_SIZE]);
int open_resume_store(const char *output_filename, uint64_t file_size, uint32_t total_chunks, const unsigned char hash[SHA256_DIGEST_SIZE], ResumeStore *store);
void close_resume_store(ResumeStore *store);
int serve_requests(Connection *connection, int source_fd, uint64_t file_size, uint32_t total_chunks, uint32_t transfer_id, const unsigned char source_hash[SHA256_DIGEST_SIZE]);

#endif /* FILESR_H */