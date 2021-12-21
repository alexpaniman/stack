#include "graphviz.h"
#include "test-framework.h"
#include "dfs-visualizer.h"

#define INSERT_CONNECTIONS(from, to, ...)                                    \
    do {                                                                     \
        char __keys_content[] = { __VA_ARGS__ };                             \
        size_t __size =  sizeof(__keys_content) / sizeof (*__keys_content);  \
                                                                             \
        for (size_t i = 0; i < __size; ++ i)                                 \
            hash_table_insert(&(from)->transition, __keys_content[i], (to)); \
    } while (false)

#define CREATE_STATE(name )                                                  \
    trie *name = NULL; /* Can't wrap it in do/while, variable declaration */ \
    TRY trie_create(&name) ASSERT_SUCCESS();                                 \

void render_graph_and_print_file(trie* root) {
    digraph graph = trie_vis_create_graph(root);
    digraph_render_and_destory(&graph);
}

// TEST(visualizer_of_dfa_test_0) {
//     CREATE_STATE(state_a);
//     CREATE_STATE(state_b);

//     INSERT_CONNECTIONS(state_a, state_b, 'a', 'm');
//     INSERT_CONNECTIONS(state_b, state_a, 'a', 'b', 'c', 'd');

//     CREATE_STATE(state_c);
//     INSERT_CONNECTIONS(state_a, state_c, 'l');

//     INSERT_CONNECTIONS(state_c, state_c, 'u', 'q');

//     CREATE_STATE(state_d);
//     state_d->accept = 1;

//     INSERT_CONNECTIONS(state_c, state_d, 'v');

//     render_graph_and_print_file(state_a);
// }


// TEST(visualizer_of_dfa_test_1) {
//     CREATE_STATE(state_a);
//     CREATE_STATE(state_b);

//     INSERT_CONNECTIONS(state_a, state_b, 'a', 'b');

//     INSERT_CONNECTIONS(state_b, state_b, 'b');

//     CREATE_STATE(state_c);
//     state_c->accept = 1;

//     INSERT_CONNECTIONS(state_b, state_c, 'a');

//     INSERT_CONNECTIONS(state_c, state_c, 'a');

//     INSERT_CONNECTIONS(state_c, state_b, 'b');

//     render_graph_and_print_file(state_a);
// }

TEST(test_2_visualizer) {
    raw_trie* root = NULL;
    raw_trie_create(&root);

    regex_parser parser0 = { "[_fi]([123abc])", 0 };
    regex_parse_expression(root, &parser0);

    regex_parser parser1 = { "for", 0 };
    regex_parse_expression(root, &parser1);

    regex_parser parser2 = { "if", 0 };
    regex_parse_expression(root, &parser2);

    trie* result = NULL; trie_nfsm_to_dfsm(root, &result);
    render_graph_and_print_file(result);
}

TEST(test_3_visualizer) {
    raw_trie* root = regex_parse("a(a)a(a)([abc]m)(aba)");

    trie* result = NULL; trie_nfsm_to_dfsm(root, &result);
    render_graph_and_print_file(result);
}


int main(void) {
    return test_framework_run_all_unit_tests();
}
