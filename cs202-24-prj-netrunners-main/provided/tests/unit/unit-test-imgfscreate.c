#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "test.h"
#include <check.h>

// ======================================================================
START_TEST(do_create_null_params)
{
    start_test_print;

    DECLARE_DUMP;
    struct imgfs_file file;

    ck_assert_invalid_arg(do_create(NULL, &file));
    ck_assert_invalid_arg(do_create(dump, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_create_correct)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file = { .header.max_files = 10,
                               .header.resized_res = { 32, 32, 32, 32 } };

    ck_assert_err_none(do_create(dump, &file));

    ck_assert_ptr_nonnull(file.metadata);
    ck_assert_ptr_nonnull(file.file);

    ck_assert_int_eq(file.header.max_files, 10);
    ck_assert_int_eq(file.header.version, 0);
    ck_assert_str_eq(file.header.name, CAT_TXT);
    ck_assert_int_eq(file.header.nb_files, 0);
    ck_assert_int_eq(file.header.resized_res[0], 32);
    ck_assert_int_eq(file.header.resized_res[1], 32);
    ck_assert_int_eq(file.header.resized_res[2], 32);
    ck_assert_int_eq(file.header.resized_res[3], 32);

    do_close(&file);

    ck_assert_err_none(do_open(dump, "rb", &file));
    ck_assert_int_eq(file.header.max_files, 10);
    ck_assert_int_eq(file.header.version, 0);
    ck_assert_str_eq(file.header.name, CAT_TXT);
    ck_assert_int_eq(file.header.nb_files, 0);
    ck_assert_int_eq(file.header.resized_res[0], 32);
    ck_assert_int_eq(file.header.resized_res[1], 32);
    ck_assert_int_eq(file.header.resized_res[2], 32);
    ck_assert_int_eq(file.header.resized_res[3], 32);

    struct img_metadata empty_metadata = {0};
    for (size_t i = 0; i < 10; ++i) {
        ck_assert_mem_eq(&file.metadata[i], &empty_metadata, sizeof(empty_metadata));
    }

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_create_cmd_null_params)
{
    start_test_print;

    ck_assert_invalid_arg(do_create_cmd(0, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_create_cmd_no_filename)
{
    start_test_print;

    char *argv[] = {};
    ck_assert_err(do_create_cmd(0, argv), ERR_NOT_ENOUGH_ARGUMENTS);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_create_cmd_invalid_flag)
{
    start_test_print;
    DECLARE_DUMP;

    char *argv[] = {dump, "-invalid_flag", "13"};
    ck_assert_invalid_arg(do_create_cmd(3, argv));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_create_cmd_not_enough_flag_params)
{
    start_test_print;
    DECLARE_DUMP_PREFIXED(1);
    DECLARE_DUMP_PREFIXED(2);
    DECLARE_DUMP_PREFIXED(3);

    char *argv1[] = {dump1, "-max_files"};
    ck_assert_err(do_create_cmd(2, argv1), ERR_NOT_ENOUGH_ARGUMENTS);

    char *argv2[] = {dump2, "-thumb_res",};
    ck_assert_err(do_create_cmd(2, argv2), ERR_NOT_ENOUGH_ARGUMENTS);

    char *argv3[] = {dump2, "-small_res",};
    ck_assert_err(do_create_cmd(2, argv3), ERR_NOT_ENOUGH_ARGUMENTS);

    char *argv4[] = {dump2, "-thumb_res", "32"};
    ck_assert_err(do_create_cmd(3, argv4), ERR_NOT_ENOUGH_ARGUMENTS);

    char *argv5[] = {dump2, "-small_res", "32"};
    ck_assert_err(do_create_cmd(3, argv5), ERR_NOT_ENOUGH_ARGUMENTS);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_create_cmd_res_too_big)
{
    start_test_print;
    DECLARE_DUMP_PREFIXED(1);
    DECLARE_DUMP_PREFIXED(2);

    char *argv1[] = {dump1, "-thumb_res", "256", "32"};
    ck_assert_err(do_create_cmd(4, argv1), ERR_RESOLUTIONS);

    char *argv2[] = {dump2, "-small_res", "1024", "1024"};
    ck_assert_err(do_create_cmd(4, argv2), ERR_RESOLUTIONS);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_create_cmd_no_flags)
{
    start_test_print;
    DECLARE_DUMP;

    char *argv[] = {dump};
    ck_assert_err_none(do_create_cmd(1, argv));

    struct imgfs_file file;
    ck_assert_err_none(do_open(argv[0], "rb", &file));

    ck_assert_int_eq(file.header.max_files, 128);
    ck_assert_int_eq(file.header.version, 0);
    ck_assert_str_eq(file.header.name, CAT_TXT);
    ck_assert_int_eq(file.header.nb_files, 0);
    ck_assert_int_eq(file.header.resized_res[2 * THUMB_RES], 64);
    ck_assert_int_eq(file.header.resized_res[2 * THUMB_RES + 1], 64);
    ck_assert_int_eq(file.header.resized_res[2 * SMALL_RES], 256);
    ck_assert_int_eq(file.header.resized_res[2 * SMALL_RES + 1], 256);

    struct img_metadata empty_metadata = {0};
    for (size_t i = 0; i < 128; ++i) {
        ck_assert_mem_eq(&file.metadata[i], &empty_metadata, sizeof(empty_metadata));
    }

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_create_cmd_all_flags)
{
    start_test_print;
    DECLARE_DUMP;

    char *argv[] = {dump, "-max_files", "10", "-thumb_res", "32", "32", "-small_res", "64", "64"};
    ck_assert_err_none(do_create_cmd(9, argv));

    struct imgfs_file file;
    ck_assert_err_none(do_open(argv[0], "rb", &file));

    ck_assert_int_eq(file.header.max_files, 10);
    ck_assert_int_eq(file.header.version, 0);
    ck_assert_str_eq(file.header.name, CAT_TXT);
    ck_assert_int_eq(file.header.nb_files, 0);
    ck_assert_int_eq(file.header.resized_res[2 * THUMB_RES], 32);
    ck_assert_int_eq(file.header.resized_res[2 * THUMB_RES + 1], 32);
    ck_assert_int_eq(file.header.resized_res[2 * SMALL_RES], 64);
    ck_assert_int_eq(file.header.resized_res[2 * SMALL_RES + 1], 64);

    struct img_metadata empty_metadata = {0};
    for (size_t i = 0; i < 10; ++i) {
        ck_assert_mem_eq(&file.metadata[i], &empty_metadata, sizeof(empty_metadata));
    }

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_create_cmd_repeating_flags)
{
    start_test_print;
    DECLARE_DUMP;

    char *argv[] = {dump, "-max_files", "10", "-max_files", "16",         "-thumb_res", "32",
                    "32", "-small_res", "64", "64",         "-small_res", "128",        "128"
                   };
    ck_assert_err_none(do_create_cmd(14, argv));

    struct imgfs_file file;
    ck_assert_err_none(do_open(argv[0], "rb", &file));

    ck_assert_int_eq(file.header.max_files, 16);
    ck_assert_int_eq(file.header.version, 0);
    ck_assert_str_eq(file.header.name, CAT_TXT);
    ck_assert_int_eq(file.header.nb_files, 0);
    ck_assert_int_eq(file.header.resized_res[2 * THUMB_RES], 32);
    ck_assert_int_eq(file.header.resized_res[2 * THUMB_RES + 1], 32);
    ck_assert_int_eq(file.header.resized_res[2 * SMALL_RES], 128);
    ck_assert_int_eq(file.header.resized_res[2 * SMALL_RES + 1], 128);

    struct img_metadata empty_metadata = {0};
    for (size_t i = 0; i < 16; ++i) {
        ck_assert_mem_eq(&file.metadata[i], &empty_metadata, sizeof(empty_metadata));
    }

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_create_cmd_ignores_irrelevant_fields)
{
    start_test_print;
    DECLARE_DUMP;

    char *argv[] = {dump};
    ck_assert_err_none(do_create_cmd(1, argv));

    struct imgfs_file file;
    file.header.version = 12347;
    file.header.nb_files = 3;
    ck_assert_err_none(do_open(argv[0], "rb", &file));

    ck_assert_int_eq(file.header.max_files, 128);
    ck_assert_int_eq(file.header.version, 0);
    ck_assert_str_eq(file.header.name, CAT_TXT);
    ck_assert_int_eq(file.header.nb_files, 0);
    ck_assert_int_eq(file.header.resized_res[2 * THUMB_RES], 64);
    ck_assert_int_eq(file.header.resized_res[2 * THUMB_RES + 1], 64);
    ck_assert_int_eq(file.header.resized_res[2 * SMALL_RES], 256);
    ck_assert_int_eq(file.header.resized_res[2 * SMALL_RES + 1], 256);

    struct img_metadata empty_metadata = {0};
    for (size_t i = 0; i < 128; ++i) {
        ck_assert_mem_eq(&file.metadata[i], &empty_metadata, sizeof(empty_metadata));
    }

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
Suite *imgfs_do_create_test_suite()
{
    Suite *s = suite_create("Tests for do_create & do_create_cmd implementation");

    Add_Test(s, do_create_null_params);
    Add_Test(s, do_create_correct);

    Add_Test(s, do_create_cmd_null_params);
    Add_Test(s, do_create_cmd_invalid_flag);
    Add_Test(s, do_create_cmd_not_enough_flag_params);
    Add_Test(s, do_create_cmd_no_filename);
    Add_Test(s, do_create_cmd_res_too_big);
    Add_Test(s, do_create_cmd_no_flags);
    Add_Test(s, do_create_cmd_all_flags);
    Add_Test(s, do_create_cmd_repeating_flags);
    Add_Test(s, do_create_cmd_ignores_irrelevant_fields);

    return s;
}

TEST_SUITE(imgfs_do_create_test_suite)
