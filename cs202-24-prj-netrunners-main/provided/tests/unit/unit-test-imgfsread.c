#include "image_content.h"
#include "imgfs.h"
#include "test.h"
#include <check.h>
#include <vips/vips.h>

#if VIPS_MINOR_VERSION >= 15
// these are the values we got in 8.15.1
#define SMALL_RES_SIZE_A  16296
#else
// these are the values we got in 8.12.1
#define SMALL_RES_SIZE_A  16299
#endif

// ======================================================================
START_TEST(do_read_null_params)
{
    start_test_print;

    char id;
    char *output_buffer;
    uint32_t size;
    struct imgfs_file file;

    ck_assert_invalid_arg(do_read(NULL, ORIG_RES, &output_buffer, &size, &file));
    ck_assert_invalid_arg(do_read(&id, ORIG_RES, NULL, &size, &file));
    ck_assert_invalid_arg(do_read(&id, ORIG_RES, &output_buffer, NULL, &file));
    ck_assert_invalid_arg(do_read(&id, ORIG_RES, &output_buffer, &size, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_read_not_found)
{
    start_test_print;

    struct imgfs_file file;
    char *buffer;
    uint32_t size;

    ck_assert_err_none(do_open(IMGFS("empty"), "rb", &file));
    ck_assert_err(do_read("pic", ORIG_RES, &buffer, &size, &file), ERR_IMAGE_NOT_FOUND);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_read_valid)
{
    start_test_print;

    struct imgfs_file file;
    char expected_buffer[72876];
    char *buffer;
    uint32_t size;

    read_file(expected_buffer, DATA_DIR "/papillon.jpg", 72876);
    ck_assert_err_none(do_open(IMGFS("test02"), "rb", &file));

    ck_assert_err_none(do_read("pic1", ORIG_RES, &buffer, &size, &file));

    ck_assert_int_eq(size, 72876);
    ck_assert_mem_eq(expected_buffer, buffer, 72876);

    free(buffer);
    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_read_resize)
{
    start_test_print;

    DECLARE_DUMP;
    struct imgfs_file file;
    char expected_buffer[SMALL_RES_SIZE_A];
    uint32_t size;

    DUPLICATE_FILE(dump, IMGFS("test02"));

#define FILENAME DATA_DIR "papillon256_256-" VIPS_VERSION ".jpg"
    FILE *imgfile = fopen(FILENAME, "rb");
    ck_assert_msg(imgfile != NULL, "cannot open file \"" FILENAME "\"\n"
                  "Please check your VIPS version with:\n  vips --version\n"
                  "(on the command line) and maybe let us know (not guaranted.\n"
                  "  Currently supported versions are:\n   - 8.12.1 (on Ubuntu 22.04; incl. EPFL VMs); and\n"
                  "   - 8.15.1 (latest stable version when the project was developped).\n)");

    ck_assert_uint_eq(fread(expected_buffer, 1, SMALL_RES_SIZE_A, imgfile), SMALL_RES_SIZE_A);
    fclose(imgfile);

    ck_assert_err_none(do_open(dump, "rb+", &file));

    char *buffer = {0};
    ck_assert_err_none(do_read("pic1", SMALL_RES, &buffer, &size, &file));

    ck_assert_int_eq(size, SMALL_RES_SIZE_A);
    ck_assert_mem_eq(expected_buffer, buffer, SMALL_RES_SIZE_A);

    free(buffer);
    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_read_resize_invalid_mode)
{
    start_test_print;

    DECLARE_DUMP;
    struct imgfs_file file;
    char *buffer;
    uint32_t size;

    DUPLICATE_FILE(dump, IMGFS("test02"));

    ck_assert_err_none(do_open(dump, "rb", &file));

    ck_assert_err(do_read("pic1", SMALL_RES, &buffer, &size, &file), ERR_IO);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
Suite *imgfs_read_test_suite()
{
    Suite *s = suite_create("Tests do_read implementation");

    Add_Test(s, do_read_null_params);
    Add_Test(s, do_read_not_found);
    Add_Test(s, do_read_valid);
    Add_Test(s, do_read_resize);
    Add_Test(s, do_read_resize_invalid_mode);

    return s;
}

TEST_SUITE_VIPS(imgfs_read_test_suite)
