#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "test.h"
#include "util.h"
#include <check.h>

START_TEST(do_open_null_params)
{
    start_test_print;

    const char *filename = "asdf";
    const char *mode = "rb";
    struct imgfs_file file;

    ck_assert_invalid_arg(do_open(NULL, mode, &file));
    ck_assert_invalid_arg(do_open(filename, NULL, &file));
    ck_assert_invalid_arg(do_open(filename, mode, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_open_inexistant_file)
{
    start_test_print;

    struct imgfs_file file;
    ck_assert_err(do_open("not a file", "rb", &file), ERR_IO);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_open_invalid_mode)
{
    start_test_print;

    struct imgfs_file file;
    ck_assert_err(do_open(DATA_DIR "empty.imgfs", "not a mode", &file), ERR_IO);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_open_correct_header)
{
    start_test_print;

    struct imgfs_file file;
    ck_assert_err_none(do_open(DATA_DIR "empty.imgfs", "rb", &file));

    ck_assert_str_eq(file.header.name, "EPFL ImgFS 2024");
    ck_assert_int_eq(file.header.version, 0);
    ck_assert_int_eq(file.header.nb_files, 0);
    ck_assert_int_eq(file.header.resized_res[0], 64);
    ck_assert_int_eq(file.header.resized_res[1], 64);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_open_correct_metadata)
{
    start_test_print;

    struct imgfs_file file;
    ck_assert_err_none(do_open(DATA_DIR "test02.imgfs", "rb", &file));

    unsigned char pic1_sha[SHA256_DIGEST_LENGTH] = {0x66, 0xac, 0x64, 0x8b, 0x32, 0xa8, 0x26, 0x8e, 0xd0, 0xb3, 0x50,
                                                    0xb1, 0x84, 0xcf, 0xa0, 0x4c, 0x00, 0xc6, 0x23, 0x6a, 0xf3, 0xa2,
                                                    0xaa, 0x44, 0x11, 0xc0, 0x15, 0x18, 0xf6, 0x06, 0x1a, 0xf8
                                                   };
    unsigned char pic2_sha[SHA256_DIGEST_LENGTH] = {0x95, 0x96, 0x2b, 0x09, 0xe0, 0xfc, 0x97, 0x16, 0xee, 0x4c, 0x2a,
                                                    0x1c, 0xf1, 0x73, 0xf9, 0x14, 0x77, 0x58, 0x23, 0x53, 0x60, 0xd7,
                                                    0xac, 0x0a, 0x73, 0xdf, 0xa3, 0x78, 0x85, 0x8b, 0x8a, 0x10
                                                   };

    ck_assert_str_eq(file.metadata[0].img_id, "pic1");
    ck_assert_mem_eq(file.metadata[0].SHA, pic1_sha, SHA256_DIGEST_LENGTH);
    ck_assert_int_eq(file.metadata[0].orig_res[0], 1200);
    ck_assert_int_eq(file.metadata[0].orig_res[1], 800);
    ck_assert_int_eq(file.metadata[0].size[ORIG_RES], 72876);
    ck_assert_int_eq(file.metadata[0].size[THUMB_RES], 0);
    ck_assert_int_eq(file.metadata[0].size[SMALL_RES], 0);
    ck_assert_int_eq(file.metadata[0].offset[ORIG_RES], 21664);
    ck_assert_int_eq(file.metadata[0].offset[THUMB_RES], 0);
    ck_assert_int_eq(file.metadata[0].offset[SMALL_RES], 0);
    ck_assert_int_eq(file.metadata[0].is_valid, NON_EMPTY);

    ck_assert_str_eq(file.metadata[1].img_id, "pic2");
    ck_assert_mem_eq(file.metadata[1].SHA, pic2_sha, SHA256_DIGEST_LENGTH);
    ck_assert_int_eq(file.metadata[1].orig_res[0], 1200);
    ck_assert_int_eq(file.metadata[1].orig_res[1], 800);
    ck_assert_int_eq(file.metadata[1].size[ORIG_RES], 98119);
    ck_assert_int_eq(file.metadata[1].size[THUMB_RES], 0);
    ck_assert_int_eq(file.metadata[1].size[SMALL_RES], 0);
    ck_assert_int_eq(file.metadata[1].offset[ORIG_RES], 94540);
    ck_assert_int_eq(file.metadata[1].offset[THUMB_RES], 0);
    ck_assert_int_eq(file.metadata[1].offset[SMALL_RES], 0);
    ck_assert_int_eq(file.metadata[1].is_valid, NON_EMPTY);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_close_null_param)
{
    start_test_print;

    do_close(NULL);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_close_null_file)
{
    start_test_print;

    struct imgfs_file file;
    file.file = NULL;
    file.metadata = malloc(sizeof(struct img_metadata));

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
Suite *imgfs_tools_suite_RES()
{
    Suite *s = suite_create("Tests for ImgFS do_open and do_close");

    Add_Test(s, do_open_null_params);
    Add_Test(s, do_open_inexistant_file);
    Add_Test(s, do_open_invalid_mode);
    Add_Test(s, do_open_correct_header);
    Add_Test(s, do_open_correct_metadata);

    Add_Test(s, do_close_null_param);
    Add_Test(s, do_close_null_file);

    return s;
}

TEST_SUITE(imgfs_tools_suite_RES)
