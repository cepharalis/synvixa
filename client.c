#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define LOCAL_PORT 8888
#define BUFFER_SIZE 262144

int main() {
    int sock_fd;
    struct sockaddr_in local_addr, peer_addr;
    char peer_ip[100];
    int peer_port;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // 1. Create UDP Socket
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    // 2. Bind Socket to Local Port
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(LOCAL_PORT);

    if (bind(sock_fd, (const struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Manual Endpoint Exchange (Signaling Simulation)
    printf("UDP Hole Punch\n");
    printf("Your local port is %d.\n", LOCAL_PORT);
    printf("Enter target peer's PUBLIC IP address: ");
    if (scanf("%99s", peer_ip) != 1) {
        printf("Invalid IP input.\n");
        close(sock_fd);
        return 1;
    }
    printf("Enter target peer's NAT-MAPPED PUBLIC Port: ");
    if (scanf("%d", &peer_port) != 1) {
        printf("Invalid port input.\n");
        close(sock_fd);
        return 1;
    }

    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(peer_port);
    inet_pton(AF_INET, peer_ip, &peer_addr.sin_addr);

    // 4. Hole Punching Phase (Burst Packets)
    printf("Punching hole to %s:%d...\n", peer_ip, peer_port);
    for (int i = 0; i < 5; i++) {
        sendto(sock_fd, "PUNCH", 5, 0, (const struct sockaddr *)&peer_addr, addr_len);
        usleep(100000); // 100ms between attempts
    }

    // Set a timeout on recvfrom so it doesn't block indefinitely
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    printf("Hole punched! Entering live transmission loop...\n");

    // 5. Main Execution Loop
    while (1) {
        // --- STEP A: Read and format text.txt fresh every iteration ---
        FILE *file = fopen("text.txt", "r");
        if (file == NULL) {
            sleep(1);
            continue;
        }

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        // Allocate memory for escaped content + quotes + null terminator
        char *file_content = malloc(file_size * 2 + 3);
        if (file_content == NULL) {
            perror("Memory allocation failed");
            fclose(file);
            sleep(1);
            continue;
        }

        long character_count = 0;
        long pos = 0;
        file_content[pos++] = '"';

        int ch;
        while ((ch = fgetc(file)) != EOF) {
            if (ch == '\n') {
                file_content[pos++] = '\\';
                file_content[pos++] = 'n';
                character_count += 2;
            } else if (ch == '\r') {
                continue;
            } else {
                file_content[pos++] = (char)ch;
                character_count++;
            }
        }
        file_content[pos++] = '"';
        file_content[pos] = '\0';
        fclose(file); // Close file handle immediately

        // --- STEP B: Listen for incoming peer packets ---
        int n = recvfrom(sock_fd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&peer_addr, &addr_len);
        if (n > 0) {
            buffer[n] = '\0';
            printf("\n[receive]: %s\n", buffer);

            // Print debug info showing the live file size detected right now
            printf("File Size: %ld bytes, Formatted Chars: %ld\n", file_size, character_count);

            // Send the freshly read, formatted file content back to the peer
            sendto(sock_fd, file_content, strlen(file_content), 0, (const struct sockaddr *)&peer_addr, addr_len);
            printf("Sent\n");
        }

        // --- STEP C: Clean up memory allocated this loop ---
        free(file_content);

        // Send a periodic keep-alive / ping payload to keep NAT mapping alive
        sendto(sock_fd, " ", 4, 0, (const struct sockaddr *)&peer_addr, addr_len);

        usleep(500000); // 500ms delay between loop ticks
    }

    close(sock_fd);
    return 0;
}