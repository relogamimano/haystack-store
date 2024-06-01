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

#include "error.h"
#include "util.h" // atouint16
#include "imgfs.h"
#include "http_net.h"
#include "imgfs_server_service.h"

// Main in-memory structure for imgFS
static struct imgfs_file fs_file;
static uint16_t server_port;

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

    int port_number = argv[2]; 

    if (port_number == NULL) {
        port_number = DEFAULT_LISTENING_PORT; 
    }

    int init = http_init(port_number, handle_http_message); 

    if (init < 0) {
        return init; 
    }

    fprintf("ImgFS server started on http_//localhost:%s", port_number); 

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



// Heavy TODO, these 4 methods 
int handle_list_call(int connection, struct http_message* msg) {
    char* json_out = NULL; 
    int list = do_list(&fs_file, JSON, &json_out); 
    if (list != ERR_NONE) {
        return reply_error_msg(connection, list); 
    }
    const char* header = "Content-Type: application/json" HTTP_LINE_DELIM;  
    //char* body; 
    //size_t bodylen; 
    int repl = http_reply(connection, HTTP_OK, header, json_out, sizeof(json_out)); 
    if (repl != ERR_NONE) {
        close(connection); 
    }
    free(json_out); 
    return repl; 
}

int handle_read_call(int connection, struct http_message* msg) {
 
    char* img_id = malloc(MAX_IMG_ID+1); 
    if (img_id == NULL) {
        free(img_id); 
        return reply_error_msg(connection, ERR_OUT_OF_MEMORY); 
    }
    int get_id = http_get_var(&msg->uri, "res", img_id, sizeof(img_id)); 
    if (get_id <= 0) {
        return reply_error_msg(connection, get_id); 
    }
    int res_str = 10; // enough to hold "orig" "small" and "thumb"
    char* res = malloc(res_str); 
    if (res == NULL) {
        free(img_id); 
        free(res); 
        return reply_error_msg(connection, ERR_OUT_OF_MEMORY);
    }
    int get_res = http_get_var(&msg->uri, "img_id", res, sizeof(res)); 
    if (get_res < 0) {
        free(res); 
        free(img_id); 
        return get_res; 
    }
    int res_code = resolution_atoi(res); 
    char** buf = malloc(sizeof(msg->body)); 
    uint32_t size = sizeof(buf);  
    int read = do_read(img_id, res_code, buf, size, &fs_file); 
    if (read != ERR_NONE) {
        free(img_id); 
        free(res); 
        free(buf); 
        return reply_error_msh(connection, read); 
    }
    const char* header =  "Content-Type: image/jpeg" HTTP_LINE_DELIM;
    int repl = http_reply(connection, HTTP_OK, header, buf, size); 
    if (repl != ERR_NONE) {
        free(img_id); 
        free(res); 
        free(res); 
        return reply_error_msg(connection, repl); 
    } 
    return reply_302_msg(connection); 
}

int handle_delete_call(int connection, struct http_message* msg) {
    char* img_id = malloc(MAX_IMG_ID+1); 
    if (img_id == NULL) {
        free(img_id); 
        return reply_error_msg(connection, ERR_OUT_OF_MEMORY); 
    }
    int get_id = http_get_var(&msg->uri, "img_id", img_id, sizeof(img_id));
    if (get_id <= 0) {
        free(img_id); 
        return reply_error_msg(connection, get_id);
    }
    int delete = do_delete(img_id, &fs_file); 
    if (delete != ERR_NONE) {
        free(img_id); 
        return reply_error_msg(connection, delete); 
    }
    return reply_302_msg(connection); 


}

int handle_insert_call(int connection, struct http_message* msg)
{
    char img_id[MAX_IMG_ID + 1];
    int res = http_get_var(&msg->uri, "name", img_id, sizeof(img_id));
    if (res <= 0) {
        return reply_error_msg(connection, ERR_INVALID_ARGUMENT);
    }

    if (msg->body.len == 0) {
        return reply_error_msg(connection, ERR_INVALID_ARGUMENT);
    }

    res = do_insert(msg->body.val, msg->body.len, img_id, &fs_file);
    if (res != ERR_NONE) {
        return reply_error_msg(connection, res);
    }

    return reply_302_msg(connection);
}

