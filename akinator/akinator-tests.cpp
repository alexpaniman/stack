#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "akinator.h"
#include "graphviz.h"
#include "test-framework.h"
#include "trace.h"

TEST(russian) {
    akinator_speak(L"Привет, жалкий человечешка!");
}

TEST(name_parsing) {
    parser parse = { L"Sample?", 0 };

    wchar_t* string = NULL;
    TRY akinator_parse_node_name(&parse, &string) ASSERT_SUCCESS();

    TEST_FINALIZER({
        free(string);
        string = NULL;
    })

    ASSERT_EQUAL(wcscmp(string, L"Sample?") == 0, true);

    CALL_TEST_FINALIZER()
}

TEST(tree_parsing_and_writing) {
    parser parse = { L"(Question? (Another Question? (Third? Answer 2. Answer 3) Answer 4) Leaf)", 0 };

    binary_tree<akinator_node>* akinator_tree = NULL;
    TEST_FINALIZER({
        akinator_destroy(akinator_tree);
    })

    TRY akinator_parse_tree(&parse, &akinator_tree) ASSERT_SUCCESS();
    digraph graph = akinator_visualize(akinator_tree);

    akinator_print_tree(stdout, akinator_tree);

    digraph_render_and_destory(&graph);

    CALL_TEST_FINALIZER();
}

// TEST(speak) {
//     freopen(NULL, "w", stdout);
//     akinator_speak(L"Hello\n");
//     freopen(NULL, "w", stdout);
// }

int main() {
    return test_framework_run_all_unit_tests();
}
