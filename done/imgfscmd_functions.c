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
    int h = fprintf(stdout,
        "imgfscmd [COMMAND] [ARGUMENTS]\n"
        "  help: displays this help.\n"
        "  list <imgFS_filename>: list imgFS content.\n"
        "  create <imgFS_filename> [options]: create a new imgFS.\n"
        "      options are:\n"
        "          -max_files <MAX_FILES>: maximum number of files.\n"
        "                                  default value is %u\n"
        "                                  maximum value is %u\n"
        "          -thumb_res <X_RES> <Y_RES>: resolution for thumbnail images.\n"
        "                                  default value is %ux%u\n"
        "                                  maximum value is %ux%u\n"
        "         -small_res <X_RES> <Y_RES>: resolution for small images.\n"
        "                                  default value is %ux%u\n"
        "                                  maximum value is %ux%u\n"
        "  delete <imgFS_filename> <imgID>: delete image imgID from imgFS.\n",
        default_max_files, MAX_FLAG_MAX_FILES,
        default_thumb_res, default_thumb_res,
        MAX_THUMB_RES, MAX_THUMB_RES, 
        default_small_res, default_small_res,
        MAX_SMALL_RES, MAX_SMALL_RES
    );

    
    return h < 0 ? ERR_IO : ERR_NONE; 
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

    struct imgfs_file imgfs_file = {0};

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
    
    
    // do_close(&imgfs_file);
    //return do_create(argv[ARG_FILE_NAME_INDEX], &imgfs_file);

    struct imgfs_header header = {
        .max_files = max_files,
        .resized_res = { thumb_width, thumb_height, small_width, small_height }
    };

    struct imgfs_file imgfs_file = {
        .header = header
    };

    int result = do_create(argv[ARG_FILE_NAME_INDEX], &imgfs_file);

    if (imgfs_file.metadata) {
        free(imgfs_file.metadata);
        imgfs_file.metadata = NULL;
    }

    return result;
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

    if (argc < 3)  {
        return ERR_NOT_ENOUGH_ARGUMENTS; 
    }

    const char *files = argv[1];
    const char *img_id = argv[2];

    if (files == NULL || img_id == NULL) {
        return ERR_INVALID_ARGUMENT; 
    }

    if (strlen(img_id) > MAX_IMG_ID) {
        return ERR_INVALID_IMGID;
    }

    struct imgfs_file imgfs_file; 

    int open = do_open(files, "rb+", &imgfs_file); 

    if (open != ERR_NONE) {
        return open; 
    }

    int delete = do_delete(img_id, &imgfs_file);
    do_close(&imgfs_file);

    return delete; 
}
