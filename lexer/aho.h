#pragma once

#include "safe-alloc.h"
#include "trace.h"
#include "graphviz.h"
#include "hash-set.h"
#include "default-hash-functions.h"
#include "simple-stack.h"
#include "stdint.h"
#include "simple-stack.h"

struct trie {
    hash_table<char, trie*> transition;
    int accept;
};

inline stack_trace* trie_create(trie** output_trie) {
    trie* new_trie;

    TRY safe_calloc(1, &new_trie)
        FAIL("Failed to allocate memory for new trie!");

    new_trie->accept = -1;

    hash_table_create(&new_trie->transition, char_hash);
    *output_trie = new_trie;

    return SUCCESS();
}

struct regex_parser {
    const char* regex;
    int current_index;
};

inline char regex_parser_current(regex_parser* parser) {
    return parser->regex[parser->current_index];
}

inline char regex_parser_next(regex_parser* parser) {
    return parser->regex[parser->current_index ++];
}

inline void regex_parser_rewind(regex_parser* parser, int count) {
    parser->current_index -= count;
}

inline void trie_create_transition(char transition_char, trie* from, trie* to) {
    hash_table_insert(&from->transition, transition_char, to);
}

inline trie* regex_parse_one_of(trie* root, regex_parser* parser) {
    trie* next_state = NULL;
    TRY trie_create(&next_state) THROW("Failed to create trie!");

    char current = '\0';
    if    ((current = regex_parser_next(parser)) != '[') {
        // Single symbol detected
        trie_create_transition(current, root, next_state);
        return next_state;
    }

    while ((current = regex_parser_next(parser)) != ']')
        trie_create_transition(current, root, next_state);

    return next_state;
}

inline trie* regex_parse_group(trie* root, regex_parser* parser);

inline trie* regex_parse_expression(trie* root, regex_parser* parser);

inline uint32_t trie_pointer_hash(trie *graph) {
    return int_hash((int) (uintptr_t) graph);
}

inline void trie_collect_nodes(trie *graph, hash_set<trie*>* nodes) {
    if (graph == NULL || hash_set_contains(nodes, graph))
        return;

    // Insert current node, it will display as '*'
    hash_set_insert(nodes, graph);

    // Visit all the nodes nearby, and add them too
    HASH_TABLE_TRAVERSE(&graph->transition, char, trie*, current)
        trie_collect_nodes(VALUE(current), nodes);
}

inline void trie_replace_state(trie* graph, trie* from, trie* to) {
    hash_set<trie*> nodes;
    hash_set_create(&nodes, trie_pointer_hash);

    trie_collect_nodes(graph, &nodes);

    HASH_SET_TRAVERSE(&nodes, trie*, current) {
        bool affected_transition_table = false;
        HASH_TABLE_TRAVERSE(&SET_VALUE(current)->transition,
                            char, trie*, transition) {

            if (VALUE(transition) == from) {
                VALUE(transition) = to;

                // This breaks set's contracts!
                // Makes sure to rehash afterwards!
                affected_transition_table = true;
            }
        }

        // We changed values, so we need to rehash!
        if (affected_transition_table)
            hash_table_rehash_keep_size(&SET_VALUE(current)->transition);
    }

    hash_set_destroy(&nodes);
}


inline void trie_destroy_without_neighbours(trie* graph) {
    hash_table_destroy(&graph->transition);
    free(graph);
}

inline trie* regex_kleene_transform(trie* begin, trie* end) {
    trie_replace_state(begin, end, begin);
    trie_destroy_without_neighbours(end);

    return begin;
}

inline trie* regex_parse_group(trie* root, regex_parser* parser) {
    char current = '\0';
    if    ((current = regex_parser_next(parser)) != '(') {
        regex_parser_rewind(parser, 1);
        return regex_parse_one_of(root, parser);
    }

    // Parse many groups
    trie* result = regex_parse_expression(root, parser);

    if (regex_parser_next(parser) != ')')
        assert(0);

    return regex_kleene_transform(root, result);
}

inline trie* regex_parse_expression(trie* root, regex_parser* parser) {
    trie* current = root;
    while (true) {
        char current_symbol = regex_parser_current(parser);
        switch (current_symbol) {
        case  ')':
        case '\0':
            break; // Break out of a loop

        default:
            current = regex_parse_group(current, parser);
            continue;
        }

        break; // Break out of loop if finalizing symbol encourted
    }

    return current;
}

inline trie* regex_parse(const char* string) {
    trie* root = NULL;
    trie_create(&root);

    regex_parser parser = { string, 0 };
    regex_parse_expression(root, &parser);

    return root;
}
 
inline void trie_destroy(trie* graph) {
    if (graph == NULL)
        return;

    hash_set<trie*> nodes;
    hash_set_create(&nodes, trie_pointer_hash);

    trie_collect_nodes(graph, &nodes);

    HASH_SET_TRAVERSE(&nodes, trie*, current)
        trie_destroy_without_neighbours(SET_VALUE(current));

    hash_set_destroy(&nodes);
}
