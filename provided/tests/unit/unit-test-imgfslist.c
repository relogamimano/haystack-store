#include "imgfs.h"
#include "imgfscmd_functions.h"
#include "test.h"
#include "util.h"
#include <check.h>

// ======================================================================
START_TEST(do_list_null_params)
{
    start_test_print;

    struct imgfs_file file;
    char *str = NULL;
    ck_assert_invalid_arg(do_list(NULL, STDOUT, &str));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_list_cmd_null_params)
{
    start_test_print;

    ck_assert_invalid_arg(do_list_cmd(0, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_list_cmd_too_many_params)
{
    start_test_print;

    char *argv[] = {"", "", ""};
    ck_assert_err(do_list_cmd(3, argv), ERR_INVALID_COMMAND);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(do_list_cmd_inexistent_file)
{
    start_test_print;

    char *argv[] = {"not a file"};
    ck_assert_err(do_list_cmd(1, argv), ERR_IO);

    end_test_print;
}
END_TEST

// ======================================================================
Suite *imgfs_structures_test_suite()
{
    Suite *s = suite_create("Tests for do_list and do_list_cmd implementation");

    Add_Test(s, do_list_null_params);

    Add_Test(s, do_list_cmd_null_params);
    Add_Test(s, do_list_cmd_too_many_params);
    Add_Test(s, do_list_cmd_inexistent_file);

    return s;
}

TEST_SUITE(imgfs_structures_test_suite)
