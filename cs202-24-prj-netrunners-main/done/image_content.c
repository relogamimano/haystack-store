#include "imgfs.h"
#include "util.h"
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

    // Do not resize if resolution is ORIG_RES
    if (metadata->is_valid == EMPTY || resolution == ORIG_RES || metadata->size[resolution] > 0) {
        return ERR_NONE;
    }

    // create a buffer 
    uint32_t orig_size = metadata->size[ORIG_RES];
    uint64_t orig_offset = metadata->offset[ORIG_RES];
    void *buf = malloc(orig_size);
    if (!buf) {
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
    if (vips_thumbnail_image(orig_image, &resized_image, 
        imgfs_file->header.resized_res[resolution * ORIG_RES], NULL)) {
        g_object_unref(orig_image);
        free(buf);
        return ERR_IMGLIB;
    }
    g_object_unref(orig_image);

    // save the new resized image in a new buffer  
    void * resized_buf = NULL;
    size_t resized_size = 0;
    if (vips_jpegsave_buffer(resized_image, &resized_buf, &resized_size, NULL)) {
        g_object_unref(resized_image);
        free(buf);
        return ERR_IMGLIB;
    }
    g_object_unref(resized_image);
    free(buf);

    // save the resized image 

    if (fseek(imgfs_file->file, 0, SEEK_END) != 0 ||
        fwrite(resized_buf,resized_size, 1, imgfs_file->file) != 1) {
        g_free(resized_buf);
        return ERR_IO;
    }
    g_free(resized_buf);

    // Update metadata
    metadata->size[resolution] = (uint32_t)resized_size;
    metadata->offset[resolution] = (uint64_t)(ftell(imgfs_file->file) - resized_size);
    
    if(fseek(imgfs_file->file, sizeof(struct imgfs_header) + index * sizeof(struct img_metadata), SEEK_SET) ||
        fwrite(metadata, sizeof(struct img_metadata), 1, imgfs_file->file) != 1) {
        return ERR_IO; 
    }

    return ERR_NONE;
}

// Prov ded method from week 10
int get_resolution(uint32_t *height, uint32_t *width,
                   const char *image_buffer, size_t image_size)
{
    M_REQUIRE_NON_NULL(height);
    M_REQUIRE_NON_NULL(width);
    M_REQUIRE_NON_NULL(image_buffer);

    VipsImage* original = NULL;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
    const int err = vips_jpegload_buffer((void*) image_buffer, image_size,
                                         &original, NULL);
#pragma GCC diagnostic pop
    if (err != ERR_NONE) return ERR_IMGLIB;
    
    *height = (uint32_t) vips_image_get_height(original);
    *width  = (uint32_t) vips_image_get_width(original);
    
    g_object_unref(VIPS_OBJECT(original));
    return ERR_NONE;
}