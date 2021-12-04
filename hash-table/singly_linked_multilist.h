#pragma once

#include <cstring>
#include <malloc.h>
#include <stddef.h>

#include "trace.h"

enum singly_linked_element_status {
    ELEMENT_FREE, ELEMENT_BUSY
};

typedef int singly_linked_element_index_t;
typedef singly_linked_element_index_t singly_linked_sublist_t;

template <typename E>
struct singly_linked_element {
    singly_linked_element_index_t next_index;
    singly_linked_element_status status;

    E value; // Element's value in place
};

template <typename E>
struct singly_linked_multilist {
    singly_linked_element<E>* elements;
    singly_linked_element_index_t free_head;

    size_t capacity;
};


template <typename E>
stack_trace* singly_linked_multilist_add_free_element(singly_linked_multilist<E>* list,
                                                      singly_linked_element_index_t index) {

    list->elements[index] = {
        .next_index = list->free_head,
        .status = ELEMENT_FREE, .value = (E) {}
    };

    list->free_head = index;
}

template <typename E>
stack_trace* singly_linked_multilist_resize(singly_linked_multilist<E>* list,
                                            const size_t new_capacity) {
    singly_linked_element<E>* new_space =
        (singly_linked_element<E>*) realloc(list->elements, // Could be NULL if list is empty
                                            new_capacity * sizeof(*new_space));

    if (new_space == NULL) // Realloc failed, notify user
        return FAILURE(RUNTIME_ERROR, strerror(errno));

    for (size_t i = list->capacity; i < new_capacity; ++ i) {
        singly_linked_multilist_add_free_element(list, i);
    }

    list->capacity = new_capacity; // Update list's capacity
}

template <typename E>
stack_trace* singly_linked_multilist_create(singly_linked_multilist<E>* list,
                                            const size_t init_capacity) {

    list->free_head = -1; // Free list is empty, resize will handle it
    singly_linked_multilist_resize(list, init_capacity);
}

template <typename E>
stack_trace* singly_linked_multilist_get_free_element(singly_linked_multilist<E>* list,
                                                      singly_linked_element_index_t* index,
                                                      const double grow_ratio) {
    if (list->free_head == -1) {
        const size_t new_size = grow_ratio * list->capacity;
        TRY singly_linked_multilist_resize(list, new_size)
            FAIL("List resizing failed!" "\n"
                 "Old size: %d"          "\n"
                 "New size: %d", list->capacity, new_size);
    }

    *index = list->free_head;
    list->free_head = list->elements[list->free_head].next_index;

    return SUCCESS();
}

template <typename E>
stack_trace* singly_linked_multilist_insert(singly_linked_multilist<E>* list,
                                            singly_linked_sublist_t* sublist, E value) {

    // Let's define some good default grow ratio, that avoids
    // too many reallocations, while keeping list relatively dense
    const double default_grow_ratio = 1.5;

    // Get place where new element will appear in the list
    singly_linked_element_index_t new_place_ind; 
    TRY singly_linked_multilist_get_free_element(list,
            new_place_ind, default_grow_ratio)
        FAIL("Getting free element in a list failed!");

    // Now let's handle case where /sublist/ is empty
    singly_linked_element_index_t next_index = -1;
    if (sublist != NULL)
        next_index = *sublist;

    // Let's update our element's value, and link it to the list
    list->elements[new_place_ind] = {
        .next_index = next_index,
        .status = ELEMENT_BUSY, .value = value
    };

    // Head of the list has changed, so we need to update it
    *sublist = new_place_ind;

    return SUCCESS();
}
