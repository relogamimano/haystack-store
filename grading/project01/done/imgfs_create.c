#include "imgfs.h"
#include "util.h"
#include <inttypes.h>      // for PRIxN macros
#include <openssl/sha.h>   // for SHA256_DIGEST_LENGTH
#include <stdint.h>        // for uint8_t
#include <stdio.h>         // for sprintf
#include <stdlib.h>        // for calloc
#include <string.h>  


int do_create(const char* imgfs_filename, struct imgfs_file* imgfs_file) {
    //  required checks 
    M_REQUIRE_NON_NULL(imgfs_filename);
    M_REQUIRE_NON_NULL(imgfs_file);
    
    // open the file in which we save the imgfs_file 
    FILE* outfile = fopen(imgfs_filename,"wb");
    if(outfile == NULL) {
        fclose(outfile);
        return ERR_IO;
    }
    
    // create header 
    strcpy(imgfs_file->header.name, CAT_TXT);
    imgfs_file->header.version = 0; // start at version 0 !
    imgfs_file->header.nb_files = 0;
    imgfs_file->header.unused_32 = 0;
    imgfs_file->header.unused_64 = 0;
    imgfs_file->file = outfile;
    
    if(fwrite(&imgfs_file->header, sizeof(struct imgfs_header), 1, outfile) != 1) {
        fclose(outfile);
        return ERR_IO;
    }

    // create metadata array
    imgfs_file->metadata = (struct img_metadata*)calloc(sizeof(struct img_metadata), imgfs_file->header.max_files);
    if(imgfs_file->metadata == NULL) {
        fclose(outfile);
        return ERR_IO;
    }

    if(fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, outfile) != imgfs_file->header.max_files) {
        fclose(outfile);
        free(imgfs_file->metadata);
        return ERR_IO;
    }

    printf("%d item(s) written\n", imgfs_file->header.max_files + 1);
    
    return ERR_NONE;
}
