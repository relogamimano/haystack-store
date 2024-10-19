#include "http_prot.h"
#include "test.h"
#include <check.h>

#define ck_assert_http_str_eq(a, b)                                                                                    \
    ck_assert_msg(strncmp(a.val, b, a.len) == 0 && a.len == strlen(b),                                                 \
                  "Assertion " #a " == " #b " failed, " #a " == \"%*.*s\"", (int) a.len, (int) a.len, a.val)

inline static void ck_assert_has_header(const struct http_message *msg, const char *key, const char *value)
{
    for (size_t i = 0; i < msg->num_headers; ++i) {
        if (http_match_verb(&msg->headers[i].key, key) && http_match_verb(&msg->headers[i].value, value)) {
            return;
        }
    }

    ck_abort_msg("Missing header %s: %s", key, value);
}

// ======================================================================
START_TEST(http_match_uri_null_params)
{
    start_test_print;

    const char *str = "";
    struct http_message http_msg;

    ck_assert_invalid_arg(http_match_uri(NULL, str));
    ck_assert_invalid_arg(http_match_uri(&http_msg, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_match_uri_valid)
{
    start_test_print;

    const char *str = "/u/r/i";
    struct http_message http_msg;
    http_msg.uri.val = str;
    http_msg.uri.len = 6;

    ck_assert_int_eq(http_match_uri(&http_msg, "/u/r/i"), 1);
    ck_assert_int_eq(http_match_uri(&http_msg, "/u/r"), 1);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_match_verb_null_params)
{
    start_test_print;

    const char *str = "";
    struct http_string http_str;

    ck_assert_invalid_arg(http_match_verb(NULL, str));
    ck_assert_invalid_arg(http_match_verb(&http_str, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_match_verb_valid)
{
    start_test_print;

    const char *str = "POST /u/r/i";
    struct http_string http_str;
    http_str.val = str;
    http_str.len = 4;

    ck_assert_int_eq(http_match_verb(&http_str, "POST"), 1);
    ck_assert_int_eq(http_match_verb(&http_str, "POS"), 0);
    ck_assert_int_eq(http_match_verb(&http_str, "POST /u/r/i"), 0);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_get_var_null_params)
{
    start_test_print;

    char buf;
    const char *str = "";
    struct http_string http_str;

    ck_assert_invalid_arg(http_get_var(NULL, str, &buf, 1));
    ck_assert_invalid_arg(http_get_var(&http_str, NULL, &buf, 1));
    ck_assert_invalid_arg(http_get_var(&http_str, str, NULL, 1));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_get_var_not_found)
{
    start_test_print;

    char buf;

    const char *str = "http://localhost:8000/imgfs/create?max_files=10&thumbres_x=64&thumbres_y=64";
    struct http_string http_str = {.val = str, .len = strlen(str)};

    ck_assert_int_eq(http_get_var(&http_str, "smallres_x", &buf, 1), 0);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_get_var_too_big)
{
    start_test_print;

    char buf;

    const char *str = "http://localhost:8000/imgfs/create?max_files=10&thumbres_x=64&thumbres_y=64";
    struct http_string http_str = {.val = str, .len = strlen(str)};

    ck_assert_err(http_get_var(&http_str, "max_files", &buf, 1), ERR_RUNTIME);
    ck_assert_err(http_get_var(&http_str, "thumbres_y", &buf, 1), ERR_RUNTIME);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_get_var_valid)
{
    start_test_print;

    char buf[10];

    const char *str = "http://localhost:8000/imgfs/create?max_files=10&thumbres_x=64&thumbres_y=64";
    struct http_string http_str = {.val = str, .len = strlen(str)};

    ck_assert_int_eq(http_get_var(&http_str, "max_files", buf, 10), 2);
    buf[2] = 0;
    ck_assert_str_eq(buf, "10");

    ck_assert_int_eq(http_get_var(&http_str, "thumbres_x", buf, 10), 2);
    buf[2] = 0;
    ck_assert_str_eq(buf, "64");

    ck_assert_int_eq(http_get_var(&http_str, "thumbres_y", buf, 10), 2);
    buf[2] = 0;
    ck_assert_str_eq(buf, "64");

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_parse_message_null_params)
{
    start_test_print;

    const char *str = "";
    struct http_message msg;
    int content_len;

    ck_assert_invalid_arg(http_parse_message(NULL, 0, &msg, &content_len));
    ck_assert_invalid_arg(http_parse_message(str, 0, NULL, &content_len));
    ck_assert_invalid_arg(http_parse_message(str, 0, &msg, NULL));

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_parse_message_partial_headers)
{
    start_test_print;

    const char *str = "GET /imgfs/read?res=orig&img_id=mure.jpg HTTP/1.1" HTTP_LINE_DELIM
                      "Host: localhost:8000" HTTP_LINE_DELIM "User-Agent: cur" HTTP_LINE_DELIM;
    struct http_message msg;
    int content_len;

    ck_assert_int_eq(http_parse_message(str, strlen(str), &msg, &content_len), 0);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_parse_message_full_headers_no_content)
{
    start_test_print;

    const char *str =
    "GET /imgfs/read?res=orig&img_id=mure.jpg HTTP/1.1" HTTP_LINE_DELIM "Host: localhost:8000" HTTP_LINE_DELIM
    "User-Agent: curl/8.5.0" HTTP_LINE_DELIM "Accept: */*" HTTP_HDR_END_DELIM;
    struct http_message msg;
    int content_len;

    ck_assert_int_eq(http_parse_message(str, strlen(str), &msg, &content_len), 1);

    ck_assert_http_str_eq(msg.method, "GET");
    ck_assert_http_str_eq(msg.uri, "/imgfs/read?res=orig&img_id=mure.jpg");

    ck_assert_int_eq(msg.num_headers, 3);
    ck_assert_has_header(&msg, "Host", "localhost:8000");
    ck_assert_has_header(&msg, "User-Agent", "curl/8.5.0");
    ck_assert_has_header(&msg, "Accept", "*/*");

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_parse_message_full_headers_partial_content)
{
    start_test_print;

    const char *str = "GET /imgfs/read?res=orig&img_id=mure.jpg HTTP/1.1" HTTP_LINE_DELIM
                      "Host: localhost:8000" HTTP_LINE_DELIM "User-Agent: curl/8.5.0" HTTP_LINE_DELIM
                      "Accept: */*" HTTP_LINE_DELIM "Content-Length: 12" HTTP_HDR_END_DELIM "Hello ";
    struct http_message msg;
    int content_len;

    ck_assert_int_eq(http_parse_message(str, strlen(str), &msg, &content_len), 0);

    ck_assert_int_eq(content_len, 12);

    end_test_print;
}
END_TEST

// ======================================================================
START_TEST(http_parse_message_full_headers_full_content)
{
    start_test_print;

    const char *str =
    "POST /imgfs/insert?&name=papillon.jpg HTTP/1.1" HTTP_LINE_DELIM "Host: localhost:8000" HTTP_LINE_DELIM
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0" HTTP_LINE_DELIM
    "User-Agent: curl/8.5.0" HTTP_LINE_DELIM "Accept: */*" HTTP_LINE_DELIM
    "Accept-Language: fr,fr-FR;q=0.8,en-US;q=0.5,en;q=0.3" HTTP_LINE_DELIM
    "Accept-Encoding: gzip, deflate, br" HTTP_LINE_DELIM "Referer: http://localhost:8000/index.html" HTTP_LINE_DELIM
    "Content-Length: 12" HTTP_LINE_DELIM "Origin: http://localhost:8000" HTTP_LINE_DELIM "DNT: 1" HTTP_LINE_DELIM
    "Connection: keep-alive" HTTP_LINE_DELIM "Sec-Fetch-Dest: empty" HTTP_LINE_DELIM
    "Sec-Fetch-Mode: cors" HTTP_LINE_DELIM "Sec-Fetch-Site: same-origin" HTTP_HDR_END_DELIM "Hello world!";
    struct http_message msg;
    int content_len;

    ck_assert_int_eq(http_parse_message(str, strlen(str), &msg, &content_len), 1);

    ck_assert_http_str_eq(msg.method, "POST");
    ck_assert_http_str_eq(msg.uri, "/imgfs/insert?&name=papillon.jpg");

    ck_assert_int_eq(msg.num_headers, 14);
    ck_assert_has_header(&msg, "Host", "localhost:8000");
    ck_assert_has_header(&msg, "User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0");
    ck_assert_has_header(&msg, "User-Agent", "curl/8.5.0");
    ck_assert_has_header(&msg, "Accept", "*/*");
    ck_assert_has_header(&msg, "Accept-Language", "fr,fr-FR;q=0.8,en-US;q=0.5,en;q=0.3");
    ck_assert_has_header(&msg, "Accept-Encoding", "gzip, deflate, br");
    ck_assert_has_header(&msg, "Referer", "http://localhost:8000/index.html");
    ck_assert_has_header(&msg, "Content-Length", "12");
    ck_assert_has_header(&msg, "Origin", "http://localhost:8000");
    ck_assert_has_header(&msg, "DNT", "1");
    ck_assert_has_header(&msg, "Connection", "keep-alive");
    ck_assert_has_header(&msg, "Sec-Fetch-Dest", "empty");
    ck_assert_has_header(&msg, "Sec-Fetch-Mode", "cors");
    ck_assert_has_header(&msg, "Sec-Fetch-Site", "same-origin");

    ck_assert_http_str_eq(msg.body, "Hello world!");

    end_test_print;
}
END_TEST

// ======================================================================
Suite *http_test_suite()
{
    Suite *s = suite_create("Tests http_prot implementation");

    Add_Test(s, http_match_uri_null_params);
    Add_Test(s, http_match_uri_valid);

    Add_Test(s, http_match_verb_null_params);
    Add_Test(s, http_match_verb_valid);

    Add_Test(s, http_get_var_null_params);
    Add_Test(s, http_get_var_not_found);
    Add_Test(s, http_get_var_too_big);
    Add_Test(s, http_get_var_valid);

    Add_Test(s, http_parse_message_null_params);
    Add_Test(s, http_parse_message_partial_headers);
    Add_Test(s, http_parse_message_full_headers_no_content);
    Add_Test(s, http_parse_message_full_headers_partial_content);
    Add_Test(s, http_parse_message_full_headers_full_content);

    return s;
}

TEST_SUITE(http_test_suite)
