
#include "imgfs.h"
#include "util.h"

#include <inttypes.h>      // for PRIxN macros
#include <openssl/sha.h>   // for SHA256_DIGEST_LENGTH
#include <stdint.h>        // for uint8_t
#include <stdio.h>         // for sprintf
#include <stdlib.h>        // for calloc
#include <string.h>  

/**
 * @brief Creates the imgFS called imgfs_filename. Writes the header and the
 *        preallocated empty metadata array to imgFS file.
 *
 * @param imgfs_filename Path to the imgFS file
 * @param imgfs_file In memory structure with header and metadata.
 */
int do_create(const char* imgfs_filename, struct imgfs_file* imgfs_file) {
    M_REQUIRE_NON_NULL(imgfs_filename);
    M_REQUIRE_NON_NULL(imgfs_file);
    struct imgfs_header header = imgfs_file->header;
    struct img_metadata * metadata = imgfs_file->metadata;

    FILE* outfile = fopen(imgfs_filename,"r");
    // SHA256(*igfs_filename, strlen(*imgfs_filename), imgfs_file->metadata.SHA)
    if(outfile == NULL) {
        fclose(outfile);
        return ERR_IO;
    }
    imgfs_file->metadata = (struct img_metadata*)calloc(sizeof(struct img_metadata), header.max_files);
    if(imgfs_file->metadata == NULL) {
        fclose(outfile);
        return ERR_IO;
    }
    
    strcpy(imgfs_file->header.name, CAT_TXT);
    header.version = 1;
    header.nb_files = 0;
    header.unused_32 = 0;
    header.unused_64 = 0;
    
    if(fwrite(&header, sizeof(struct imgfs_header), 1, outfile) != 1) {
        fclose(outfile);
        return ERR_IO;
    }

    if(fwrite(metadata, sizeof(struct img_metadata), header.max_files, outfile) != header.max_files) {
        fclose(outfile);
        return ERR_IO;
    }

    // printf("%d item(s) written", 1 + metadata->nb_files);

    // free(metadata)
    // metadata = NULL

    return ERR_NONE;
}