#include "akinator.h"
#include "test-framework.h"
#include "trace.h"

TEST(tree_reading) {
    wchar_t string[] =
        L"[Is tree [Has leafs [Leaf] [Trap]] [Unknown]]";

    akinator_tree* tree = EMPTY_NODE<akinator_node>;

    TRY load_tree(string, &tree) ASSERT_SUCCESS();
    print_tree(tree);

    binary_tree_destroy(tree);
}

int main() {
    return test_framework_run_all_unit_tests();
}
