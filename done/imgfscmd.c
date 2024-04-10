/**
 * @file imgfscmd.c
 * @brief imgFS command line interpreter for imgFS core commands.
 *
 * Image Filesystem Command Line Tool
 *
 * @author Mia Primorac
 */

#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "util.h"   // for _unused

#include <stdlib.h>
#include <string.h>

typedef int (*command)(int argc, char* argv[]); 

struct command_mapping {
    const char* name; 
    command command; 
}; 

struct command_mapping commands[] = {
    {"list", do_list_cmd}, {"create", do_create_cmd}, {"delete", do_delete_cmd},{"help", help} 
}; 




/*******************************************************************************
 * MAIN
 */
int main(int argc, char* argv[])
{
    int ret = 0;

    if (argc < 2) {
        ret = ERR_NOT_ENOUGH_ARGUMENTS;
    } else {
        /* **********************************************************************
         * TODO WEEK 07: THIS PART SHALL BE EXTENDED.
         * **********************************************************************
         */

        argc--; argv++; // skips command call name

        int i; 
        for(i = 0; i < 4; i++) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                ret = commands[i].command(argc, argv); 
                break;
            }
        }
        if (i == 4) {
            ret = ERR_INVALID_COMMAND; 
        }
    }

    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERR_MSG(ret));
        help(argc, argv);
    }

    return ret;
}
