#pragma once

#include <cstdint>
#include <cstdio>
#include <math.h>

#include "trace.h"
#include "linked-list.h"
#include "safe-alloc.h"

template <typename K, typename V>
struct hash_table_pair {
    K key;
    V value;
};

struct hash_table_bucket {
    element_index_t value_index;
    size_t size;
};

template <typename K, typename V>
struct hash_table {
    int (*key_hash_function) (K key);

    hash_table_bucket* hash_table;
    linked_list<hash_table_pair<K, V>> values;

    size_t buckets_used, buckets_capacity;
};

// TODO: Custom comparison for 
template <typename K, typename V>
stack_trace* hash_table_create(hash_table<K, V>* table,
                               int (*key_hash_function) (K key),
                               size_t bucket_capacity = 32,
                               size_t value_list_size = 10) {

    // Bucket capacity should be power of two
    size_t next_power_of_2 = (size_t) pow(2, (int) ceil(log2(bucket_capacity)));
    if (next_power_of_2 != bucket_capacity) {
        fprintf(stderr, "[WARNING] Hash table size isn't power of 2!" "\n"
                        "                   Current size: %zu"        "\n"
                        "          Will be rounded up to: %zu"        "\n", 
                bucket_capacity, next_power_of_2);

        bucket_capacity = next_power_of_2;
    }

    *table = {
        // Comparator function
        .key_hash_function = key_hash_function,

        // Initialize hash table array and linked
        // list of values with zeroes.

        // They will be initialized properly soon
        .hash_table = NULL, .values = {},

        // Number of buckets in use, useful for
        // calculating load factor of hash table
        .buckets_used = 0,
        
        // Number of buckets available for elements,
        // this can change when table gets resized
        .buckets_capacity = bucket_capacity
    };

    TRY linked_list_create(&table->values, value_list_size)
        FAIL("Linked list initialization of size %d failed!", value_list_size);

    FINALIZER(list_destroy, { linked_list_destroy(&table->values); });

    TRY safe_calloc(bucket_capacity, &table->hash_table)
        FINALIZE_AND_FAIL(list_destroy, "Hash table allocation failed!");

    // TODO: Should I rely on calloc for zeroing memory?

    return SUCCESS();
}

template<typename K, typename V>
size_t __hash_table_get_position(hash_table<K, V>* table, K key) {
    int key_hash = table->key_hash_function(key);

    // We can use fast modulo since /bucket_capacity/ is power of 2
    return key_hash & (table->buckets_capacity - 1);
}

template<typename K, typename V>
inline static
hash_table_bucket* __hash_table_lookup_bucket(hash_table<K, V>* table, K key) {
    size_t position = __hash_table_get_position(table, key);
    return &table->hash_table[position];
}

template<typename K, typename V>
inline static
element_index_t __hash_table_lookup_index(hash_table<K, V>* table, K key,
                                          hash_table_bucket** key_bucket = NULL) {

    hash_table_bucket* bucket = __hash_table_lookup_bucket(table, key);

    // Return bucket number, to avoid hashing key second time
    if (key_bucket != NULL)
        *key_bucket = bucket;

    if (bucket->size != 0) {
        element<hash_table_pair<K, V>> *current =
            linked_list_get_pointer(&table->values, bucket->value_index);

        for (size_t index = 0; index < bucket->size; ++ index) {
            if (current->element.key == key)
                return linked_list_get_index(&table->values, current);

            current = linked_list_next(&table->values, current);
        }
    }

    return linked_list_end_index;
}

template<typename K, typename V>
V* hash_table_lookup(hash_table<K, V>* table, K key) {
    element_index_t index = __hash_table_lookup_index(table, key);
    if (index == linked_list_end_index)
        return NULL; // Element not found

    return &linked_list_get_pointer(&table->values, index)->element.value;
}

template<typename K, typename V>
bool hash_table_insert(hash_table<K, V>* table, K key, V value) {
    hash_table_bucket* bucket;
    if (__hash_table_lookup_index(table, key, &bucket) != linked_list_end_index)
        return false; // There's same key in the hash table 

    if (bucket->size > 0)
        linked_list_insert_after(&table->values, { key, value },  bucket->value_index);
    else
        linked_list_push_back(   &table->values, { key, value }, &bucket->value_index);

    ++ bucket->size;

    return true; // Inserted successfully
}

// TODO: remove, rehash, traverse

template <typename K, typename V>
void hash_table_destroy(hash_table<K, V>* table) {
    linked_list_destroy(&table->values);
    free(table->hash_table), table->hash_table = NULL;
}
