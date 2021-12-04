#pragma once

#include <stddef.h>
#include <malloc.h>
#include <stdio.h>

template <typename E>
struct binary_tree {
    E element;

    binary_tree<E>* left;
    binary_tree<E>* right;
};

template <typename E>
binary_tree<E>* EMPTY_NODE = NULL;

template <typename E>
binary_tree<E>* binary_tree_create(E value,
        binary_tree<E>* left, binary_tree<E>* right) {

    binary_tree<E>* tree = (binary_tree<E>*) calloc(1, sizeof(*tree));
    *tree = { .element = value, .left = left, .right = right };

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

template <typename E>
binary_tree<E>* binary_tree_search(binary_tree<E>* tree, E value) {
    if (tree->element == value)
        return tree;

    bool  left_is_empty = tree->left  == EMPTY_NODE<E>,
         right_is_empty = tree->right == EMPTY_NODE<E>;

    // Cut of extra recursive calls
    bool leaf_is_empty = left_is_empty || right_is_empty;

    if (tree == EMPTY_NODE<E> || leaf_is_empty)
        return EMPTY_NODE<E>;

    binary_tree<E>* search_left = binary_tree_search(tree->left, value);
    if (search_left != EMPTY_NODE<E>)
        return search_left;
    else
        return binary_tree_search(tree->right, value);
}
