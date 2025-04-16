/**
 * @file http_prot.h
 * @brief Minimal HTTP protocol services.
 *
 * @author Konstantinos Prasopoulos
 */

#pragma once

#define MAX_HEADERS 40

#define HTTP_HDR_KV_DELIM  ": "
#define HTTP_LINE_DELIM    "\r\n"
#define HTTP_HDR_END_DELIM HTTP_LINE_DELIM HTTP_LINE_DELIM
#define HTTP_PROTOCOL_ID   "HTTP/1.1 "
#define HTTP_OK            "200 OK"
#define HTTP_BAD_REQUEST   "400 Bad Request"

#include <stddef.h>

#ifdef IN_CS202_UNIT_TEST
#define static_unless_test
#else
#define static_unless_test static
#endif 

struct http_string {
    const char *val; // Warning! This is *NOT* null-terminated (thus len field below)
    size_t len;
};

struct http_header {
    struct http_string key;
    struct http_string value;
};

struct http_message {
    struct http_string method;
    struct http_string uri;
    struct http_header headers[MAX_HEADERS];
    size_t num_headers;
    struct http_string body;
};

/**
 * @brief Checks whether the `message` URI starts with the provided `target_uri`.
 *
 * Returns: 1 if it does, 0 if it does not.
 *
 */
int http_match_uri(const struct http_message *message, const char *target_uri);

/**
 * @brief Accepts a potentially partial TCP stream and parses an HTTP message.
 *
 * Assumes that all characters of stream that are not filled by reading are set to 0.
 *
 * Places the complete HTTP message in out.
 * Also writes the content of header "Content Length" to content_len upon parsing the header in the stream.
 * content_len can be used by the caller to allocate memory to receive the whole HTTP message.
 *
 * Returns:
 *  a negative int if there was an error
 *  0 if the message has not been received completely (partial treatment)
 *  1 if the message was fully received and parsed
 */
int http_parse_message(const char *stream, size_t bytes_received, struct http_message *out, int *content_len);

/**
 * @brief Writes the value of parameter `name` from URL in message to buffer out.
 *
 * Return the length of the value.
 * 0 or negative return values indicate an error.
 */
int http_get_var(const struct http_string* url, const char* name, char* out, size_t out_len);

/**
 * @brief Compare method with verb and return 1 if they are equal, 0 otherwise
 */
int http_match_verb(const struct http_string* method, const char* verb);

/**
 * @brief Retrieves the next token from a given message string.
 *
 * This function searches for the next token in the message string, delimited by the specified delimiter.
 * The token is returned as a null-terminated string and is stored in the output parameter.
 *
 * @param message The message string to search for tokens.
 * @param delimiter The delimiter used to separate tokens.
 * @param output A pointer to a struct http_string where the token will be stored.
 * @return A pointer to the next token in the message string, or NULL if no more tokens are found.
 */
static_unless_test const char* get_next_token(const char* message, const char* delimiter, struct http_string* output);

/**
 * @brief Parses the headers of an HTTP message.
 *
 * This function takes a pointer to the start of the headers in the HTTP message and a pointer to a struct http_message.
 * It parses the headers and populates the struct http_message with the parsed data.
 *
 * @param header_start A pointer to the start of the headers in the HTTP message.
 * @param output A pointer to a struct http_message where the parsed data will be stored.
 * @return A pointer to the next character after the parsed headers in the HTTP message.
 */
static_unless_test const char* http_parse_headers(const char* header_start, struct http_message* output);
