#pragma once

#include "aho.h"

void trie_vis_create_nodes(SUBGRAPH_CONTEXT, trie* graph,
                           hash_table<trie*, node_id>* nodes);

uint32_t trie_pointer_hash(trie *graph);

void trie_vis_create_edges(SUBGRAPH_CONTEXT, trie* graph,
                           hash_table<trie*, node_id>* nodes,
                           hash_set<trie*>* visited_nodes);

void trie_vis_create_graph(SUBGRAPH_CONTEXT, trie* graph); 

digraph trie_vis_create_graph(trie* graph);
