#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define LOCAL_PORT 8888
#define BUFFER_SIZE 1024

int main() {
    int sock_fd;
    struct sockaddr_in local_addr, peer_addr;
    char peer_ip[100];
    int peer_port;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // 1. Create a standard UDP socket
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Allow local address reuse
    int reuse = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 2. Bind to local port
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(LOCAL_PORT);

    if (bind(sock_fd, (const struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Manually exchange endpoints (Simulating the Signaling Server)
    printf("Your local port is %d. Check your NAT-mapped public IP/Port.\n", LOCAL_PORT);
    printf("Enter target peer's PUBLIC IP address: ");
    scanf("%99s", peer_ip);
    printf("Enter target peer's NAT-MAPPED PUBLIC Port: ");
    scanf("%d", &peer_port);

    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(peer_port);
    inet_pton(AF_INET, peer_ip, &peer_addr.sin_addr);

    // 4. Hole Punching Phase
    // Send a burst of initial packets to force the local NAT to create a mapping rule
    printf("Punching hole to %s:%d...\n", peer_ip, peer_port);
    for (int i = 0; i < 5; i++) {
        sendto(sock_fd, "PUNCH", 5, 0, (const struct sockaddr *)&peer_addr, addr_len);
        usleep(100000); // Sleep 100ms between attempts
    }

    // Set a timeout on receive so it doesn't block forever if a packet drops
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // 5. Establish Communication Loop
    printf("Hole punched. Listening for peer and sending messages...\n");
    while (1) {
        // Try to read incoming data
        int n = recvfrom(sock_fd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&peer_addr, &addr_len);
        if (n > 0) {
            buffer[n] = '\0';
            printf("\n[Peer]: %s\n", buffer);

            // Send a response back immediately to keep the connection alive
            sendto(sock_fd, "Hello Jago", 10, 0, (const struct sockaddr *)&peer_addr, addr_len);
        }

        // Send periodic keep-alive payloads to prevent NAT timeout
        sendto(sock_fd, "Hello World", 11, 0, (const struct sockaddr *)&peer_addr, addr_len);
        sleep(2);
    }

    close(sock_fd);
    return 0;
}
