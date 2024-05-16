#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket_layer.h" 

#define BUFFER_SIZE 2048

int main(int argc, char *argv[]) {
    if (argc != 2) {printf("only need port number"); return 1;}

    uint16_t port = atoi(argv[1]);
    int sockfd = tcp_server_init(port); 
    if (sockfd < 0) { fprintf(stderr, "fail on tcp_server_init()\n"); return 1;}

    int client_socket;
    while ((client_socket = tcp_accept(sockfd)) >= 0) { 
        char buffer[BUFFER_SIZE];
        tcp_read(client_socket, buffer, sizeof(buffer)); 
        long size;
        sscanf(buffer, "%ld|", &size);
        snprintf(buffer, sizeof(buffer), size < 1024 ? "Small file" : "File too large");
        tcp_send(client_socket, buffer, strlen(buffer));
        if (size < 1024) {
            FILE *output = fopen("received.txt", "wb");
            long received = 0;
            while (received < size) {
                ssize_t r = tcp_read(client_socket, buffer, sizeof(buffer)); 
                fwrite(buffer, 1, r, output);
                received += r;
            }
            fclose(output);

            snprintf(buffer, sizeof(buffer), "Received and saved");
            tcp_send(client_socket, buffer, strlen(buffer));  
        }
    }

    close(sockfd);
    return 0;
}
