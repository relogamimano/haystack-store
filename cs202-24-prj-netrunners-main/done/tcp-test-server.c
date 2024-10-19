#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket_layer.h" 
#include "error.h"

#define BUFFER_SIZE 2048
#define MAX_SERVER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("only need port number"); 
        return ERR_INVALID_ARGUMENT;
    }
    M_REQUIRE_NON_NULL(argv);

    char buffer[MAX_SERVER_SIZE] = {0};

    uint16_t port = atoi(argv[1]);
    printf("Server started on port %d\n", port);
    int sockfd = tcp_server_init(port); // Server initialization

    ssize_t error;
    if (error = sockfd < 0) { // Error check
        close(sockfd);
        perror("fail on tcp_server_init()\n");
        return error;
    }
    
    // Infinite loop to keep the server running
    while (1) {
        printf("Waiting for a size...\n");
        int clientfd = tcp_accept(sockfd); // Accepting a client
        if( error = clientfd < 0) { // Error check
            close(clientfd); close(sockfd);
            perror("Error on tcp_accept\n");
            return error;
        }
        // Receiving the file length
        if( error = tcp_read(clientfd, buffer, sizeof(buffer)) < 0) { // Error check
            close(clientfd); close(sockfd);
            perror("Error on tcp_read\n");
            return error;
        }

        long len = strtol(buffer, NULL, 10); // Convert the string to a long

        if (len > MAX_SERVER_SIZE) { // If the file is too large
            printf("Received a size: %ld --> rejected\n", len);
            error = tcp_send(sockfd, "file too large", strlen("file too large") + 1);
            if(error < 0 || error != strlen("file too large") + 1) { // Error check while sending a negative response
                close(clientfd); close(sockfd);
                perror("Error on tcp_send\n");
                return error;
            }
            continue;
        }
        
        printf("Received a size: %ld --> accepted\n", len);

        // Acknowledge receiving the size
        error = tcp_send(clientfd, "Small file", strlen("Small file") + 1);
        if(error < 0 || error != strlen("Small file") + 1) { // Error check 
            close(clientfd); close(sockfd);
            perror("Error on tcp_send\n");
            return error;
        }

        printf("About to receive file of %ld bytes\n", len);

        if(error = tcp_read(clientfd, buffer, len) < 0) { // Error check
            close(clientfd); close(sockfd);
            perror("Error on tcp_read\n");
            return error;
        }
        
        printf("Received a file:\n%s\n", buffer);

        // Acknowledge receiving the file
        error = tcp_send(clientfd, "Accepted", strlen("Accepted") + 1);
        if(error < 0 || error != strlen("Accepted") + 1) {
            close(clientfd); close(sockfd);
            perror("Error on tcp_send\n");
            return error;
        }
        close(clientfd);
    }
    close(sockfd); 
    return ERR_NONE;
}