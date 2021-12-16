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
    

TEST(visualizer_of_dfa_test_0) {
    CREATE_STATE(state_a);
    CREATE_STATE(state_b);

    INSERT_CONNECTIONS(state_a, state_b, 'a', 'm');
    INSERT_CONNECTIONS(state_b, state_a, 'a', 'b', 'c', 'd');

    CREATE_STATE(state_c);
    INSERT_CONNECTIONS(state_a, state_c, 'l');

    INSERT_CONNECTIONS(state_c, state_c, 'u', 'q');

    CREATE_STATE(state_d);
    state_d->accept = 1;

    INSERT_CONNECTIONS(state_c, state_d, 'v');

    digraph graph = trie_vis_create_graph(state_a);

    char* output_name = digraph_render(&graph);
    printf("%s\n", output_name);

    free(output_name), output_name = NULL;

    trie_destroy(state_a);
    digraph_destroy(&graph);
}


TEST(visualizer_of_dfa_test_1) {
    CREATE_STATE(state_a);
    CREATE_STATE(state_b);

    INSERT_CONNECTIONS(state_a, state_b, 'a', 'b');

    INSERT_CONNECTIONS(state_b, state_b, 'b');

    CREATE_STATE(state_c);
    state_c->accept = 1;

    INSERT_CONNECTIONS(state_b, state_c, 'a');

    INSERT_CONNECTIONS(state_c, state_c, 'a');

    INSERT_CONNECTIONS(state_c, state_b, 'b');

    digraph graph = trie_vis_create_graph(state_a);

    char* output_name = digraph_render(&graph);
    printf("%s\n", output_name);

    free(output_name), output_name = NULL;

    trie_destroy(state_a);
    digraph_destroy(&graph);
}

TEST(test_3_visualizer) {
    trie* root = regex_parse("[abc][dfs]");

    digraph graph = trie_vis_create_graph(root);

    char* output_name = digraph_render(&graph);
    printf("%s\n", output_name);

    free(output_name), output_name = NULL;

    trie_destroy(root);
    digraph_destroy(&graph);
}


int main(void) {
    return test_framework_run_all_unit_tests();
}
