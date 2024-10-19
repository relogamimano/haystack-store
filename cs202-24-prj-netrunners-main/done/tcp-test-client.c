#include "imgfs.h"
#include "error.h"
#include "imgfscmd_functions.h"
#include "socket_layer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
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

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Invalid number of arguments");
        return ERR_INVALID_ARGUMENT;
    }
    M_REQUIRE_NON_NULL(argv);

    ssize_t error;

    char buffer[MAX_CLIENT_SIZE];
    
    uint16_t port = atoi(argv[1]);
    printf("Talking to %d\n", port);

    // Get the file and copy its content into the buffer
    const char *filename = argv[2];
    FILE* file = fopen(filename, "r");
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if(error = snprintf(buffer, sizeof(buffer), "%ld", file_size) < 0 ) {
        close(file);
        perror("Error on snprintf\n");
        return error;
    }
    
    
    int sockfd = tcp_client_init(port); // Client initialization
    
    if(error = sockfd < 0) {
        close(sockfd); close(file);
        perror("TCP client failed to init");
        return sockfd;
    }

    // Send length
    error = tcp_send(sockfd, buffer, strlen(buffer));
    if(error < 0|| error != strlen(buffer)) { // Error check
        close(sockfd); close(file);
        perror("Error on tcp_send\n");
        return error;
    }
    
    printf("Sending size %s:\n", buffer);

    // Receive size acknowledgement ("Small file")
    
    if(error = tcp_read(sockfd, buffer, sizeof(buffer)) < 0) { // Error check
        close(sockfd); close(file);
        perror("Error on tcp_read\n");
        return error;
    }

    // Verify that the server acknowledged the file size
    printf("buffer: \"%s\"\n", buffer);
    if (error = strcmp(buffer, "Small file") != 0) { // Error check
        close(sockfd); close(file);
        perror("Server did not acknowledge file size\n");
        return error;
    }
    printf("Server responded: \"%s\"\n", buffer);
    
    // Write the file to the buffer
    if( fseek(file, 0, SEEK_SET) || fread(buffer, sizeof(buffer), 1, file) ) { // IO Error check
        close(sockfd); close(file);
        return ERR_IO;
    }
    close(file);
    buffer[file_size] = '\0';
    // Send the file
    error = tcp_send(sockfd, buffer, file_size);
    if(error < 0 || error != file_size) { // Error check
        close(sockfd);
        perror("Error on tcp_send\n");
        return error;
    }
    
    printf("Sending %s:\n", filename);

    // Receive file acknoweledgement
    if(error = tcp_read(sockfd, buffer, sizeof(buffer)) < 0) { // Error check
        close(sockfd);
        perror("Error on tcp_read\n");
        return error;
    }

    // Verify that the server received the file content
    if (error = strcmp(buffer, "Accepted") != 0) { // Error check
        close(sockfd);
        perror("Server did not acknowledge receving the complete file\n");
        return error;
    }
    
    printf("Done\n");
    close(sockfd); 

    
    return ERR_NONE;
}