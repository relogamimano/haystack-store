/*
 * @file imgfs_server.c
 * @brief ImgFS server part, main
 *
 * @author Konstantinos Prasopoulos
 */

#include "util.h"
#include "imgfs.h"
#include "socket_layer.h"
#include "http_net.h"
#include "imgfs_server_service.h"

#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h> // abort()
#include <bits/sigaction.h>
#include <vips/vips.h>



/********************************************************************/
static void signal_handler(int sig_num _unused)
{
    server_shutdown(); 
    exit(0); 
}

/********************************************************************/
static void set_signal_handler(void)
{
    struct sigaction action;
    if (sigemptyset(&action.sa_mask) == -1) {
        perror("sigemptyset() in set_signal_handler()");
        abort();
    }
    action.sa_handler = signal_handler;
    action.sa_flags   = 0;
    if ((sigaction(SIGINT,  &action, NULL) < 0) ||
        (sigaction(SIGTERM,  &action, NULL) < 0)) {
        perror("sigaction() in set_signal_handler()");
        abort();
    }
}

/********************************************************************/

int main (int argc, char *argv[])
{

    M_REQUIRE_NON_NULL(argv); 
    if (argc < 2 ) {
        return ERR_NOT_ENOUGH_ARGUMENTS; 
    }
    if (argc > 3) {
        return ERR_INVALID_ARGUMENT; 
    }

    if(VIPS_INIT(argv[0])) {
        vips_error_exit("unable to start VIPS");
    }

    int err = server_startup(argc, argv); 

    if (err != ERR_NONE) {
        // add juicy log messages here 
        return err; 
    }

    set_signal_handler(); 

    while ((err = http_receive()) == ERR_NONE);
    fprintf(stderr, "http_receive() failed\n");
    fprintf(stderr, "%s\n", ERR_MSG(err));
    
    server_shutdown(); 
    
    return err;
}
