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

    // require non null ingfsfile et imgfs_file->metadata 

    M_REQUIRE_NON_NULL(imgfs_file); 
    M_REQUIRE_NON_NULL(imgfs_file->metadata);  

    // if index > header-maxfiles return err image not found 

    if (index > imgfs_file->header.max_files) {
        return ERR_IMAGE_NOT_FOUND; 
    }

    if (imgfs_file == NULL || index >= imgfs_file->header.max_files || 
        imgfs_file->metadata[index].is_valid == EMPTY) {
        return ERR_INVALID_ARGUMENT; 
    }

    struct img_metadata *indexed_image = &imgfs_file->metadata[index];

    int found_dup = 0; 

    for (uint32_t i = 0; i < imgfs_file->header.max_files; i++) {
        if (i != index && imgfs_file->metadata[i].is_valid == NON_EMPTY) {
            struct img_metadata *other_image = &imgfs_file->metadata[i];
            if (strcmp(indexed_image->img_id, other_image->img_id) == 0) {
                return ERR_DUPLICATE_ID;
            }
            if (memcmp(indexed_image->SHA, other_image->SHA, SHA256_DIGEST_LENGTH) == 0) {
                // peutetre faire tout manuellement comme le pote de thomas 
                memcpy(indexed_image->offset, other_image->offset, sizeof(other_image->offset));
                memcpy(indexed_image->size, other_image->size, sizeof(other_image->size));
                found_dup = 1; 
                break; 
            }
        }
        if (!found_dup) {
            indexed_image->offset[ORIG_RES] = 0; 
        }
    }
    return ERR_NONE;

    
    
}

