#include "assembler.h"
#include "test-framework.h"

TEST(assembler_test) {
    const char* text = R"(
        push bx
        pop ab
    )";

    simple_stack<token> tokens;
    stack_trace* trace = tokenize(text, &tokens);

    command_t* commands = (command_t*) calloc(512, sizeof(*commands));

    assembly(tokens.elements, tokens.next_index, commands);

    simple_stack_destruct(&tokens);

}


TEST_MAIN()
