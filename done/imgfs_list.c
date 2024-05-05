#include "error.h" 
#include "imgfs.h"      
#include "util.h" 




int do_list(const struct imgfs_file* imgfs_file, 
        enum do_list_mode output_mode, char** json) {


    M_REQUIRE_NON_NULL(imgfs_file); 
    // at this point of the project json is unused 
    //M_REQUIRE_NON_NULL(json); 

    if (output_mode == STDOUT) {
        print_header(&imgfs_file->header); 
    

        uint32_t nbfiles = imgfs_file->header.nb_files; 


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
        }
    } else if (output_mode == JSON) {
        TO_BE_IMPLEMENTED(); 
    }

    return ERR_NONE; 

}
