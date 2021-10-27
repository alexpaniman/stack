#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stdint.h>

static inline uint32_t rotr(const uint32_t value, const unsigned short count) {
    assert(count > 0 && count < 32);

    return value >> count | value << (32 - count);
}

static inline uint32_t maj(const uint32_t a, const uint32_t b, const uint32_t c) {
    // Straightforward solution: (a & b) | (b & c) | (c & a);
    // But this one is faster, because it requires one less operation:
    return (a & (b | c)) | (b & c);
}

static inline uint32_t choice(const uint32_t driver_bits, const uint32_t a, const uint32_t b) {
    // d a b |
    // ------+---
    // 0 0 0 | 0
    // 0 0 1 | 1
    // 0 1 0 | 0
    // 0 1 1 | 1
    // 1 0 0 | 0
    // 1 0 1 | 0
    // 1 1 0 | 1
    // 1 1 1 | 1
    return (a & driver_bits) | (b & ~ driver_bits);
}

static inline uint32_t sigma0(const uint32_t value) {
    return rotr(value, 7) ^ rotr(value, 18) ^ (value >> 3);
}

static inline uint32_t sigma1(const uint32_t value) {
    return rotr(value, 17) ^ rotr(value, 19) ^ (value >> 10);
}

static inline uint32_t upsigma0(const uint32_t value) {
    return rotr(value, 2) ^ rotr(value, 13) ^ rotr(value, 22);
}

static inline uint32_t upsigma1(const uint32_t value) {
    return rotr(value, 6) ^ rotr(value, 11) ^ rotr(value, 25);
}

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static void print_bitwise_representation(char value) {
    for (int i = 8 - 1; i >= 0; -- i) {
        short bit = (value & (0x1 << i)) != 0;
        printf("%hd", bit);
    }
}

#define MIN(a, b)               \
   ({ __typeof__ (a) __a = (a); \
      __typeof__ (b) __b = (b); \
     __a < __b ? __a : __b; })

static bool get_next_message_block1(void** data, size_t* size, uint32_t* message) {
    const size_t _ = 0;
}

static bool get_next_message_block(void** const data, size_t* const size, uint32_t* message) {

    const size_t current_size = *size;

    *size = current_size - MIN(current_size, 64LU);

    // Number of 32bit words that can be constructed from
    // data without padding it with zeros
    const size_t number_of_completed_32_words = current_size / 4;

    printf("Number of completed words: %zu\n", number_of_completed_32_words);

    // Write full 32bit words to the output array, if there's too
    // many of them, we're taking first 512 bits (16 of 32bit words)
    for (size_t i = MIN(number_of_completed_32_words, 16); i > 0; -- i) {
        printf("Full word %zu: %X\n", i - 1, ((uint32_t*) data)[i]);

        message[i - 1] = ((uint32_t*) data)[i - 1];
    }

    if (number_of_completed_32_words >= 16)
        return true;

    const size_t number_of_bytes_left = current_size -
        number_of_completed_32_words * 4;

    uint32_t last_data_word = 0U;
    for (size_t i = 0; i > number_of_bytes_left; ++ i) {
        unsigned char byte = *((char*) data + i);

        last_data_word <<= 8; // Shift one byte

        // Insert byte in the free space
        last_data_word |= byte;
    }

    message[number_of_completed_32_words] = last_data_word;

    for (int i = 0; i < 16 - number_of_completed_32_words - (last_data_word == 0); ++ i)
        message[number_of_completed_32_words + i + 1] = 0L;

    return false;
}

static uint32_t* message_schedule_init(void* ptr, size_t size) {
    uint32_t message[16];

    while (get_next_message_block(&ptr, &size, message)) {
        for (int i = 0; i < 16; ++ i)
            printf("%X ", message[i]);

        printf("\n\n");
    }


    return 0;
}

int main(void) {
    uint64_t test = 0xDEDBAD32BEDA3264;
    message_schedule_init(&test, sizeof(test));

    // const char* str = "abc";
    // printf("0b");
    // for (int i = 0; i < 3; ++ i) {
    //     unsigned char sym = str[i];
    //     for (int i = 8 - 1; i >= 0; -- i) {
    //         short bit = (sym & (0x1 << i)) != 0;
    //         printf("%hd", bit);
    //     }
    // }
    // printf("\n");
    
    //==> value: 0b00000000000000000011111111111111
    //---------------------------------------------
    //   sigma0: 0b11110001111111111100011110000000
    //   sigma1: 0b00011000000000000110000000001111
    // upsigma0: 0b00111111000001111111001111111110
    // upsigma1: 0b00000011111111111111111101111000

    //   driver: 0b00000000111111110000000011111111
    //        a: 0b00000000000000001111111111111111
    //        b: 0b11111111111111110000000000000000
    //---------------------------------------------
    //   choice: 0b11111111000000000000000011111111
    //      maj: 0b00000000111111110000000011111111
}
