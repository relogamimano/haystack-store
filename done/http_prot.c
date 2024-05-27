#include <string.h> 
#include "http_prot.h"
#include "error.h"




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


int http_get_var(const struct http_string* url, const char* name, char* out, size_t out_len) {

    M_REQUIRE_NON_NULL(url); 
    M_REQUIRE_NON_NULL(name); 
    M_REQUIRE_NON_NULL(out); 

    char name_eq[strlen(name) + 2];
    snprintf(name_eq, sizeof(name_eq), "%s=", name); // name_eq is now "name="

    const char* s = strstr(url->val, name_eq); 
    if (s == NULL) { return 0; } //parameter not found, return 0

    s+= strlen(name_eq); //go the = and look for the actual parameter

    //look for the end of parameter, either & or eos 
    const char* t = memchr(s, '&', url->val + url->len - s);
    if (t == NULL) { t = url->val+url->len; }

    size_t length = t - s; 
    if (length > out_len) { return ERR_RUNTIME; }

    memcpy(out, s, length); 
    out[length] = '\0'; //of course 

    return length; 
}

