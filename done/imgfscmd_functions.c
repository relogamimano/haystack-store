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

#define ARG_FILE_PATH_INDEX 0
#define ARG_ID_INDEX 1
#define MIN_NB_ARG 8
#define MAX_FILE_ARGC 1
#define SIZE_ARGC 2

/**********************************************************************
 * Displays some explanations.
 ********************************************************************** */
int help(int useless _unused, char** useless_too _unused)
{
    int help = printf(
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
        "          -small_res <X_RES> <Y_RES>: resolution for small images.\n"
        "                                  default value is %ux%u\n"
        "                                  maximum value is %ux%u\n"
        "  read   <imgFS_filename> <imgID> [original|orig|thumbnail|thumb|small]:\n"
        "      read an image from the imgFS and save it to a file.\n"
        "      default resolution is \"original\".\n"
        "  insert <imgFS_filename> <imgID> <filename>: insert a new image in the imgFS.\n"
        "  delete <imgFS_filename> <imgID>: delete image imgID from imgFS.\n",
        default_max_files, MAX_FLAG_MAX_FILES,
        default_thumb_res, default_thumb_res,
        MAX_THUMB_RES, MAX_THUMB_RES, 
        default_small_res, default_small_res,
        MAX_SMALL_RES, MAX_SMALL_RES
    );    
    
    return help < 0 ? ERR_IO : ERR_NONE; 
}

/**********************************************************************
 * Opens imgFS file and calls do_list().
 ********************************************************************** */
int do_list_cmd(int argc, char** argv)
{
    M_REQUIRE_NON_NULL(argv);
    
    if (argc > 1)  { 
        return ERR_INVALID_COMMAND; 
    } else if( argc < 1) { 
        return ERR_INVALID_ARGUMENT;
    }

    const char *files = argv[0]; 

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
    M_REQUIRE_NON_NULL(argv);
    
    if( argc < 1) {
        return ERR_NOT_ENOUGH_ARGUMENTS;
    }
    
    uint32_t max_files = default_max_files;
    uint16_t thumb_width = default_thumb_res, thumb_height = default_thumb_res;
    uint16_t small_width = default_small_res, small_height = default_small_res;

    const char * filename = argv[0];
    argc--; argv++;

    for(int i = 0; i < argc; i++) {
        if(!strcmp(argv[i], "-max_files")) {
            if (argc - i <= MAX_FILE_ARGC) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
            uint32_t uint32_arg = atouint32(argv[++i]);
            if(uint32_arg > MAX_FLAG_MAX_FILES || uint32_arg <= 0) {
                return ERR_MAX_FILES;
            }
            max_files = uint32_arg;
        } else if (!strcmp(argv[i], "-thumb_res")) {
            if (argc - i <= SIZE_ARGC) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
            thumb_width = atouint16(argv[++i]);
            thumb_height = atouint16(argv[++i]);
            
            if(thumb_width > MAX_THUMB_RES || thumb_height > MAX_THUMB_RES || thumb_width <= 0 || thumb_height <= 0) {
                return ERR_RESOLUTIONS;
            }
        } else if (!strcmp(argv[i], "-small_res")) {
            if (argc - i <= SIZE_ARGC) {
                return ERR_NOT_ENOUGH_ARGUMENTS;
            }
            small_width = atouint16(argv[++i]);
            small_height = atouint16(argv[++i]);
            if(small_width > MAX_SMALL_RES || small_height > MAX_SMALL_RES || small_width <= 0 || small_height <= 0) {
                return ERR_RESOLUTIONS;
            }
        } else {
            return ERR_INVALID_ARGUMENT;
        }
    }

    struct imgfs_file imgfs_file = {
        .header.max_files = max_files,
        .header.resized_res = { thumb_width, thumb_height, small_width, small_height }
    };

    int result = do_create(filename, &imgfs_file);

    do_close(&imgfs_file);

    return result;
}

/**********************************************************************
 * Deletes an image from the imgFS.
 */
int do_delete_cmd(int argc, char** argv)
{
    M_REQUIRE_NON_NULL(argv);

    if (argc < 2)  {
        return ERR_NOT_ENOUGH_ARGUMENTS; 
    }

    const char *files = argv[ARG_FILE_PATH_INDEX];
    const char *img_id = argv[ARG_ID_INDEX];
    
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

int do_read_cmd(int argc, char **argv)
{
    M_REQUIRE_NON_NULL(argv);
    if (argc != 2 && argc != 3) return ERR_NOT_ENOUGH_ARGUMENTS;

    const char * const img_id = argv[1];

    const int resolution = (argc == 3) ? resolution_atoi(argv[2]) : ORIG_RES;
    if (resolution == -1) return ERR_RESOLUTIONS;

    struct imgfs_file myfile;
    zero_init_var(myfile);
    int error = do_open(argv[0], "rb+", &myfile);
    if (error != ERR_NONE) return error;

    char *image_buffer = NULL;
    uint32_t image_size = 0;
    error = do_read(img_id, resolution, &image_buffer, &image_size, &myfile);
    do_close(&myfile);
    if (error != ERR_NONE) {
        return error;
    }

    // Extracting to a separate image file.
    char* tmp_name = NULL;
    create_name(img_id, resolution, &tmp_name);
    if (tmp_name == NULL) return ERR_OUT_OF_MEMORY;
    error = write_disk_image(tmp_name, image_buffer, image_size);
    free(tmp_name);
    free(image_buffer);

    return error;
}

int do_insert_cmd(int argc, char **argv)
{
    M_REQUIRE_NON_NULL(argv);
    if (argc != 3) return ERR_NOT_ENOUGH_ARGUMENTS;

    struct imgfs_file myfile;
    zero_init_var(myfile);
    int error = do_open(argv[0], "rb+", &myfile);
    if (error != ERR_NONE) return error;

    char *image_buffer = NULL;
    uint32_t image_size;

    // Reads image from the disk.
    error = read_disk_image(argv[2], &image_buffer, &image_size);
    if (error != ERR_NONE) {
        do_close(&myfile);
        return error;
    }

    error = do_insert(image_buffer, image_size, argv[1], &myfile);
    free(image_buffer);
    do_close(&myfile);
    return error;
}

static void create_name(const char* img_id, int resolution, char** new_name) {
    const char *resolution_suffix;
    switch (resolution) {
        case THUMB_RES:
            resolution_suffix = "_thumb";
            break;
        case SMALL_RES:
            resolution_suffix = "_small";
            break;
        case ORIG_RES:
            resolution_suffix = "_orig";
            break;
        default:
            resolution_suffix = "_unknown";
            break;
    }
    size_t name_length = strlen(img_id) + strlen(resolution_suffix) + strlen(".jpg") + 1;
    *new_name = (char *)malloc(name_length);
    snprintf(*new_name, name_length, "%s%s%s", img_id, resolution_suffix, ".jpg");
}


static int write_disk_image(const char *filename, const char *image_buffer, uint32_t image_size) {
    FILE* file = fopen(filename, "wb+");
    if(!file) {
        fclose(file);
        return ERR_IO;
    }
    // Write the image from the buffer onto the disk
    if (fseek(file, 0, SEEK_END) ||
        fwrite(image_buffer, image_size, 1, file) != 1){
        fclose(file);
        return ERR_IO;
    }
    fclose(file);
    return ERR_NONE;
}


static int read_disk_image(const char *path, char **image_buffer, uint32_t *image_size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) { return ERR_IO; }

    // Move to the end of the file to determine the size
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return ERR_IO;
    }
    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return ERR_IO;
    }
    // Move back to the beginning of the file
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return ERR_IO;
    }

    // Allocate memory for the image buffer
    *image_buffer = (char *)malloc(size);
    if (*image_buffer == NULL) {
        fclose(file);
        return ERR_OUT_OF_MEMORY;
    }

    // Read the file into the buffer
    if (fread(*image_buffer, size, 1, file) != 1) {
        free(*image_buffer);
        fclose(file);
        return ERR_IO;
    }

    *image_size = (uint32_t)size;

    fclose(file);
    return ERR_NONE;
}