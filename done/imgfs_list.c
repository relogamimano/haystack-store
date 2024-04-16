#include "error.h" /* Not needed in this very file,
                    * but we provide it here, as it is required by
                    * all the functions of this lib.
                    */
#include "imgfs.h"      

#include "util.h" // for TO_BE_IMPLEMENTED() 




int do_list(const struct imgfs_file* imgfs_file, 
        enum do_list_mode output_mode, char** json) {
    
    if (output_mode == JSON) {
        TO_BE_IMPLEMENTED();
    }

    if (output_mode == STDOUT) {
        print_header(&imgfs_file->header); 
    }

    int nbfiles = imgfs_file->header.nb_files; 

    if (nbfiles == 0) {
        printf("<< empty imgFS >>\n");
        return ERR_NONE; 
    }
        // TODO loop a travers toutes les images et print uniquement les valides 
        //print_metadata(&imgfs_file->metadata); 

    int found_image = 0; 

    for(int i = 0; i < imgfs_file->header.max_files; i++) {
        if (imgfs_file->metadata[i].is_valid == NON_EMPTY) {
            print_metadata(&imgfs_file->metadata[i]); 
            found_image = 1; 
        }
    }

    if (!found_image) {
        printf("<< empty imgFS >>\n");
    }

    return ERR_NONE; 

}