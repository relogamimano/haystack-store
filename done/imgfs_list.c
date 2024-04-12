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
    } else {
        // TODO loop a travers toutes les images et print uniquement les valides 
        //print_metadata(&imgfs_file->metadata); 
    }

    return ERR_NONE; 

}