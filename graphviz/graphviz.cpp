#include "graphviz.h"

#include <cstdlib>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "trace.h"
#include "hash-table.h"
#include "default-hash-functions.h"
#include "printf-utils.h"

hash_table<int, const char*> graphviz_rank_names =
    HASH_TABLE(int, const char*, int_hash,
               PAIR(RANK_SAME           , "same"        ),
               PAIR(RANK_MAX            , "max"         ),
               PAIR(RANK_MIN            , "min"         ),
               PAIR(RANK_SOURCE         , "source"      ),
               PAIR(RANK_SINK           , "sink"        ));


hash_table<int, const char*> graphviz_colors =
    HASH_TABLE(int, const char*, int_hash,
               PAIR(GRAPHVIZ_RED        , "red"         ),
               PAIR(GRAPHVIZ_YELLOW     , "yellow"      ),
               PAIR(GRAPHVIZ_GREEN      , "green"       ),
               PAIR(GRAPHVIZ_BLUE       , "blue"        ),
               PAIR(GRAPHVIZ_BLACK      , "black"       ),
               PAIR(GRAPHVIZ_ORANGE     , "orange"      ));


hash_table<int, const char*> graphviz_styles =
    HASH_TABLE(int, const char*, int_hash,
               PAIR(STYLE_FILLED        , "filled"      ),
               PAIR(STYLE_ROUNDED       , "rounded"     ),
               PAIR(STYLE_DASHED        , "dashed"      ),
               PAIR(STYLE_DIAGONALS     , "diagonals"   ),
               PAIR(STYLE_INVIS         , "invis"       ),
               PAIR(STYLE_BOLD          , "bold"        ),
               PAIR(STYLE_DOTTED        , "dotted"      ),
               PAIR(STYLE_SOLID         , "solid"       ));


hash_table<int, const char*> graphviz_node_shapes =
    HASH_TABLE(int, const char*, int_hash,
               PAIR(SHAPE_BOX          , "box"          ),
               PAIR(SHAPE_POLYGON      , "polygon"      ),
               PAIR(SHAPE_ELLIPSE      , "ellipse"      ),
               PAIR(SHAPE_OVAL         , "oval"         ),
               PAIR(SHAPE_CIRCLE       , "circle"       ),
               PAIR(SHAPE_POINT        , "point"        ),
               PAIR(SHAPE_EGG          , "egg"          ),
               PAIR(SHAPE_TRIANGLE     , "triangle"     ),
               PAIR(SHAPE_PLAINTEXT    , "plaintext"    ),
               PAIR(SHAPE_PLAIN        , "plain"        ),
               PAIR(SHAPE_DIAMOND      , "diamond"      ),
               PAIR(SHAPE_TRAPEZIUM    , "trapezium"    ),
               PAIR(SHAPE_PARALLELOGRAM, "parallelogram"),
               PAIR(SHAPE_HOUSE        , "house"        ),
               PAIR(SHAPE_PENTAGON     , "pentagon"     ),
               PAIR(SHAPE_HEXAGON      , "hexagon"      ),
               PAIR(SHAPE_SEPTAGON     , "septagon"     ),
               PAIR(SHAPE_OCTAGON      , "octagon"      ),
               PAIR(SHAPE_DOUBLECIRCLE , "doublecircle" ),
               PAIR(SHAPE_DOUBLEOCTAGON, "doubleoctagon"),
               PAIR(SHAPE_TRIPLEOCTAGON, "tripleoctagon"),
               PAIR(SHAPE_INVTRIANGLE  , "invtriangle"  ),
               PAIR(SHAPE_INVTRAPEZIUM , "invtrapezium" ),
               PAIR(SHAPE_INVHOUSE     , "invhouse"     ));


digraph digraph_create() {
    digraph graph = {};

    const size_t default_subgraph_count = 3;
    TRY linked_list_create(&graph.subgraphs, default_subgraph_count)
        THROW("Linked list creation failed!");

    return graph;
}


subgraph* digraph_get_subgraph(digraph* graph, subgraph_id subgraph) {
    return &linked_list_get_pointer(&graph->subgraphs, subgraph)->element;
}


subgraph_id digraph_create_subgraph(digraph* graph, graphviz_rank_type rank) {
    subgraph new_subgraph = {};
    new_subgraph.rank = rank;

    // Create new subgraph, initialize it's edge and node lists

    const size_t default_edge_list_capacity = 10;
    TRY linked_list_create(&new_subgraph.edges, default_edge_list_capacity)
        THROW("Failed to create list of edges in a new subgraph!");

    const size_t default_node_list_capacity = 10;
    TRY linked_list_create(&new_subgraph.nodes, default_node_list_capacity)
        THROW("Failed to create list of nodes in a new subgraph!");

    // Insert new subgraph
    TRY linked_list_push_back(&graph->subgraphs, new_subgraph)
        THROW("Failed to add new subgraph to the list!");

    // We inserted to the end of the list, let's get that element:
    return linked_list_tail_index(&graph->subgraphs);
}


node_id subgraph_insert_node(digraph* graph, subgraph_id subgraph_pos, node new_node) {
    subgraph* current_subgraph = digraph_get_subgraph(graph, subgraph_pos);

    TRY linked_list_push_back(&current_subgraph->nodes, new_node)
        THROW("Failed to insert new node!");

    // We inserted to the end of the list, let's get that element:
    return linked_list_tail_index(&current_subgraph->nodes);
}

node vnode_from_default(node default_node, const char* format, va_list args) {
    // Default node is copied
    default_node.label = vsprintf_to_new_buffer(format, args);

    return default_node;
}

node  node_from_default(node default_node, const char* format, ...) {
    va_list args;
    va_start(args, format);

    node output_node = vnode_from_default(default_node, format, args);

    va_end(args);

    return output_node;
}

node_id subgraph_insert_default_node(digraph* graph, subgraph_id subgraph_pos,
                                     node default_node, const char* format, ...) {
    va_list args;
    va_start(args, format);

    node_id output_node = subgraph_insert_node(graph, subgraph_pos,
        vnode_from_default(default_node, format, args));

    va_end(args);

    return output_node;
}


void subgraph_insert_edge(digraph* graph, subgraph_id subgraph_pos, edge new_edge) {
    subgraph* current_subgraph = digraph_get_subgraph(graph, subgraph_pos);

    TRY linked_list_push_back(&current_subgraph->edges, new_edge)
        THROW("Failed to insert new edge!");
}


edge vedge_from_default(edge default_edge, node_id from, node_id to,
                        const char* format, va_list args) {

    // Default edge is copied
    default_edge.from = from;
    default_edge.to   = to;

    if (format != NULL)
        default_edge.label = vsprintf_to_new_buffer(format, args);

    return default_edge;
}

edge edge_from_default(edge default_edge, node_id from, node_id to,
                       const char* format, ...) {


    va_list args;
    va_start(args, format);

    edge new_edge = vedge_from_default(default_edge, from, to, format, args);

    va_end(args);

    return new_edge;
}

void subgraph_insert_default_edge(digraph* graph, subgraph_id subgraph_pos, edge default_edge,
                                  node_id node_from, node_id node_to, const char* format, ...) {

    va_list args;
    va_start(args, format);

    edge edge_to_insert = vedge_from_default(default_edge, node_from,
                                             node_to, format, args);

    va_end(args);

    subgraph_insert_edge(graph, subgraph_pos, edge_to_insert);
}


void subgraph_write_to_file(FILE* file, subgraph* graph) {
    fprintf(file, "\t" "subgraph {" "\n");

    const char** rank =
        hash_table_lookup(&graphviz_rank_names, (int) graph->rank);

    if (rank != NULL)
        fprintf(file, "\t\t" "rank = %s;" "\n", *rank);

    LINKED_LIST_TRAVERSE(&graph->nodes, node, current) {
        node* current_node = &current->element;

        const char* shape =
            *hash_table_lookup(&graphviz_node_shapes, (int) current_node->shape);

        const char* color =
            *hash_table_lookup(&graphviz_colors,      (int) current_node->color);

        const char* style =
            *hash_table_lookup(&graphviz_styles,      (int) current_node->style);

        node_id node_identity = linked_list_get_index(&graph->nodes, current);

        fprintf(file, "\t\t" "node_%d [" "label = \"%s\","
                "shape = \"%s\", color = \"%s\", style = \"%s\"];" "\n",
                node_identity, current_node->label, shape, color, style);
    }

    LINKED_LIST_TRAVERSE(&graph->edges, edge, current) {
        edge* current_edge = &current->element;

        node_id from_node_id = current_edge->from, to_node_id = current_edge->to;

        const char* color =
            *hash_table_lookup(&graphviz_colors, (int) current_edge->color);

        const char* style =
            *hash_table_lookup(&graphviz_styles, (int) current_edge->style);

        fprintf(file, "\t\t" "node_%d -> node_%d [label = \" %s \","
                "color = %s, style = %s, margin = \"1.5\"];" "\n",
                from_node_id, to_node_id, current_edge->label, color, style);

    }

    fprintf(file, "\t" "}"          "\n");
}

void digraph_write_to_file(FILE* file, digraph* graph) {
    fprintf(file, "digraph {" "\n");

    LINKED_LIST_TRAVERSE(&graph->subgraphs, subgraph, current)
        subgraph_write_to_file(file, &current->element);

    fprintf(file, "}" "\n");
}


void digraph_destroy(digraph *graph) {
    LINKED_LIST_TRAVERSE(&graph->subgraphs, subgraph, current) {
        LINKED_LIST_TRAVERSE(&current->element.nodes, node, current_node)
            free(current_node->element.label);

        linked_list_destroy(&current->element.nodes);

        LINKED_LIST_TRAVERSE(&current->element.edges, edge, current_edge)
            free(current_edge->element.label);

        linked_list_destroy(&current->element.edges);
    }

    linked_list_destroy(&graph->subgraphs),
        graph->subgraphs = {};
};


char* digraph_render(digraph* graph) {
    char* graph_tmp_name = tmpnam(NULL);

    FILE* tmp = fopen(graph_tmp_name, "w");
    digraph_write_to_file(tmp, graph);

    char dot_buffer[256] = {};
    strcat(dot_buffer, "dot -Tpng ");
    strcat(dot_buffer, graph_tmp_name);

    fclose(tmp), tmp = NULL;

    char* image_tmp_name = (char*)
        calloc(MAX_TMP_NAME_SIZE, sizeof(char));
    strcat(image_tmp_name, ".png");

    tmpnam(image_tmp_name);
    strcat(dot_buffer, " -o ");
    strcat(dot_buffer, image_tmp_name);

    system(dot_buffer);

    return image_tmp_name;
}

void digraph_render_and_destory(digraph* graph) {
    char* name = digraph_render(graph);
    char buffer[256] = {};

    strcat(buffer, "sxiv ");
    strcat(buffer, name);

    system(buffer);

    free(name), name = NULL;
    digraph_destroy(graph);
}
