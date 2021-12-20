#pragma once

#include <simple-stack.h>
#include <stddef.h>
#include <malloc.h>
#include <stdio.h>

template<typename E>
using equality_fn_t = bool (*) (const E* fst, const E* snd);

template <typename E>
struct binary_tree {
    E element;

    binary_tree<E>* left;
    binary_tree<E>* right;
};

template <typename E>
binary_tree<E>* EMPTY_NODE = NULL;

template <typename E>
bool equivalence_function(const E* first, const E* second) {
    return *first == *second;
}

template <typename E>
binary_tree<E> *binary_tree_create(E node_value,
                                   binary_tree<E> *left  = NULL,
                                   binary_tree<E> *right = NULL) {

  binary_tree<E> *tree = (binary_tree<E>*) calloc(1, sizeof(*tree));
  *tree = { .element = node_value,
            .left = left, .right = right };

  return tree;
}

template <typename E>
binary_tree<E>* binary_tree_create_leaf(E value) {
    return binary_tree_create(value, EMPTY_NODE<E>, EMPTY_NODE<E>);
}

template <typename E>
void binary_tree_destroy(binary_tree<E>* subtree) {
    if (subtree == EMPTY_NODE<E>)
        return;

    if (subtree != EMPTY_NODE<E>)
        binary_tree_destroy(subtree->left);

    if (subtree != EMPTY_NODE<E>)
        binary_tree_destroy(subtree->right);

    free(subtree);
}

enum path_choice { GO_LEFT, GO_RIGHT };

template <typename E>
struct path_node {
    path_choice how_to_get_here;
    E* node_value;
};

template <typename E>
bool binary_tree_is_leaf(binary_tree<E>* tree) {
    return tree->left == EMPTY_NODE<E> && tree->right == EMPTY_NODE<E>;
}

template <typename E>
using binary_tree_path_t = simple_stack<path_node<E>>;

template <typename E>
binary_tree<E>* binary_tree_search(binary_tree<E>* tree, E value,
                                   binary_tree_path_t<E>* path = NULL,
                                   equality_fn_t<E> equality_function =
                                        equivalence_function) {
    // Precaution, in case search is called on an empty tree 
    if (tree == EMPTY_NODE<E>)
        return EMPTY_NODE<E>;

    // Test if desired value found
    if (equality_function(&tree->element, &value))
        return tree;

    // If it's a leaf, but a wrong one, than we will
    // not find anything interesting in here, return
    if (binary_tree_is_leaf(tree))
        return EMPTY_NODE<E>;

    if (path != NULL)
        // We're going left, mark our path there
        simple_stack_push(path, { GO_LEFT, &tree->element });

    binary_tree<E>* found_left = binary_tree_search(tree->left, value, path,
                                                    equality_function);
    if (found_left != EMPTY_NODE<E>)
        return found_left;

    if (path != NULL) {
        // We didn't find anything in the left, remove it from path
        simple_stack_pop(path); 

        // Mark our path to the right
        simple_stack_push(path, { GO_RIGHT, &tree->element });
    }

    binary_tree<E>* found_right = binary_tree_search(tree->right, value, path,
                                                     equality_function);
    if (found_right == EMPTY_NODE<E> && path != NULL)
        // Both searches didn't succeed, remove path to the right as well
        simple_stack_pop(path);

    return found_right;
}
