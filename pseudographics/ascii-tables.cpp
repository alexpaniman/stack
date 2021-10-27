#include "ascii-tables.h"

void ascii_table_create(table* const tbl) {
    simple_stack_create(tbl);
}

void ascii_table_add_row(table* const tbl) {
    row new_row = {};
    simple_stack_create(&new_row);

    simple_stack_push(tbl, new_row);
}

void ascii_table_add_text(table* const tbl, const char* const text) {
    row current_row = simple_stack_peek(tbl);
    simple_stack_push(&current_row, text);
}

void ascii_table_print(table* const tbl) {
}

void ascii_table_destruct(table* const tbl) {
    simple_stack_destruct(tbl);
}
