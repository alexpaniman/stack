#include "test-framework.h"

#include <stdio.h>

TEST(differentiator_test) {
    printf("Hello, world!");
}

int main() {
    return test_framework_run_all_unit_tests();
}
