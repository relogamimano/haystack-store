/**
 * @file image_content.h
 * @brief Image content manipulation functions.
 *
 * @author Mia Primorac
 */

#pragma once

#include "imgfs.h" // for struct imgfs_header, struct img_metadata, struct imgfs_file

#include <stdio.h> // for FILE
#include <stdint.h> // for uint16_t, uint32_t, uint64_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gets the resolution of an image.
 *
 * @param height Where to put the calculated image height.
 * @param width Where to put the calculated image width.
 * @param filename The image file name.
 * @return Some error code. 0 if no error.
 */
int get_resolution(uint32_t *height, uint32_t *width, const char *image_buffer, size_t image_size);

/**
 * @brief Calls the create_resized_img function and updates the metadata on the disk
 *
 * @param resolution
 * @param imgfs_file The main in-memory structure
 * @param index The index of the image in the metadata array
 * @return Some error code. 0 if no error.
 */
int lazily_resize(int resolution, struct imgfs_file* imgfs_file, size_t index);

#ifdef __cplusplus
}
#endif
