#pragma once

#include "trace.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

typedef int element_index_t;

template <typename E>
struct element {
    element_index_t next_index;
    element_index_t prev_index;

    E element;
};

template <typename E>
struct linked_list {
    element<E>* elements;
    size_t capacity;

    element_index_t free;
    bool is_linearized;
};


template <typename E>
inline element<E>* linked_list_next(linked_list<E>* list,
                                    element<E>* current) {

    return &list->elements[current->next_index];
}

template <typename E>
inline element<E>* linked_list_prev(linked_list<E>* list,
                                    element<E>* current) {

    return &list->elements[current->prev_index];
}

template <typename E>
inline element<E>* linked_list_end(linked_list<E>* list) {
    return &list->elements[0];
}


template <typename E>
inline element_index_t linked_list_head_index(linked_list<E>* list) {
    return list->elements[0].next_index;
}

template <typename E>
inline element<E>* linked_list_head(linked_list<E>* list) {
    return &list->elements[linked_list_head_index(list)];
}


template <typename E>
inline element_index_t linked_list_tail_index(linked_list<E>* list) {
    return list->elements[0].prev_index;
}

template <typename E>
inline element<E>* linked_list_tail(linked_list<E>* list) {
    return &list->elements[linked_list_tail_index(list)];
}


template <typename E>
inline element<E>* linked_list_get_pointer(linked_list<E>* list,
                                           element_index_t actual_index) {

    return &list->elements[actual_index];
}

template <typename E>
inline element_index_t linked_list_get_index(linked_list<E>* list,
                                             element<E>* element_ptr) {

    return element_ptr - list->elements;
}

template <typename E>
stack_trace* linked_list_create(linked_list<E>* list, const size_t capacity) {
    element<E>* new_space = (element<E>*)
        calloc(capacity + 1, sizeof(*new_space));

    if (new_space == NULL)
        return FAILURE(RUNTIME_ERROR, strerror(errno));

    list->elements = new_space;
    list->capacity = capacity;

    list->is_linearized = true;

    // Singly link list of free elements
    for (element_index_t i = 1; i <= (element_index_t) capacity; ++ i)
        list->elements[i] = {
            .next_index = i + 1,
            .prev_index = -1
        };

    // Finish list of free elements with -1
    list->elements[capacity].next_index = -1,

    list->free = 1; // Free points to the first element
    return SUCCESS();
}


template <typename E>
stack_trace* check_element(linked_list<E>* list, element_index_t index) {
    if (index > (element_index_t) list->capacity)
        return FAILURE(RUNTIME_ERROR, "Index overflows list capacity!");

    if (index < 0)
        return FAILURE(RUNTIME_ERROR, "Index underflows list capacity!");

    element<E>* current = &list->elements[index];

    if (current->prev_index == -1)
        return FAILURE(RUNTIME_ERROR, "Index points to free element!");

    return SUCCESS();
}


template <typename E>
stack_trace* resize_linked_list(linked_list<E>* list, const size_t new_capacity) {
    E* new_space = (E*) realloc(list->elements, new_capacity + 1 /* For zero */);
    for (size_t i = list->capacity + 1; i <= (size_t) new_capacity; ++ i) {
        
    }
}

template <typename E>
stack_trace* get_free_element(linked_list<E>* list, element_index_t* element_index) {
    if (list->free != 0) {
        // Return index of last free element
        *element_index = list->free;

        // Update free elements head
        list->free = list->elements[list->free].next_index;
        return SUCCESS();
    }
        
    return FAILURE(RUNTIME_ERROR, "List is full! No place for new elements!");
}

template <typename E>
void add_free_element(linked_list<E>* list, element_index_t element_index) {
    list->elements[element_index] = {
        .next_index = list->free,
        .prev_index = -1, .element = (E) 0
    };
        
    // Update free elements head
    list->free = element_index;
}

template <typename E>
inline bool is_free_element(linked_list<E>* list, element_index_t element_index) {
    return list->elements[element_index].prev_index == -1;
}


template <typename E>
stack_trace* linked_list_insert_after(linked_list<E>* list, E value,
                                      element_index_t    prev_index,
                                      element_index_t* actual_index = NULL) {
    // Check if prev_index is valid index of list
    TRY check_element(list, prev_index) FAIL("Illegal index passed!");

    // Get free space for inserting new element
    element_index_t place_for_new_element = -1;
    TRY get_free_element(list, &place_for_new_element)
        FAIL("Can't get free element!");

    // Index that will go after our new element
    element_index_t next_index = list->elements[prev_index].next_index;

    //          next                        next          next
    // +------+ ~~~> +------+      +------x ~~~> /------x ~~~> /------+
    // | PREV | prev | NEXT |  =>  | PREV | prev | FREE | prev | NEXT |
    // +------+ <~~~ +------+      +------/ <~~~ x------/ <~~~ x------+

    // Update neighbours
    list->elements[prev_index].next_index = place_for_new_element;
    list->elements[next_index].prev_index = place_for_new_element;

    // Construct new element in the new place
    list->elements[place_for_new_element] = {
        .next_index = next_index,
        .prev_index = prev_index, .element = value
    };

    if (actual_index != NULL)
        *actual_index = place_for_new_element;

    return SUCCESS();
}

template <typename E>
inline stack_trace* linked_list_push_front(linked_list<E>* list, E element,
                                           element_index_t* actual_index = NULL) {

    // Inserting before head will result in pushing element to front
    return linked_list_insert_after(list, element, 0, actual_index);
}

template <typename E>
inline stack_trace* linked_list_push_back(linked_list<E>* list, E element,
                                          element_index_t* actual_index = NULL) {

    // Inserting after tail will result in pushing element to back
    return linked_list_insert_after(list, element,
                linked_list_tail_index(list), actual_index);
}


template <typename E>
stack_trace* linked_list_delete(linked_list<E>* list, element_index_t actual_index) {
    // Check if element has is valid index in the list
    TRY check_element(list, actual_index) FAIL("Illegal index passed!");

    element<E>* current = &list->elements[actual_index];
    element_index_t prev_index = current->prev_index,
                    next_index = current->next_index;

    //          next          next                        next
    // +------x ~~~> /------x ~~~> /------+      +------+ ~~~> +------+
    // | PREV | prev | CURR | prev | NEXT |  =>  | PREV | prev | NEXT |
    // +------/ <~~~ x------/ <~~~ x------+      +------+ <~~~ +------+

    list->elements[prev_index].next_index = current->next_index;
    list->elements[next_index].prev_index = current->prev_index;

    add_free_element(list, actual_index);

    return SUCCESS();
}

template <typename E>
stack_trace* linked_list_pop_back(linked_list<E>* list) {
    return linked_list_delete(list, linked_list_head_index(list));
}

template <typename E>
stack_trace* linked_list_pop_front(linked_list<E>* list) {
    return linked_list_delete(list, linked_list_head_index(list));
}


#define SWAP(x, y)               \
    do {                         \
        __typeof__(x) __tmp = x; \
        x = y;                   \
        y = __tmp;               \
    } while (false)

template <typename E>
stack_trace* linked_list_move(linked_list<E>* list,
                              element_index_t elem_index,
                              element_index_t free_index) {

    if (!is_free_element(list, free_index))
        return FAILURE(RUNTIME_ERROR, "Element is not free!");

    TRY check_element(list, elem_index) FAIL("Illegal elem index!");

    element<E>* curr = &list->elements[elem_index];
    list->elements[free_index] = *curr;

    list->elements[curr->prev_index].next_index = free_index;
    list->elements[curr->next_index].prev_index = free_index;

    add_free_element(list, elem_index);

    return SUCCESS();
}

/**
 * Swap physical positons of elements prev and next
 * without changing their logical order in a list. 
 */
template <typename E>
stack_trace* linked_list_swap_real(linked_list<E>* list,
                              element_index_t prev_index,
                              element_index_t next_index) {

    if (prev_index == next_index)
        return SUCCESS();

    TRY check_element(list, prev_index) FAIL("Illegal prev index!");

    TRY check_element(list, next_index) FAIL("Illegal next index!");

    //                       next
    //         next +------+ ~~~> +------+ prev
    // (PREV1) ~~~> | PREV | prev | NEXT | <~~~ (NEXT1)
    //              +------+ <~~~ +------+
    //             (prev_ind)    (next_ind)

    // ==> Should become:
    //                  +--------------------------+ P
    //                  |                          | R
    //                  .    next                  | E
    //              +------+ <~~~ +------+         | V
    // (PREV1)      | NEXT | prev | PREV |      (NEXT1)
    //  N |         +------+ ~~~> +------+          
    //  E |        (prev_ind)    (next_ind)        
    //  X |                           ^            
    //  T +---------------------------+            

    element<E>* prev = &list->elements[prev_index];
    element<E>* next = &list->elements[next_index];

    prev->next_index = prev_index;
    list->elements[next->next_index].prev_index = prev_index;

    next->prev_index = next_index;
    list->elements[prev->prev_index].next_index = next_index;

    SWAP(*prev, *next); // We've prepared elements, now we can swap

    return SUCCESS();
}

#define LINKED_LIST_TRAVERSE(list, type, current)         \
    for (element<type>* current = linked_list_head(list); \
            current != linked_list_end (list);            \
            current  = linked_list_next(list, current))

#define PRINT_LIST(list)                          \
    printf("[ ");                                 \
    for (int i = 0; i <= list->capacity; ++ i)    \
        printf("%d ", list->elements[i].element); \
    printf("]\n");                                \
                                                 
    // printf("[ ");                                 \
    // LINKED_LIST_TRAVERSE(list, int, __current)    \
    //     printf("%d ", __current->element);        \
    // printf("] \n");

template <typename E>
stack_trace* linked_list_linealize(linked_list<E>* list) {
    element_index_t logical_index = 1;
    for (element<E> *current =  linked_list_head(list);
            current != linked_list_end (list);
            current =  linked_list_next(list, current), ++ logical_index) {
        element_index_t actual_index = linked_list_get_index(list, current);

        if (is_free_element(list, logical_index)) {
            // Exchange element with free space, and mark hole as free
            TRY linked_list_move(list, actual_index, logical_index)
                FAIL("Failed to move element onto a free space!");
        } else {
            // Exchange element with place where it belongs in linealized list
            TRY linked_list_swap_real(list, actual_index, logical_index)
                FAIL("Failed to exchange actual index with logical one!");
        }

        current = linked_list_get_pointer(list, logical_index);
    }

    // Move free after the end of all elements
    list->free = logical_index <= list->capacity? logical_index : -1;

    // Reindex free elements in order
    for (int index = logical_index; index <= list->capacity; ++ index) {
        if (index == list->capacity) {
            list->elements[index].next_index = -1;
            break;
        }
            
        list->elements[index].next_index = index + 1;
    }

    return SUCCESS();
}

template <typename E>
inline stack_trace* linked_list_get_logical(linked_list<E>*  const list,
                                            const element_index_t logical_index,
                                            element_index_t* const element_index) {
    if (list->is_linearized) {
        *element_index = linked_list_head_index(list) + logical_index;
        return SUCCESS();
    }

    element_index_t index = 0;
    LINKED_LIST_TRAVERSE(list, E, current)
        if (index ++ == logical_index) {
            *element_index = linked_list_get_index(list, current);
            return SUCCESS();
        }
}



template <typename E>
void linked_list_destroy(linked_list<E> *list) {
    if (list != NULL) {
        free(list->elements);
        *list = {}; // Zero list out
    }

    // Do nothing if list is NULL (like free)
}

// ---------------------------------------------------------------------------------------------

template <typename E>
void print_text_dump(linked_list<E>* list) {
    printf("+------------------------------+\n");
    for (int i = 0; i < list->capacity + 1; ++ i) {
        element<E>* elem = &list->elements[i];
        printf("| %2d: (%02d) | (<-) %02d | (->) %02d | \n",
               i, elem->element, elem->prev_index, elem->next_index);
    }
    printf("+------------------------------+\n");
}

template <typename E>
void linked_list_create_graph(FILE* file, linked_list<E>* list) {
    fprintf(file, "digraph { \n");

    fprintf(file, "\t\t node_000 [label = \"cycle\", fontcolor=\"blue\", shape = rectangle, style = rounded];\n");

    fprintf(file, "\t subgraph { \n"
                  "\t\t rank = same; \n"
                  "\t\t node [shape=\"plaintext\"]; \n");

    for (int i = 1; i <= (int) list->capacity; ++ i) {
        const element<E>* el = &list->elements[i];
        fprintf(file, "\t\t "
                R"(node_%03d [label = <<table border="0" cellborder="1" cellspacing="0">
                       <tr> <td port="index" colspan="2"> %d </td> </tr>
                       <tr> <td> elem </td> <td port="elem"> %d </td> </tr>
                       <tr> <td> prev </td> <td port="prev"> %d </td> </tr>
                       <tr> <td> next </td> <td port="next"> %d </td> </tr>
                   </table>>];)" "\n", i, i, el->element, el->prev_index, el->next_index);
    }

    // fprintf(file, "\t\t edge [constraint = true, style = \"invis\"]; \n");
    // for (int i = 1; i < (int) list->capacity; ++ i)
    //     fprintf(file, "\t\t node_%03d -> node_%03d;\n", i, i + 1);

    // fprintf(file, "\t\t edge [constraint = false, style = \"solid\"]; \n");
    for (int i = 1; i <= (int) list->capacity; ++ i) {
        const element<E>* el = &list->elements[i];

        if (el->next_index != -1 && el->next_index != 0)
            fprintf(file, "\t\t node_%03d:next -> node_%03d; \n", i, el->next_index);

        if (el->prev_index != -1 && el->prev_index != 0)
            fprintf(file, "\t\t node_%03d:prev -> node_%03d; \n", i, el->prev_index);
    }

    fprintf(file, "\t } \n");

    for (int i = list->elements[0].next_index != 0? 0 : 1; i <= (int) list->capacity; ++ i) {
        const element<E>* el = &list->elements[i];

        if (el->next_index == 0)
            fprintf(file, "\t\t node_%03d:next -> node_%03d; \n", i, el->next_index);

        if (el->prev_index == 0)
            fprintf(file, "\t\t node_%03d:prev -> node_%03d; \n", i, el->prev_index);
    }

    fprintf(file, "\t\t node [shape=\"rectangle\", style=\"rounded\"]; \n"
                  "\t\t free [label = \"free\", fontcolor = \"seagreen\"];"
                  "\t\t head [label = \"head\", fontcolor = \"crimson\"]; \n"
                  "\t\t tail [label = \"tail\", fontcolor = \"darkmagenta\"]; \n");

    if (list->free != 0)
        fprintf(file, "free -> node_%03d;", list->free);

    #define head list->elements[0].next_index
    if (head != 0)
        fprintf(file, "head -> node_%03d;", head);

    #define tail list->elements[0].prev_index
    if (tail != 0)
        fprintf(file, "tail -> node_%03d;", tail);

    #undef head
    #undef tail

    fprintf(file, "} \n");
}

const size_t MAX_TMP_NAME_SIZE = 128;

inline char* linked_list_call_graphviz(linked_list<int>* list) {
    char* graph_tmp_name = tmpnam(NULL);

    FILE* tmp = fopen(graph_tmp_name, "w");
    linked_list_create_graph(tmp, list);

    char dot_buffer[256] = {};
    strcat(dot_buffer, "dot -Tpng ");
    strcat(dot_buffer, graph_tmp_name);

    fclose(tmp), tmp = NULL;


    char* image_tmp_name = (char*)
        calloc(MAX_TMP_NAME_SIZE, sizeof(char));
    strcat(image_tmp_name, ".png");

    tmpnam(image_tmp_name);
    strcat(dot_buffer, " -o ");
    strcat(dot_buffer, image_tmp_name);

    system(dot_buffer);

    return image_tmp_name;
}
