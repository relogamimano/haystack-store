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
    M_REQUIRE_NON_NULL(resolution);
    M_REQUIRE_NON_NULL(imgfs_file);
    M_REQUIRE_NON_NULL(index);
    if(resolution == ORIG_RES) {
        return ERR_NONE;
    } else if (resolution > ORIG_RES || resolution < 0) {
        return ERR_RESOLUTIONS;
    }
    

    struct imgfs_header header = imgfs_file->header;
    struct img_metadata * metadata = &imgfs_file->metadata[index];
    uint32_t orig_size = metadata->size[ORIG_RES];
    uint64_t orig_offset = metadata->offset[ORIG_RES];
    
    

    // uint16_t thumbnail_res = header.resized_res[0] * header.resized_res[1];
    // uint16_t small_res = header.resized_res[2] * header.resized_res[3];

    if (orig_size == 0) {

        // create a new variant  of the specified image, in the specified resolution;     
        
        VipsImage * orig_image = NULL;
        if(!fseek(imgfs_file->file, (long)orig_offset, SEEK_SET)) { return ERR_IO; }
        void * buf = NULL;
        if (buf == NULL) { return ERR_IO; }
        if(fread(buf, 1, orig_size, imgfs_file->file) != orig_size) { return ERR_IO; }
        
        if(!vips_jpegload_buffer(buf, metadata->size[ORIG_RES], &orig_image, NULL)) {
            g_free(buf); 
            vips_error_exit("unable to load image");
        }
        g_free(buf);

        VipsImage * resized_image = NULL;
        int r = resolution * ORIG_RES;
        vips_thumbnail_image(orig_image, &resized_image, header.resized_res[r], header.resized_res[r+1], NULL);
        
        void* resized_buf = NULL;
        size_t* new_img_size = 0;
        if(!vips_jpegsave_buffer(resized_image, &resized_buf, new_img_size, NULL)) {
            g_free(resized_buf);
            vips_error_exit("unable to save image");
        }
        g_free(resized_buf);
        metadata->size[resolution] = (uint32_t)new_img_size;
        // copy the content of this new image ot the end of the imgFS file;
        if(!fseek(imgfs_file->file, 0, SEEK_END)) {return ERR_IO;}
        metadata->offset[resolution] = (uint64_t)ftell(imgfs_file->file);
        if(!fwrite(resized_image, 1, orig_size, imgfs_file->file)) { return ERR_IO; }

        // update the contents of the metadata in memory and on disk
        // imgfs_file->header.resized_res =;
        // imgfs_file->metadata[index]
        g_object_unref(resized_image);
        g_object_unref(orig_image);
    } 
    
    
    return ERR_NONE;

}
