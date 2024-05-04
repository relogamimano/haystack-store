#include "imgfs.h"
#include "util.h"
#include <vips/vips.h>



int lazily_resize(int resolution, struct imgfs_file* imgfs_file, size_t index) {

    M_REQUIRE_NON_NULL(imgfs_file);

    if (resolution != ORIG_RES && (resolution < 0 || resolution >= NB_RES)) {
        return ERR_RESOLUTIONS;
    }

    if (index >= imgfs_file->header.nb_files) {
        return ERR_INVALID_IMGID;
    }

    struct img_metadata *metadata = &imgfs_file->metadata[index];

    // if the requested image already exists in the corresponding resolution, do nothing 
    if (metadata->is_valid != NON_EMPTY || resolution == ORIG_RES || metadata->size[resolution] > 0) {
        return ERR_NONE; 
    }

    size_t orig_size = metadata->size[ORIG_RES];
    uint64_t orig_offset = metadata->offset[ORIG_RES];
    void *buf = malloc(orig_size);
    if (!buf) {
        return ERR_OUT_OF_MEMORY;
    }

    if (fseek(imgfs_file->file, orig_offset, SEEK_SET) ||
        fread(buf, orig_size, 1, imgfs_file->file) != 1) {
        free(buf);
        return ERR_IO;
    }

    VipsImage *orig_image = NULL;
    if (vips_jpegload_buffer(buf, orig_size, &orig_image, NULL)) {
        free(buf);
        return ERR_IMGLIB;
    }

    VipsImage *resized_image = NULL;
    int r = imgfs_file->header.resized_res[resolution * 2];
    if (vips_thumbnail_image(orig_image, &resized_image, r, NULL)) {
        g_object_unref(orig_image);
        free(buf);
        return ERR_IMGLIB;
    }
    g_object_unref(orig_image);

    void *resized_buf = NULL;
    size_t resized_size = 0;
    if (vips_jpegsave_buffer(resized_image, &resized_buf, &resized_size, NULL)) {
        g_object_unref(resized_image);
        free(buf);
        return ERR_IMGLIB;
    }
    g_object_unref(resized_image);
    free(buf);

    if (fseek(imgfs_file->file, 0, SEEK_END) != 0 ||
        fwrite(resized_buf, 1, resized_size, imgfs_file->file) != resized_size) {
        g_free(resized_buf);
        return ERR_IO;
    }
    g_free(resized_buf);


    //metadata->offset[resolution] = ftell(imgfs_file->file);
    metadata->offset[resolution] = ftell(imgfs_file->file) - resized_size;

    metadata->size[resolution] = resized_size;

    fseek(imgfs_file->file, sizeof(struct imgfs_header) + index * sizeof(struct img_metadata), SEEK_SET);
    fwrite(metadata, sizeof(struct img_metadata), 1, imgfs_file->file);

    return ERR_NONE;
}
