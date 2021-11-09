#include "textlib.h"
#include "test-framework.h"

TEST(read_file_with_lots_of_lines) {
    char file_name[256] = { 0 };
    ASSERT_EQUAL((tmpnam(file_name) != 0), true);

    const size_t number_of_lines = 100;

    FILE* file = fopen(file_name, "w");
    for (size_t i = 0; i < number_of_lines; ++ i) {
        for (size_t j = 0; j < i; ++ j)
            fprintf(file, "%zu", j % 10);

        if (i != number_of_lines - 1)
            fprintf(file, "\n");
    }
    fclose(file), file = NULL;

    text txt;
    stack_trace* trace = get_text(file_name, &txt);

    trace_print_stack_trace(stdout, trace);

    ASSERT_EQUAL((int) txt.number_of_lines, 100);

    for (int i = 0; i < (int) txt.number_of_lines; ++ i)
        if (txt.lines[i].length > 0) {
            ASSERT_EQUAL(txt.lines[i].begin[i - 1] - L'0', (i - 1) % 10);
            ASSERT_EQUAL(txt.lines[i].begin[0] - L'0', 0);
        }

    text_destruct(&txt); trace_destruct(trace);
}

TEST_MAIN()
