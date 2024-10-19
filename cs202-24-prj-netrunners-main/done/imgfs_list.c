#include "error.h" 
#include "imgfs.h"      
#include "util.h"
#include "string.h"
#include <json-c/json.h>




int do_list(const struct imgfs_file* imgfs_file, 
        enum do_list_mode output_mode, char** json) {

    M_REQUIRE_NON_NULL(imgfs_file); 
    int err;

    if (output_mode == STDOUT) {
        print_header(&imgfs_file->header); 

        uint32_t nbfiles = imgfs_file->header.nb_files;

        if (nbfiles == 0) {
            printf("<< empty imgFS >>\n");
        } else {
            int is_image_found = 0;
            for(uint32_t i = 0; i < imgfs_file->header.max_files; i++) {
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
        const char * IMG_KEY = "Images";
        uint32_t nbfiles = imgfs_file->header.nb_files; 
        int is_image_found = 0;
        // Create a json object array
        struct json_object * json_obj_arr = json_object_new_array();
        if(json_obj_arr == NULL) {
            return ERR_RUNTIME;
        }
        for(uint32_t i = 0; i < imgfs_file->header.max_files; i++) {
            if(imgfs_file->metadata[i].is_valid == NON_EMPTY){
                // Add the image id to the json array
                struct json_object * json_obj_str = json_object_new_string(&imgfs_file->metadata[i].img_id);
                if(json_obj_str == NULL) {
                    return ERR_RUNTIME;
                }
                if(json_object_array_add(json_obj_arr, json_obj_str) < 0) {
                    return ERR_RUNTIME;
                }
                is_image_found = 1;
            }
        }
        struct json_object * json_obj_obj = json_object_new_object();
        if(json_object_object_add(json_obj_obj, IMG_KEY, json_obj_arr) < 0) {
            return ERR_RUNTIME;
        }
        // Copy the json object to the output
        const char * json_str = json_object_to_json_string(json_obj_obj);
        *json = malloc(strlen(json_str) + 1);
        strcpy(*json, json_str);
        json_object_put(json_obj_obj); // free the json object
    } else {
        TO_BE_IMPLEMENTED();
    }

    return ERR_NONE; 

}
