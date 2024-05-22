#include "imgfs.h"
#include "error.h"
#include "imgfscmd_functions.h"
#include "socket_layer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>




#define MAX_CLIENT_SIZE 2048

#define ERR_SIZE -201

#define LISTEN_BACKLOG 4096 // man page says 4096 is default value 

#define DELIMITER "<EOF>"

int tcp_client_init(uint16_t port) {
    struct sockaddr_in address;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) {
        perror("Error on socket creation"); 
        close(sockfd);
        return ERR_IO; 
    }

    memset(&address, "\0", sizeof(address));

    address.sin_family = AF_INET; 
    address.sin_port = htons(port); 
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (connect(sockfd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Error on binding"); 
        close(sockfd); 
        return ERR_IO; 
    }

    return sockfd; 

}


/**
 * Calculates the size of a file.
 *
 * @param filename The name of the file.
 * @return The size of the file in bytes, or -1 if an error occurred.
 */
off_t fsize(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1; 
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Invalid number of arguments");
        return ERR_INVALID_ARGUMENT;
    }
    M_REQUIRE_NON_NULL(argv);

    char buffer[MAX_CLIENT_SIZE]; // Buffer meant to store the server's response
    char file_content_buf[MAX_CLIENT_SIZE]; // Buffer meant to store the file's content before sending it
    
    uint16_t port = atoi(argv[1]);
    printf("Talking to %d\n", port);

    // Get the file and copy its content into the buffer
    const char *filename = argv[2];
    FILE* file = fopen(filename, "r");
    ssize_t len = fsize(filename);
    fread(file_content_buf, MAX_CLIENT_SIZE, 1, file);
    // strcat(&file_content_buf, &DELIMITER);
    close(file);
    
    int sockfd = tcp_client_init(port); // Client initialization
    int error;
    if(error = sockfd < 0) {
        perror("TCP client failed to init");
        return sockfd;
    }

    // Send length
    if(error = tcp_send(sockfd, &len, sizeof(len)) < 0) { // Error check
        close(sockfd);
        perror("Error on tcp_send\n");
        return error;
    }
    
    printf("Sending size %d:\n", len);

    // Receive size acknowledgement ("Small file")
    if(error = tcp_read(sockfd, &buffer, sizeof(buffer)) < 0){ // Error check
        close(sockfd);
        perror("Error on tcp_read\n");
        return error;
    }

    if (error = strcmp(buffer, "Small file") != 0) { // Error check
        close(sockfd);
        perror("Server did not acknowledge file size\n");
        return error;
    }
    printf("Server responded: \"%s\"\n", buffer);
    
    // Send file
    if(error = tcp_send(sockfd, &file_content_buf, strlen(file_content_buf)) < 0) { // Error check
        close(sockfd);
        perror("Error on tcp_send\n");
        return error;
    }
    
    printf("Sending %s:\n", filename);

    // Receive file acknoweledgement
    if(error = tcp_read(sockfd, &buffer, sizeof(buffer)) < 0) { // Error check
        close(sockfd); perror("Error on tcp_read\n");
        return error;
    }

    if (error = strcmp(buffer, "Accepted") != 0) { // Error check
        close(sockfd);
        perror("Server did not acknowledge receving the complete file\n");
        return error;
    }
    
    printf("Done\n");
    close(sockfd); 
    
    return ERR_NONE;
}