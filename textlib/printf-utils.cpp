#include "printf-utils.h"

#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>

char* vsprintf_to_new_buffer(const char* format, va_list args) {
    // This allows to then restore changes to diagnostic rules
    #pragma clang diagnostic push

    // This function itself is intended for use with string
    // literals, so it should be ok to disable warning:

    #pragma clang diagnostic ignored "-Wformat-nonliteral"

    // We need to copy args, because we will use them twice
    va_list args_copy;
    va_copy(args_copy, args);

    int buffer_size = vsnprintf(NULL, 0, format, args_copy) + 1;

    va_end(args_copy); // We are done with this args

    char *buffer = (char *) calloc((size_t) buffer_size, sizeof(*buffer));

    // Now let's finally print our string!
    vsnprintf(buffer, (size_t) buffer_size, format, args);

    // And restore warning back
    #pragma clang diagnostic pop

    return buffer;
}

char* sprintf_to_new_buffer(const char* format, ...) {
    va_list args;
    va_start(args, format);

    char* buffer = vsprintf_to_new_buffer(format, args);

    va_end(args);

    return buffer;
}
