#pragma once

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <math.h>

#include "trace.h"
#include "linked-list.h"
#include "macro-utils.h"
#include "safe-alloc.h"

template <typename K, typename V>
struct hash_table_pair {
    K key;
    V value;
};

template <typename K, typename V>
hash_table_pair<K, V> hash_table_pair_create(K key, V value) {
    return { key, value };
}

struct hash_table_bucket {
    element_index_t value_index;
    size_t size;
};

template <typename K, typename V>
struct hash_table {
    uint32_t (*key_hash_function) (K key);
    bool (*key_equals_function) (K* first, K* second);

    hash_table_bucket* hash_table;
    linked_list<hash_table_pair<K, V>> values;

    size_t buckets_used, buckets_capacity;
};

template <typename K>
bool hash_table_simple_key_equality(K* key_first, K* key_second) {
    return *key_first == *key_second;
}

template <typename K, typename V>
stack_trace* hash_table_create(hash_table<K, V>* table,
                               uint32_t (*key_hash_function) (K key),
                               size_t bucket_capacity = 32,
                               size_t value_list_size = 10, 
                               bool (*key_equals_function) (K* first, K* second) =
                                    hash_table_simple_key_equality<K>) {

    // Bucket capacity should be power of two
    bucket_capacity = (size_t) pow(2, (int) ceil(log2(bucket_capacity)));

    *table = {
        // Comparator function
        .key_hash_function = key_hash_function,

        // Function that checks for equality
        .key_equals_function = key_equals_function,

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
    uint32_t key_hash = table->key_hash_function(key);

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
            if (table->key_equals_function(&current->element.key, &key))
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


#define HASH_TABLE_PAIR_T(key_type, value_type) hash_table_pair<key_type, value_type>

#define HASH_TABLE_TRAVERSE(table, key_type, value_type, current)                            \
    LINKED_LIST_TRAVERSE(&(table)->values, HASH_TABLE_PAIR_T(key_type, value_type), current)

#define KEY(  current) ((current)->element.key)
#define VALUE(current) ((current)->element.value) 

template <typename K, typename V>
void hash_table_rehash(hash_table<K, V>* table,
                       const size_t new_bucket_capacity,
                       const size_t new_values_capacity) {

    hash_table<K, V> new_table;
    hash_table_create(&new_table, table->key_hash_function,
                      new_bucket_capacity,
                      new_values_capacity,
                      table->key_equals_function);

    HASH_TABLE_TRAVERSE(table, K, V, current)
        hash_table_insert(&new_table, KEY(current), VALUE(current));

    hash_table_destroy(table);
    *table = new_table; // Replace hash_table with a new one
}

template <typename K, typename V>
void hash_table_rehash_keep_size(hash_table<K, V>* table) {
    hash_table_rehash(table, table->buckets_capacity, table->values.capacity);
}

template <typename K, typename V>
bool hash_table_delete(hash_table<K, V>* table, K key) {
    hash_table_bucket* bucket = NULL;
    element_index_t index =
        __hash_table_lookup_index(table, key, &bucket);

    if (index == linked_list_end_index)
        return false;

    TRY linked_list_delete(&table->values, index)
        THROW("Value deletion failed!");

    -- bucket->size; // Since we found element
    // Bucket should be bigger than 1 in any case

    return true; // Deletion succeeded
}

template <typename K, typename V>
bool hash_table_contains(hash_table<K, V>* table, K key) {
    return __hash_table_lookup_index(table, key) != linked_list_end_index;
}

template <typename K, typename V>
bool hash_table_insert(hash_table<K, V>* table, K key, V value) {
    hash_table_bucket* bucket;
    if (__hash_table_lookup_index(table, key, &bucket) != linked_list_end_index)
        return false; // There's same key in the hash table 

    if (bucket->size > 0)
        TRY linked_list_insert_after(&table->values, { key, value },  bucket->value_index)
            THROW("Failed to insert new value in existing bucket!");
    else {
        ++ table->buckets_used;
        TRY linked_list_push_back(   &table->values, { key, value }, &bucket->value_index)
            THROW("Failed to insert new value in a new bucket (size: %d)!", bucket->size);
    }

    ++ bucket->size;

    const double GROW = 2.0;
    const double MAX_LOAD_FACTOR = 0.5;

    if ((double) table->buckets_used /
        (double) table->buckets_capacity >= MAX_LOAD_FACTOR)
        hash_table_rehash(table, table->buckets_capacity * GROW,
                                 table-> values.capacity * GROW);

    return true; // Inserted successfully
}

template <typename K, typename V>
void hash_table_destroy(hash_table<K, V>* table) {
    linked_list_destroy(&table->values);
    free(table->hash_table), table->hash_table = NULL;
}

template <typename K, typename V>
hash_table<K, V> create_hash_table(uint32_t (*key_hash_function)(K), int pair_count, ...) {
    hash_table<K, V> table;

    // This should be bigger than /pair_count/ to reduce hash clashes 
    const double target_fill_percent = 0.5;

    const size_t bucket_capacity = (size_t) (pair_count / target_fill_percent);

    TRY hash_table_create(&table, key_hash_function, bucket_capacity, pair_count)
    // This function is meant for inline initialization, we can't return trace :(
        THROW("Hash table creation failed!");

    // So we have no other way, except printing user error message, and aborting

    // Populate hash table with our values
    va_list args;
    va_start(args, pair_count);

    for (int i = 0; i < pair_count; ++ i) {
        // We need this hack to pass , in macro argument because /va_arg/
        // is a macro and <K, V> can't be surrounded with round braces ()
        #define _ ,
        hash_table_pair<K, V> pair = va_arg(args, hash_table_pair<K _ V>);
        #undef  _

        hash_table_insert(&table, pair.key, pair.value);
    }

    va_end(args);

    return table; // Table's struct will be copied, but it's small so it's ok
}

#define PAIR(key, value) hash_table_pair_create(key, value)
#define HASH_TABLE(key_type, value_type, key_hash_function, ...)                             \
    create_hash_table<key_type, value_type>(key_hash_function,                               \
                                            MACRO_UTILS_NARG(__VA_ARGS__), __VA_ARGS__)
