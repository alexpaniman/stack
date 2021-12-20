#pragma once

#include "graphviz.h"
#include "trace.h"
#include "binary-tree.h"

typedef wchar_t* akinator_node;
typedef binary_tree<akinator_node>* akinator_tree;

struct parser {
    const wchar_t* string;
    int index;
};

stack_trace* akinator_parse_node_name(parser* parse, wchar_t** new_string);
stack_trace* akinator_parse_tree(parser* parse, binary_tree<akinator_node>** tree);

void akinator_print_tree(FILE* file, akinator_tree tree);
digraph akinator_visualize(akinator_tree tree);

void akinator_ask_and_define(akinator_tree tree);
void akinator_ask_and_find_difference(akinator_tree tree);

void akinator_speak(const wchar_t* format, ...);

enum game_mode {
    MODE_PLAY    = 1,
    MODE_DEFINE  = 2,
    MODE_COMPARE = 3
};

game_mode read_game_mode(void);

void akinator_write_tree(const char* file_name, const akinator_tree tree);
akinator_tree akinator_read_tree(const char* file_name);

void akinator_play(akinator_tree tree);
void akinator_describe(akinator_tree tree, wchar_t* object);

void akinator_destroy(akinator_tree tree);

