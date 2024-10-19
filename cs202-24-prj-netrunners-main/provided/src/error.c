/**
 * @file error.c
 * @brief error messages
 */

const char* const ERR_MESSAGES[] = {
    "", // no error
    "I/O Error",
    "Runtime error",
    "(re|m|c)alloc failed",
    "Not enough arguments",
    "Invalid filename",
    "Invalid command",
    "Invalid argument",
    "Threading error",
    "Invalid max_files number",
    "Invalid resolution(s)",
    "Invalid image ID",
    "imgFS is full",
    "File not found",
    "Not implemented (yet?)",
    "Existing image ID",
    "Image manipulation library error",
    "Debug",
    "no error (shall not be displayed)" // ERR_LAST
};
