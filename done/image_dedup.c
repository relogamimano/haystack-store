/**
 * @file image_dedup.h
 * @brief Image deduplication.
 *
 * All functions to avoid duplication of images content in a imgFS.
 *
 * @author Mia Primorac
 */

#include "imgfs.h"  // for struct imgfs_file

#include <stdio.h>  // for FILE
#include <stdint.h> // for uint32_t
#include <error.h>
#include <string.h>




int do_name_and_content_dedup(struct imgfs_file* imgfs_file, uint32_t index) {

    if (imgfs_file == NULL || index >= imgfs_file->header.max_files || 
        imgfs_file->metadata[index].is_valid == EMPTY) {
        return ERR_INVALID_ARGUMENT; 
    }

    struct img_metadata *indexed_image = &imgfs_file->metadata[index];

    for (uint32_t i = 0; i < imgfs_file->header.max_files; i++) {
        if (i != index && imgfs_file->metadata[i].is_valid == NON_EMPTY) {
            struct img_metadata *other_image = &imgfs_file->metadata[i];
            if (strcmp(indexed_image->img_id, other_image->img_id) == 0) {
                return ERR_DUPLICATE_ID;
            }
            if (strcmp((char *)indexed_image->SHA, (char *)other_image->SHA) == 0) {
                memcpy(indexed_image->offset, other_image->offset, sizeof(other_image->offset));
                memcpy(indexed_image->size, other_image->size, sizeof(other_image->size));
                return ERR_NONE;
            }
        }
    }
    return ERR_NONE;

    
    
}

