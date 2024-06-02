#include <string.h> 
#include "http_prot.h"
#include "error.h"

int http_match_uri(const struct http_message *message, const char *target_uri) {
    M_REQUIRE_NON_NULL(message); M_REQUIRE_NON_NULL(target_uri);
    size_t len = message->uri.len; 
    char* val = message->uri.val; 

    // Compare the lengths of the URIs
    if (len < strlen(target_uri)) {
        return 0; 
    }

    // Compare the URIs using strncmp
    if (strncmp(val, target_uri, strlen(target_uri)) != 0) {
        return 0; 
    } 
    return 1; 
}

int http_parse_message(const char *stream, size_t bytes_received, struct http_message *out, int *content_len) {
    M_REQUIRE_NON_NULL(stream); M_REQUIRE_NON_NULL(out); M_REQUIRE_NON_NULL(content_len);
    struct http_string method; 
    struct http_string uri; 

    // Parse the method and URI from the stream
    const char* message;
    
    message = get_next_token(stream, " ", &method); 
    message = get_next_token(message, " ", &uri); 

    // Store the method and URI in the output struct
    out->method = method; 
    out->uri = uri;

    // Parse the headers from the stream
    message = get_next_token(message, HTTP_LINE_DELIM, &method);
    message = http_parse_headers(message, out); 

    // If header has not been fully parsed, return 0
    if(message == NULL) {
        return 0; 
    }

    // Get the "Content-Length" value from the parsed header lines
    for (size_t i = 0; i < out->num_headers; i++) {
        if (strncmp(out->headers[i].key.val, "Content-Length", out->headers[i].key.len) == 0) {
            char content_len_str[out->headers[i].value.len + 1];
            strncpy(content_len_str, out->headers[i].value.val, out->headers[i].value.len);
            content_len_str[out->headers[i].value.len] = '\0';
            *content_len = atoi(content_len_str);
            break;
        }
        // If no Content-Length key has been found, return 0
        if (i == out->num_headers - 1) {
            return 1;
        }
    }

    // If there is no body (Content-Length value is 0) or you were able to read the full body
    if (message == NULL || bytes_received - (message - stream) < *content_len || *content_len == 0) {
        return 0; 
    }   
    
    // Store the body in the output struct
    out->body.val = message; 
    out->body.len = bytes_received - (message - stream);  
    
    return 1;
}

int http_get_var(const struct http_string* url, const char* name, char* out, size_t out_len) {
    M_REQUIRE_NON_NULL(url); M_REQUIRE_NON_NULL(name); M_REQUIRE_NON_NULL(out);
    size_t len = url->len; 
    char* val = url->val; 

    
    char name_equals[strlen(name) + 2];
    strcpy(name_equals, name);
    strcat(name_equals, "=");

    // Find the start of the variable value in the URL
    char* start = strstr(val, name_equals); 
    if (start == NULL) {
        return 0; 
    }
    start += strlen(name_equals); 

    // Find the end of the variable value in the URL
    char* end = strchr(start, '&'); 
    if (end == NULL) {
        end = url->val + url->len; 
    }

    // Check if the output buffer is large enough
    if (end - start > out_len) {
        return ERR_RUNTIME; 
    }

    // Copy the variable value to the output buffer
    strncpy(out, start, end - start); 
    return end - start; 
}


int http_match_verb(const struct http_string* method, const char* verb) {
    M_REQUIRE_NON_NULL(method); M_REQUIRE_NON_NULL(verb);
    size_t len = method->len; 
    char* val = method->val; 

    // Compare the lengths of the method and verb
    if (len != strlen(verb)) {
        return 0; 
    }

    // Compare the method and verb using strncmp
    if (strncmp(val, verb, len) != 0) {
        return 0; 
    } 
    return 1; 
}


static_unless_test const char* get_next_token(const char* message, const char* delimiter, struct http_string* output) {
    M_REQUIRE_NON_NULL(message); M_REQUIRE_NON_NULL(delimiter); M_REQUIRE_NON_NULL(output);
    const char* end_token = strstr(message, delimiter);
    if (end_token == NULL) {
        return NULL; 
    }
    output->val = message;
    output->len = end_token - message;
    return end_token + strlen(delimiter);
}


static_unless_test const char* http_parse_headers(const char* header_start, struct http_message* output) {
    M_REQUIRE_NON_NULL(header_start); M_REQUIRE_NON_NULL(output); 
    const char* message = header_start; 
    
    output->num_headers = 0; 
    while(1) {
        struct http_string key; 
        struct http_string value; 

        // Check if the maximum number of headers has been reached
        if (output->num_headers == MAX_HEADERS) {
            break; 
        }
        
        // Check if the end of the headers has been reached
        if(strncmp(message, HTTP_LINE_DELIM, strlen(HTTP_LINE_DELIM)) == 0) {
            message += strlen(HTTP_LINE_DELIM); 
            break; 
        }

        // Parse the key and value of the header
        message = get_next_token(message, HTTP_HDR_KV_DELIM, &key); 
        if (message == NULL) {
            break; 
        }
        message = get_next_token(message, HTTP_LINE_DELIM, &value); 
        if (message == NULL) {
            break; 
        }
        
        // Store the key and value in the output struct
        output->headers[output->num_headers].key = key; 
        output->headers[output->num_headers].value = value; 
        output->num_headers++;
    }
    return message; 
}