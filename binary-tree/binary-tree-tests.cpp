#include "binary-tree.h"
#include "test-framework.h"

#define N(num, left, right) binary_tree_create(num, left, right)
#define L(num) binary_tree_create_leaf(num)
#define _ ((binary_tree<int>*) EMPTY_NODE<int>)

TEST(test_binary_tree) {
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

    ASSERT_EQUAL(binary_tree_search(tree, 12)->element, 12);

    ASSERT_EQUAL(binary_tree_search(tree, 6)->element, 6);

    binary_tree_destroy(tree);
}


int main(void) {
    return test_framework_run_all_unit_tests();
}
