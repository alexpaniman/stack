#pragma once

#include "hash-table.h"

typedef struct {} dummy_t;

template <typename E>
struct hash_set {
    hash_table<E, dummy_t> table;
};

template <typename E>
stack_trace* hash_set_create(hash_set<E>* hash_set,
                            uint32_t (*key_hash_function) (E key),
                            size_t bucket_capacity = 32,
                            size_t value_list_size = 10) {

    *hash_set = {};
    TRY hash_table_create(&hash_set->table, key_hash_function,
                          bucket_capacity, value_list_size)
        FAIL("Failed to create hash map for use in hash set!");

    return SUCCESS();
}

template <typename E>
void hash_set_destroy(hash_set<E>* set) {
    hash_table_destroy(&set->table);
}

template <typename E>
bool hash_set_insert(hash_set<E>* set, E value) {
    return hash_table_insert(&set->table, value, {});
}

template <typename E>
void hash_set_rehash_keep_size(hash_set<E>* set) {
    hash_table_rehash_keep_size(set);
}


template <typename E>
bool hash_set_contains(hash_set<E>* set, E value) {
    return hash_table_contains(&set->table, value);
}

#define HASH_SET_TRAVERSE(set, value_type, current)                 \
    HASH_TABLE_TRAVERSE(&(set)->table, value_type, dummy_t, current)

#define SET_VALUE(current) KEY(current)
