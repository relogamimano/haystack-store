/**
 * @file image_dedup.h
 * @brief Image deduplication.
 *
 * All functions to avoid duplication of images content in a imgFS.
 *
 * @author Mia Primorac
 */

#pragma once

#include "imgfs.h"  // for struct imgfs_file

#include <stdio.h>  // for FILE
#include <stdint.h> // for uint32_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Does image deduplication.
 *
 * @param imgfs_file The main in-memory structure
 * @param index The order number in the metadata array
 * @return Some error code. 0 if no error.
 */
int do_name_and_content_dedup(struct imgfs_file* imgfs_file, uint32_t index);

#ifdef __cplusplus
}
#endif
