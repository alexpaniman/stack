#pragma once

#include "trace.h"

#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>

struct line {
    const wchar_t* begin;
    size_t length;
};

struct text {
    wchar_t* buffer;

    line* lines;
    size_t number_of_lines;
};

// Return file size in bytes
stack_trace* get_file_size(FILE* const file, size_t* const size);

stack_trace* count_new_lines(const wchar_t* buffer, int* const line_count);

stack_trace* read_file(const char* const file_name, wchar_t** const file_content);

stack_trace* split_in_lines_with_terminator(wchar_t* const buffer,
    line** const lines_array, size_t* const number_of_lines);

stack_trace* get_text(const char* const file_name, text* txt);

void text_destruct(text* txt);
