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
 * @return ERR_NONE on success, ERR_IO on failure
 */
int do_create(const char* imgfs_filename, struct imgfs_file* imgfs_file) {
    // Check for null pointers
    M_REQUIRE_NON_NULL(imgfs_filename);
    M_REQUIRE_NON_NULL(imgfs_file);

    // Open the file for writing
    FILE* outfile = fopen(imgfs_filename,"wb");
    if(outfile == NULL) {
        fclose(outfile);
        return ERR_IO; // Return ERR_IO if file opening fails
    }
    
    // Initialize header values
    strcpy(imgfs_file->header.name, CAT_TXT); // Set the name
    imgfs_file->header.version = 0; // Start at version 0
    imgfs_file->header.nb_files = 0; // No files initially
    imgfs_file->header.unused_32 = 0; // Unused
    imgfs_file->header.unused_64 = 0; // Unused
    
    // Write header to file
    if(fwrite(&imgfs_file->header, sizeof(struct imgfs_header), 1, outfile) != 1) {
        fclose(outfile);
        return ERR_IO; // Return ERR_IO if header writing fails
    }

    // Allocate memory for metadata
    imgfs_file->metadata = (struct img_metadata*)calloc(sizeof(struct img_metadata), imgfs_file->header.max_files);
    if(imgfs_file->metadata == NULL) {
        fclose(outfile);
        return ERR_IO; // Return ERR_IO if memory allocation fails
    }

    // Write empty metadata array to file
    if(fwrite(imgfs_file->metadata, sizeof(struct img_metadata), imgfs_file->header.max_files, outfile) != imgfs_file->header.max_files) {
        fclose(outfile);
        free(imgfs_file->metadata);
        return ERR_IO; // Return ERR_IO if metadata writing fails
    }

    // Success message
    printf("%d item(s) written\n", imgfs_file->header.max_files + 1);

    return ERR_NONE; // Return ERR_NONE indicating success
}
