/*
 * @file http_net.c
 * @brief HTTP server layer for CS-202 project
 *
 * @author Konstantinos Prasopoulos
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#include "http_prot.h"
#include "http_net.h"
#include "socket_layer.h"
#include "error.h"

static int passive_socket = -1;
static EventCallback cb;

#define MK_OUR_ERR(X) \
static int our_ ## X = X

MK_OUR_ERR(ERR_NONE);
MK_OUR_ERR(ERR_INVALID_ARGUMENT);
MK_OUR_ERR(ERR_OUT_OF_MEMORY);
MK_OUR_ERR(ERR_IO);

/*******************************************************************
 * Handle connection
 */
static void *handle_connection(void *arg)
{
    if (arg == NULL) return &our_ERR_INVALID_ARGUMENT;

    return &our_ERR_NONE;
}


/*******************************************************************
 * Init connection
 */
int http_init(uint16_t port, EventCallback callback)
{
    passive_socket = tcp_server_init(port);
    cb = callback;
    return passive_socket;
}

/*******************************************************************
 * Close connection
 */
void http_close(void)
{
    if (passive_socket > 0) {
        if (close(passive_socket) == -1)
            perror("close() in http_close()");
        else
            passive_socket = -1;
    }
}

/*******************************************************************
 * Receive content
 */
int http_receive(void)
{
    int connect = tcp_accept(passive_socket); 
    if (connect < 0) {
        return ERR_IO; 
    }

    handle_connection(&passive_socket); 


}


static void* handle_connection(void* arg) {

    int socket = *(int*)arg;  
    char buf[MAX_HEADER_SIZE]; 
    int bytes_read = tcp_read(socket, buf, MAX_HEADER_SIZE - 1);
    
    int total_bytes_read; 
    int* result = malloc(sizeof(int)); 

    while (total_bytes_read < MAX_HEADER_SIZE) {
        bytes_read = tcp_read(socket, buf + total_bytes_read, MAX_HEADER_SIZE - total_bytes_read);
        if (bytes_read <= 0) {
            perror("Error on read");
            close(socket);
            *result = ERR_IO;
            return result;
        }

        total_bytes_read += bytes_read;
        buf[total_bytes_read] = '\0';

        if (strstr(buf, HTTP_HDR_END_DELIM) != NULL) {
            break;
        }
    }

    if (strstr(buf, HTTP_HDR_END_DELIM) == NULL) {
        perror("Did not find end delimiter"); 
        close(socket); 
        *result = ERR_IO; 
        return result; 
    }

    if (strstr(buf, "test: ok") != NULL) {
        // http reply HTTP_OK
    } else {
        // http reply HTTP_BAD_REQUEST
    }

    close(socket); 
    result = ERR_NONE; 
    return result; 



}
/*******************************************************************
 * Serve a file content over HTTP
 */
int http_serve_file(int connection, const char* filename)
{
    int ret = ERR_NONE;
    return ret;
}

/*******************************************************************
 * Create and send HTTP reply
 */
int http_reply(int connection, const char* status, const char* headers, const char *body, size_t body_len)
{
    return ERR_NONE;
}
