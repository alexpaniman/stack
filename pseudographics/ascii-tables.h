#pragma once

#include "simple-stack.h"

#define HLINE  '─'
#define VLINE  '│'
#define LT_CORNER '┌'
#define RT_CORNER '┐'
#define LB_CORNER '└'
#define RB_CORNER '┘'

typedef simple_stack<const char*> row;
typedef simple_stack<row> table;

void ascii_table_create(table* const tbl);

void ascii_table_add_row(table* const tbl);

void ascii_table_add_text(table* const tbl, const char* const text);

void ascii_table_print(table* const tbl);

void ascii_table_destruct(table* const tbl);
