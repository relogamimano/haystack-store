#include "imgfs.h"
#include "util.h"

#include <inttypes.h>      // for PRIxN macros
#include <openssl/sha.h>   // for SHA256_DIGEST_LENGTH
#include <stdint.h>        // for uint8_t
#include <stdio.h>         // for sprintf
#include <stdlib.h>        // for calloc
#include <string.h>
#include <vips/vips.h>

int lazily_resize(int resolution, struct imgfs_file* imgfs_file, size_t index) {
    
    M_REQUIRE_NON_NULL(imgfs_file);
    

    
    
    // Check if resolution is within bounds
    if (resolution != ORIG_RES && (resolution >= NB_RES || resolution < 0)) {
        return ERR_RESOLUTIONS;
    }

    // Check if index is within bounds
    if (index >= imgfs_file->header.nb_files) {
        return ERR_INVALID_IMGID;
    }

    struct img_metadata * metadata = &imgfs_file->metadata[index];
    
    uint32_t orig_size = metadata[index].size[ORIG_RES];
    uint64_t orig_offset = metadata[index].offset[ORIG_RES];

    // Do not resize if resolution is ORIG_RES
    if (metadata[index].is_valid == EMPTY || resolution == ORIG_RES || metadata[index].size[resolution] > 0) {
        return ERR_NONE;
    }

    // create a new variant of the specified image, in the specified resolution
        
    void * buf = malloc(orig_size);
    // Read image content from file into the buffer
    if(!buf) {
        free(buf);
        return ERR_OUT_OF_MEMORY;
    }
    if (fseek(imgfs_file->file, (long)orig_offset, SEEK_SET)
    || fread(buf, orig_size, 1, imgfs_file->file) != 1) {
        free(buf);
        return ERR_IO;
    }

    // Load the original image
    VipsImage * orig_image = NULL;
    if (vips_jpegload_buffer(buf, orig_size, &orig_image, NULL)) {
        free(buf);
        return ERR_IMGLIB;
    }

    // Resize the image
    VipsImage * resized_image = NULL;
    if (vips_thumbnail_image(orig_image, &resized_image, imgfs_file->header.resized_res[resolution * ORIG_RES], NULL)) {
        g_object_unref(orig_image);
        return ERR_IMGLIB;
    }
    g_object_unref(orig_image);

    // Save the resized image to buffer
    void* resized_buf = NULL;
    size_t resized_img_size = 0;
    if (vips_jpegsave_buffer(resized_image, &resized_buf, &resized_img_size, NULL)) {
        g_object_unref(resized_image);
        free(buf);
        return ERR_IMGLIB;
    }
    g_object_unref(resized_image);
    free(buf);
    
    
    // Write resized image content to file;
    if (fseek(imgfs_file->file, 0, SEEK_END) ||
        fwrite(resized_buf, resized_img_size, 1, imgfs_file->file) != 1) {
        g_free(resized_buf);
        return ERR_IO;
    }
    g_free(resized_buf);

    // Update metadata
    metadata[index].size[resolution] = (uint32_t)resized_img_size;
    metadata[index].offset[resolution] = (uint64_t)(ftell(imgfs_file->file) - resized_img_size);
    
    if(fseek(imgfs_file->file, sizeof(struct imgfs_header) + index * sizeof(struct img_metadata), SEEK_SET) ||
        fwrite(metadata, sizeof(struct img_metadata), 1, imgfs_file->file) != 1) {
        return ERR_IO; 
    }

    return ERR_NONE;
}

