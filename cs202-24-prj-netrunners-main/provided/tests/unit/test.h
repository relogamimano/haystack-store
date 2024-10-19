#pragma once

/**
 * @file tests.h
 * @brief PPS (CS-212) Utilities for tests
 *
 * @author Valérian Rousset, J.-C. Chappelier, E. Bugnion, L. Mermod
 * @date 2017-2023
 */

#include <check.h>
#include <stdlib.h>   // EXIT_FAILURE

#ifndef ck_assert_mem_eq
// exists since check 0.11.0
// copied from check.h 0.15.2
// Copyright (C) 2001, 2002 Arien Malec
// cf https://github.com/libcheck/check/blob/master/src/check.h.in

#define ck_assert_mem_eq(X, Y, L) _ck_assert_mem(X, ==, Y, L)

#ifndef CK_MAX_ASSERT_MEM_PRINT_SIZE
#define CK_MAX_ASSERT_MEM_PRINT_SIZE 64
#endif

#define _ck_assert_mem(X, OP, Y, L)                                                                                    \
    do {                                                                                                               \
        const uint8_t *_ck_x = (const uint8_t *) (X);                                                                  \
        const uint8_t *_ck_y = (const uint8_t *) (Y);                                                                  \
        size_t _ck_l = (L);                                                                                            \
        char _ck_x_str[CK_MAX_ASSERT_MEM_PRINT_SIZE * 2 + 1];                                                          \
        char _ck_y_str[CK_MAX_ASSERT_MEM_PRINT_SIZE * 2 + 1];                                                          \
        static const char _ck_hexdigits[] = "0123456789abcdef";                                                        \
        size_t _ck_i;                                                                                                  \
        size_t _ck_maxl = (_ck_l > CK_MAX_ASSERT_MEM_PRINT_SIZE) ? CK_MAX_ASSERT_MEM_PRINT_SIZE : _ck_l;               \
        for (_ck_i = 0; _ck_i < _ck_maxl; _ck_i++) {                                                                   \
            _ck_x_str[_ck_i * 2] = _ck_hexdigits[(_ck_x[_ck_i] >> 4) & 0xF];                                           \
            _ck_y_str[_ck_i * 2] = _ck_hexdigits[(_ck_y[_ck_i] >> 4) & 0xF];                                           \
            _ck_x_str[_ck_i * 2 + 1] = _ck_hexdigits[_ck_x[_ck_i] & 0xF];                                              \
            _ck_y_str[_ck_i * 2 + 1] = _ck_hexdigits[_ck_y[_ck_i] & 0xF];                                              \
        }                                                                                                              \
        _ck_x_str[_ck_i * 2] = 0;                                                                                      \
        _ck_y_str[_ck_i * 2] = 0;                                                                                      \
        if (_ck_maxl != _ck_l) {                                                                                       \
            _ck_x_str[_ck_i * 2 - 2] = '.';                                                                            \
            _ck_y_str[_ck_i * 2 - 2] = '.';                                                                            \
            _ck_x_str[_ck_i * 2 - 1] = '.';                                                                            \
            _ck_y_str[_ck_i * 2 - 1] = '.';                                                                            \
        }                                                                                                              \
        ck_assert_msg(0 OP memcmp(_ck_y, _ck_x, _ck_l), "Assertion '%s' failed: %s == \"%s\", %s == \"%s\"",           \
                      #X " " #OP " " #Y, #X, _ck_x_str, #Y, _ck_y_str);                                                \
    } while (0)
#endif   // ck_assert_mem_eq

#include "error.h"
#include "imgfs.h"

#if CHECK_MINOR_VERSION >= 13
#define TEST_FUNCTION_POSTFIX "_fn"
#else
#define TEST_FUNCTION_POSTFIX ""
#endif

static const char *const ERR_NAMES[] = {"ERR_FIRST",
                                        "ERR_IO",
                                        "ERR_RUNTIME",
                                        "ERR_OUT_OF_MEMORY",
                                        "ERR_NOT_ENOUGH_ARGUMENTS",
                                        "ERR_INVALID_FILENAME",
                                        "ERR_INVALID_COMMAND",
                                        "ERR_INVALID_ARGUMENT",
                                        "ERR_THREADING",
                                        "ERR_MAX_FILES",
                                        "ERR_RESOLUTIONS",
                                        "ERR_INVALID_IMGID",
                                        "ERR_IMGFS_FULL",
                                        "ERR_IMAGE_NOT_FOUND",
                                        "NOT_IMPLEMENTED",
                                        "ERR_DUPLICATE_ID",
                                        "ERR_IMGLIB",
                                        "ERR_DEBUG",
                                        "ERR_LAST"
                                       };

#define ERR_NAME(err)                                                                                                  \
    (err < ERR_FIRST || ERR_LAST < err ? err > 0 ? "VALUE" : (err == 0 ? "ERR_NONE" : "UNKNOWN")                       \
                                       : ERR_NAMES[err - ERR_FIRST])

#define ck_assert_err_core(value, op, err)                                                                             \
    do {                                                                                                               \
        int __value = (value);                                                                                         \
        if (!(__value op err)) {                                                                                       \
            ck_abort_msg("Assertion %s " #op " %s failed, got %s (%d)", #value, #err, ERR_NAME(__value), __value);     \
        }                                                                                                              \
        mark_point();                                                                                                  \
    } while (0)

#define ck_assert_err(value, err) ck_assert_err_core(value, ==, err)

#define ck_assert_fails(value) ck_assert_err_core(value, <, ERR_NONE)

#define ck_assert_invalid_arg(value) ck_assert_err(value, ERR_INVALID_ARGUMENT)

#define ck_assert_err_mem(value) ck_assert_err(value, ERR_NOMEM)

#define ck_assert_err_none(value) ck_assert_err(value, ERR_NONE)

#ifndef ck_assert_ptr_nonnull
#define ck_assert_ptr_nonnull(ptr) ck_assert_ptr_ne(ptr, NULL)
#endif

#ifndef ck_assert_ptr_null
#define ck_assert_ptr_null(ptr) ck_assert_ptr_eq(ptr, NULL)
#endif

#define Add_Case(S, C, Title)                                                                                          \
    TCase *C = tcase_create(Title);                                                                                    \
    suite_add_tcase(S, C)

#define Add_Test(S, Title)                                                                                             \
    do {                                                                                                               \
        Add_Case(S, tc, #Title);                                                                                       \
        tcase_add_test(tc, Title);                                                                                     \
    } while (0)

#define TEST_SUITE(get_suite)                                                                                          \
    int main(void) {                                                                           \
        SRunner *sr = srunner_create(get_suite());                                                                     \
        srunner_run_all(sr, CK_VERBOSE);                                                                               \
                                                                                                                       \
        int number_failed = srunner_ntests_failed(sr);                                                                 \
                                                                                                                       \
        if (number_failed > 0) {                                                                                       \
            TestResult **results = srunner_failures(sr);                                                               \
                                                                                                                       \
            puts("\033[31m==================== SUMMARY ====================\033[000m");                                \
                                                                                                                       \
            for (int i = 0; i < number_failed; ++i) {                                                                  \
                char buf[4097] = {0}; /* Max path length on Unix is 4096 */                                            \
                strcat(buf, __FILE__);                                                                                 \
                buf[strlen(buf) - 2] = 0; /* skip the trailing '.c' */                                                 \
                                                                                                                       \
                printf("\033[31m|\033[000m Test %s failed. To run in gdb, use: \033[001mmake dbg "                     \
                       "TEST=%s" TEST_FUNCTION_POSTFIX " "                                                             \
                       "EXE=%s\033[000m\n",                                                                            \
                       tr_tcname(results[i]), tr_tcname(results[i]), buf + 10);                                        \
            }                                                                                                          \
                                                                                                                       \
            puts("\033[31m=================================================\033[000m");                                \
                                                                                                                       \
            free(results);                                                                                             \
        }                                                                                                              \
                                                                                                                       \
        srunner_free(sr);                                                                                              \
        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;                                                     \
    }

#define TEST_SUITE_VIPS(get_suite)                                                                                     \
    int main(__attribute__((unused)) int argc, const char *argv[]) {                                                                           \
        VIPS_INIT(argv[0]);                                                                                            \
        SRunner *sr = srunner_create(get_suite());                                                                     \
        srunner_run_all(sr, CK_VERBOSE);                                                                               \
                                                                                                                       \
        int number_failed = srunner_ntests_failed(sr);                                                                 \
                                                                                                                       \
        if (number_failed > 0) {                                                                                       \
            TestResult **results = srunner_failures(sr);                                                               \
                                                                                                                       \
            puts("\033[31m==================== SUMMARY ====================\033[000m");                                \
                                                                                                                       \
            for (int i = 0; i < number_failed; ++i) {                                                                  \
                char buf[4097] = {0}; /* Max path length on Unix is 4096 */                                            \
                strcat(buf, __FILE__);                                                                                 \
                buf[strlen(buf) - 2] = 0; /* skip the trailing '.c' */                                                 \
                                                                                                                       \
                printf("\033[31m|\033[000m Test %s failed. To run in gdb, use: \033[001mmake dbg TEST=%s"              \
                       TEST_FUNCTION_POSTFIX " EXE=%s\033[000m\n",                                                     \
                       tr_tcname(results[i]), tr_tcname(results[i]), buf + 10);                                        \
            }                                                                                                          \
                                                                                                                       \
            puts("\033[31m=================================================\033[000m");                                \
                                                                                                                       \
            free(results);                                                                                             \
        }                                                                                                              \
                                                                                                                       \
        srunner_free(sr);                                                                                              \
        vips_shutdown();                                                                                               \
        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;                                                     \
    }

#ifdef WITH_PRINT
#define test_print(...) printf(__VA_ARGS__)
#else
#define test_print(...)                                                                                                \
    do {                                                                                                               \
    } while (0)
#endif

#define start_test_print test_print("=== %s:\n", __func__)
#define end_test_print   test_print("=== END of %s:\n", __func__)

#define IMGFS(name) DATA_DIR name ".imgfs"

// 4096 is the maximum size of a path on Unix
#define DECLARE_DUMP_PREFIXED(prefix)                                                                                  \
    char dump##prefix[4096] = {0};                                                                                     \
    strcat(dump##prefix, DATA_DIR "dump-");                                                                            \
    strcat(dump##prefix, __func__);                                                                                    \
    strcat(dump##prefix, ".imgfs");                                                                                    \
    strcat(dump##prefix, #prefix)
#define DECLARE_DUMP DECLARE_DUMP_PREFIXED()
#define DUPLICATE_FILE(dst, src)                                                                                       \
    do {                                                                                                               \
        char command[8200] = {0};                                                                                      \
        strcat(command, "cp '");                                                                                       \
        strcat(command, src);                                                                                          \
        strcat(command, "' '");                                                                                        \
        strcat(command, dst);                                                                                          \
        strcat(command, "'");                                                                                          \
        system(command);                                                                                               \
    } while (0)

#define NON_NULL ((void *) 1)

static void read_file(void *buffer, const char *filename, size_t size)
{
    FILE *file = fopen(filename, "r");

    ck_assert_ptr_nonnull(file);
    ck_assert_uint_eq(fread(buffer, 1, size, file), size);

    fclose(file);
}

static void read_file_and_size(void **buffer, const char *filename, size_t* size)
{
    FILE *file = fopen(filename, "r");
    ck_assert_ptr_nonnull(file);

    ck_assert_int_eq(fseek(file, 0, SEEK_END), 0);
    *size = (size_t)ftell(file);
    rewind(file);

    *buffer = calloc(*size, 1);
    ck_assert_ptr_nonnull(buffer);

    ck_assert_uint_eq(fread(*buffer, 1, *size, file), *size);

    fclose(file);
}
static size_t locate_sos(char *buffer, size_t size) {
    for (size_t i = 0; i < size - 1; ++i) {
        if (buffer[i] == (char)0xff && buffer[i+1] == (char)0xda) {
            return i;
        }
    }

    return 0;
}