#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket_layer.h" // Assuming this includes necessary custom definitions

#define MAX_SIZE 2048

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <port> <filename>\n", argv[0]);
        return 1;
    }

    uint16_t port = atoi(argv[1]);
    const char *filename = argv[2];

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    // Server address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Connect to localhost

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return 1;
    }

    // Open file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        close(sockfd);
        return 1;
    }

    // Check file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size > MAX_SIZE) {
        fclose(file);
        close(sockfd);
        fprintf(stderr, "File too large to send\n");
        return 1;
    }
    fseek(file, 0, SEEK_SET);

    // Send file size
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%ld|", file_size);
    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        perror("Failed to send data");
        fclose(file);
        close(sockfd);
        return 1;
    }

    // Receive acknowledgment
    if (recv(sockfd, buffer, sizeof(buffer), 0) < 0) {
        perror("Failed to receive acknowledgment");
        fclose(file);
        close(sockfd);
        return 1;
    }
    printf("Server responded: %s\n", buffer);

    // Send the file content
    while (!feof(file)) {
        size_t read_bytes = fread(buffer, 1, sizeof(buffer), file);
        if (send(sockfd, buffer, read_bytes, 0) < 0) {
            perror("Failed to send file data");
            fclose(file);
            close(sockfd);
            return 1;
        }
    }

    // Close file and socket
    fclose(file);
    close(sockfd);

    printf("File sent successfully\n");
    return 0;
}
