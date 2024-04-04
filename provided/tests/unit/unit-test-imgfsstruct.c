#include "test.h"
#include <check.h>

// ======================================================================
#define SIZE_imgfs_header 64
#define SIZE_img_metadata 216
#define SIZE_imgfs_file   80

#define OFFSET_imgfs_header_name        0
#define OFFSET_imgfs_header_version     32
#define OFFSET_imgfs_header_nb_files    36
#define OFFSET_imgfs_header_max_files   40
#define OFFSET_imgfs_header_resized_res 44

#define OFFSET_img_metadata_img_id   0
#define OFFSET_img_metadata_SHA      128
#define OFFSET_img_metadata_orig_res 160
#define OFFSET_img_metadata_size     168
#define OFFSET_img_metadata_offset   184
#define OFFSET_img_metadata_is_valid 208

#define OFFSET_imgfs_file_file     0
#define OFFSET_imgfs_file_header   8
#define OFFSET_imgfs_file_metadata 72

// ======================================================================
#define test_member(T, M)                                                                                              \
    do {                                                                                                               \
        if (offsetof(struct T, M) != (size_t) OFFSET_##T##_##M) {                                                      \
            ck_abort_msg(#M " wrongly placed in struct " #T ": %zu instead of %d\n"                                    \
                            "(maybe you didn't put them in the same order as we expect.)\n",                           \
                         offsetof(struct T, M), OFFSET_##T##_##M);                                                     \
        }                                                                                                              \
        mark_point();                                                                                                  \
    } while (0)

// ======================================================================
#define test_size(T)                                                                                                   \
    do {                                                                                                               \
        if (sizeof(struct T) != (size_t) SIZE_##T) {                                                                   \
            ck_abort_msg("struct " #T " does not have the right size: "                                                \
                         "%zu instead of %d\n",                                                                        \
                         sizeof(struct T), SIZE_##T);                                                                  \
        }                                                                                                              \
        mark_point();                                                                                                  \
    } while (0)

// ======================================================================
START_TEST(imgfs_header)
{
    start_test_print;

    test_size(imgfs_header);

    test_member(imgfs_header, name);
    test_member(imgfs_header, version);
    test_member(imgfs_header, nb_files);
    test_member(imgfs_header, max_files);
    test_member(imgfs_header, resized_res);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(img_metadata)
{
    start_test_print;

    test_size(img_metadata);

    test_member(img_metadata, img_id);
    test_member(img_metadata, SHA);
    test_member(img_metadata, orig_res);
    test_member(img_metadata, size);
    test_member(img_metadata, offset);
    test_member(img_metadata, is_valid);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(imgfs_file)
{
    start_test_print;

    test_size(imgfs_file);

    test_member(imgfs_file, file);
    test_member(imgfs_file, header);
    test_member(imgfs_file, metadata);

    end_test_print;
}
END_TEST

// ======================================================================
Suite *imgfs_structures_test_suite()
{
    Suite *s = suite_create("Tests for ImgFS data structures implementation");

    Add_Test(s, imgfs_header);
    Add_Test(s, img_metadata);
    Add_Test(s, imgfs_file);

    return s;
}

TEST_SUITE(imgfs_structures_test_suite)
