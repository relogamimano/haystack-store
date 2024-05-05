#include "imgfs.h"
#include "error.h"
#include <string.h>

int do_delete(const char* img_id, struct imgfs_file* imgfs_file) {
    if (imgfs_file == NULL || img_id == NULL) {
        return ERR_INVALID_ARGUMENT;  
    }

    if (imgfs_file->file == NULL) { puts("debug trace imgfs_file-> file is NULL");}

    int found = 0;
    for (uint32_t i = 0; i < imgfs_file->header.max_files; i++) {
        if (strcmp(imgfs_file->metadata[i].img_id, img_id) == 0 && imgfs_file->metadata[i].is_valid == NON_EMPTY) {
            imgfs_file->metadata[i].is_valid = EMPTY;  
            found = 1;
            break;
        }
    }

    if (!found) {
        return ERR_IMAGE_NOT_FOUND; 
    }

    imgfs_file->header.version++; 
    imgfs_file->header.nb_files--;  

    // 
    fseek(imgfs_file->file, sizeof(struct imgfs_header), SEEK_SET);
    if(fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, imgfs_file->file) != imgfs_file->header.max_files) { return ERR_IO;}
    rewind(imgfs_file->file);
    fwrite(&(imgfs_file->header), sizeof(struct imgfs_header), 1, imgfs_file->file);

    return ERR_NONE;
}
