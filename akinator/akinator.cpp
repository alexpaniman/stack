#include <cstdarg>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

#include "akinator.h"
#include "graphviz.h"
#include "safe-alloc.h"
#include "textlib.h"
#include "binary-tree.h"
#include "trace.h"
#include "simple-stack.h"

wchar_t parser_get_next(parser* parse) {
    return parse->string[parse->index ++];
}

void parser_rewind(parser* parse, int num) {
    parse->index -= num;
}

wchar_t parser_get_current(parser* parse) {
    return parse->string[parse->index];
}

void create_parser(parser* parse, wchar_t* string) {
    *parse = { string, 0 };
}

void akinator_speak(const wchar_t* format, ...) {
    va_list args; va_start(args, format);

    static char* file_name = tmpnam(NULL);
    FILE* festival_output = fopen(file_name, "w");

    va_list args_copy; va_copy(args_copy, args);
    vfwprintf(festival_output, format, args_copy);
    va_end(args_copy);

    // Buffer large enough to store call to festival
    char buffer[256] = "festival --tts ";
    strcat(buffer, file_name);
    strcat(buffer, " & disown");

    fclose(festival_output), festival_output = NULL;
    system(buffer);

    vwprintf(format, args);
    va_end(args);
}

stack_trace* akinator_parse_node_name(parser* parse, wchar_t** new_string) {
    const wchar_t* begining = parse->string + parse->index;

    size_t length = 0;
    wchar_t current = '\0';
    while ((current = parser_get_next(parse)) != L'\0') {
        if (current == '(' || current == ')') {
            // Rewind '(' or ')' char for recognition in
            // the consequent parser that called current
            parser_rewind(parse, 1);

            // Brackets are not allowed, break out of the loop
            break;
        }

        if (current == '.')
            // Points are used as a leaf ending mark
            break;

        ++ length;

        if (current == '?')
            // Question marks are used as question separators
            // But instead of '.' they show up in the actual graph
            break;
    }

    TRY safe_calloc(length + 1, new_string)
        FAIL("New string allocation failed!");

    wcsncpy(*new_string, begining, length);
    return SUCCESS();
}

bool akinator_node_equality(const akinator_node* first,
                            const akinator_node* second) {

    return wcscmp(*first, *second) == 0;
}

akinator_tree akinator_create(akinator_node node) {
    return binary_tree_create<akinator_node>(node);
}

stack_trace* akinator_parse_tree(parser* parse, akinator_tree* tree) {
    while (parser_get_current(parse) == L' ')
        ++ parse->index;

    wchar_t* string = NULL;

    bool has_children = false;
    if (parser_get_next(parse) == '(')
        has_children = true;
    else {
        // Rewind symbol shifted with /get_next/
        parser_rewind(parse, 1);
    }

    TRY akinator_parse_node_name(parse, &string)
        FAIL("Unable to parse node's name!");

    *tree = akinator_create(string);

    if (!has_children)
        return SUCCESS();

    TRY akinator_parse_tree(parse, &(*tree)->left)
        FAIL("Left node parsing failed!");

    TRY akinator_parse_tree(parse, &(*tree)->right)
        FAIL("Right node parsing failed!");

    if (parser_get_next(parse) != ')')
        return FAILURE(RUNTIME_ERROR, "Unexpected symbol: ')'");

    return SUCCESS();
}


static node_id akinator_visualize_node(SUBGRAPH_CONTEXT, akinator_tree tree) {
    node_id root = NODE("%ls", tree->element);

    if (tree->left != NULL) {
        node_id left = akinator_visualize_node(CURRENT_SUBGRAPH_CONTEXT, tree->left);
        LABELED_EDGE(root, left, "Yes");
    }

    if (tree->right != NULL) {
        node_id right = akinator_visualize_node(CURRENT_SUBGRAPH_CONTEXT, tree->right);
        LABELED_EDGE(root, right, "No");
    }

    return root;
}

void akinator_print_tree(FILE* file, akinator_tree tree) {
    if (binary_tree_is_leaf(tree)) {
        fwprintf(file, L"%ls", tree->element);
        return;
    }

    fwprintf(file, L"(%ls ", tree->element);
    // Ends in '?', which is recognised as a separator

    akinator_print_tree(file, tree->left);
    if (binary_tree_is_leaf(tree->left))
        fwprintf(file, L".");

    fwprintf(file, L" ");
    // Point '.' separates left and right subtrees

    akinator_print_tree(file, tree->right);
    fwprintf(file, L")");
    // Last subtree is separated automatically by ')'
}

digraph akinator_visualize(akinator_tree tree) {
    return NEW_GRAPH({
        NEW_SUBGRAPH(RANK_NONE, {
            DEFAULT_NODE = {
                .style = STYLE_ROUNDED,
                .color = GRAPHVIZ_BLACK,
                .shape = SHAPE_BOX
            };

            DEFAULT_EDGE = {
                .color = GRAPHVIZ_ORANGE,
                .style = STYLE_SOLID
            };

            akinator_visualize_node(CURRENT_SUBGRAPH_CONTEXT, tree);
        });
    });
}

binary_tree<wchar_t*>* akinator_read_tree(const char* file_name) {
    wchar_t* text = NULL;
    read_file(file_name, &text);

    parser parse = { text, 0 };

    binary_tree<wchar_t*>* tree = NULL;
    akinator_parse_tree(&parse, &tree);

    free(text), text = NULL;
    return tree;
}

void akinator_write_tree(const char* file_name, akinator_tree tree) {
    FILE* file = fopen(file_name, "w");
    akinator_print_tree(file, tree);
    fclose(file), file = NULL;
}

void akinator_describe(akinator_tree tree, wchar_t* object) {
    binary_tree_path_t<akinator_node> path;
    simple_stack_create(&path);

    binary_tree_search(tree, object, &path, akinator_node_equality);
    SIMPLE_STACK_TRAVERSE(&path, path_node<wchar_t*>, current) {
        path_node<wchar_t*> step = SIMPLE_STACK_VALUE(current);

        // Print question
        wprintf(L"==>");
        akinator_speak(L" %ls ", *step.node_value);

        // And it's answer
        switch (step.how_to_get_here) {
        case GO_LEFT:
            akinator_speak(L"Yes!");
            break;

        case GO_RIGHT:
            akinator_speak(L"No!");
            break;
        }

        // And move to the next line
        wprintf(L"\n");
    }


    simple_stack_destruct(&path);
}

#define MIN(a, b)              \
    ({  __typeof__(a) __a = a; \
        __typeof__(b) __b = b; \
        __a < __b ? __a : __b; })

void akinator_difference(akinator_tree tree, wchar_t* first, wchar_t* second) {
    binary_tree_path_t<akinator_node> first_path, second_path;
    simple_stack_create(& first_path);
    binary_tree_search(tree, first,  & first_path, akinator_node_equality);

    simple_stack_create(&second_path);
    binary_tree_search(tree, second, &second_path, akinator_node_equality);

    const size_t min_depth = MIN(first_path.next_index,
                                 second_path.next_index);

    for (size_t i = 0; i < min_depth; ++ i) {
        path_node<akinator_node> *first_node = &first_path.elements[i],
            *second_node = &second_path.elements[i];

        if (first_node->how_to_get_here != second_node->how_to_get_here) {
            akinator_speak(L"%ls It is true for ", *first_node->node_value);

            if (first_node->how_to_get_here == GO_LEFT)
                akinator_speak(L"\"%ls\", but false for \"%ls\"", first, second);
            else
                akinator_speak(L"\"%ls\", but false for \"%ls\"", second, first);

            break; // Still need to free resources
        }
    }

    simple_stack_destruct(& first_path);
    simple_stack_destruct(&second_path);
}

enum yes_or_no_answer { UNRECOGNISED, YES, NO };

yes_or_no_answer ask_yes_or_no(void) {
    akinator_speak(L"Answer yes or no: ");

    // Longest legal input is yes (3 symbols long)
    // This needs 4 characters long array (including '\0')
    // But if user writes "yes[consequent non space symbol]"
    // We want to be able to tell, that input is wrong, so
    // we need one more symbol in the buffer.
    wchar_t answer[5];

    // Max input length is 4 (check comment above)
    wscanf(L"%4ls", answer);

    if (wcscmp(answer, L"yes") == 0)
        return YES;

    if (wcscmp(answer, L"no") == 0)
        return NO;

    return UNRECOGNISED;
}

bool ask_yes_or_no_do_not_accept_other(void) {
    yes_or_no_answer answer = UNRECOGNISED;

    while ((answer = ask_yes_or_no()) == UNRECOGNISED)
        wprintf(L"Illegal answer, please, try again!\n");

    switch (answer) {
    case YES:
        return true;

    case NO:
        return false;

    case UNRECOGNISED:
        assert(false && "Should be unreachable!");
    }
}

wchar_t* read_string() {
    wchar_t* new_name = NULL;
    wscanf(L" %ml[^\n]s", &new_name);

    return new_name;
}

bool akinator_validate_question(wchar_t* question) {
    wchar_t current = '\0';
    while ((current = *(question ++)) != '\0') {
        if (current == '.') {
            akinator_speak(L"Question shouldn't contain points!\n");
            return false;
        }

        if (*question == '\0') {
            if (current != '?') {
                akinator_speak(L"Question should end with '?'!\n");
                return false;
            }

            return true;
        }

        if (current == '?') {
            akinator_speak(L"Question shouldn't contain '?' in the middle!\n");
            return false;
        }
    }

    return true;
}

void akinator_register_new(akinator_tree tree) {
    akinator_speak(L"What was it? ");
    wchar_t* new_object = read_string();

    wchar_t* difference_quesiton = NULL;
    do {
        akinator_speak(L"Write question that distinguishes \"%ls\" from \"%ls\":\n",
                       new_object, tree->element);
        difference_quesiton = read_string();
    } while(!akinator_validate_question(difference_quesiton));

    tree->left  = akinator_create(new_object);
    tree->right = akinator_create(tree->element);

    tree->element = difference_quesiton;
}

void akinator_ask_and_define(akinator_tree tree) {
    akinator_speak(L"What should I define? ");

    wchar_t* object = read_string();
    akinator_describe(tree, object);

    free(object), object = NULL;
}

void akinator_ask_and_find_difference(akinator_tree tree) {
    akinator_speak(L"Enter  first object to compare? ");
    wchar_t* first_object = read_string();

    akinator_speak(L"Enter second object to compare? ");
    wchar_t* second_object = read_string();

    akinator_difference(tree, first_object, second_object);

    free( first_object),  first_object = NULL;
    free(second_object), second_object = NULL;
}

void akinator_play(akinator_tree tree) {
    akinator_tree current = tree;

    while (current != NULL) {
        if (binary_tree_is_leaf(current)) {
            akinator_speak(L"My guess is: \"%ls\"\n", current->element);
            akinator_speak(L"Am I right? ");

            if (ask_yes_or_no_do_not_accept_other())
                akinator_speak(L"I won! Again!\n");
            else
                akinator_register_new(current);
            return;
        }

        akinator_speak(L"%ls\n", current->element);
        if (ask_yes_or_no_do_not_accept_other())
            current = current->left;
        else
            current = current->right;
    }
}

static void akinator_destory_strings(akinator_tree tree) {
    if (tree == NULL)
        return;

    free(tree->element);

    if (tree->left  != NULL)
        akinator_destory_strings(tree->left);

    if (tree->right != NULL)
        akinator_destory_strings(tree->right);
}

void akinator_destroy(akinator_tree tree) {
    akinator_destory_strings(tree);
    binary_tree_destroy(tree);
}

game_mode read_game_mode(void) {
    akinator_speak(L"There's three game modes:"             "\n"
                   L"    (%d) -> play mode (main mode)"     "\n"
                   L"    (%d) -> definition mode"           "\n"
                   L"    (%d) -> difference searching mode" "\n"
                   L"What should I do? ",
                   MODE_PLAY, MODE_DEFINE, MODE_COMPARE);

    int number = -1; wscanf(L"%d", &number);
    if (number < MODE_PLAY || number > MODE_COMPARE)
        akinator_speak(L"Illegal input!");

    return (game_mode) number;
}
