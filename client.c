#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[]) {
    // Check if an IP address was provided in the command line
    if (argc != 2) {
        printf("Usage: ./client <your_ip_address>\n");
        return -1;
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    const char *message = "Hello from the other terminal!";

    // 1. Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    // 2. Convert the text IP string into binary network format
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // 3. Connect to your computer's IP address
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed. Check IP or firewall.\n");
        return -1;
    }

    // 4. Send the text message packet over the socket
    send(sock, message, strlen(message), 0);
    printf("Message sent successfully!\n");

    // Clean up socket
    close(sock);
    return 0;
}