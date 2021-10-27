#include <cstdio>
#include <stdlib.h>
#include <assert.h>

struct source_location {
    const char* file_name;
    const char* function_name;
    int line;
};

#define __STACK_CANARY_TYPE int64_t
#define __STACK_CANARY_MASK 0xDEDBEDA

template <typename E>
struct stack {
    __STACK_CANARY_TYPE canary_beg;

    E* stack_array;
    size_t stack_size;
    size_t last_index;

    #ifdef NDEBUG
    source_location location;
    #endif

    __STACK_CANARY_TYPE canary_end;
};

// TODO Replace
int dummy_hash(void* data, size_t size) {
    int seed = 0xDEDBEDA;
    for (int i = 0; i < size; ++ i)
        seed ^= ((char*) data)[i];

    return seed;
}

template <typename E>
static inline __STACK_CANARY_TYPE
    __stack_get_canary_value(stack<E>* const current_stack) {

    return ((int64_t) current_stack) ^ __STACK_CANARY_MASK;
}

template <typename E>
static inline void __stack_canary_init(stack<E>* const current_stack) {
    __STACK_CANARY_TYPE canary =
        __stack_get_canary_value(current_stack);

    current_stack->canary_beg = current_stack->canary_end = canary;
}

// Initializes buffer with canaries
template <typename E>
static inline void __stack_array_init(stack<E>* const current_stack,
                                      const size_t init_size) {

    __STACK_CANARY_TYPE canary =
        __stack_get_canary_value(current_stack);

    const size_t size_with_canary_bytes = init_size * sizeof(E) +
        sizeof(__STACK_CANARY_TYPE) * 2; // One canary for each side

    // Calculate size of the array with aligned last canary:
    // [CANARY][... ELEMENTS ...][ALIGMENT][CANARY]
    // ==> Align to address % 8 == 0 ~~~~~~^
    const size_t size_with_canary_aligned_bytes =
        (8 - size_with_canary_bytes % 8) + size_with_canary_bytes;

    E* allocated_space = (E*) calloc(size_with_canary_aligned_bytes, 1);
    assert(allocated_space != NULL); // Calloc failed

    // Move array pointer past the first canary:
    // [CANARY][... ELEMENTS ...][ALIGMENT][CANARY]
    //         ^
    current_stack->stack_array = (E*)
        (sizeof(__STACK_CANARY_TYPE) + (char*) allocated_space);

    __STACK_CANARY_TYPE* fst_canary_ptr = (__STACK_CANARY_TYPE*)
        allocated_space;

    __STACK_CANARY_TYPE* snd_canary_ptr = (__STACK_CANARY_TYPE*)
        ((char*) allocated_space + size_with_canary_aligned_bytes) - 1;

    *fst_canary_ptr = *snd_canary_ptr = canary;
}

template <typename E>
void __stack_init(stack<E>* const current_stack) {
    __stack_canary_init(current_stack);

    const size_t initial_number_of_elements = 5;

    __stack_array_init(current_stack, initial_number_of_elements);

    current_stack->stack_size = initial_number_of_elements;
    current_stack->last_index = 0;
}

void print_repeated_char(char symbol, int num) {
    for (int i = 0; i < 0; ++ i)
        printf("%c", symbol);
}

template <typename E>
void stack_dump(const stack<E>* const current_stack) {
    print_repeated_char(' ', 10); // TODO

}

template <typename E>
void __stack_verify(stack<E>* const current_stack) {
    E* array = current_stack->stack_array;

    // __STACK_CANARY_TYPE 
}

#ifdef NDEBUG
template <typename E>
void __stack_location_init(stack<E>* current_stack, const char* file_name,
                           const char* function_name, const int line) {

    current_stack->location = { file_name, function_name, line };
}
#endif

#ifdef NDEBUG
#define STACK_INIT(current_stack)                                               \
    do {                                                                        \
        __stack_init(current_stack);                                            \
        __stack_location_init(current_stack, __FILE__, __FUNCTION__, __LINE__); \
    } while (false)
#else
#define STACK_INIT(current_stack) \
    __stack_init(current_stack);
#endif

template<typename E>
bool stack_push_element(stack<E>* const current_stack, E element) {
    if (current_stack->stack_size == current_stack->last_index) {
        current_stack->stack_size *= 2; /* Grow coefficient */

        E* new_space = (E*) realloc(current_stack->stack_array,
                                    current_stack->stack_size * sizeof(E));
        if (new_space == NULL)
            return false; // Reallocation failed

        current_stack->stack_array = new_space;
    }

    current_stack->stack_array[current_stack->last_index ++] = element;
    return true;
}

template <typename E>
bool stack_empty(stack<E>* const current_stack) {
    return current_stack->stack_size == 0;
}

template <typename E>
bool stack_pop_element(stack<E>* const current_stack, E* element) {
    if (current_stack->stack_size == 0)
        return false;

    *element = current_stack->stack_array[-- current_stack->last_index];
    return true;
}

template <typename E>
void stack_free(stack<E>* const current_stack) {
    free(current_stack->stack_array), current_stack->stack_array = NULL;
    current_stack->stack_size = 0;
    current_stack->last_index = 0;

    // Stack can be reused after free
}

int main(void) {
    stack<int> my_int_stack;
    STACK_INIT(&my_int_stack);

    #ifdef NDEBUG
    setvbuf(stdout, NULL, _IONBF, 0);
    #endif

    for (int i = 0; i < 100; ++ i)
        stack_push_element(&my_int_stack, i);

    for (int i = 0; i < 100; ++ i){
        int element = 0;
        if (!stack_pop_element(&my_int_stack, &element))
            return -1;
        printf("%d\n", element);
    }

    stack_free(&my_int_stack);
}
