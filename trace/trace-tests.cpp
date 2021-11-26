#include "trace.h"
#include "test-framework.h"
#include <cstdlib>

stack_trace* function_that_always_succeeds() {
    return SUCCESS();
}

TEST(test_success) {
    stack_trace* trace_that_should_succeed =
        function_that_always_succeeds();

    ASSERT_EQUAL(trace_is_success(trace_that_should_succeed), true);

    trace_destruct(trace_that_should_succeed);
}


stack_trace* function_that_always_fails(void) {
    return FAILURE(RUNTIME_ERROR, "Failed!");
}

TEST(test_failure) {
    stack_trace* trace_that_should_fail = function_that_always_fails();
    ASSERT_EQUAL(trace_is_success(trace_that_should_fail), false);

    trace_destruct(trace_that_should_fail);
}


stack_trace* function_that_fails_because_of_other_function(void) {
    TRY function_that_always_fails()
        FAIL("I failed consequently!");

    return SUCCESS();
}

TEST(test_passed_failure) {
    stack_trace* trace_that_should_fail =
        function_that_fails_because_of_other_function();

    ASSERT_EQUAL(trace_is_success(trace_that_should_fail), false);

    trace_destruct(trace_that_should_fail);
}


stack_trace* test_finally(int* variable) {
    FINALIZER(my_finalizer, {
        *variable = -1;
    });

    TRY function_that_always_fails()
        FINALIZE_AND_FAIL(my_finalizer, "I failed!");

    return SUCCESS();
}

TEST(test_finally) {
    int variable = 0;
    trace_destruct(test_finally(&variable));

    ASSERT_EQUAL(variable, -1);
}


int main(void) {
    return test_framework_run_all_unit_tests();
}  
