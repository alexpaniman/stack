#pragma once

#include "hash-table.h"
#include "safe-alloc.h"
#include "trace.h"
#include "graphviz.h"
#include "hash-set.h"
#include "default-hash-functions.h"
#include "simple-stack.h"
#include "stdint.h"
#include "simple-stack.h"
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <linked-list.h>

// TODO
struct trie {
    hash_table<char, trie*> transition;
    int accept;
};


typedef int generic_token_t;

// Structure created to store non-deterministic finite
// state automata that is meant for lexer building, and
// should be compiled to deterministic finite state
// automata before it can be used to perform tokenization
struct raw_trie {
    // Each symbol in a nfsm can correspond to one or
    // more "states", which are represented by a trie
    // and stored in a linked list accordingly:
    hash_table<char, hash_set<raw_trie*>> transitions;

    // List of tokens that are considered accepted in
    // the current state of the trie:
    linked_list<generic_token_t> accept;
};

inline uint32_t raw_trie_hash(raw_trie* target_trie) {
    // Raw tries should be unique, so their
    // hash can be computed with a pointer:

    return int_hash((int) (uintptr_t) target_trie);
    //              ^~~~~ ^~~~~~~~~~~     
    // Perform two stage casting, because compiler wouldn't
    // let us to cast straight to int from raw trie pointer
}


inline uint32_t raw_trie_set_hash(hash_set<raw_trie*> tries) {
    uint32_t hash = 0;

    // Combine hashes of all raw tries in the set
    HASH_SET_TRAVERSE(&tries, raw_trie*, current) {
        raw_trie* transition_trie = SET_VALUE(current);
        hash += raw_trie_hash(transition_trie);
    }

    return hash;
}

inline stack_trace* raw_trie_create(raw_trie** output_trie) {
    raw_trie* new_trie = NULL;

    // Allocate space for a new raw trie node
    TRY safe_calloc(1, &new_trie)
        FAIL("Failed to allocate memory for new raw trie!");

    // Initialize list of accepted states
    TRY linked_list_create(&new_trie->accept)
        FAIL("List of accepted states creation failed!");

    // Initialize transition map
    TRY hash_table_create(&new_trie->transitions, char_hash)
        FAIL("Transition map creation failed!");

    *output_trie = new_trie;
    return SUCCESS();
}

static inline
void raw_trie_create_transition(char transition_char, raw_trie* from, raw_trie* to) {
    if (!hash_table_contains(&from->transitions, transition_char)) {
        // Let's create local transition set on a stack
        hash_set<raw_trie*> new_raw_trie = {}; 
        hash_set_create(&new_raw_trie, raw_trie_hash);

        // And copy it over to a transition hash table:
        hash_table_insert(&from->transitions, transition_char, new_raw_trie);
    }

    // We need to lookup set in a table again in any case, because it
    // gets copied there and using local copy on a stack won't work:
    hash_set<raw_trie*>* target_tries =
        hash_table_lookup(&from->transitions, transition_char);

    // And finally, insert our target trie to the transition set
    hash_set_insert(target_tries, to);
}

// TODO: Should probably aggregate "parsers" from all projects and
// separate them in form of a convenient static library

// Basic structure that encapsulates parsing of a regex expression
struct regex_parser {
    const char* regex;
    int current_index;
};

static inline
char regex_parser_current(regex_parser* parser) {
    return parser->regex[parser->current_index];
}

static inline
char regex_parser_next(regex_parser* parser) {
    return parser->regex[parser->current_index ++];
}

static inline
void regex_parser_rewind(regex_parser* parser, int count) {
    parser->current_index -= count;
}

static inline
raw_trie* regex_parse_one_of(raw_trie* root, regex_parser* parser) {
    raw_trie* next_state = NULL;
    TRY raw_trie_create(&next_state)
        THROW("Failed to create raw trie for a new state!");

    char current = '\0';
    if    ((current = regex_parser_next(parser)) != '[') {
        // Single symbol detected, add simple transition:
        raw_trie_create_transition(current, root, next_state);
        return next_state;
    }

    // Squarely bracketed expression detected, parse it
    // until right ']', creating transition for every char
    while ((current = regex_parser_next(parser)) != ']')
        raw_trie_create_transition(current, root, next_state);

    // Function finishes with parser set to the symbol after ']':
    return next_state;
}

inline raw_trie* regex_parse_group(raw_trie* root, regex_parser* parser);

inline raw_trie* regex_parse_expression(raw_trie* root, regex_parser* parser);

inline void raw_trie_collect_nodes(raw_trie *target_trie, hash_set<raw_trie*>* nodes) {
    // If we have already visited current node, skip it:
    if (target_trie == NULL || hash_set_contains(nodes, target_trie))
        return;

    // Insert current node, before continuing, to avoid inf loops
    hash_set_insert(nodes, target_trie);

    // Visit all the nodes nearby, and add them too
    HASH_TABLE_TRAVERSE(&target_trie->transitions, char,
                        hash_set<raw_trie*>, current) {

        // Traverse every group of transitions, and collect them:
        HASH_SET_TRAVERSE(&VALUE(current), raw_trie*, nested)
            raw_trie_collect_nodes(SET_VALUE(nested), nodes);
    }
}

inline void raw_trie_replace_state(raw_trie* target_trie, raw_trie* from, raw_trie* to) {
    // Initialize list, that will soon contain all the states
    hash_set<raw_trie*> nodes;
    hash_set_create(&nodes, raw_trie_hash);

    // Collect all trie states to it
    raw_trie_collect_nodes(target_trie, &nodes);

    HASH_SET_TRAVERSE(&nodes, raw_trie*, current) {
        // Let's avoid to destruction and reallocation in
        // case we didn't make any changes, for speed:
        bool affected_transition_table = false;

        // Traverse all the transition characters
        HASH_TABLE_TRAVERSE(&SET_VALUE(current)->transitions,
                            char, hash_set<raw_trie*>, target) {

            if (hash_set_delete(&VALUE(target), from)) {
                hash_set_insert(&VALUE(target), to);

                // This breaks table's contracts!
                // Makes sure to rehash afterwards!
                affected_transition_table = true;
            }
        }

        // We changed values, so we need to rehash!
        if (affected_transition_table)
            hash_table_rehash_keep_size(&SET_VALUE(current)->transitions);
    }

    hash_set_destroy(&nodes);
}


inline void raw_trie_destroy_without_neighbours(raw_trie* graph) {
    // Dangerous?
    hash_table_destroy(&graph->transitions);
    free(graph);
}

inline raw_trie* regex_kleene_transform(raw_trie* begin, raw_trie* end) {
    raw_trie_replace_state(begin, end, begin);
    raw_trie_destroy_without_neighbours(end);

    return begin;
}

inline raw_trie* regex_parse_group(raw_trie* root, regex_parser* parser) {
    char current = '\0';
    if    ((current = regex_parser_next(parser)) != '(') {
        regex_parser_rewind(parser, 1);
        return regex_parse_one_of(root, parser);
    }

    // Parse many groups
    raw_trie* result = regex_parse_expression(root, parser);

    if (regex_parser_next(parser) != ')')
        assert(0);

    return regex_kleene_transform(root, result);
}

inline raw_trie* regex_parse_expression(raw_trie* root, regex_parser* parser) {
    raw_trie* current = root;
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

inline stack_trace* trie_create(trie** output_trie) {
    trie* new_trie = NULL;

    // Allocate space for a new raw trie node
    TRY safe_calloc(1, &new_trie)
        FAIL("Failed to allocate memory for new raw trie!");

    // Initialize list of accepted states
    /* TRY linked_list_create(&new_trie->accept) */
    /*     FAIL("List of accepted states creation failed!"); */

    // Initialize transition map
    TRY hash_table_create(&new_trie->transition, char_hash)
        FAIL("Transition map creation failed!");

    *output_trie = new_trie;
    return SUCCESS();
}

inline void trie_create_link(trie* from, trie* to, char transition) {
    hash_table_insert(&from->transition, transition, to);
}

inline stack_trace* trie_nfsm_to_dfsm_recursion(raw_trie* nfsm, trie* root,
        hash_table<hash_set<raw_trie*>, trie*>* replaced_states) {

    HASH_TABLE_TRAVERSE(&nfsm->transitions, char, hash_set<raw_trie*>, current) {
        trie** found_state = hash_table_lookup(replaced_states, VALUE(current));

        trie* new_state = NULL;
        trie_create(&new_state);

        if (found_state == NULL) {
            raw_trie* new_raw_state = NULL;
            TRY raw_trie_create(&new_raw_state) FAIL("Raw trie creation failed!");

            hash_table_insert(replaced_states, VALUE(current), new_state);

            // For every set that can be reached from current letter
            HASH_SET_TRAVERSE(&VALUE(current), raw_trie*, current_set) {
                // Take all the letters, and for each of them
                HASH_TABLE_TRAVERSE(&SET_VALUE(current_set)->transitions, char,
                                    hash_set<raw_trie*>, subsequent_transition) {
                    // Union sets of transitions
                    HASH_SET_TRAVERSE(&VALUE(subsequent_transition), raw_trie*, subsequent_set)
                        raw_trie_create_transition(KEY(subsequent_transition), new_raw_state,
                                                   SET_VALUE(subsequent_set));
                }
            }

            // TODO: free new raw state

            // Now transform resulting node recursively, and write result to /new_state/
            trie_nfsm_to_dfsm_recursion(new_raw_state, new_state, replaced_states);
        } else
            new_state = *found_state;

        // Linke /new_state/ to the current node
        trie_create_link(root, new_state, KEY(current));
    }

    return SUCCESS();
}

inline stack_trace* trie_nfsm_to_dfsm(raw_trie* nfsm, trie** new_trie) {
    hash_table<hash_set<raw_trie*>, trie*> replaced_states;

    FINALIZER(destroy_hash_table, { hash_table_destroy(&replaced_states); });

    const size_t default_number_of_buckets = 32, default_number_of_values = 10;
    // Initialize map with custom set of equals and hash functions:
    hash_table_create(&replaced_states, raw_trie_set_hash, default_number_of_buckets,
                      default_number_of_values, hash_set_equals<raw_trie*>);

    trie_create(new_trie); // TODO

    TRY trie_nfsm_to_dfsm_recursion(nfsm, *new_trie, &replaced_states)
        FINALIZE_AND_FAIL(destroy_hash_table, "Trie conversion to dfsm failed!");

    CALL_FINALIZER(destroy_hash_table); // Finalize hash table of states
    return SUCCESS();
}



inline raw_trie* regex_parse(const char* string) {
    raw_trie* root = NULL;
    raw_trie_create(&root);

    regex_parser parser = { string, 0 };
    regex_parse_expression(root, &parser);

    return root;
}
 
inline void trie_destroy(raw_trie* graph) {
    if (graph == NULL)
        return;

    hash_set<raw_trie*> nodes;
    hash_set_create(&nodes, raw_trie_hash);

    raw_trie_collect_nodes(graph, &nodes);

    HASH_SET_TRAVERSE(&nodes, raw_trie*, current)
        raw_trie_destroy_without_neighbours(SET_VALUE(current));

    hash_set_destroy(&nodes);
}
