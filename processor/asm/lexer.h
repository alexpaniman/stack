#pragma once

#include "simple-stack.h"
#include "trace.h"

enum token_type {
    NAME, NUMBER, BR,
    COLON, COMMA,
    LSQUARE_BRACKET,
    RSQUARE_BRACKET
};

struct substring {
    const char* str;
    size_t str_length;
};

struct token {
    token_type type;
    union {
        substring name;
        double number;
    };
};

struct tokenized_input {
    token* tokens;
    size_t size;
};

stack_trace* tokenize(const char* text, simple_stack<token>* tokens);
