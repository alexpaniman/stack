#include "ascii-tables.h"

void ascii_table_create(table* const tbl) {
    simple_stack_create(tbl);
}

void ascii_table_add_row(table* const tbl) {
    simple_stack_push(tbl, { NULL });
}

void ascii_table_add_text(table* const tbl, const char* const text) {
    simple_stack_push(tbl, { text });
}

void ascii_table_print(table* const tbl, FILE* ostream) {
    fprintf(ostream, "<table>");
    
    fprintf(ostream, "<tr>");
    for (size_t i = 0; i < tbl->next_index; ++ i) {
        fprintf(ostream, "");

        if (tbl->elements[i].cell_string == NULL)
            fprintf(ostream, "</tr>\n<tr>");
        else {
            fprintf(ostream, "");
        }
    }
    fprintf(ostream, "</tr>");

    fprintf(ostream, "</table>");

    // for (size_t i = 0; i < tbl->next_index; ++ i) {
    //     if (i == 0) {
    //         fprintf(ostream, LT_CORNER);
    //         for (int j = 0; tbl->elements[j].cell_string != NULL)
    //     }

    //     cell current = tbl->elements[i];

    // }
}

void ascii_table_destruct(table* const tbl) {
    simple_stack_destruct(tbl);
}
