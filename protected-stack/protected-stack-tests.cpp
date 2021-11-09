#include "protected-stack.h"
#include "test-framework.h"

TEST(populate_stack_with_numbers) {
    protected_stack<int> stack = {};
    protected_stack_create(&stack);

    const size_t size = 100;

    for (size_t i = 0; i < size; ++ i)
        protected_stack_push(&stack, (int) i);

    for (size_t i = size - 1; i >= 0; -- i) {
        int value = -1;

        stack_trace* trace = protected_stack_pop(&stack, &value);
        trace_print_stack_trace(stdout, trace);

        ASSERT_EQUAL(value, (int) i);
    }

    protected_stack_destroy(&stack);
}

// print *stack->elements@stack->length
// print *stack->elements@stack->next_index

TEST_MAIN()
