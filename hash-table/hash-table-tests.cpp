#include "hash-table.h"
#include "test-framework.h"

int int_hash_function(int key) {
    return key;
}

TEST(populate_hash_table_with_ints) {
    hash_table<int, int> table;

    TRY hash_table_create(&table, int_hash_function);
        ASSERT_SUCCESS();

    TEST_FINALIZER({ hash_table_destroy(&table); });

    for (int i = 0; i < 10; ++ i)
        hash_table_insert(&table, i, i * i);

    for (int i = 0; i < 10; ++ i)
        ASSERT_EQUAL(*hash_table_lookup(&table, i),  i * i);

    CALL_TEST_FINALIZER();
}

int char_hash_function(char symbol) {
    return (int) symbol;
}

TEST(populate_hash_table_with_chars) {
    hash_table<char, int> table;

    TRY hash_table_create(&table, char_hash_function)
        ASSERT_SUCCESS();

    TEST_FINALIZER({ hash_table_destroy(&table); });

    hash_table_insert(&table, 'c', 42);

    ASSERT_EQUAL(*hash_table_lookup(&table, 'c'), 42);

    ASSERT_EQUAL( hash_table_lookup(&table, 'a') == NULL, true);

    CALL_TEST_FINALIZER();
}

int main(void) {
    return test_framework_run_all_unit_tests();
}
