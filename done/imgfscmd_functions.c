/**
 * @file imgfscmd_functions.c
 * @brief imgFS command line interpreter for imgFS core commands.
 *
 * @author Mia Primorac
 */

#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "util.h"   // for _unused

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

    if (argc < 2)  {
        return ERR_NOT_ENOUGH_ARGUMENTS; 
    }

    const char *files = argv[1]; 

    struct imgfs_file imgfs_file;

    int open = do_open(files, "rb", &imgfs_file);

    if (open != ERR_NONE) {
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
    if (argc < 2)  {
        return ERR_NOT_ENOUGH_ARGUMENTS; 
    }

    const char *files = argv[1]; 

    struct imgfs_file * imgfs_file;

    int create = do_create(files, imgfs_file);

    if (create != ERR_NONE) {
        return ERR_IO; 
    }

    do_list(&imgfs_file, STDOUT, NULL); 

    do_close(&imgfs_file);

    TO_BE_IMPLEMENTED();
    return NOT_IMPLEMENTED;
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
