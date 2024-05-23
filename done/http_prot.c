#include <string.h>
#include <http_prot.h>



int http_match_uri(const struct http_message *message, const char *target_uri) {
    size_t len = message->uri.len; 
    char* val = message->uri.val; 

    if (len < strlen(target_uri)) {
        return 0; 
    }

    if (strncmp(val, target_uri, strlen(target_uri)) != 0) {
        return 0; 
    } 
    return 1; 
}


int http_match_verb(const struct http_string* method, const char* verb) {
    size_t len = method->len; 
    char* val = method->val; 

    if (len != strlen(verb)) {
        return 0; 
    }
    if (strncmp(val, verb, len) != 0) {
        return 0; 
    } 
    return 1; 
}

