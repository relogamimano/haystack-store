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
    
    struct imgfs_header header = imgfs_file->header;

    // Check if resolution is within bounds
    if (resolution != ORIG_RES && (resolution >= NB_RES || resolution < 0)) {
        return ERR_RESOLUTIONS;
    }

    // Check if index is within bounds
    if (index >= header.nb_files) {
        return ERR_INVALID_IMGID;
    }

    struct img_metadata * metadata = &imgfs_file->metadata[index];
    uint32_t orig_size = metadata->size[ORIG_RES];
    uint64_t orig_offset = metadata->offset[ORIG_RES];

    // Do not resize if resolution is ORIG_RES
    if (resolution == ORIG_RES || metadata->size[resolution] > 0) {
        return ERR_NONE;
    }

    
    // create a new variant of the specified image, in the specified resolution
        
    void * buf = malloc(orig_size);
    // Read image content from file into the buffer
    if (fseek(imgfs_file->file, (long)orig_offset, SEEK_SET) != 0 ||
        fread(buf, 1, orig_size, imgfs_file->file) != orig_size) {
        g_free(buf);
        return ERR_IO;
    }

    // Load the original image
    VipsImage * orig_image = NULL;
    if (!vips_jpegload_buffer(buf, orig_size, &orig_image, NULL)) {
        g_free(buf);
        return ERR_IO;
    }
    free(buf);

    // Resize the image
    VipsImage * resized_image = NULL;
    int r = resolution * ORIG_RES;
    if (vips_thumbnail_image(orig_image, &resized_image, header.resized_res[r], header.resized_res[r+1], NULL) != 0) {
        g_object_unref(orig_image);
        return ERR_IO;
    }
    g_object_unref(orig_image);

    // Save the resized image to buffer
    void* resized_buf = NULL;
    size_t new_img_size = 0;
    if (!vips_jpegsave_buffer(resized_image, &resized_buf, &new_img_size, NULL)) {
        g_object_unref(resized_image);
        return ERR_IO;
    }
    g_object_unref(resized_image);

    // Write resized image content to file
    if (fseek(imgfs_file->file, 0, SEEK_END) != 0 ||
        fwrite(resized_buf, 1, new_img_size, imgfs_file->file) != new_img_size) {
        g_free(resized_buf);
        return ERR_IO;
    }
    g_free(resized_buf);

    // Update metadata
    metadata->size[resolution] = (uint32_t)new_img_size;
    metadata->offset[resolution] = (uint64_t)ftell(imgfs_file->file);
    

    return ERR_NONE;
}

