#include "dfs-visualizer.h"
#include "safe-alloc.h"
#include <simple-stack.h>

void trie_vis_create_nodes(SUBGRAPH_CONTEXT, trie *graph, char name,
                           hash_table<trie *, node_id> *nodes) {

    if (graph == NULL || hash_table_contains(nodes, graph))
        return;

    if (graph->accept != -1)
        DEFAULT_NODE.color = GRAPHVIZ_RED;
    else
        DEFAULT_NODE.color = GRAPHVIZ_BLACK;

    // Insert current node, it will display as '*'
    hash_table_insert(nodes, graph, NODE("%c", name ++));

    // Visit all the nodes nearby, and add them too
    HASH_TABLE_TRAVERSE(&graph->transition, char, trie*, current)
        trie_vis_create_nodes(CURRENT_SUBGRAPH_CONTEXT, VALUE(current), name, nodes);
}

static inline
void print_range(simple_stack<char>* string_builder, char previous, int streak_length) {
    if (streak_length == 1) {
        simple_stack_push(string_builder, previous);
        return;
    }

    simple_stack_push(string_builder, (char)
                      (previous - streak_length + 1));
    simple_stack_push(string_builder, '-');
    simple_stack_push(string_builder, previous);
}

static inline
char* trie_vis_create_transition_description(linked_list<char>* list) {
    simple_stack<char> string_builder;
    simple_stack_create(&string_builder);

    if (list->used > 1)
        simple_stack_push(&string_builder, '[');

    int streak_length = 0;
    char previous = linked_list_head(list)->element - 1;

    LINKED_LIST_TRAVERSE(list, char, current) {
        char symbol = current->element;
        if (symbol - previous == 1)
            ++ streak_length;
        else {
            print_range(&string_builder, previous, streak_length);
            streak_length = 1;
        }

        previous = symbol;
    }

    print_range(&string_builder, previous, streak_length);

    if (list->used > 1)
        simple_stack_push(&string_builder, ']');

    simple_stack_push(&string_builder, '\0');

    return string_builder.elements;
}

void trie_vis_create_edges(SUBGRAPH_CONTEXT, trie* graph,
                           hash_table<trie*, node_id>* nodes,
                           hash_set<trie*>* visited_nodes) {

    if (hash_set_contains(visited_nodes, graph))
        return;

    hash_set_insert(visited_nodes, graph);

    hash_table<trie*, linked_list<char>> connections;
    hash_table_create(&connections, trie_pointer_hash);

    HASH_TABLE_TRAVERSE(&graph->transition, char, trie*, current) {
        linked_list<char>* list_of_paths =
            hash_table_lookup(&connections, VALUE(current));

        if (list_of_paths == NULL) {
            linked_list<char> new_list = {}; // Useful only when there's no paths
            const size_t default_connections_size = 10;

            linked_list_create(&new_list, default_connections_size);

            // Copy the list over to the connections hash_table
            hash_table_insert(&connections, VALUE(current), new_list);
            list_of_paths = hash_table_lookup(&connections, VALUE(current));
        }

        linked_list_push_back(list_of_paths, KEY(current));
    }

    node_id from = *hash_table_lookup(nodes, graph);
    HASH_TABLE_TRAVERSE(&connections, trie*, linked_list<char>, current) {
        node_id to = *hash_table_lookup(nodes, KEY(current));

        char* edge_description =
            trie_vis_create_transition_description(&VALUE(current));

        LABELED_EDGE(from, to, "%s", edge_description);
        free(edge_description), edge_description = NULL;

        trie_vis_create_edges(CURRENT_SUBGRAPH_CONTEXT,
                              KEY(current), nodes, visited_nodes);
    }

    HASH_TABLE_TRAVERSE(&connections, trie*, linked_list<char>, current)
        linked_list_destroy(&VALUE(current));

    hash_table_destroy(&connections);
}

void trie_vis_create_graph(SUBGRAPH_CONTEXT, trie* graph) {
    hash_table<trie*, node_id> nodes;
    hash_table_create(&nodes, trie_pointer_hash);

    // Declare all the nodes in the graph
    trie_vis_create_nodes(CURRENT_SUBGRAPH_CONTEXT, graph, 'A', &nodes);

    hash_set<trie*> visited_nodes;
    hash_set_create(&visited_nodes, trie_pointer_hash);

    // Create all the edges in the nodes
    trie_vis_create_edges(CURRENT_SUBGRAPH_CONTEXT, graph,
                            &nodes, &visited_nodes);

    // Free all the memory we used so far
    hash_table_destroy(&nodes), nodes = {};
    hash_set_destroy(&visited_nodes), visited_nodes = {};
}

digraph trie_vis_create_graph(trie* graph) {
    return NEW_GRAPH({
        NEW_SUBGRAPH(RANK_NONE, {
            DEFAULT_NODE = {
                .style = STYLE_BOLD,
                .color = GRAPHVIZ_BLACK,
                .shape = SHAPE_CIRCLE,
            };

            DEFAULT_EDGE = {
                .color = GRAPHVIZ_BLACK,
                .style = STYLE_SOLID
            };

            trie_vis_create_graph(CURRENT_SUBGRAPH_CONTEXT, graph);
        });
    });
}
