#include "lexer.h"
#include "test-framework.h"

#include <cstring>
#include <stdio.h>

token name(const char* name) {
    substring str = { name, strlen(name) };
    return { .type = NAME, .name = str };
}

token number(double num) {
    return { .type = NUMBER, .number = num };
}

TEST(lexer_test) {
    const char* text = R"(
        push bx, ax, [12], [ax]
        mov ab, eax ; Line comment
    )";

    const token expected_tokens[] = {
        { BR }, name("push"), name("bx"), { COMMA }, name("ax"), { COMMA },
        { LSQUARE_BRACKET }, number(12), { RSQUARE_BRACKET }, { COMMA },
        { LSQUARE_BRACKET }, name("ax"), { RSQUARE_BRACKET }, { BR },
        name("mov"), name("ab"), { COMMA }, name("eax"), { BR }
    };

    simple_stack<token> tokens;
    stack_trace* trace = tokenize(text, &tokens);

    size_t num_elements = tokens.next_index;

    token elements[num_elements];
    memcpy(elements, tokens.elements, tokens.next_index * sizeof(token));

    simple_stack_destruct(&tokens);

    trace_print_stack_trace(stdout, trace);
    if (!trace_is_success(trace)) {
        simple_stack_destruct(&tokens);
        trace_destruct(trace);
    }

    ASSERT_EQUAL(trace_is_success(trace), true);

    trace_destruct(trace);

    for (size_t i = 0; i < num_elements; ++ i) {
        token current = elements[i];

        ASSERT_EQUAL(current.type, expected_tokens[i].type);

        if      (current.type ==   NAME)
            ASSERT_EQUAL(strncmp(current.name.str, expected_tokens[i].name.str,
                                expected_tokens[i].name.str_length), 0);
        else if (current.type == NUMBER)
            ASSERT_EPSILON_EQUAL(current.number, expected_tokens[i].number);
    }
}

TEST_MAIN()
