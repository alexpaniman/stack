#pragma once

#include "trace.h"
#include "lexer.h"
#include "binary-tree.h"

typedef wchar_t* akinator_node;
typedef binary_tree<akinator_node> akinator_tree;

stack_trace* parse_node(simple_stack<token>* tokens,
                        akinator_tree** resulting_node);

stack_trace* load_tree(wchar_t* text, akinator_tree** tree);

void print_tree(akinator_tree* tree);

enum game_mode {
    MODE_PLAY    = 1,
    MODE_DEFINE  = 2,
    MODE_COMPARE = 3
};

game_mode read_game_mode(void);
