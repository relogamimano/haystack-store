#pragma once

/**
 * @file error.h
 * @brief error codes
 *
 * @author Edouard Bugnion
 * @date summer 2022
 */

#pragma once

#include <stdio.h> // for fprintf
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief internal error codes.
 *
 */
#define ERR_NONE 0

enum error_codes {
    ERR_FIRST = -128, // not an actual error but to set the first error number
    ERR_IO,
    ERR_RUNTIME,
    ERR_OUT_OF_MEMORY,
    ERR_NOT_ENOUGH_ARGUMENTS,
    ERR_INVALID_FILENAME,
    ERR_INVALID_COMMAND,
    ERR_INVALID_ARGUMENT,
    ERR_THREADING,
    ERR_MAX_FILES,
    ERR_RESOLUTIONS,
    ERR_INVALID_IMGID,
    ERR_IMGFS_FULL,
    ERR_IMAGE_NOT_FOUND,
    NOT_IMPLEMENTED,
    ERR_DUPLICATE_ID,
    ERR_IMGLIB,
    ERR_DEBUG,
    ERR_LAST // not an actual error but to have e.g. the total number of errors
};

/*
 * Helpers (macros)
 */

// example: ASSERT(x>0);

#define ASSERT assert

#ifdef DEBUG
#define debug_printf(fmt, ...) \
    fprintf(stderr, fmt, __VA_ARGS__)
#else
#define debug_printf(fmt, ...) \
    do {} while(0)
#endif

#define M_REQUIRE_NON_NULL(arg) \
    do { \
        if (arg == NULL) { \
            debug_printf("ERROR: parameter %s is NULL when calling  %s() (defined in %s)\n", \
                        #arg, __func__, __FILE__); \
            return ERR_INVALID_ARGUMENT; \
        } \
    } while(0)

#define pps_printf printf

/**
* @brief filesystem internal error messages. Defined in error.c.
*        Should be accessed using `ERR_MSG` (see below)
*
*/
extern const char* const ERR_MESSAGES[];

/**
* @brief yields the error message corresponding to an error code
*/
# define ERR_MSG(err) (ERR_FIRST < (err) && (err) < ERR_LAST ? ERR_MESSAGES[(err) - ERR_FIRST] : "Invalid error code")

#ifdef __cplusplus
}
#endif
