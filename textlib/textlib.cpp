#include "textlib.h"

#include <cstddef>
#include <cstdio>
#include <errno.h>
#include <string.h>
#include <wchar.h>
#include <malloc.h>

// Return file size in bytes
stack_trace* get_file_size(FILE* const file, size_t* const size) {
    if (file == NULL)
        return FAILURE(RUNTIME_ERROR, "File is NULL!");

    int fd = fileno(file);

    if (fd == -1)
        return FAILURE(RUNTIME_ERROR, strerror(errno));

    // Struct keyword is necessary because it was defined in C:
    struct stat file_stats;
    int fstat_return_code = fstat(fd, &file_stats);

    if (fstat_return_code == -1)
        return FAILURE(RUNTIME_ERROR, strerror(errno));

    *size = (size_t) file_stats.st_size;
    return SUCCESS();
}

stack_trace* count_new_lines(const wchar_t* buffer,
                             size_t* const line_count) {
    if (buffer == NULL)
        return FAILURE(RUNTIME_ERROR, "Buffer is NULL!");

    size_t new_line_count = 1;

    wchar_t symbol = L'\0';
    while ((symbol = *buffer++) != L'\0')
        if (symbol == '\n')
            ++ new_line_count;

    *line_count = new_line_count;

    return SUCCESS();
}

stack_trace* split_in_lines_with_terminator(wchar_t* const buffer,
                                            line**   const lines_array,
                                            size_t*  const number_of_lines) {
    if (buffer == NULL)
        return FAILURE(RUNTIME_ERROR, "Buffer is NULL!");
    
    size_t new_line_count = 0;

    stack_trace* trace = count_new_lines(buffer, &new_line_count);
    if (!trace_is_success(trace))
        return PASS_FAILURE(trace, RUNTIME_ERROR, "Line counting failed!");

    line* lines = (line*) calloc((size_t) new_line_count, sizeof(line));
    if (lines == NULL)
        return FAILURE(RUNTIME_ERROR, strerror(errno));

    size_t line_index = 0;

    lines[line_index].begin = buffer;
    size_t line_length = 0;
    
    for (int i = 0; buffer[i] != L'\0'; ++ i) {
        wchar_t symbol = buffer[i];

        if (symbol == '\n') {
            // Replace \n with string end (null-terminator)
            buffer[i] = '\0';

            lines[line_index].length = line_length;

            // Reset line length
            line_length = 0;

            // Skip LF in the end
            if (buffer[i + 1] != L'\0')
                lines[++ line_index].begin = buffer + (i + 1);

        } else ++ line_length;
    }

    if (line_length > 0)
        lines[line_index].length = line_length;

    *lines_array = lines, *number_of_lines = new_line_count;
    return SUCCESS();
}

stack_trace* read_file(const char* const file_name, wchar_t** const file_content) {
    FILE* input_file = fopen(file_name, "r");

    if (input_file == NULL)
        return FAILURE(RUNTIME_ERROR, strerror(errno));

    size_t size_bytes = 0;
    stack_trace* trace = get_file_size(input_file, &size_bytes);

    if (!trace_is_success(trace))
        return PASS_FAILURE(trace, RUNTIME_ERROR, "Getting file size failed!");

    // Allocates wchar_t for every byte in the file, it's likely more
    // space than required, but it's always enough.

    wchar_t* buffer = (wchar_t*)
        calloc(sizeof *buffer, size_bytes + 1 /* for '\0's */);

    if (buffer == NULL)
        return FAILURE(RUNTIME_ERROR, strerror(errno));

    wchar_t* eof = buffer;
    while (fgetws_unlocked(eof, (int) size_bytes + 1, input_file) != NULL) {

        /* ┌─  Previous Line  ─┐┌────  Advance EOF Here  ────┐
         * ↓                   ↓↑ ←─────  From Here          ↓
         * ...CCCCCCCCCCCCCCCCCNCCCCCCCCCCCCCCCCCCCCCCCCCCCCN0
         *     [line chars]    ↑        [line chars]        ↑↑
         * ... ←─────────────  LF   ────────────────────────┘│
         * ... ←─────────────  EOF  ─────────────────────────┘ */

        while (*eof != L'\0')
            ++ eof;
    }

    fclose(input_file), input_file = NULL;

    *file_content = buffer;
    return SUCCESS();
}

stack_trace* get_text(const char* const file_name, text* txt) {
    wchar_t* buffer = NULL;
    stack_trace* read_trace = read_file(file_name, &buffer);

    if (buffer == NULL)
        return PASS_FAILURE(read_trace, RUNTIME_ERROR, "Reading file failed!");

    size_t number_of_lines = 0;
 
    line* lines = NULL;
    stack_trace* lines_trace =
        split_in_lines_with_terminator(buffer, &lines, &number_of_lines);

    if (lines == NULL)
        return PASS_FAILURE(lines_trace, RUNTIME_ERROR,
                            "Splitting buffer in lines failed!");

    *txt = { buffer, lines, number_of_lines };

    return SUCCESS();
}

void text_destruct(text* txt) {
    free(txt->buffer), txt->buffer = NULL;
    free(txt->lines ), txt->lines  = NULL;
}
