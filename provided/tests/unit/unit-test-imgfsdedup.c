#include "image_dedup.h"
#include "imgfs.h"
#include "test.h"
#include <check.h>

// ======================================================================
START_TEST(do_name_and_content_dedup_null_params)
{
    start_test_print;

    ck_assert_invalid_arg(do_name_and_content_dedup(NULL, 0));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_name_and_content_dedup_out_of_bounds)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file;
    DUPLICATE_FILE(dump, IMGFS("empty"));
    ck_assert_err_none(do_open(dump, "rb+", &file));

    ck_assert_err(do_name_and_content_dedup(&file, 10), ERR_IMAGE_NOT_FOUND);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_name_and_content_dedup_duplicate_id)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file;
    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));

    // Manually insert duplicate
    struct img_metadata *md = &file.metadata[2];
    strcpy(md->img_id, "pic1");
    md->is_valid = NON_EMPTY;

    ck_assert_err(do_name_and_content_dedup(&file, 2), ERR_DUPLICATE_ID);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_name_and_content_dedup_duplicate_id_empty)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file;
    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));

    // Set the pic1 metadata to empty
    file.metadata[0].is_valid = EMPTY;

    // Manually insert duplicate
    struct img_metadata *md = &file.metadata[2];
    strcpy(md->img_id, "pic1");
    md->is_valid = NON_EMPTY;

    ck_assert_err(do_name_and_content_dedup(&file, 2), ERR_NONE);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_name_and_content_dedup_duplicate_content)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file;
    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));

    // Manually insert duplicate
    struct img_metadata *orig_md = &file.metadata[0];
    struct img_metadata *md = &file.metadata[2];
    strcpy(md->img_id, "pic3");
    md->is_valid = NON_EMPTY;
    memcpy(md->SHA, orig_md->SHA, SHA256_DIGEST_LENGTH);
    memcpy(md->size, orig_md->size, sizeof(orig_md->size));

    ck_assert_err(do_name_and_content_dedup(&file, 2), ERR_NONE);

    ck_assert_int_eq(orig_md->offset[0], md->offset[0]);
    ck_assert_int_eq(orig_md->offset[1], md->offset[1]);
    ck_assert_int_eq(orig_md->offset[2], md->offset[2]);
    ck_assert_int_eq(orig_md->size[0], md->size[0]);
    ck_assert_int_eq(orig_md->size[1], md->size[1]);
    ck_assert_int_eq(orig_md->size[2], md->size[2]);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_name_and_content_dedup_duplicate_content_empty)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file;
    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));

    // Set the pic1 metadata to empty
    file.metadata[0].is_valid = EMPTY;

    // Manually insert duplicate
    unsigned char sha[] = {0x66, 0xac, 0x64, 0x8b, 0x32, 0xa8, 0x26, 0x8e, 0xd0, 0xb3, 0x50,
                           0xb1, 0x84, 0xcf, 0xa0, 0x4c, 0x00, 0xc6, 0x23, 0x6a, 0xf3, 0xa2,
                           0xaa, 0x44, 0x11, 0xc0, 0x15, 0x18, 0xf6, 0x06, 0x1a, 0xf8
                          };
    struct img_metadata *md = &file.metadata[2];
    strcpy(md->img_id, "pic3");
    md->is_valid = NON_EMPTY;
    memcpy(md->SHA, sha, SHA_DIGEST_LENGTH);
    md->offset[0] = 1234;
    md->offset[1] = 1234;
    md->offset[2] = 1234;

    ck_assert_err(do_name_and_content_dedup(&file, 2), ERR_NONE);

    ck_assert_int_eq(md->offset[ORIG_RES], 0);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_name_and_content_dedup_no_duplicate)
{
    start_test_print;
    DECLARE_DUMP;

    struct imgfs_file file;
    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));

    // Manually insert entry
    struct img_metadata *md = &file.metadata[2];
    strcpy(md->img_id, "pic3");
    md->is_valid = NON_EMPTY;

    md->offset[ORIG_RES] = 1234;
    md->offset[SMALL_RES] = 1235;
    md->offset[THUMB_RES] = 1236;

    ck_assert_err(do_name_and_content_dedup(&file, 2), ERR_NONE);
    ck_assert_int_eq(md->offset[ORIG_RES], 0);
    ck_assert_int_eq(md->offset[SMALL_RES], 1235);
    ck_assert_int_eq(md->offset[THUMB_RES], 1236);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
Suite *imgfs_dedup_suite()
{
    Suite *s = suite_create("Tests for do_name_and_content_dedup implementation");

    Add_Test(s, do_name_and_content_dedup_null_params);
    Add_Test(s, do_name_and_content_dedup_out_of_bounds);
    Add_Test(s, do_name_and_content_dedup_duplicate_id);
    Add_Test(s, do_name_and_content_dedup_duplicate_id_empty);
    Add_Test(s, do_name_and_content_dedup_duplicate_content);
    Add_Test(s, do_name_and_content_dedup_no_duplicate);
    Add_Test(s, do_name_and_content_dedup_duplicate_content_empty);

    return s;
}

TEST_SUITE(imgfs_dedup_suite)
