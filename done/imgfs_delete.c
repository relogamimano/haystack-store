#include "imgfs.h"
#include "error.h"
#include <string.h>

int do_delete(const char* img_id, struct imgfs_file* imgfs_file) {

    M_REQUIRE_NON_NULL(imgfs_file); 
    M_REQUIRE_NON_NULL(img_id); 

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

    // faire peutetre des ERR_IO et inverser version++ et files-- si ces 4 b ails fonctionnent pas 
    fseek(imgfs_file->file, sizeof(struct imgfs_header), SEEK_SET);
    fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, imgfs_file->file); // on est d'accord 3)
    // non rewind(imgfs_file->file);
    fwrite(&(imgfs_file->header), sizeof(struct imgfs_header), 1, imgfs_file->file); // ça on est d'accord 2)
    // 


    fseek(imgfs_file->file, 0, SEEK_SET); 


    if (fseek(imgfs_file->file, 0, SEEK_SET)) {
        imgfs_file->header.version--, 
        imgfs_file->header.nb_files++; 
        return ERR_IO; 
    }


    return ERR_NONE;
}
