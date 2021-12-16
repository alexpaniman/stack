#include "default-hash-functions.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

uint32_t int_hash(const int number) {
    uint32_t hash = (uint32_t) number;

    // Magic number has been calculated with a test
    // that calculated the avalanche effect
    const uint32_t magic_number = 0x45D9F3B;

    const uint32_t BITS_IN_BYTE = 8UL;
    const uint32_t  bits_in_int =
        sizeof(int) * BITS_IN_BYTE / 2;

    hash = ((hash >> bits_in_int) ^ hash) * magic_number;
    hash = ((hash >> bits_in_int) ^ hash) * magic_number;
    hash =  (hash >> bits_in_int) ^ hash;
    return hash;
}

uint32_t char_hash(const char symbol) {
    // Delagate char hash to int hash
    return int_hash((int) symbol);
}

uint32_t str_hash(const char* string) {
    // This implements murmur hash for strings
    uint32_t hash = 3323198485UL;

    for (int i = 0; string[i] != '\0'; ++ i) {
        hash ^= (uint32_t) string[i];

        // Magic numbers from murmur hash implementation
        hash *= 0x5BD1E995;
        hash ^= hash >> 15;
    }

    return hash;
}

uint32_t combine_hash(uint32_t lhs, uint32_t rhs) {
    // Idea borrowed from boost's /hash_combine/
    return lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
}
