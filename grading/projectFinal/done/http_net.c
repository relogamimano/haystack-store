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
#include <pthread.h>

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

MK_OUR_ERR(ERR_DEBUG); 



/*******************************************************************
 * Handle connection
*/

// ask TA mercredi about this method. 
static void *handle_connection(void *arg)
{
    // sigset_t mask;
    // sigemptyset(&mask);
    // sigaddset(&mask, SIGINT );
    // sigaddset(&mask, SIGTERM);
    // pthread_sigmask(SIG_BLOCK, &mask, NULL);

    if (arg == NULL) return &our_ERR_INVALID_ARGUMENT;
    int socket = *((int*)arg);
    //free(arg); 
    if (socket < 0) return &our_ERR_INVALID_ARGUMENT;

    char* rcvbuf = malloc(MAX_HEADER_SIZE); 

    if (rcvbuf == NULL) { 
        close(socket); 
        return &our_ERR_OUT_OF_MEMORY; 
    }

    char* buff_excess = NULL; 
    size_t total_bytes_read = 0;
    int content_len = 0; 
    ssize_t max_read = MAX_HEADER_SIZE; 
    struct http_message message;  
    char* pos_ptr = NULL; 

    int already_extended = 0; 

    while (1) {
        ssize_t cur_read = tcp_read(socket, rcvbuf + total_bytes_read, max_read);
        
        if (cur_read < 0) {
            close(socket); // added this 
            free(rcvbuf); 
            rcvbuf = NULL; 
            return &our_ERR_IO; 
        }

        total_bytes_read += (size_t)cur_read; 

        //check closed connection (pas sur) 
        if (cur_read == 0) {
            free(rcvbuf); 
            rcvbuf = NULL; 
            close(socket); 
            return &our_ERR_NONE; 
        }

        int parsed = http_parse_message(rcvbuf, total_bytes_read, &message, &content_len);

        if (parsed < 0) { 
            free(rcvbuf); 
            rcvbuf = NULL; 
            close(socket); 
            return &our_ERR_IO; 
        }

        if (parsed == 0 && content_len > 0 && total_bytes_read < content_len && !already_extended) {
            char* new_buf = realloc(rcvbuf, MAX_HEADER_SIZE + (size_t)content_len);
            if (new_buf == NULL) {
                free(rcvbuf); 
                rcvbuf = NULL; 
                close(socket); 
                return &our_ERR_OUT_OF_MEMORY;
            }
            rcvbuf = new_buf; 
            //new_buf = NULL;  //maybe re add this 
            already_extended = 1; 
            //rcvbuf+= total_bytes_read; // ? 
            max_read = (size_t)content_len; // = ou += 
            continue; //maybe remove this 
        }
        
        if (parsed > 0) {
            int ret_cb = cb(&message, socket); 
            if (ret_cb < 0) {
                free(rcvbuf); 
                rcvbuf = NULL; 
                close(socket); 
                return &our_ERR_IO; 
            }
            pos_ptr = strstr(rcvbuf, HTTP_HDR_END_DELIM); 
            if (pos_ptr) {
                pos_ptr += strlen(HTTP_HDR_END_DELIM); 
                size_t header_size = (size_t)(pos_ptr - rcvbuf); 

                if (total_bytes_read > header_size + (size_t)content_len) {
                    buff_excess = malloc(total_bytes_read - (header_size + (size_t)content_len)); 
                    if (buff_excess == NULL) {
                        free(rcvbuf); 
                        close(socket); 
                        return &our_ERR_OUT_OF_MEMORY; 
                    }
                    memcpy(buff_excess, pos_ptr + content_len, total_bytes_read - (header_size + (size_t)content_len));
                }
            }
            //pos_ptr += strlen(HTTP_HDR_END_DELIM); 
            //size_t header_size = (size_t)(pos_ptr - rcvbuf); 

            //if (total_bytes_read > header_size + (size_t)content_len) {
            //    buff_excess= malloc(total_bytes_read - (header_size + (size_t)content_len)); 
            //    if (buff_excess == NULL) {
            //        free(rcvbuf); 
            //        rcvbuf = NULL; 
            //        close(socket); 
            //        return &our_ERR_OUT_OF_MEMORY; 
            //    }
            //}


            char* new_buff = realloc(rcvbuf, MAX_HEADER_SIZE); 
            //check si ça rate ET si excess_buff est pas nul (y'a pas forcement d ele'0xcess)
            if (new_buff == NULL) {
                free(rcvbuf); 
                if (buff_excess != NULL) {
                    free(buff_excess); 
                    buff_excess = NULL; 
                }
                close(socket); 
                return &our_ERR_OUT_OF_MEMORY; 
            }
            rcvbuf = new_buff; 
            //new_buff = NULL;  //maybe re add this 

            max_read = MAX_HEADER_SIZE; 
            total_bytes_read = 0; 
            memset(&message, 0, sizeof(struct http_message)); 
            memset(rcvbuf, 0, MAX_HEADER_SIZE); 
            content_len = 0;    
            pos_ptr = NULL; 
            already_extended = 0; 

            if (buff_excess != NULL) {
                memcpy(rcvbuf, buff_excess, total_bytes_read - (pos_ptr - rcvbuf) - (size_t)content_len); 
                total_bytes_read = strlen(rcvbuf); 
                free(buff_excess); 
                buff_excess = NULL; 
            }

            if (total_bytes_read >= max_read) {
                free(rcvbuf); 
                rcvbuf = NULL; 
                close(socket); 
                return &our_ERR_IO; 
            }


    }

    }

    // pas obligé psk while (1) 
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

    int *err = handle_connection(&fd);
    if (*err != ERR_NONE) {
        close(fd);
        return *err;
    }

    close(fd);
    return ERR_NONE;
    // pthread_attr_t attr;
    // pthread_attr_init(&attr);
    // pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // int *active_socket = malloc(sizeof(int));
    // *active_socket = tcp_accept(passive_socket);
    // if (*active_socket < 0) {
    //     free(active_socket);
    //     return ERR_IO; 
    // }
    // pthread_t thread;
    // if (pthread_create(&thread, &attr, handle_connection, active_socket) != 0) {
    //     free(active_socket);
    //     return ERR_THREADING;
    // }
    // if(handle_connection(*active_socket) < 0) {
    //     pthread_attr_destroy(&attr);
    //     free(active_socket);
    //     return ERR_IO;
    // }
    // pthread_attr_destroy(&attr);
    // free(active_socket);
    // return ERR_NONE;
}


/*******************************************************************
 * Serve a file content over HTTP
 */
int http_serve_file(int connection, const char* filename)
{
    M_REQUIRE_NON_NULL(filename);

    // open file
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "http_serve_file(): Failed to open file \"%s\"\n", filename);
        return http_reply(connection, "404 Not Found", "", "", 0);
    }

    // get its size
    fseek(file, 0, SEEK_END);
    const long pos = ftell(file);
    if (pos < 0) {
        fprintf(stderr, "http_serve_file(): Failed to tell file size of \"%s\"\n",
                filename);
        fclose(file);
        return ERR_IO;
    }
    rewind(file);
    const size_t file_size = (size_t) pos;

    // read file content
    char* const buffer = calloc(file_size + 1, 1);
    if (buffer == NULL) {
        fprintf(stderr, "http_serve_file(): Failed to allocate memory to serve \"%s\"\n", filename);
        fclose(file);
        return ERR_IO;
    }

    const size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        fprintf(stderr, "http_serve_file(): Failed to read \"%s\"\n", filename);
        fclose(file);
        return ERR_IO;
    }

    // send the file
    const int  ret = http_reply(connection, HTTP_OK,
                                "Content-Type: text/html; charset=utf-8" HTTP_LINE_DELIM,
                                buffer, file_size);

    // garbage collecting
    fclose(file);
    free(buffer);
    return ret;
}


/*******************************************************************
 * Create and send HTTP reply
 */
int http_reply(int connection, const char* status, const char* headers, const char *body, size_t body_len) {
    size_t header_size = strlen(HTTP_PROTOCOL_ID) + strlen(" ") + strlen(status) + strlen(HTTP_LINE_DELIM) +
                        strlen(headers) + strlen("Content-Length: ") + sizeof(body_len) + strlen(HTTP_HDR_END_DELIM);

    size_t total_size = header_size + body_len; 
    // Allocate the buffer
    char *buffer = malloc(total_size);
    if (buffer == NULL) {
        return ERR_OUT_OF_MEMORY;
    }

    //
    int header_len = snprintf(buffer, total_size, "%s%s%s%sContent-Length: %zu%s",
                              HTTP_PROTOCOL_ID, status, HTTP_LINE_DELIM, headers, body_len, HTTP_HDR_END_DELIM);

    // Copy the body to the end of the buffer
    if (body != NULL && body_len > 0) {
        memcpy(buffer + header_len, body, body_len);
    }
    
    // Send the buffer to the socket
    ssize_t bytes_sent = tcp_send(connection, buffer, total_size);
    free(buffer); // dont need it anymore s
    if (bytes_sent < 0) {
        return ERR_IO; 
    }

    if (bytes_sent != total_size || bytes_sent < 0) {
        perror("send error");
        return ERR_IO;
    }

    return ERR_NONE;
}


