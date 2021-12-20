#pragma once

#include "linked-list.h"
#include "hash-table.h"
#include "trace.h"

/** Different node placements inside of a subgraph */
enum graphviz_rank_type {
    RANK_SAME,   RANK_MIN,  RANK_MAX,
    RANK_SOURCE, RANK_SINK, RANK_NONE
};

// Store graphviz's rank names
extern hash_table<int, const char*> graphviz_rank_names;


/** Colors that can be applied to nodes and edges  */
enum graphviz_color {
    GRAPHVIZ_RED,   GRAPHVIZ_BLUE,   GRAPHVIZ_GREEN,
    GRAPHVIZ_BLACK, GRAPHVIZ_YELLOW, GRAPHVIZ_ORANGE
};

// Store graphviz's color names
extern hash_table<int, const char*> graphviz_colors;


/** Styles that can be applied to nodes and edges */
enum graphviz_style {
    STYLE_FILLED,    STYLE_ROUNDED, STYLE_DASHED,
    STYLE_DIAGONALS, STYLE_INVIS,   STYLE_BOLD,
    STYLE_DOTTED,    STYLE_SOLID
};

// Store graphviz's style names
extern hash_table<int, const char*> graphviz_styles;


/** Various node shapes */
enum graphviz_node_shape {
    SHAPE_BOX,           SHAPE_POLYGON,       SHAPE_ELLIPSE,
    SHAPE_OVAL,          SHAPE_CIRCLE,        SHAPE_POINT,
    SHAPE_DOUBLECIRCLE,  SHAPE_DOUBLEOCTAGON, SHAPE_TRIPLEOCTAGON,
    SHAPE_INVTRIANGLE,   SHAPE_INVTRAPEZIUM,  SHAPE_INVHOUSE,
    SHAPE_EGG,           SHAPE_TRIANGLE,      SHAPE_PLAINTEXT,
    SHAPE_PLAIN,         SHAPE_DIAMOND,       SHAPE_TRAPEZIUM,
    SHAPE_PARALLELOGRAM, SHAPE_HOUSE,         SHAPE_PENTAGON,
    SHAPE_HEXAGON,       SHAPE_SEPTAGON,      SHAPE_OCTAGON
};

// Store graphviz's shape names
extern hash_table<int, const char*> graphviz_node_shapes;


struct node {
    graphviz_style style;
    graphviz_color color;
    graphviz_node_shape shape;

    char* label;
};

typedef element_index_t node_id;

struct edge {
    node_id from, to;

    graphviz_color color;
    graphviz_style style;

    char* label;
};

struct subgraph {
    linked_list<node> nodes;
    linked_list<edge> edges;

    graphviz_rank_type rank;
};

struct digraph {
    linked_list<subgraph> subgraphs;
};


digraph digraph_create();

typedef element_index_t subgraph_id;

/**
 * Create new subgraph and immediately insert it in a graph
 *
 * @return id of newly created subgraph
 */
subgraph_id digraph_create_subgraph(digraph* graph, graphviz_rank_type rank);


/**
 * Get subgraph by it's id in graph, this mostly used in graphviz's
 * functions and traversal macros
 * 
 * @return Pointer to subgraph identified by id
 *
 * @note Pointer to a subgraph shouldn't be used as a subgraph
 * identifier, because list of subgraphs in graph doesn't have
 * reference stability, buffer may be reallocated and subgraph moved
 */
subgraph* digraph_get_subgraph(digraph* graph, subgraph_id subgraph);

element_index_t subgraph_insert_node(digraph* graph, subgraph_id subgraph, node new_node);
void  subgraph_insert_edge(digraph* graph, subgraph_id subgraph, edge new_edge);

node node_from_default(node default_node, const char *format, ...);
edge edge_from_default(edge default_edge, node_id from, node_id to,
                       const char *format, ...);

/**
 * Insert new node with style inherited from default node
 *
 * @return value that identifies node and can be used to create edges
 */
node_id subgraph_insert_default_node(digraph* graph, subgraph_id subgraph,
                                     node default_node, const char* format, ...);

/**
 * Insert new edge that connects two nodes, identified by their id's
 */
void    subgraph_insert_default_edge(digraph* graph, subgraph_id subgraph, edge default_edge,
                                     node_id from, node_id to, const char* format, ...);

/**
 * Write to @arg file description of the @arg graph in graphviz dot's lang
 */
void  digraph_write_to_file(FILE* file,  digraph* graph);


char* digraph_render(digraph* graph);

void digraph_destroy(digraph* graph);

void digraph_render_and_destory(digraph* graph);


#define NEW_GRAPH(...) ({                                                               \
        digraph __current_graph = digraph_create();                                     \
        __VA_ARGS__                                                                     \
        __current_graph;                                                                \
    })

#define NEW_SUBGRAPH(rank, ...)                                                         \
    do {                                                                                \
        subgraph_id __current_subgraph =                                                \
            digraph_create_subgraph(&__current_graph, rank);                            \
                                                                                        \
        node __default_node = {};                                                       \
        edge __default_edge = {};                                                       \
        __VA_ARGS__                                                                     \
    } while(false)

#define DEFAULT_NODE                                                                    \
    __default_node

#define DEFAULT_EDGE                                                                    \
    __default_edge

#define NODE(...)                                                                       \
    subgraph_insert_default_node(&__current_graph, __current_subgraph,                  \
                                 __default_node, __VA_ARGS__)

#define EDGE(from, to)                                                                  \
    subgraph_insert_default_edge(&__current_graph, __current_subgraph,                  \
                                 __default_edge, from, to, "")

#define LABELED_EDGE(from, to, ...)                                                     \
    subgraph_insert_default_edge(&__current_graph, __current_subgraph,                  \
                                 __default_edge, from, to, __VA_ARGS__)

#define SUBGRAPH_CONTEXT                                                                \
    digraph __current_graph,                                                            \
    subgraph_id __current_subgraph,                                                     \
    node __default_node,                                                                \
    edge __default_edge

#define CURRENT_SUBGRAPH_CONTEXT                                                        \
    __current_graph, __current_subgraph, __default_node, __default_edge

#define TRAVERSE_NODES(current)                                                         \
    LINKED_LIST_TRAVERSE(&digraph_get_subgraph(&__current_graph,                        \
                                               __current_subgraph)->nodes,              \
                         node, current)

#define NODE_ID(num_defined) ({                                                         \
        node_id __defined_node_id = linked_list_end_index;                              \
        TRY linked_list_get_logical_index(&digraph_get_subgraph(&__current_graph,       \
                                                                __current_subgraph)     \
                                          ->nodes, num_defined, &__defined_node_id)     \
            THROW("Failed to determine element by it's logical index!");                \
        __defined_node_id;                                                              \
    })
