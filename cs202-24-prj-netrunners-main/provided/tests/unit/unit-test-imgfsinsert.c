#include "imgfs.h"
#include "test.h"
#include <check.h>
#include <string.h>
#include <vips/vips.h>

// ======================================================================
START_TEST(do_insert_null_params)
{
    start_test_print;

    char image_buffer, img_id;
    struct imgfs_file file;

    ck_assert_invalid_arg(do_insert(NULL, 0, &img_id, &file));
    ck_assert_invalid_arg(do_insert(&image_buffer, 0, NULL, &file));
    ck_assert_invalid_arg(do_insert(&image_buffer, 0, &img_id, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_insert_full)
{
    start_test_print;

    DECLARE_DUMP;
    char image[72876];
    struct imgfs_file file;

    DUPLICATE_FILE(dump, IMGFS("full"));
    ck_assert_err_none(do_open(dump, "rb+", &file));
    read_file(image, DATA_DIR "/papillon.jpg", 72876);

    ck_assert_err(do_insert(image, 72876, "pic", &file), ERR_IMGFS_FULL);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_insert_duplicate_id)
{
    start_test_print;

    DECLARE_DUMP;
    char image[72876];
    struct imgfs_file file;

    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));
    read_file(image, DATA_DIR "/papillon.jpg", 72876);

    ck_assert_err(do_insert(image, 72876, "pic1", &file), ERR_DUPLICATE_ID);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_insert_invalid_image)
{
    start_test_print;

    DECLARE_DUMP;
    char image[72876] = {0};
    struct imgfs_file file;

    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));

    ck_assert_err(do_insert(image, 72876, "pic42", &file), ERR_IMGLIB);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_insert_invalid_file_mode)
{
    start_test_print;

    DECLARE_DUMP;
    char image[72876];
    struct imgfs_file file;

    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb", &file));
    read_file(image, DATA_DIR "/papillon.jpg", 72876);

    ck_assert_err(do_insert(image, 72876, "pic3", &file), ERR_IO);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_insert_duplicate)
{
    start_test_print;

    DECLARE_DUMP;
    char image[72876];
    struct imgfs_file file;

    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));
    read_file(image, DATA_DIR "/papillon.jpg", 72876);

    ck_assert_err_none(do_insert(image, 72876, "pic3", &file));

    const struct img_metadata *md = NULL;
    for (uint32_t i = 0; i < file.header.max_files; ++i) {
        if (strcmp(file.metadata[i].img_id, "pic3") == 0) {
            md = &file.metadata[i];
            break;
        }
    }
    ck_assert_msg(md != NULL, "the inserted metadata could not be found by image id");

    unsigned char pic_sha[SHA256_DIGEST_LENGTH] = {0x66, 0xac, 0x64, 0x8b, 0x32, 0xa8, 0x26, 0x8e, 0xd0, 0xb3, 0x50,
                                                   0xb1, 0x84, 0xcf, 0xa0, 0x4c, 0x00, 0xc6, 0x23, 0x6a, 0xf3, 0xa2,
                                                   0xaa, 0x44, 0x11, 0xc0, 0x15, 0x18, 0xf6, 0x06, 0x1a, 0xf8
                                                  };
    ck_assert_mem_eq(md->SHA, pic_sha, SHA256_DIGEST_LENGTH);
    ck_assert_int_eq(md->orig_res[0], 1200);
    ck_assert_int_eq(md->orig_res[1], 800);
    ck_assert_int_eq(md->size[ORIG_RES], 72876);
    ck_assert_int_eq(md->size[THUMB_RES], 0);
    ck_assert_int_eq(md->size[SMALL_RES], 0);
    ck_assert_int_eq(md->offset[ORIG_RES], 21664);
    ck_assert_int_eq(md->offset[THUMB_RES], 0);
    ck_assert_int_eq(md->offset[SMALL_RES], 0);
    ck_assert_int_eq(md->is_valid, NON_EMPTY);

    ck_assert_int_eq(file.header.version, 3);
    ck_assert_int_eq(file.header.nb_files, 3);

    do_close(&file);

    // Checks that the metadata and headers are persisted
    ck_assert_err_none(do_open(dump, "rb+", &file));
    md = NULL;
    for (uint32_t i = 0; i < file.header.max_files; ++i) {
        if (strcmp(file.metadata[i].img_id, "pic3") == 0) {
            md = &file.metadata[i];
            break;
        }
    }
    ck_assert_msg(md != NULL, "the inserted metadata could not be found by image id");

    ck_assert_mem_eq(md->SHA, pic_sha, SHA256_DIGEST_LENGTH);
    ck_assert_int_eq(md->orig_res[0], 1200);
    ck_assert_int_eq(md->orig_res[1], 800);
    ck_assert_int_eq(md->size[ORIG_RES], 72876);
    ck_assert_int_eq(md->size[THUMB_RES], 0);
    ck_assert_int_eq(md->size[SMALL_RES], 0);
    ck_assert_int_eq(md->offset[ORIG_RES], 21664);
    ck_assert_int_eq(md->offset[THUMB_RES], 0);
    ck_assert_int_eq(md->offset[SMALL_RES], 0);
    ck_assert_int_eq(md->is_valid, NON_EMPTY);

    ck_assert_int_eq(file.header.version, 3);
    ck_assert_int_eq(file.header.nb_files, 3);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_insert_valid)
{
    start_test_print;

    DECLARE_DUMP;
    char image[82234];
    struct imgfs_file file;

    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));
    read_file(image, DATA_DIR "/brouillard.jpg", 82234);

    ck_assert_err_none(do_insert(image, 82234, "pic3", &file));

    const struct img_metadata *md = NULL;
    for (uint32_t i = 0; i < file.header.max_files; ++i) {
        if (strcmp(file.metadata[i].img_id, "pic3") == 0) {
            md = &file.metadata[i];
            break;
        }
    }
    ck_assert_msg(md != NULL, "the inserted metadata could not be found by image id");

    unsigned char pic_sha[SHA256_DIGEST_LENGTH] = {0xf8, 0x88, 0xf0, 0xdd, 0xd4, 0xf8, 0x24, 0x75, 0x99, 0xf6, 0xde,
                                                   0x79, 0x7e, 0x0a, 0x6f, 0x55, 0x76, 0xd3, 0xd1, 0xe7, 0x41, 0x97,
                                                   0xd3, 0x3d, 0xac, 0x09, 0x08, 0x94, 0xdb, 0x07, 0xbf, 0x1e
                                                  };
    ck_assert_mem_eq(md->SHA, pic_sha, SHA256_DIGEST_LENGTH);
    ck_assert_int_eq(md->orig_res[0], 600);
    ck_assert_int_eq(md->orig_res[1], 400);
    ck_assert_int_eq(md->size[ORIG_RES], 82234);
    ck_assert_int_eq(md->size[THUMB_RES], 0);
    ck_assert_int_eq(md->size[SMALL_RES], 0);
    ck_assert_int_eq(md->offset[ORIG_RES], 192659);
    ck_assert_int_eq(md->offset[THUMB_RES], 0);
    ck_assert_int_eq(md->offset[SMALL_RES], 0);
    ck_assert_int_eq(md->is_valid, NON_EMPTY);

    ck_assert_int_eq(file.header.version, 3);
    ck_assert_int_eq(file.header.nb_files, 3);

    do_close(&file);

    // Checks that the metadata and headers are persisted
    ck_assert_err_none(do_open(dump, "rb+", &file));
    md = NULL;
    for (uint32_t i = 0; i < file.header.max_files; ++i) {
        if (strcmp(file.metadata[i].img_id, "pic3") == 0) {
            md = &file.metadata[i];
            break;
        }
    }
    ck_assert_msg(md != NULL, "the inserted metadata could not be found by image id");

    ck_assert_mem_eq(md->SHA, pic_sha, SHA256_DIGEST_LENGTH);
    ck_assert_int_eq(md->orig_res[0], 600);
    ck_assert_int_eq(md->orig_res[1], 400);
    ck_assert_int_eq(md->size[ORIG_RES], 82234);
    ck_assert_int_eq(md->size[THUMB_RES], 0);
    ck_assert_int_eq(md->size[SMALL_RES], 0);
    ck_assert_int_eq(md->offset[ORIG_RES], 192659);
    ck_assert_int_eq(md->offset[THUMB_RES], 0);
    ck_assert_int_eq(md->offset[SMALL_RES], 0);
    ck_assert_int_eq(md->is_valid, NON_EMPTY);

    ck_assert_int_eq(file.header.version, 3);
    ck_assert_int_eq(file.header.nb_files, 3);

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_insert_write_correct_metadata)
{
    start_test_print;

    DECLARE_DUMP;
    char image[82234];
    struct imgfs_file file;

    DUPLICATE_FILE(dump, IMGFS("empty"));
    ck_assert_err_none(do_open(dump, "rb+", &file));
    read_file(image, DATA_DIR "/brouillard.jpg", 82234);

    for (uint32_t i = 0; i < file.header.max_files; ++i) {
        file.metadata[i].offset[THUMB_RES] = 1;
    }

    ck_assert_err_none(do_insert(image, 82234, "pic3", &file));

    do_close(&file);
    ck_assert_err_none(do_open(dump, "rb+", &file));

    for (uint32_t i = 0; i < file.header.max_files; ++i) {
        ck_assert_int_eq(file.metadata[i].offset[THUMB_RES], 0);
    }

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_insert_write_initializes_metadata)
{
    start_test_print;

    DECLARE_DUMP;
    char image[82234];
    struct imgfs_file file;

    DUPLICATE_FILE(dump, IMGFS("test02"));
    ck_assert_err_none(do_open(dump, "rb+", &file));
    read_file(image, DATA_DIR "/brouillard.jpg", 82234);

    for (uint32_t i = 0; i < file.header.max_files; ++i) {
        if(file.metadata[i].is_valid == EMPTY) {
            memset(&file.metadata[i], 0xffffffff, sizeof(file.metadata[i]));
            file.metadata[i].is_valid = EMPTY;
        }
    }

    ck_assert_err_none(do_insert(image, 82234, "pic3", &file));

    do_close(&file);
    ck_assert_err_none(do_open(dump, "rb+", &file));

    for (uint32_t i = 0; i < file.header.max_files; ++i) {
        if (strcmp(file.metadata[i].img_id, "pic3") == 0) {
            struct img_metadata* md = &file.metadata[i];
            printf("#### %s\n", md->img_id);

            unsigned char pic_sha[SHA256_DIGEST_LENGTH] = {0xf8, 0x88, 0xf0, 0xdd, 0xd4, 0xf8, 0x24, 0x75, 0x99, 0xf6, 0xde,
                                                        0x79, 0x7e, 0x0a, 0x6f, 0x55, 0x76, 0xd3, 0xd1, 0xe7, 0x41, 0x97,
                                                        0xd3, 0x3d, 0xac, 0x09, 0x08, 0x94, 0xdb, 0x07, 0xbf, 0x1e
                                                        };
            ck_assert_mem_eq(md->SHA, pic_sha, SHA256_DIGEST_LENGTH);
            ck_assert_int_eq(md->orig_res[0], 600);
            ck_assert_int_eq(md->orig_res[1], 400);
            ck_assert_int_eq(md->size[ORIG_RES], 82234);
            ck_assert_int_eq(md->size[THUMB_RES], 0);
            ck_assert_int_eq(md->size[SMALL_RES], 0);
            ck_assert_int_eq(md->offset[ORIG_RES], 192659);
            ck_assert_int_eq(md->offset[THUMB_RES], 0);
            ck_assert_int_eq(md->offset[SMALL_RES], 0);
            ck_assert_int_eq(md->is_valid, NON_EMPTY);
        }
    }

    do_close(&file);

    end_test_print;
}
END_TEST

// ======================================================================
Suite *imgfs_content_test_suite()
{
    Suite *s = suite_create("Tests do_insert implementation");

    Add_Test(s, do_insert_null_params);
    Add_Test(s, do_insert_full);
    Add_Test(s, do_insert_duplicate_id);
    Add_Test(s, do_insert_invalid_image);
    Add_Test(s, do_insert_invalid_file_mode);
    Add_Test(s, do_insert_duplicate);
    Add_Test(s, do_insert_valid);
    Add_Test(s, do_insert_write_correct_metadata);
    Add_Test(s, do_insert_write_initializes_metadata);

    return s;
}

TEST_SUITE_VIPS(imgfs_content_test_suite)
