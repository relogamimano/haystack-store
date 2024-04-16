#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "test.h"
#include <check.h>

// ======================================================================
START_TEST(do_delete_null_params)
{
    start_test_print;

    struct imgfs_file file;
    const char *str = "/eaccess";
    ck_assert_invalid_arg(do_delete(NULL, &file));
    ck_assert_invalid_arg(do_delete(str, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_delete_image_not_found)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file;
    DUPLICATE_FILE(dump, IMGFS("empty"));
    ck_assert_err_none(do_open(dump, "rb+", &file));

    ck_assert_err(do_delete("myimage", &file), ERR_IMAGE_NOT_FOUND);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_delete_read_only)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file;
    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb", &file));

    ck_assert_err(do_delete("pic1", &file), ERR_IO);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_delete_correct)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file;
    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));

    ck_assert_err_none(do_delete("pic1", &file));
    ck_assert_int_eq(file.metadata[0].is_valid, EMPTY);
    ck_assert_int_eq(file.header.version, 3);
    ck_assert_int_eq(file.header.nb_files, 1);

    do_close(&file);

    ck_assert_err_none(do_open(dump, "rb", &file));
    ck_assert_int_eq(file.metadata[0].is_valid, EMPTY);
    ck_assert_int_eq(file.header.version, 3);
    ck_assert_int_eq(file.header.nb_files, 1);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_delete_bad_open_mode)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file;
    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb", &file));

    ck_assert_err(do_delete("pic1", &file), ERR_IO);
    ck_assert_int_eq(file.header.version, 2);
    ck_assert_int_eq(file.header.nb_files, 2);

    do_close(&file);

    ck_assert_err_none(do_open(dump, "rb", &file));
    ck_assert_int_eq(file.metadata[0].is_valid, NON_EMPTY);
    ck_assert_int_eq(file.header.version, 2);
    ck_assert_int_eq(file.header.nb_files, 2);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_delete_cmd_null_params)
{
    start_test_print;

    ck_assert_invalid_arg(do_delete_cmd(0, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_delete_cmd_not_enough_arguments)
{
    start_test_print;
    DECLARE_DUMP;
    DUPLICATE_FILE(dump, IMGFS("empty"));

    char *argv[] = {dump};
    ck_assert_err(do_delete_cmd(1, argv), ERR_NOT_ENOUGH_ARGUMENTS);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_delete_cmd_image_not_found)
{
    start_test_print;
    DECLARE_DUMP;
    DUPLICATE_FILE(dump, IMGFS("empty"));

    char *argv[] = {dump, "pic1"};
    ck_assert_err(do_delete_cmd(2, argv), ERR_IMAGE_NOT_FOUND);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_delete_cmd_correct)
{
    start_test_print;
    DECLARE_DUMP;
    DUPLICATE_FILE(dump, IMGFS("test02"));

    char *argv[] = {dump, "pic1"};
    ck_assert_err_none(do_delete_cmd(2, argv));

    struct imgfs_file file;
    ck_assert_err_none(do_open(dump, "rb", &file));
    ck_assert_int_eq(file.metadata[0].is_valid, EMPTY);
    ck_assert_int_eq(file.header.version, 3);
    ck_assert_int_eq(file.header.nb_files, 1);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
Suite *imgfs_do_delete_test_suite()
{
    Suite *s = suite_create("Tests for do_delete implementation");

    Add_Test(s, do_delete_null_params);
    Add_Test(s, do_delete_image_not_found);
    Add_Test(s, do_delete_read_only);
    Add_Test(s, do_delete_cmd_not_enough_arguments);
    Add_Test(s, do_delete_correct);
    Add_Test(s, do_delete_bad_open_mode);
    Add_Test(s, do_delete_cmd_null_params);
    Add_Test(s, do_delete_cmd_image_not_found);
    Add_Test(s, do_delete_cmd_correct);

    return s;
}

TEST_SUITE(imgfs_do_delete_test_suite)
