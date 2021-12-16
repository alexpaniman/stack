#include "graphviz.h"
#include "test-framework.h"

void create_tree(SUBGRAPH_CONTEXT, node_id current, int depth,
                 int max_depth, int branch_factor) {

    if (depth > max_depth)
        return;

    for (int i = 0; i < branch_factor; ++ i) {
        node_id node = NODE("%d", i);
        EDGE(current, node);

        create_tree(CURRENT_SUBGRAPH_CONTEXT, node, depth + 1,
                    max_depth, branch_factor);
    }
}

TEST(graphviz_test) {
    digraph my_graph = NEW_GRAPH({

        NEW_SUBGRAPH(RANK_NONE, {

            DEFAULT_NODE = {
                .style = STYLE_ROUNDED,
                .color = GRAPHVIZ_RED,
                .shape = SHAPE_BOX
            };

            DEFAULT_EDGE = {
                .color = GRAPHVIZ_ORANGE,
                .style = STYLE_SOLID,
            };

            node_id root = NODE("root");

            const int branch_factor = 3,
                      max_depth     = 2;

            create_tree(CURRENT_SUBGRAPH_CONTEXT, root, 0,
                        max_depth, branch_factor);

        });

    });

    printf("~~~~> %s\n", digraph_render(&my_graph));

    digraph_destroy(&my_graph);
}

int main(void) {
    return test_framework_run_all_unit_tests();
}
