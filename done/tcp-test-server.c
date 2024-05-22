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

    char buffer[MAX_SERVER_SIZE];

    uint16_t port = atoi(argv[1]);
    printf("Server started on port %d\n", port);
    int sockfd = tcp_server_init(port); // Server initialization

    int error;
    if (error = sockfd < 0) { // Error check
        close(sockfd);
        perror("fail on tcp_server_init()\n");
        return error;
    }

    while (1) {
        printf("Waiting for a size...\n");
        int clientfd = tcp_accept(sockfd); // Accepting a client
        
        // Receiving the file length
        ssize_t len;
        if(error = tcp_read(clientfd, &len, sizeof(len)) < 0) { // Error check
            close(clientfd);
            perror("Error on tcp_read\n");
            return error;
        }

        if (len > MAX_SERVER_SIZE) { // If the file is too large
            printf("Received a size: %ld --> rejected\n", len);
            if(error = tcp_send(sockfd, "file too large", strlen("file too large")) < 0) { // Error check while sending a negative response
                close(clientfd);
                perror("Error on tcp_send\n");
                return error;
            }
            continue;
        }
        
        printf("Received a size: %ld --> accepted\n", len);

        // Acknowledge receiving the size
        if(error = tcp_send(clientfd, "Small file", strlen("Small file") + 1) < 0) { // Error check 
            close(clientfd);
            perror("Error on tcp_send\n");
            return error;
        }

        printf("About to receive file of %ld bytes\n", len);

        // Receiving the file
        if(error = tcp_read(clientfd, buffer, sizeof(buffer)) < 0) { // Error check
            close(clientfd);
            perror("Error on tcp_read\n");
            return error;
        }
        
        buffer[len] = '\0';
        printf("Received a file:\n%s\n", buffer);

        // Acknowledge receiving the file
        if(error = tcp_send(clientfd, "Accepted", strlen("Accepted") + 1) < 0) {
            close(clientfd);
            perror("Error on tcp_send\n");
            return error;
        }
    }
    
    return ERR_NONE;
}