#include "hash-set.h"
#include "default-hash-functions.h"

#include "test-framework.h"
#include <cstdint>

#define HASH_SET_INSERT(set, value, ...)                                         \
    do {                                                                         \
        __typeof__(value) __values[] = { value, __VA_ARGS__ };                   \
        for (size_t __i = 0; __i < sizeof(__values) / sizeof(*__values); ++ __i) \
            hash_set_insert(set, __values[__i]);                                 \
    } while(false)

#define HASH_SET_ASSERT_PRESENT(set, value, ...)                                 \
    do {                                                                         \
        __typeof__(value) __values[] = { value, __VA_ARGS__ };                   \
        for (size_t __i = 0; __i < sizeof(__values) / sizeof(*__values); ++ __i) \
            ASSERT_EQUAL(hash_set_contains(set, __values[__i]), true);           \
    } while(false)

#define HASH_SET_ASSERT_ABSENT(set, value, ...)                                  \
    do {                                                                         \
        __typeof__(value) __values[] = { value, __VA_ARGS__ };                   \
        for (size_t __i = 0; __i < sizeof(__values) / sizeof(*__values); ++ __i) \
            ASSERT_EQUAL(hash_set_contains(set, __values[__i]), false);          \
    } while(false)

TEST(hash_set_on_few_values) {
    hash_set<int> set = {};
    hash_set_create(&set, int_hash);

    TEST_FINALIZER({ hash_set_destroy(&set); });

    HASH_SET_INSERT(&set, 5, 2, 3);
    HASH_SET_ASSERT_PRESENT(&set, 5, 2, 3);
    HASH_SET_ASSERT_ABSENT(&set, 1, 7, 12);

    CALL_TEST_FINALIZER();
}

TEST(hash_set_on_many_values) {
    hash_set<int> set = {};
    hash_set_create(&set, int_hash);

    TEST_FINALIZER({ hash_set_destroy(&set); });

    const int max_number = 100000;
    for (int i = 0; i <= max_number; ++ i)
        hash_set_insert(&set, i);

    for (int i = 0; i <= max_number; ++ i)
        HASH_SET_ASSERT_PRESENT(&set, i);

    for (int i = max_number + 1; i < 2 * max_number; ++ i)
        HASH_SET_ASSERT_ABSENT(&set, i);

    CALL_TEST_FINALIZER();
}

TEST(uniqueness_test) {
    hash_set<int> set = {};
    hash_set_create(&set, int_hash);

    TEST_FINALIZER({ hash_set_destroy(&set); });

    const int max_number = 100000;
    for (int i = 0; i <= max_number; ++ i)
        ASSERT_EQUAL(hash_set_insert(&set, i), true);

    for (int i = 0; i <= max_number; ++ i)
        ASSERT_EQUAL(hash_set_insert(&set, i), false);

    CALL_TEST_FINALIZER();
}

int main(void) {
    return test_framework_run_all_unit_tests();
}
