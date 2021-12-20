#include "binary-tree.h"
#include "test-framework.h"
#include <simple-stack.h>

#define N(num, left, right) binary_tree_create(num, left, right)
#define L(num) binary_tree_create_leaf(num)
#define _ ((binary_tree<int>*) EMPTY_NODE<int>)

TEST(search_in_binary_tree) {
    binary_tree<int>* tree =
        N(0,
          N(1,
            N(2, L(3), L(4)),
            N(5,
              N(6,
                N(7, L(8), _),
                L(9)
               ),
              L(9)
             )
           ),
          N(10,
            N(11, L(12), L(13)),
            L(14)
           )
         );

    simple_stack<path_node<int>> path;
    simple_stack_create(&path);

    TEST_FINALIZER({
        binary_tree_destroy(tree);
        simple_stack_destruct(&path);
    })

    ASSERT_EQUAL(binary_tree_search(tree, 12)->element, 12);
    ASSERT_EQUAL(binary_tree_search(tree,  6)->element,  6);

    // Let's try to find path to the 13
    binary_tree_search(tree, 13, &path);
    ASSERT_EQUAL((int) path.next_index, 3);

    CALL_TEST_FINALIZER();
}


int main(void) {
    return test_framework_run_all_unit_tests();
}
