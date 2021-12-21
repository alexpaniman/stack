#include <cstdlib>
#include <string.h>
#include <wchar.h>
#include <stdio.h>

#include "lexer.h"
#include "test-framework.h"

token name(const wchar_t* name) {
    substring str = { (wchar_t*) name, wcslen(name) };
    return { .type = NAME, .name = str };
}

token number(double num) {
    return { .type = NUMBER, .number = num };
}

TEST(lexer_test) {
    const char* char_text = R"(
        push bx, ax, [12], [ax]
        mov ab, eax ; Line comment
    )";

    const size_t buffer_size =
        mbstowcs(NULL, char_text, strlen(char_text)) + 1;

    wchar_t* text = (wchar_t*) calloc(sizeof(wchar_t*), buffer_size);
    mbstowcs(text, char_text, strlen(char_text));

    const token expected_tokens[] = {
        { BR }, name(L"push"), name(L"bx"), { COMMA }, name(L"ax"), { COMMA },
        { LSQUARE_BRACKET }, number(12), { RSQUARE_BRACKET }, { COMMA },
        { LSQUARE_BRACKET }, name(L"ax"), { RSQUARE_BRACKET }, { BR },
        name(L"mov"), name(L"ab"), { COMMA }, name(L"eax"), { BR }
    };

    simple_stack<token> tokens;
    simple_stack_create(&tokens);

    stack_trace* trace = tokenize(text, false, &tokens);

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
            ASSERT_EQUAL(wcsncmp(current.name.str, expected_tokens[i].name.str,
                                 expected_tokens[i].name.str_length), 0);
        else if (current.type == NUMBER)
            ASSERT_EPSILON_EQUAL(current.number, expected_tokens[i].number);
    }

    free(text), text = NULL;
}

TEST_MAIN()
