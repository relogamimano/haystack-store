#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "error.h"
#include "util.h"
#include <sys/socket.h>
#include <netinet/in.h>


#define LISTEN_BACKLOG 4096 // man page says 4096 is default value  

int tcp_server_init(uint16_t port) {
    struct sockaddr_in address;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) {
        perror("Error on socket creation"); 
        //close(sockfd); elle a pas réussi a etre créée donc pk la close 
        return ERR_IO; 
    }

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET; 
    address.sin_port = htons(port); 
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Error on binding"); 
        close(sockfd); 
        return ERR_IO; 
    }

    if (listen(sockfd, LISTEN_BACKLOG) < 0) {
        perror("Error on listen"); 
        close(sockfd); 
        return ERR_IO; 
    }

    return sockfd; 

}


// TODO maybe add checks to explicitely return appropriate errors 
int tcp_accept(int passive_socket) {
    return accept(passive_socket, NULL, NULL); 
}

ssize_t tcp_read(int active_socket, char* buf, size_t buflen) {
    M_REQUIRE_NON_NULL(buf);
    if(active_socket < 0 || buflen <= 0) { return ERR_INVALID_ARGUMENT;}
    return recv(active_socket, buf, buflen, 0); 
}


ssize_t tcp_send(int active_socket, const char* response, size_t response_len) {
    M_REQUIRE_NON_NULL(response);
    if(active_socket < 0 || response_len <= 0) { return ERR_INVALID_ARGUMENT;}
    return send(active_socket, response, response_len, 0); 
}