#include "imgfs.h"  
#include <stdio.h> 
#include <stdint.h>
#include <error.h>
#include <string.h>

int do_name_and_content_dedup(struct imgfs_file* imgfs_file, uint32_t index) {

    // requireed checks 
    M_REQUIRE_NON_NULL(imgfs_file); 
    M_REQUIRE_NON_NULL(imgfs_file->metadata);  


    if (index > imgfs_file->header.nb_files || imgfs_file->metadata[index].is_valid == EMPTY) {
        return ERR_IMAGE_NOT_FOUND; 
    }



    struct img_metadata *indexed_image = &imgfs_file->metadata[index];

    int found_dup = 0; 

    // go through the images and copy the one with the same SHA as the requested image
    // also throw an error if it finds a duplicate 

    for (uint32_t i = 0; i < imgfs_file->header.max_files; i++) {
        if (i != index && imgfs_file->metadata[i].is_valid == NON_EMPTY) {
            struct img_metadata *other_image = &imgfs_file->metadata[i];
            if (!strcmp(indexed_image->img_id, other_image->img_id)) {
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