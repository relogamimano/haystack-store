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

    int init = http_init(port_number, NULL); 

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

