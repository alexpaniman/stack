#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include <stddef.h>

const size_t HASH_SIZE = 8;

void hash_with_sha_256(const void* const data_ptr,
                       const size_t size,
                       uint32_t output_hash[HASH_SIZE]);

#endif
