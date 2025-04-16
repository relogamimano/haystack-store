#include "imgfs.h"
#include "error.h"
#include "image_content.h"
#include "string.h"

#include <stdio.h>
#include <openssl/sha.h> // for SHA256_DIGEST_LENGTH and SHA256()

int do_insert(const char* image_buffer, size_t image_size, const char* img_id, struct imgfs_file* imgfs_file) {
    M_REQUIRE_NON_NULL(image_buffer);
    M_REQUIRE_NON_NULL(img_id);
    M_REQUIRE_NON_NULL(imgfs_file);

    // FIND A FREE POSITION IN THE INDEX
    if(imgfs_file->header.max_files <= imgfs_file->header.nb_files) {
        return ERR_IMGFS_FULL;
    }

    // ITERATE THROUGH ALL THE METADATA OF EACH IMAGE UNTIL WE FIND A NON-EMPTY ONE
    for(uint32_t i = 0; i < imgfs_file->header.max_files; i++) {
        struct img_metadata *metadata = &imgfs_file->metadata[i];
        if(metadata->is_valid == EMPTY) {
            SHA256((const unsigned char*)image_buffer, image_size, &metadata->SHA);
            strcpy(metadata->img_id, img_id);
            metadata->size[ORIG_RES] = (uint32_t)image_size;
           
            int is_res_valid = get_resolution(&metadata->orig_res[HEIGHT_I], &metadata->orig_res[WIDTH_I], image_buffer, image_size);
            if(is_res_valid) { return is_res_valid; }
            
            metadata->is_valid = NON_EMPTY;
            
            int is_duplicate = do_name_and_content_dedup(imgfs_file, i);
            if(is_duplicate) {
                metadata->is_valid = EMPTY;
                return is_duplicate;
            }
            
            if (metadata->offset[ORIG_RES] == 0) {// IF THE IMAGE IS NOT A DUPLICATE (OFFSET == 0)
                // GOING TO THE END OF THE FILE AND ADDING THE NEW IMAGE ON THE DISK
                if (fseek(imgfs_file->file, 0, SEEK_END) ||
                    fwrite(image_buffer,image_size, 1, imgfs_file->file) != 1) {
                    return ERR_IO;
                }
                // UPDATING THE METADATA
                metadata->offset[ORIG_RES] = (uint64_t)(ftell(imgfs_file->file) - image_size);
                metadata->offset[THUMB_RES] = 0; metadata->size[THUMB_RES] = 0;
                metadata->offset[SMALL_RES] = 0; metadata->size[SMALL_RES] = 0;
            }


            // GOING TO THE METADATA AND UPDATING IT ON THE DISK
            if(fseek(imgfs_file->file, sizeof(struct imgfs_header) + i * sizeof(struct img_metadata), SEEK_SET) ||
                fwrite(metadata, sizeof(struct img_metadata), 1, imgfs_file->file) != 1) {
                return ERR_IO; 
            }

            // UPDATING THE HEADER
            imgfs_file->header.version++;
            imgfs_file->header.nb_files++;

            // GOING TO THE HEADER AND UPDATING IT ON THE DISK
            if (fseek(imgfs_file->file, 0, SEEK_SET) ||
                fwrite(&imgfs_file->header, sizeof(struct imgfs_header), 1, imgfs_file->file) != 1){
                return ERR_IO;
            }
            break;
        }   
    }
    return ERR_NONE;
}

