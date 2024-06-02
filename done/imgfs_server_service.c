/*
 * @file imgfs_server_services.c
 * @brief ImgFS server part, bridge between HTTP server layer and ImgFS library
 *
 * @author Konstantinos Prasopoulos
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // uint16_t
#include <signal.h> // signal
#include <pthread.h> // pthread_mutex_t

#include "error.h"
#include "util.h" // atouint16
#include "imgfs.h"
#include "http_net.h"
#include "imgfs_server_service.h"

// Main in-memory structure for imgFS
static struct imgfs_file fs_file;
static uint16_t server_port;

pthread_mutex_t imgfs_mutex;

#define URI_ROOT "/imgfs"

/********************************************************************//**
 * Startup function. Create imgFS file and load in-memory structure.
 * Pass the imgFS file name as argv[1] and optionnaly port number as argv[2]
 ********************************************************************** */
int server_startup (int argc, char **argv)
{
    if (argc < 2) return ERR_NOT_ENOUGH_ARGUMENTS;

    const char* file_name = argv[1]; 

    int open = do_open(file_name, "rb+", &fs_file); 

    if (open != ERR_NONE) { return open; }

    print_header(&fs_file.header); 

    uint16_t port_number = DEFAULT_LISTENING_PORT; 
    if (argc >= 3) {
        port_number = atouint16(argv[2]); 
        if (port_number == 0) {
            do_close(&fs_file);
            return ERR_INVALID_ARGUMENT;
        }
    }

    server_port = port_number; 

    int init = http_init(port_number, handle_http_message); 

    if (init < 0) {
        do_close(&fs_file);
        return init; 
    }

    fprintf(stdout, "ImgFS server started on http://localhost:%d\n", port_number); 

    pthread_mutex_init(&imgfs_mutex, NULL); // Initialize mutex

    return ERR_NONE; 

}

/********************************************************************//**
 * Shutdown function. Free the structures and close the file.
 ********************************************************************** */
void server_shutdown (void)
{
    fprintf(stderr, "Shutting down...\n");
    http_close();
    do_close(&fs_file);
    pthread_mutex_destroy(&imgfs_mutex); // Destroy mutex
}



/**********************************************************************
 * Sends error message.
 ********************************************************************** */
static int reply_error_msg(int connection, int error)
{
#define ERR_MSG_SIZE 256
    char err_msg[ERR_MSG_SIZE]; // enough for any reasonable err_msg
    if (snprintf(err_msg, ERR_MSG_SIZE, "Error: %s\n", ERR_MSG(error)) < 0) {
        fprintf(stderr, "reply_error_msg(): sprintf() failed...\n");
        return ERR_RUNTIME;
    }
    return http_reply(connection, "500 Internal Server Error", "",
                      err_msg, strlen(err_msg));
}

/**********************************************************************
 * Sends 302 OK message.
 ********************************************************************** */
static int reply_302_msg(int connection)
{
    char location[ERR_MSG_SIZE];
    if (snprintf(location, ERR_MSG_SIZE, "Location: http://localhost:%d/" BASE_FILE HTTP_LINE_DELIM,
                 server_port) < 0) {
        fprintf(stderr, "reply_302_msg(): sprintf() failed...\n");
        return ERR_RUNTIME;
    }
    return http_reply(connection, "302 Found", location, "", 0);
}

int handle_list_call(int connection, struct http_message* msg) {
    M_REQUIRE_NON_NULL(msg);
    // pthread_mutex_lock(&imgfs_mutex);
    char* json_out = NULL; 
    int list = do_list(&fs_file, JSON, &json_out); 
    // pthread_mutex_unlock(&imgfs_mutex);
    if (list != ERR_NONE) {
        return reply_error_msg(connection, list); 
    }
    const char* header = "Content-Type: application/json" HTTP_LINE_DELIM;  
    //char* body; 
    //size_t bodylen; 
    int repl = http_reply(connection, HTTP_OK, header, json_out, strlen(json_out)); 
    free(json_out); 
    json_out = NULL; 
    if (repl != ERR_NONE) {
        return reply_error_msg(connection, repl); 
    }
    return ERR_NONE; 
}

int handle_read_call(int connection, struct http_message* msg) {

    M_REQUIRE_NON_NULL(msg); 
 
    char img_id[MAX_IMG_ID + 1]; 
    memset(img_id, 0, sizeof(img_id));
    int get_id = http_get_var(&msg->uri, "img_id", img_id, sizeof(img_id)); 
    if (get_id <= 0) {
        return reply_error_msg(connection, get_id); 
    }
    if (get_id == 0) {
        return reply_error_msg(connection, ERR_NOT_ENOUGH_ARGUMENTS); 
    }
    char res[100]; // enough for "orig" "thumb" and "small"
    memset(res, 0, sizeof(res));
    int get_res = http_get_var(&msg->uri, "res", res, sizeof(res)); 
    if (get_res < 0) {
        return reply_error_msg(connection, get_res); 
    }
    if (get_res == 0) {
        return reply_error_msg(connection, ERR_NOT_ENOUGH_ARGUMENTS); 
    }
    int res_code = resolution_atoi(res); 
    if (res_code < 0) {
        return reply_error_msg(connection, ERR_RESOLUTIONS);
    }

    char* buf = NULL; 
    uint32_t size = 0; 
    // pthread_mutex_lock(&imgfs_mutex);
    int read = do_read(img_id, res_code, &buf, &size, &fs_file); 
    // pthread_mutex_unlock(&imgfs_mutex);
    if (read != ERR_NONE) {
        free(buf); 
        return reply_error_msg(connection, read); 
    }
    const char* header =  "Content-Type: image/jpeg" HTTP_LINE_DELIM;
    int repl = http_reply(connection, HTTP_OK, header, buf, size); 
    free(buf); 
    buf = NULL; 
    if (repl != ERR_NONE) {
        return reply_error_msg(connection, repl); 
    } 
    return repl; 
}

int handle_delete_call(int connection, struct http_message* msg) {

    M_REQUIRE_NON_NULL(msg); 
    char img_id[MAX_IMG_ID+1]; 
    int get_id = http_get_var(&msg->uri, "img_id", img_id, sizeof(img_id));
    if (get_id < 0) {
        return reply_error_msg(connection, get_id);
    }
    if (get_id == 0) {
        return reply_error_msg(connection, ERR_NOT_ENOUGH_ARGUMENTS); 
    }
    // pthread_mutex_lock(&imgfs_mutex);
    int delete = do_delete(img_id, &fs_file); 
    // pthread_mutex_unlock(&imgfs_mutex);
    if (delete != ERR_NONE) {
        return reply_error_msg(connection, delete); 
    }
    char location_header[256];
    snprintf(location_header, sizeof(location_header), "Location: http://localhost:%d/index.html" HTTP_LINE_DELIM, server_port); 
    int repl = http_reply(connection, "302 Found", location_header, "", 0); 
    if (repl != ERR_NONE) {
        return reply_error_msg(connection, repl); 
    } 
    return repl; 
}


int handle_insert_call(int connection, struct http_message* msg)
{
    M_REQUIRE_NON_NULL(msg); 
    char img_id[MAX_IMG_ID + 1];
    int res = http_get_var(&msg->uri, "name", img_id, sizeof(img_id));
    if (res < 0) {
        return reply_error_msg(connection, res);
    }
    if (res == 0) {
        return reply_error_msg(connection, ERR_NOT_ENOUGH_ARGUMENTS); 
    }
    if (msg->body.len == 0) {
        return reply_error_msg(connection, ERR_INVALID_ARGUMENT);
    }

    char* image_content = malloc(msg->body.len);
    if (image_content == NULL) {
        return reply_error_msg(connection, ERR_OUT_OF_MEMORY);
    }

    memcpy(image_content, msg->body.val, msg->body.len);

    // pthread_mutex_lock(&imgfs_mutex);
    int insert = do_insert(image_content, msg->body.len, img_id, &fs_file);
    // pthread_mutex_unlock(&imgfs_mutex);
    free(image_content); 

    if (insert != ERR_NONE) {
        return reply_error_msg(connection, insert);
    }

    char location_header[256];
    snprintf(location_header, sizeof(location_header), "Location: http://localhost:%d/index.html" HTTP_LINE_DELIM, server_port); 
    int repl = http_reply(connection, "302 Found", location_header, "", 0); 
    if (repl != ERR_NONE) {
        return reply_error_msg(connection, repl); 
    } 
    return repl; 
}


/**********************************************************************
 * Simple handling of http message. TO BE UPDATED WEEK 13
 ********************************************************************** */
int handle_http_message(struct http_message* msg, int connection)
{
    M_REQUIRE_NON_NULL(msg);
    debug_printf("handle_http_message() on connection %d. URI: %.*s\n",
                 connection,
                 (int) msg->uri.len, msg->uri.val);

    if (http_match_verb(&msg->uri, "/") || http_match_uri(msg, "/index.html")) {
        return http_serve_file(connection, BASE_FILE);
    }

    if (http_match_uri(msg, URI_ROOT "/list")) {
        return handle_list_call(connection, msg); 
    }
    else if (http_match_uri(msg, URI_ROOT "/insert") && http_match_verb(&msg->method, "POST")) {
        return handle_insert_call(connection, msg); 
    }
    else if (http_match_uri(msg, URI_ROOT "/read")) {
        return handle_read_call(connection, msg); 
    }
    else if (http_match_uri(msg, URI_ROOT "/delete")) {
        return handle_delete_call(connection, msg); 
    }
    else
        return reply_error_msg(connection, ERR_INVALID_COMMAND);
}