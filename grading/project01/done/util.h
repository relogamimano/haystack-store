#pragma once

/**
 * @file util.h
 * @brief Tool macros and functions
 *
 * @author Jean-Cédric Chappelier
 * @date 2017-2024
 */

#include <assert.h>   // see TO_BE_IMPLEMENTED
#include <stddef.h>   // for size_t
#include <stdint.h>   // for uint16_t, uint32_t

/**
 * @brief tag a variable as POTENTIALLY unused, to avoid compiler warnings
 */
#define _unused __attribute__((unused))

/**
 * @brief useful for partial implementation
 */
#define TO_BE_IMPLEMENTED() \
    do { fprintf(stderr, "TO_BE_IMPLEMENTED!\n"); assert(0); } while (0)

/**
 * @brief useful to free pointers to const without warning. Use with care!
 */
#define free_const_ptr(X) free((void *) X)

/**
 * @brief useful to have C99 (!) %zu to compile in Windows
 */
#if defined _WIN32 || defined _WIN64
#define SIZE_T_FMT "%u"
#else
#define SIZE_T_FMT "%zu"
#endif

/**
 * @brief useful to specify a length defined by a macro for format strings
 */
#define STR(x)            #x
#define STR_LENGTH_FMT(x) "%." STR(x) "s"

/**
 * @brief usual macro for min() and max() functions
 */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * @brief returns the first non null value
 */
#define COALESCE(a, b) (((a) == 0) ? (b) : (a))

/**
 * @brief useful to init a variable (typically a struct) directly or through a pointer
 */
#define zero_init_var(X) memset(&X, 0, sizeof(X))
#define zero_init_ptr(X) memset(X, 0, sizeof(*X))

/**
 * @brief String to uint16_t conversion function
 *
 * @param str a string containing some integer value to be extracted
 * @return converted value in uint16_t format
 */
uint16_t atouint16(const char* str);

/**
 * @brief String to uint32_t conversion function
 *
 * @param str a string containing some integer value to be extracted
 * @return converted value in uint32_t format
 */
uint32_t atouint32(const char* str);

/**
 * @brief Find the first occurrence of find in s, where the search is limited to the
 *        first slen characters of s.
 * @param str  a string to search into
 * @param find the string searched for
 * @param slen the max search length
 * @return first occurrence of find in str or NULL
 */
char* strnstr(const char* str, const char* find, size_t slen);

/**
 * @brief prints the description of the last error from the std library,
 * preceded by the file and line number
 */
#ifdef WITH_PRINT
#define print_error() \
  do { perror(__FILE__ ":" STR(__LINE__) ":"); } while (0)
#else
#define print_error() \
  do { } while (0)
#endif
