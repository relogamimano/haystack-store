#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket_layer.h" 

#define MAX_SIZE 2048

int main(int argc, char *argv[]) {

    if (argc != 3) {printf("only need port number and file"); return 1;}
 
    uint16_t port = atoi(argv[1]);
    const char *filename = argv[2];

    FILE *file = fopen(filename, "rb");
    if (!file) { printf("error on file open"); return 1; }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size > MAX_SIZE) {
        fclose(file);
        fprintf(stderr, "file too big");
        return 1;
    }
    fseek(file, 0, SEEK_SET);

    int sockfd = tcp_server_init(port); 
    if (sockfd < 0) { fprintf(stderr, "fail on tcp_server_init()\n"); return 1;}
    printf("Server started on port %d", port); 

    char buffer[1024]; 
    snprintf(buffer, sizeof(buffer), "%ld|", file_size); 
    tcp_send(sockfd, buffer, strlen(buffer)); 

    tcp_read(sockfd, buffer, sizeof(buffer)); 
    printf("acknowledgement response: %s\n", buffer);

    while (!feof(file)) {
        size_t read = fread(buffer, 1, sizeof(buffer), file);
        if (tcp_send(sockfd, buffer, read) < 0) {  
            perror("fail on send");
            fclose(file);
            close(sockfd);
            return 1;
        }
    }

    tcp_read(sockfd, buffer, sizeof(buffer));  
    printf("full file response: %s\n", buffer);

    fclose(file); 
    close(sockfd); 
    return 0; 
}