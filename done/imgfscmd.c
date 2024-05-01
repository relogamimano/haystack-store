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

typedef struct {
    const char* name;
    command command;
} command_mapping;

command_mapping commands[] = {
    {"list", do_list_cmd},
    {"create", do_create_cmd},
    {"delete", do_delete_cmd},
    {"help", help}
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))
#define HELP_INDEX 3

/*******************************************************************************
 * MAIN
 */
int main(int argc, char* argv[])
{
    VIPS_INIT(argv[0]);
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
        for(i = 0; i < NUM_COMMANDS; i++) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                ret = commands[i].command(argc, argv);
                break; 
            }
        }
        if (i == NUM_COMMANDS) {
            ret = ERR_INVALID_COMMAND; 
        }
    }

    if (ret) {
        fprintf(stderr, "ERROR: %s\n", ERR_MSG(ret));
        help(argc, argv);
    }

    vips_shutdown();
    return ret;
}