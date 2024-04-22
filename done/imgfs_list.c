#include "error.h" /* Not needed in this very file,
                    * but we provide it here, as it is required by
                    * all the functions of this lib.
                    */
#include "imgfs.h"      

#include "util.h" // for TO_BE_IMPLEMENTED() 




int do_list(const struct imgfs_file* imgfs_file, 
        enum do_list_mode output_mode, char** json) {

    
    M_REQUIRE_NON_NULL(imgfs_file); 
    //M_REQUIRE_NON_NULL(json); 

    
    

    if (output_mode == STDOUT) {
        print_header(&imgfs_file->header); 
    

        int nbfiles = imgfs_file->header.nb_files; 


        if (nbfiles == 0) {
            printf("<< empty imgFS >>\n");
        } else {
            int is_image_found = 0;
            for(unsigned int i=0;i<imgfs_file->header.max_files; i++) {
                if(imgfs_file->metadata[i].is_valid == NON_EMPTY){
                    print_metadata(&imgfs_file->metadata[i]);
                    is_image_found = 1;
                }
            }
            if(!is_image_found){
                printf("<< empty imgFS >>\n");
            }
        }//TODO : can nb_files != 0 and no metadata elements valid ?
    } else if (output_mode == JSON) {
        TO_BE_IMPLEMENTED(); 
    }

    return ERR_NONE; 

}