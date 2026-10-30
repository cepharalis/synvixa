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
    uint32_t transfer_id, type, chunk_index;
    uint32_t total_chunks, payload_length, reserved;
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
    int part_fd, state_fd;
    unsigned char *bitmap;
    size_t bitmap_size;
    char *part_path, *state_path;
} ResumeStore;

int prepare_connection(Connection *, const char *, int);
int sha256_fd(int, uint64_t, unsigned char [SHA256_DIGEST_SIZE]);
int open_resume_store(const char *, uint64_t, uint32_t,
                      const unsigned char [SHA256_DIGEST_SIZE],
                      ResumeStore *);
void close_resume_store(ResumeStore *);
int serve_requests(Connection *, int, uint64_t, uint32_t, uint32_t,
                   const unsigned char [SHA256_DIGEST_SIZE]);

int fileSR_send(const char *, const char *, int);
int fileSR_receive(const char *, const char *, int);

#endif