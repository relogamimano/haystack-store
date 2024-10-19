#include "imgfs.h"
#include "error.h"
#include "image_content.h"
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

int do_read(const char* img_id, int resolution, char** image_buffer, uint32_t* image_size, struct imgfs_file* imgfs_file) {
    M_REQUIRE_NON_NULL(img_id); 
    M_REQUIRE_NON_NULL(image_buffer); 
    M_REQUIRE_NON_NULL(image_size); 
    M_REQUIRE_NON_NULL(imgfs_file); 

    struct img_metadata* metadata = NULL; 
    size_t index;

    for (index = 0; index < imgfs_file->header.max_files; index++) {
        if(strcmp(imgfs_file->metadata[index].img_id, img_id) == 0 && 
                imgfs_file->metadata[index].is_valid != EMPTY) {
                    metadata = &imgfs_file->metadata[index];
                    break; // ok ? break since we found it 
                }
    }

    if (metadata == NULL) {
        return ERR_IMAGE_NOT_FOUND; 
    }

    if (metadata->size[resolution] == 0 || metadata->offset[resolution] == 0) {
        if (resolution != ORIG_RES) {
            int resize = lazily_resize(resolution, imgfs_file, index); 
            if (resize) {
                return resize; 
            }
        }
    }

    *image_buffer = malloc(metadata->size[resolution]);
    if (*image_buffer == NULL) {
        return ERR_OUT_OF_MEMORY;
    }

    if (fseek(imgfs_file->file, metadata->offset[resolution], SEEK_SET) != 0 ||
        fread(*image_buffer, metadata->size[resolution], 1, imgfs_file->file) != 1) {
        free(*image_buffer);
        *image_buffer = NULL;
        return ERR_IO;
    }

    *image_size = metadata->size[resolution];
    return ERR_NONE;
}