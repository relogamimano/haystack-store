#include "image_content.h"
#include "imgfs.h"
#include "test.h"
#include <check.h>
#include <vips/vips.h>

#if VIPS_MINOR_VERSION >= 15
// these are the values we got in 8.15.1
#define SMALL_RES_SIZE_A  16296
#define FILE_SIZE_A      208955
#else
// these are the values we got in 8.12.1
#define SMALL_RES_SIZE_A  16299
#define FILE_SIZE_A      208958
#endif

// ======================================================================
START_TEST(lazily_resize_null_params)
{
    start_test_print;

    ck_assert_invalid_arg(lazily_resize(ORIG_RES, NULL, 0));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(lazily_resize_out_of_range_index)
{
    start_test_print;
    DECLARE_DUMP;
    DUPLICATE_FILE(dump, IMGFS("test02"));

    struct imgfs_file file;
    ck_assert_err_none(do_open(dump, "rb+", &file));

    ck_assert_err(lazily_resize(ORIG_RES, &file, 218), ERR_INVALID_IMGID);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(lazily_resize_empty_index)
{
    start_test_print;
    DECLARE_DUMP;
    DUPLICATE_FILE(dump, IMGFS("test02"));

    struct imgfs_file file;

    ck_assert_err_none(do_open(dump, "rb+", &file));

    ck_assert_err(lazily_resize(ORIG_RES, &file, 3), ERR_INVALID_IMGID);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(lazily_resize_res_orig)
{
    start_test_print;
    DECLARE_DUMP;
    DUPLICATE_FILE(dump, IMGFS("test02"));

    long file_size;
    struct imgfs_file file;

    ck_assert_err_none(do_open(dump, "rb+", &file));

    ck_assert_err_none(lazily_resize(ORIG_RES, &file, 0));

    ck_assert_int_eq(fseek(file.file, 0, SEEK_END), 0);
    file_size = ftell(file.file);

    ck_assert_int_eq(file_size, 192659);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(lazily_resize_invalid_mode)
{
    start_test_print;
    DECLARE_DUMP;
    DUPLICATE_FILE(dump, IMGFS("test02"));

    struct imgfs_file file;
    ck_assert_err_none(do_open(dump, "rb", &file));

    ck_assert_err(lazily_resize(THUMB_RES, &file, 0), ERR_IO);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(lazily_resize_already_exists)
{
    start_test_print;
    DECLARE_DUMP;
    DUPLICATE_FILE(dump, IMGFS("test03"));
    long file_size;
    struct imgfs_file file;

    ck_assert_err_none(do_open(dump, "rb+", &file));

    ck_assert_err_none(lazily_resize(THUMB_RES, &file, 1));

    ck_assert_int_eq(fseek(file.file, 0, SEEK_END), 0);
    file_size = ftell(file.file);

    ck_assert_uint_eq(file_size, 250754);
    ck_assert_uint_eq(file.metadata[1].offset[THUMB_RES], 221108);
    ck_assert_uint_eq(file.metadata[1].size[THUMB_RES], 12319);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(lazily_resize_valid)
{
    start_test_print;
    DECLARE_DUMP;
    DUPLICATE_FILE(dump, IMGFS("test02"));

    FILE *reference;
    char reference_buffer[SMALL_RES_SIZE_A], buffer[SMALL_RES_SIZE_A];
    long file_size;
    struct imgfs_file file;

#define FILENAME DATA_DIR "papillon256_256-" VIPS_VERSION ".jpg"
    reference = fopen(FILENAME, "rb");
    ck_assert_msg(reference != NULL, "cannot open file \"" FILENAME "\"\n"
                  "Please check your VIPS version with:\n  vips --version\n"
                  "(on the command line) and maybe let us know (not guaranted.\n"
                  "  Currently supported versions are:\n   - 8.12.1 (on Ubuntu 22.04; incl. EPFL VMs); and\n"
                  "   - 8.15.1 (latest stable version when the project was developped).\n)");

    ck_assert_int_eq(fread(reference_buffer, 1, SMALL_RES_SIZE_A, reference), SMALL_RES_SIZE_A);

    ck_assert_err_none(do_open(dump, "rb+", &file));

    ck_assert_err_none(lazily_resize(SMALL_RES, &file, 0));

    ck_assert_int_eq(fseek(file.file, 0, SEEK_END), 0);
    file_size = ftell(file.file);

    ck_assert_uint_eq(file_size, FILE_SIZE_A);

    ck_assert_uint_eq(file.metadata[0].offset[SMALL_RES], 192659);
    ck_assert_uint_eq(file.metadata[0].size[SMALL_RES]  , SMALL_RES_SIZE_A);

    ck_assert_int_eq(fseek(file.file, 192659, SEEK_SET), 0);
    ck_assert_int_eq(fread(buffer, 1, SMALL_RES_SIZE_A, file.file), SMALL_RES_SIZE_A);
    ck_assert_mem_eq(reference_buffer, buffer, SMALL_RES_SIZE_A);

    fclose(reference);
    do_close(&file);

    // Checks that metadata is correctly persisted
    ck_assert_err_none(do_open(dump, "rb+", &file));

    ck_assert_int_eq(fseek(file.file, 0, SEEK_END), 0);
    file_size = ftell(file.file);

    ck_assert_uint_eq(file_size, FILE_SIZE_A);
    ck_assert_uint_eq(file.metadata[0].offset[SMALL_RES], 192659);
    ck_assert_uint_eq(file.metadata[0].size[SMALL_RES], SMALL_RES_SIZE_A);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
Suite *imgfs_content_test_suite()
{
    Suite *s = suite_create("Tests imgfs_content implementation");

    Add_Test(s, lazily_resize_null_params);
    Add_Test(s, lazily_resize_out_of_range_index);
    Add_Test(s, lazily_resize_empty_index);
    Add_Test(s, lazily_resize_res_orig);
    Add_Test(s, lazily_resize_invalid_mode);
    Add_Test(s, lazily_resize_already_exists);
    Add_Test(s, lazily_resize_valid);

    return s;
}

TEST_SUITE_VIPS(imgfs_content_test_suite)
