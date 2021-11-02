#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include <stddef.h>

void hash_with_sha_256(const void* const data_ptr,
                       const size_t size,
                       uint32_t output_hash[8]);
#endif
