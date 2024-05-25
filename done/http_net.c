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
#include <errno.h>

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
    int socket = *((int*)arg);
    if (socket < 0) return &our_ERR_INVALID_ARGUMENT;

    // Read the HTTP header from the socket
    char buffer[MAX_HEADER_SIZE] = {0};

    size_t total_bytes_read = 0;
    ssize_t bytes_read;
    while (total_bytes_read < MAX_HEADER_SIZE) {
        bytes_read = tcp_read(socket, buffer + total_bytes_read, sizeof(buffer + total_bytes_read));
        
        if (bytes_read < 0) {
            printf("socket: %ld\n", socket);
            perror("tcp_read() in failed in handle_connection()\n");
            return &our_ERR_IO;
        }
        // Check if the headers contain HTTP_HDR_END_DELIM
        if (strstr(buffer, HTTP_HDR_END_DELIM) != NULL) {
            break;
        }
        total_bytes_read += bytes_read;
    }

    // Check if the headers contain "test: ok"
    if (strstr(buffer, "test: ok") != NULL) {
        // Send HTTP_OK status
        http_reply(socket, HTTP_OK, "", NULL, 0);
    } else {
        // Send HTTP_BAD_REQUEST status
        http_reply(socket, HTTP_BAD_REQUEST, "", NULL, 0);
    }

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
    
    int fd = tcp_accept(passive_socket);
    if (fd < 0) {
        return ERR_IO; 
    }

    if(handle_connection(&fd) < 0) {
        return ERR_IO;
    }
    close(fd);
    return ERR_NONE;
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
int http_reply(int connection, const char* status, const char* headers, const char *body, size_t body_len) {
    // Compute the total size of the header and body
    size_t total_size = strlen(HTTP_PROTOCOL_ID) + strlen(" ") + strlen(status) + strlen(HTTP_LINE_DELIM) +
                        strlen(headers) + strlen("Content-Length: ") + body_len + strlen(HTTP_HDR_END_DELIM);

    // Allocate the buffer
    char *buffer = malloc(total_size);
    if (buffer == NULL) {
        return our_ERR_OUT_OF_MEMORY;
    }

    // Format the header
    int header_len = snprintf(buffer, total_size, "%s %s%s%sContent-Length: %zu%s",
                              HTTP_PROTOCOL_ID, status, HTTP_LINE_DELIM, headers, body_len, HTTP_HDR_END_DELIM);

    // Copy the body to the end of the buffer
    if (body != NULL && body_len > 0) {
        memcpy(buffer + header_len, body, body_len);
    }
    
    // Send the buffer to the socket
    ssize_t bytes_sent = tcp_send(connection, buffer, total_size);

    // Free the buffer
    free(buffer);

    if (bytes_sent != total_size || bytes_sent < 0) {
        perror("send error");
        return our_ERR_IO;
    }

    return ERR_NONE;
}
