#include "crypto.h"
#include "config.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <stdlib.h>
#include <trace.h>
#include <assert.h>
#include <errno.h>
#include <sys/random.h>

typedef uint64_t __PROTECTED_STACK_CANARY_TYPE;

#define PROTECTED_STACK_USE_HASH

template <typename E>
struct protected_stack {
    #ifdef PROTECTED_STACK_USE_CANARY
    __PROTECTED_STACK_CANARY_TYPE begin_canary;
    #endif

    #ifdef PROTECTED_STACK_USE_SALT
    uint32_t salt;
    #endif

    #ifdef PROTECTED_STACK_USE_HASH
    uint32_t hash[HASH_SIZE];
    #endif

    E* elements;

    size_t length; size_t next_index;

    #ifdef PROTECTED_STACK_USE_CANARY
    __PROTECTED_STACK_CANARY_TYPE   end_canary;
    #endif
};

// --------------------------- CANARIES ---------------------------
static const __PROTECTED_STACK_CANARY_TYPE __PROTECTED_STACK_CANARY_MASK =
    (__PROTECTED_STACK_CANARY_TYPE) 0xDED32'6DE'BEEF'F00D;

template <typename T>
inline static __PROTECTED_STACK_CANARY_TYPE
__protected_stack_calculate_canary(protected_stack<T>* stack) {
    return (uintptr_t) stack ^ __PROTECTED_STACK_CANARY_MASK;
}

template <typename T>
inline static void __protected_stack_init_canary(protected_stack<T>* stack) {
    __PROTECTED_STACK_CANARY_TYPE canary =
        __protected_stack_calculate_canary(stack); 

    stack->begin_canary = stack->end_canary = canary;
}

#ifdef PROTECTED_STACK_USE_CANARY
    #define __PROTECTED_STACK_INIT_CANARY(stack) \
        __protected_stack_init_canary(stack)
#else
    #define __PROTECTED_STACK_INIT_CANARY(stack) \
        ((void) 0)
#endif

// ---------------------------- POISON ----------------------------
static const char __PROTECTED_STACK_POISON = (char) 0xFE;

template <typename E>
inline static void __protected_stack_poison_struct(protected_stack<E>* stack) {
    memset(stack, __PROTECTED_STACK_POISON, sizeof(stack));
}

inline static void __protected_stack_poison_array(void* array,
                                                  size_t old_size,
                                                  size_t new_size) {
    if (new_size > old_size && array != NULL) {
        size_t diff = new_size - old_size;
        void* start_ptr = ((char*) array) + old_size;

        memset(array, __PROTECTED_STACK_POISON, diff);
    }
}

#ifdef PROTECTED_STACK_USE_POISON
    #define __PROTECTED_STACK_POISON_STRUCT(stack)                \
        __protected_stack_poison_struct(stack)

#define __PROTECTED_STACK_POISON_ARRAY(array, old_size, new_size) \
        __protected_stack_poison_array(array, old_size, new_size)
#else
    #define __PROTECTED_STACK_POISON_STRUCT(stack)                \
        ((void) 0)

    #define __PROTECTED_STACK_POISON_ARRAY(array, size)           \
        ((void) 0)
#endif

template <typename E>
inline static void __protected_stack_clear_hash(protected_stack<E>* stack) {
    memset(stack->hash, 0, HASH_SIZE);
}

template <typename E>
inline static void __protected_stack_poison_hash(protected_stack<E>* stack) {
    memset(stack->hash, __PROTECTED_STACK_POISON, HASH_SIZE);
}

#ifdef PROTECTED_STACK_USE_POISON
    #define __PROTECTED_STACK_CLEAR_HASH(stack)         \
        __protected_stack_poison_hash(stack)
#else
    #define __PROTECTED_STACK_CLEAR_HASH(stack)         \
        __protected_stack_clear_hash(stack)
#endif

// ----------------------------- HASH -----------------------------
template <typename E>
inline static stack_trace* __protected_stack_init_salt(protected_stack<E>* stack) {
    if (getrandom(&stack->salt, sizeof(stack->salt), 0) == -1)
        return FAILURE(RUNTIME_ERROR, strerror(errno));

    return SUCCESS();
}

#ifdef PROTECTED_STACK_USE_SALT
    #define __PROTECTED_STACK_INIT_SALT(stack) \
        __protected_stack_init_salt(stack);
#else
    #define __PROTECTED_STACK_INIT_SALT(stack) \
        (void) 0
#endif

template <typename E>
inline static void __protected_stack_calculate_hash(protected_stack<E>* stack,
                                                    uint32_t hash[HASH_SIZE]) {
    // Old hash should be zeroed out before this function

    uint32_t stack_hash[HASH_SIZE];
    hash_with_sha_256(stack, sizeof(stack), stack_hash);

    hash_with_sha_256(stack->elements,
        sizeof(E) * stack->length, hash);

    for (size_t i = 0; i < HASH_SIZE; ++ i)
        hash[i] ^= stack_hash[i];
}

template <typename E>
inline static void __protected_stack_update_hash(protected_stack<E>* stack) {
    __PROTECTED_STACK_CLEAR_HASH(stack);
    __protected_stack_calculate_hash(stack, stack->hash);
}

template <typename E>
inline static bool __protected_stack_test_hash(protected_stack<E>* stack) {
    uint32_t stored_hash[HASH_SIZE];

    // Save hash from stack
    memcpy(stored_hash, stack->hash, HASH_SIZE);

    // Clear it before recalculation
    __PROTECTED_STACK_CLEAR_HASH(stack);

    uint32_t actual_hash[HASH_SIZE];
    __protected_stack_calculate_hash(stack, actual_hash);

    // Restore hash
    memcpy(stack->hash, stored_hash, HASH_SIZE);

    for (size_t i = 0; i < HASH_SIZE; ++ i)
        if (stored_hash[i] != actual_hash[i])
            return false;

    return true;
}

#ifdef PROTECTED_STACK_USE_HASH
    #define INIT_HASH(stack) __protected_stack_update_hash()
#else
    #define INIT_HASH(stack) __protected_stack_update_hash()
#endif

inline static size_t align_index(size_t index, size_t alignment) {
    return index + alignment - index % alignment;
}

template <typename E> inline static stack_trace*
__protected_stack_resize_array(protected_stack<E>* stack, const size_t nmemb) {
    // Align first array element (in case canary has unaligned size)
    size_t first_element_byte = align_index(
        sizeof(__PROTECTED_STACK_CANARY_TYPE), sizeof(E));

    // Align second canary
    size_t second_canary_byte = align_index(
        first_element_byte + nmemb * sizeof(E),
        sizeof(__PROTECTED_STACK_CANARY_TYPE));

    const size_t elements_buffer_size = second_canary_byte +
        sizeof(__PROTECTED_STACK_CANARY_TYPE);

    E* new_space = NULL;

    if (stack->elements == NULL)
        new_space = (E*) calloc(1, elements_buffer_size);
    else {
        char* allocated_ptr = (char*) stack->elements
            - first_element_byte;

        new_space = (E*) realloc(allocated_ptr, elements_buffer_size);
    }

    if (new_space == NULL)
        return FAILURE(RUNTIME_ERROR, strerror(errno));

    stack->elements = new_space;

    // Completly disregard calloc's work. I'm sad (c) Calloc
    if (nmemb > stack->length)
        __PROTECTED_STACK_POISON_ARRAY(new_space, stack->length * sizeof(E) + first_element_byte,
                                    (nmemb - stack->length) * sizeof(E) + first_element_byte);

    __PROTECTED_STACK_CANARY_TYPE
        * first_canary = (__PROTECTED_STACK_CANARY_TYPE*) stack->elements,
        *second_canary = (__PROTECTED_STACK_CANARY_TYPE*)
            ((char*) stack->elements + second_canary_byte);

    *first_canary = *second_canary = __protected_stack_calculate_canary(stack);

    // We could use initializer: { ... }, but it could've zeroed struct's
    // alignment (in case there's some), which was filled with poison previously
    stack->length = nmemb;

    // Move pointer to elements past first canary
    stack->elements = (E*) ((char*) stack->elements + first_element_byte);

    printf("(%d, %d, %d)\n", stack->next_index, first_element_byte, (uintptr_t) stack->elements);
    printf("[\n");
    for (int i = 0; i < stack->next_index; ++ i)
        printf("    %x\n", stack->elements[i]);

    for (int i = stack->next_index; i < stack->length; ++ i)
        printf("   (%x)\n", stack->elements[i]);
    printf("]\n");

    return SUCCESS();
}

template <typename E>
stack_trace* protected_stack_destroy(protected_stack<E>* stack) {
    size_t first_element_byte = align_index(
        sizeof(__PROTECTED_STACK_CANARY_TYPE), sizeof(E));

    free((char*) stack->elements - first_element_byte),
        stack->elements = NULL;
}

static const double GROW = 2.0;
static const double INIT_NMEMB = 10;

template <typename T>
stack_trace* protected_stack_create(protected_stack<T>* stack) {
    __PROTECTED_STACK_POISON_STRUCT(stack);

    stack_trace* salt_trace = __PROTECTED_STACK_INIT_SALT(stack);
    if (!trace_is_success(salt_trace))
        return PASS_FAILURE(salt_trace, RUNTIME_ERROR, "Salt initialization failed!");

    stack_trace* resize_trace = __protected_stack_resize_array(stack, INIT_NMEMB);
    if (!trace_is_success(resize_trace))
        return PASS_FAILURE(resize_trace, RUNTIME_ERROR, "Array creating failed!");

    __PROTECTED_STACK_INIT_CANARY(stack);

    return SUCCESS();
}

template <typename T>
stack_trace* protected_stack_push(protected_stack<T>* stack, const T value) {
    if (stack->length == stack->next_index) {
        stack_trace* trace = 
            __protected_stack_resize_array(stack, stack->length * GROW);

        if (!trace_is_success(trace))
            return PASS_FAILURE(trace, RUNTIME_ERROR, "Stack expanding failed!");
    }

    stack->elements[stack->next_index ++] = value;
    return SUCCESS();
}

template <typename T>
stack_trace* protected_stack_pop(protected_stack<T>* stack, T* const value) {
    const size_t shrinked_size = stack->length / GROW;

    if (stack->next_index - 1 < shrinked_size && shrinked_size >= INIT_NMEMB) {
        setvbuf(stdout, NULL, _IONBF, 0);

        stack_trace* trace = 
            __protected_stack_resize_array(stack, shrinked_size);

        if (!trace_is_success(trace))
            return PASS_FAILURE(trace, RUNTIME_ERROR, "Stack shrinking failed!");
    }

    *value = stack->elements[-- stack->next_index];
    return SUCCESS();
}

template <typename T>
stack_trace* protected_stack_peek(protected_stack<T>* stack, T* const value) {
    if (stack->next_index <= 0)
        return FAILURE(RUNTIME_ERROR, "Top element peeking failed because stack is empty!");

    *value = stack->elements[stack->next_index - 1];
    return SUCCESS();
}

template <typename T>
bool protected_stack_empty(protected_stack<T>* stack) {
    return stack->next_index == 0;
}
