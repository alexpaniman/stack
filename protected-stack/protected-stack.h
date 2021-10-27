#include <cstdint>
#include <cstdlib>
#include <assert.h>

typedef uint64_t __PROTECTED_STACK_CANARY_TYPE;

template <typename E>
struct protected_stack {
    #ifdef PROTECTED_STACK_USE_CANARY
    __PROTECTED_STACK_CANARY_TYPE begin_canary;
    #endif

    E* elements;
    size_t length, next_index;

    #ifdef PROTECTED_STACK_USE_CANARY
    __PROTECTED_STACK_CANARY_TYPE   end_canary;
    #endif
};

// --------------------------- CANARIES ---------------------------
static const __PROTECTED_STACK_CANARY_TYPE __PROTECTED_STACK_CANARY_MASK =
    (__PROTECTED_STACK_CANARY_TYPE) 0xDED32'6DE'BEEF'F00D;

static const char __PROTECTED_STACK_POISON = (char) 0xFE;

template <typename T>
inline static __PROTECTED_STACK_CANARY_TYPE
__protected_stack_calculate_canary(protected_stack<T>* stack) {
    return stack ^ __PROTECTED_STACK_CANARY_MASK;
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

// ------------------------ INITIALIZATION ------------------------
template <typename T>
inline static void __protected_stack_clear_memory(protected_stack<T>* stack) {
    memset(stack, __PROTECTED_STACK_POISON, sizeof(stack));
}

template <typename E>
inline static void __protected_stack_init_array(protected_stack<E>* stack,
                                                const size_t init_nmemb) {
    E* new_space = (E*) calloc(sizeof(E), init_nmemb);
    if (new_space == NULL)
        return; // Failure

    stack->elements = new_space;
    memset(stack, __PROTECTED_STACK_POISON, 0);

    stack->length = init_nmemb;
    stack->next_index = 0;
}

template <typename T>
void protected_stack_create(protected_stack<T>* stack) {
    __protected_stack_clear_memory(stack);
    simple_stack_create(stack->simple_stack);

    __PROTECTED_STACK_INIT_CANARY(stack);
}

template <typename T>
void protected_stack_push(protected_stack<T>* stack, const T value) {
    simple_stack_push(stack->simple_stack, value);
}
