/**
 * @file imgfscmd_functions.c
 * @brief imgFS command line interpreter for imgFS core commands.
 *
 * This file contains implementations of functions for handling imgFS command line commands.
 * These functions provide functionalities like listing imgFS content, creating a new imgFS,
 * and deleting an image from imgFS.
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

// Default values
static const uint32_t default_max_files = 128;
static const uint16_t default_thumb_res =  64;
static const uint16_t default_small_res = 256;

// Maximum values
static const uint16_t MAX_THUMB_RES = 128;
static const uint16_t MAX_SMALL_RES = 512;

static const uint32_t MAX_FLAG_MAX_FILES = 4294967295;

#define ARG_FILE_PATH_INDEX 0
#define ARG_ID_INDEX 1
#define MIN_NB_ARG 8
#define NB_MAX_FILE_FLAG_QUANTITY 1
#define WIDTH_AND_HEIGHT_FLAG_QUANTITY 2

/**********************************************************************
 * Displays help information.
 **********************************************************************/
int help(int useless _unused, char** useless_too _unused)
{
    // Print help information to stdout
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
 * Lists the content of imgFS.
 **********************************************************************/
int do_list_cmd(int argc, char** argv)
{
    // Check for null pointer
    M_REQUIRE_NON_NULL(argv);

    // Ensure correct number of arguments
    if (argc != 1) {
        return ERR_INVALID_COMMAND; 
    }

    // Get the filename from arguments
    const char *files = argv[0]; 

    // Create imgfs_file struct
    struct imgfs_file imgfs_file = {0};

    // Open the imgFS file
    int open = do_open(files, "rb", &imgfs_file);

    // Handle file open error
    if (open != ERR_NONE) {
        do_close(&imgfs_file); 
        return ERR_IO; 
    }

    // List the content of imgFS to stdout
    do_list(&imgfs_file, STDOUT, NULL); 

    // Close the imgFS file
    do_close(&imgfs_file); 

    return ERR_NONE;
}

/**********************************************************************
 * Creates a new imgFS.
 **********************************************************************/
int do_create_cmd(int argc, char** argv)
{
    // Check for null pointer
    M_REQUIRE_NON_NULL(argv);

    // Ensure at least one argument (filename)
    if (argc < 1) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }
    
    // Initialize default values
    uint32_t max_files = default_max_files;
    uint16_t thumb_width = default_thumb_res, thumb_height = default_thumb_res;
    uint16_t small_width = default_small_res, small_height = default_small_res;

    // Get the filename from arguments
    const char * filename = argv[0];
    argc--; argv++;

    // Loop through the arguments to parse options
    for(int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-max_files")) {
            // Ensure enough arguments for the flag
            if (argc - i <= NB_MAX_FILE_FLAG_QUANTITY) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
            // Parse and validate max_files value
            uint32_t uint32_arg = atouint32(argv[++i]);
            if (uint32_arg > MAX_FLAG_MAX_FILES || uint32_arg <= 0) {
                return ERR_MAX_FILES;
            }
            max_files = uint32_arg;
        } else if (!strcmp(argv[i], "-thumb_res")) {
            // Ensure enough arguments for the flag
            if (argc - i <= WIDTH_AND_HEIGHT_FLAG_QUANTITY) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
            // Parse and validate thumb_res values
            thumb_width = atouint16(argv[++i]);
            thumb_height = atouint16(argv[++i]);
            if (thumb_width > MAX_THUMB_RES || thumb_height > MAX_THUMB_RES || thumb_width <= 0 || thumb_height <= 0) {
                return ERR_RESOLUTIONS;
            }
        } else if (!strcmp(argv[i], "-small_res")) {
            // Ensure enough arguments for the flag
            if (argc - i <= WIDTH_AND_HEIGHT_FLAG_QUANTITY) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
            // Parse and validate small_res values
            small_width = atouint16(argv[++i]);
            small_height = atouint16(argv[++i]);
            if (small_width > MAX_SMALL_RES || small_height > MAX_SMALL_RES || small_width <= 0 || small_height <= 0) {
                return ERR_RESOLUTIONS;
            }
        } else {
            return ERR_INVALID_ARGUMENT;
        }
    }
    
    // Create imgfs_file struct with specified parameters
    struct imgfs_file imgfs_file = {
        .header.max_files = max_files,
        .header.resized_res = { thumb_width, thumb_height, small_width, small_height }
    };

    // Call do_create command
    int result = do_create(filename, &imgfs_file);

    return result;
}

/**********************************************************************
 * Deletes an image from imgFS.
 **********************************************************************/
int do_delete_cmd(int argc, char** argv)
{
    // Check for null pointer
    M_REQUIRE_NON_NULL(argv);

    // Ensure at least two arguments (filename and imgID)
    if (argc < 2) {
        return ERR_NOT_ENOUGH_ARGUMENTS; 
    }

    // Get filename and imgID from arguments
    const char *files = argv[ARG_FILE_PATH_INDEX];
    const char *img_id = argv[ARG_ID_INDEX];
    
    // Check for NULL arguments
    if (files == NULL || img_id == NULL) {
        return ERR_INVALID_ARGUMENT; 
    }

    // Check if imgID length is valid
    if (strlen(img_id) > MAX_IMG_ID) {
        return ERR_INVALID_IMGID;
    }

    // Create imgfs_file struct
    struct imgfs_file imgfs_file; 

    // Open the imgFS file
    int open = do_open(files, "rb+", &imgfs_file); 

    // Handle file open error
    if (open != ERR_NONE) {
        return open; 
    }

    // Call do_delete command
    int delete = do_delete(img_id, &imgfs_file);
    
    // Close the imgFS file
    do_close(&imgfs_file);

    return delete; 
}
