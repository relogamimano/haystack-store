/**
 * @file imgfscmd_functions.c
 * @brief imgFS command line interpreter for imgFS core commands.
 *
 * @author Mia Primorac
 */

#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "util.h"   // for _unused

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

// default values
static const uint32_t default_max_files = 128;
static const uint16_t default_thumb_res =  64;
static const uint16_t default_small_res = 256;

// max values
static const uint16_t MAX_THUMB_RES = 128;
static const uint16_t MAX_SMALL_RES = 512;

static const uint32_t MAX_FLAG_MAX_FILES = 4294967295;

#define ARG_FILE_NAME_INDEX 1
#define OPTIONAL_ARG_START_INDEX 2
#define MIN_NB_ARG 8
#define NB_MAX_FILE_FLAG_QUANTITY 1
#define WIDTH_AND_HEIGHT_FLAG_QUANTITY 2

/**********************************************************************
 * Displays some explanations.
 ********************************************************************** */
int help(int useless _unused, char** useless_too _unused)
{
    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE.
     * **********************************************************************
     */

    //TO_BE_IMPLEMENTED();
    //return NOT_IMPLEMENTED;
    return 4; 
}

/**********************************************************************
 * Opens imgFS file and calls do_list().
 ********************************************************************** */
int do_list_cmd(int argc, char** argv)
{
    /* **********************************************************************
     * TODO WEEK 07: WRITE YOUR CODE HERE.
     * **********************************************************************
     */

    if (argc < OPTIONAL_ARG_START_INDEX)  {
        return ERR_NOT_ENOUGH_ARGUMENTS; 
    }

    const char *files = argv[ARG_FILE_NAME_INDEX]; 

    struct imgfs_file imgfs_file;

    int open = do_open(files, "rb", &imgfs_file);

    if (open != ERR_NONE) {
        do_close(&imgfs_file); 
        return ERR_IO; 
    }

    do_list(&imgfs_file, STDOUT, NULL); 

    do_close(&imgfs_file); 

    return ERR_NONE;
}

/**********************************************************************
 * Prepares and calls do_create command.
********************************************************************** */
int do_create_cmd(int argc, char** argv)
{

    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */
    // imgfscmd create tests/data/ -max_files
    //            0        1         2          ==3
    if (argc < OPTIONAL_ARG_START_INDEX - 1)  {
        return ERR_NOT_ENOUGH_ARGUMENTS; 
    }
    
    uint32_t max_files = default_max_files;
    uint16_t thumb_width = default_thumb_res, thumb_height = default_thumb_res;
    uint16_t small_width = default_small_res, small_height = default_small_res;


    for(int i = OPTIONAL_ARG_START_INDEX; i < argc; i++) {
        if(!strcmp(argv[i], "-max_files")) {
            if (argc - 1 - i < NB_MAX_FILE_FLAG_QUANTITY) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
            uint32_t uint32_arg = atouint32(argv[++i]);
            if(uint32_arg > MAX_FLAG_MAX_FILES || uint32_arg <= 0) {
                return ERR_MAX_FILES;
            }
            max_files = uint32_arg;
        } else if (!strcmp(argv[i], "-thumb_res")) {
            if (argc - 1 - i < WIDTH_AND_HEIGHT_FLAG_QUANTITY) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
            thumb_width = atouint16(argv[++i]);
            thumb_height = atouint16(argv[++i]);
            uint16_t thumb_res = thumb_height * thumb_width;
            if(thumb_res > MAX_THUMB_RES * MAX_THUMB_RES || thumb_res <= 0) {
                return ERR_RESOLUTIONS;
            }
        } else if (!strcmp(argv[i], "-small_res")) {
            if (argc - 1 - i < WIDTH_AND_HEIGHT_FLAG_QUANTITY) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
            small_width = atouint16(argv[++i]);
            small_height = atouint16(argv[++i]);
            uint16_t small_res = small_height * small_width;
            if(small_res > MAX_SMALL_RES * MAX_SMALL_RES || small_res <= 0) {
                return ERR_RESOLUTIONS;
            }
        } else {
            return ERR_INVALID_ARGUMENT;
        }
    }
    
    struct imgfs_header header = {
        .max_files = max_files,
        .resized_res = { thumb_width, thumb_height, small_width, small_height }
    };

    struct imgfs_file imgfs_file = {
        .header = header
    };
    
    // do_close(&imgfs_file);
    return do_create(argv[ARG_FILE_NAME_INDEX], &imgfs_file);
}

/**********************************************************************
 * Deletes an image from the imgFS.
 */
int do_delete_cmd(int argc, char** argv)
{
    /* **********************************************************************
     * TODO WEEK 08: WRITE YOUR CODE HERE (and change the return if needed).
     * **********************************************************************
     */
    

    TO_BE_IMPLEMENTED();
    return NOT_IMPLEMENTED;
}
