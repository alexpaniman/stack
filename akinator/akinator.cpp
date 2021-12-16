#include <simple-stack.h>
#include <wchar.h>
#include <stdio.h>

#include "akinator.h"
#include "textlib.h"

stack_trace* parse_node(simple_stack<token>* tokens, akinator_tree** resulting_node) {
    if (!token_is(simple_stack_pop(tokens), LSQUARE_BRACKET))
        return FAILURE(RUNTIME_ERROR, "Illegal node tree! Expected '['!");

    token question_or_name = simple_stack_pop(tokens);
    substring name = question_or_name.name;

    // Terminate string, we can do it because it will replace '[' or ']' symbol
    name.str[name.str_length] = L'\0';

    *resulting_node = binary_tree_create_leaf(name.str);

    token bracket = simple_stack_pop(tokens);
    if (token_is(bracket, RSQUARE_BRACKET))
        return SUCCESS();
    
    if (token_is(bracket, LSQUARE_BRACKET)) {
        parse_node(tokens, &(*resulting_node)->left);
        parse_node(tokens, &(*resulting_node)->right);
        return SUCCESS();
    }

    return FAILURE(RUNTIME_ERROR, "Illegal node tree! Expected ']'!");
}

stack_trace* load_tree(wchar_t* text, akinator_tree** tree) {
    simple_stack<token> tokens;
    simple_stack_create(&tokens);

    FINALIZER(tokens_finalizer, {
        simple_stack_destruct(&tokens);
    })

    TRY tokenize(text, true /* Allow spaces */, &tokens)
        FINALIZE_AND_FAIL(tokens_finalizer, "Unable to tokenize read input!");

    simple_stack_reverse(&tokens);

    TRY parse_node(&tokens, tree)
        FINALIZE_AND_FAIL(tokens_finalizer, "Unable to parse tree!");

    CALL_FINALIZER(tokens_finalizer);
    return SUCCESS();
}

void print_tree(akinator_tree* tree) {
    if (tree == EMPTY_NODE<akinator_node>)
        return;

    printf("[%ls ", tree->element);
    print_tree(tree->left);
    printf(" ");
    print_tree(tree->right);
    printf("]");
}


game_mode read_game_mode(void) {
    wprintf(L"Выберите режим игры: \n"
            L"    (%d) -> режим игры (основной)\n"
            L"    (%d) -> режим определения    \n"
            L"    (%d) -> режим поиска отличий \n",
            MODE_PLAY, MODE_DEFINE, MODE_COMPARE);

    int number = -1; wscanf(L"%d", &number);
    if (number < MODE_PLAY || number > MODE_COMPARE)
        printf("Неверный ввод!");

    return (game_mode) number;
}
