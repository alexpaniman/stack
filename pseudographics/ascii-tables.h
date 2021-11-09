#pragma once

#include "simple-stack.h"

#define HLINE  "─"
#define VLINE  "│"
#define LT_CORNER "┌"
#define RT_CORNER "┐"
#define LB_CORNER "└"
#define RB_CORNER "┘"
#define R_CONNECT "┤"
#define L_CONNECT "├"
#define T_CONNECT "┬"
#define B_CONNECT "┴"

struct cell {
    const char* cell_string;
    int col_span, row_span;
};

typedef simple_stack<cell> table;

void ascii_table_create(table* const tbl);

void ascii_table_add_text(table* const tbl, const char* const text);

void ascii_table_print(table* const tbl);

void ascii_table_destruct(table* const tbl);
