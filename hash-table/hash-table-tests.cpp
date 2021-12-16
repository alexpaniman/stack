#include "hash-table.h"
#include "default-hash-functions.h"

#include "test-framework.h"

#define HASH_TABLE_ASSERT_VALUE(table, key, value)                  \
    do {                                                            \
        __typeof__(value)* __value_ptr =                            \
            hash_table_lookup(table, key);                          \
                                                                    \
        ASSERT_EQUAL(__value_ptr != 0 && "<=> Value found!", true); \
        ASSERT_EQUAL(*__value_ptr == value &&                       \
                     "<=> Value is the same!", true);               \
    } while(false)                                                         
                                                                    \
#define HASH_TABLE_ASSERT_NOT_PRESENT(table, key)                   \
    do {                                                            \
        ASSERT_EQUAL(hash_table_lookup(table, key) == NULL &&       \
                     "<=> Value not found!", true);                 \
    } while(false)

TEST(populate_hash_table_with_ints) {
    hash_table<int, int> table;

    TRY hash_table_create(&table, int_hash)
        ASSERT_SUCCESS();

    TEST_FINALIZER({ hash_table_destroy(&table); });

    for (int i = 0; i < 10; ++ i)
        hash_table_insert(&table, i, i * i);

    for (int i = 0; i < 10; ++ i)
        HASH_TABLE_ASSERT_VALUE(&table, i,  i * i);

    CALL_TEST_FINALIZER();
}

TEST(populate_hash_table_with_chars) {
    hash_table<char, int> table;

    TRY hash_table_create(&table, char_hash)
        ASSERT_SUCCESS();

    TEST_FINALIZER({ hash_table_destroy(&table); });

    hash_table_insert(&table, 'c', 42);

    HASH_TABLE_ASSERT_VALUE(&table, 'c', 42);
    HASH_TABLE_ASSERT_NOT_PRESENT(&table, 'a');

    CALL_TEST_FINALIZER();
}

TEST(test_hash_table_inline_construction) {
    hash_table<char, int> table =
        HASH_TABLE(char, int, char_hash,
                   PAIR('a', 5), PAIR('b', 4));

    TEST_FINALIZER({ hash_table_destroy(&table); });

    HASH_TABLE_ASSERT_VALUE(&table, 'a', 5);
    HASH_TABLE_ASSERT_VALUE(&table, 'b', 4);

    HASH_TABLE_ASSERT_NOT_PRESENT(&table, 'd');

    CALL_TEST_FINALIZER();
}

int main(void) {
    return test_framework_run_all_unit_tests();
}
