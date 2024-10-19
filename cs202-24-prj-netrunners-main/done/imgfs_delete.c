#include "imgfs.h"
#include "error.h"
#include <string.h>

/**
 * @brief Deletes an image in the imgfs_file by deferencing it 
 *
 * @param img_id the id of the image 
 * @param imgfs_file the imgfs_file we delete the image from 
 * @return Some error code. 0 if no error.
 */

int do_delete(const char* img_id, struct imgfs_file* imgfs_file) {

    // checks 

    M_REQUIRE_NON_NULL(imgfs_file); 
    M_REQUIRE_NON_NULL(img_id); 


    // look for the image we want to delete, deference it if found 
    int found = 0;
    for (uint32_t i = 0; i < imgfs_file->header.max_files; i++) {
        if (strcmp(imgfs_file->metadata[i].img_id, img_id) == 0 && imgfs_file->metadata[i].is_valid == NON_EMPTY) {
            imgfs_file->metadata[i].is_valid = EMPTY;  
            found = 1;
            break;
        }
    }

    // throw an error if the image is not found 
    if (!found) {
        return ERR_IMAGE_NOT_FOUND; 
    }

    // update metadata 
    if (fseek(imgfs_file->file, sizeof(struct imgfs_header), SEEK_SET) != 0 ||
        fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, imgfs_file->file) != imgfs_file->header.max_files) {
        return ERR_IO; 
    }

    // if the metadata update was successful, update header 
    imgfs_file->header.version++;
    imgfs_file->header.nb_files--;
    if (fseek(imgfs_file->file, 0, SEEK_SET) != 0 ||
        fwrite(&(imgfs_file->header), sizeof(struct imgfs_header), 1, imgfs_file->file) != 1) {
        imgfs_file->header.version--; 
        imgfs_file->header.nb_files++; 
        return ERR_IO;
    }

    return ERR_NONE;
}
