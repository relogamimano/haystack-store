#include "image_content.h"
#include "imgfs.h"
#include "test.h"
#include <check.h>
#include <vips/vips.h>

// ======================================================================
START_TEST(get_resolution_null)
{
    start_test_print;

    uint32_t height, width;
    char buffer;

    ck_assert_invalid_arg(get_resolution(NULL, &width, &buffer, 1));
    ck_assert_invalid_arg(get_resolution(&height, NULL, &buffer, 1));
    ck_assert_invalid_arg(get_resolution(&height, &width, NULL, 1));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(get_resolution_invalid_buffer)
{
    start_test_print;

    uint32_t height, width;
    char buffer[1000] = {0};

    ck_assert_err(get_resolution(&height, &width, buffer, sizeof(buffer)), ERR_IMGLIB);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(get_resolution_valid)
{
    start_test_print;

    char image_buffer[82234];
    read_file(image_buffer, DATA_DIR "/brouillard.jpg", 82234);

    uint32_t height = 0, width = 0;
    ck_assert_err_none(get_resolution(&height, &width, image_buffer, 82234));

    ck_assert_uint_eq(height, 400);
    ck_assert_uint_eq(width, 600);

    end_test_print;
}
END_TEST

// ======================================================================
Suite *imgfs_get_resolution_test_suite()
{
    Suite *s = suite_create("Tests get_resolution() implementation");

    Add_Test(s, get_resolution_null);
    Add_Test(s, get_resolution_invalid_buffer);
    Add_Test(s, get_resolution_valid);

    return s;
}

TEST_SUITE_VIPS(imgfs_get_resolution_test_suite)
