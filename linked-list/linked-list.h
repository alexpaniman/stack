#pragma once

#include "trace.h"

#include <cstdio>
#include <cstdlib>
#include <malloc.h>
#include <string.h>
#include <errno.h>

template <typename E>
struct element {
    E element;
    int next_index;
    int prev_index;
};

template <typename E>
struct linked_list {
    element<E>* elements;
    size_t capacity;

    int head, tail, free;
    bool is_linearized;
};

template <typename E>
stack_trace* linked_list_create(linked_list<E>* list, const size_t capacity) {
    if (capacity == 0)
        return FAILURE(RUNTIME_ERROR, "Illegal list capacity: 0!");

    element<E>* new_space = (element<E>*)
        calloc(capacity + 1, sizeof(*new_space));

    if (new_space == NULL)
        return FAILURE(RUNTIME_ERROR, strerror(errno));

    list->elements = new_space;
    list->capacity = capacity;

    list->is_linearized = true;

    list->head = list->tail = 0;

    for (int i = 1; i < (int) capacity; ++ i) {
        element<E>* element = &list->elements[i];

        element->next_index = i + 1;
        element->prev_index = -1;
    }

    list->elements[capacity].prev_index = -1;

    list->free = 1; // Free points to the first element
    return SUCCESS();
}

template <typename E>
static inline int linearize_cmp(const void* first, const void* second) {
    const element<E> * first_element = (const element<E>*) first,
                     *second_element = (const element<E>*) second;

    printf("~~~~> %d %d\n", first_element->element, second_element->element);

    if ((first_element ->prev_index == -1) ^
        (second_element->prev_index == -1))
        return second_element->next_index < first_element->next_index;

    return first_element->next_index < second_element->next_index;
}

template <typename E>
stack_trace* linked_list_linearize(linked_list<E>* list) {
    qsort(list->elements, list->capacity, sizeof(element<E>), linearize_cmp<E>);

    if (list->head != 0)
        list->head = 1;

    int previous = 0;
    for (int i = 1; i <= list->capacity; ++ i) {
        if (list->elements[i].prev_index == -1)
            break;

        list->elements[i].prev_index = previous;
        previous = i;
    }

    list->tail = previous;

    int next = 0;
    for (int i = list->capacity; i > 0; -- i) {
        list->elements[i].next_index = next;
        next = i;

        if (list->elements[i].prev_index == -1)
            list->free = i;
    }

    list->is_linearized = true;

    return SUCCESS();
}

template <typename E>
stack_trace* check_element(linked_list<E>* list, int index) {
    if (index > list->capacity)
        return FAILURE(RUNTIME_ERROR, "Index overflows list capacity.");

    if (index < 0)
        return FAILURE(RUNTIME_ERROR, "Index underflows list capacity.");

    element<E>* current = &list->elements[index];

    if (current->prev_index == -1)
        return FAILURE(RUNTIME_ERROR, "Index points to free element!");

    return SUCCESS();
}

template <typename E>
stack_trace* linked_list_insert_after(linked_list<E>* list, E value,
                                      int prev_index, int* actual_index) {

    stack_trace* index_check = check_element(list, prev_index);
    if (!trace_is_success(index_check))
        return PASS_FAILURE(index_check, RUNTIME_ERROR,
                            "Attempt to insert after illegal element.");

    element<E>* elements = list->elements;

    if (list->free == 0)
        return FAILURE(RUNTIME_ERROR, "List is full!");

    const int next_free_index =
        elements[list->free].next_index;

    const int next_index = elements[prev_index].next_index;

    elements[list->free] = {
        .element    = value,

        .next_index = next_index,
        .prev_index = prev_index
    };

    if (prev_index != 0)
        elements[prev_index].next_index = list->free;

    if (next_index != 0)
        elements[next_index].prev_index = list->free;

    // Update head and tail if needed

    if (list->tail == prev_index)
        list->tail = list->free;

    if (list->head == 0)
        list->head = list->free;

    // Return actual index of inserted value

    if (actual_index != NULL)
        *actual_index = list->free;

    // Update head of list with free space

    list->free = next_free_index;

    return SUCCESS();
}

template <typename E>
inline stack_trace* linked_list_insert(linked_list<E>* list,
                                       E element, int* actual_index) {

    return linked_list_insert_after(list, element, list->tail, actual_index);
}

template <typename E>
stack_trace* linked_list_delete(linked_list<E>* list, int actual_index) {
    element<E>* elements = list->elements;

    stack_trace* index_check = check_element(list, actual_index);
    if (!trace_is_success(index_check))
        return PASS_FAILURE(index_check, RUNTIME_ERROR,
                            "Attempt to delete illegal element.");

    element<E>* current = &elements[actual_index];

    elements[current->prev_index].next_index = current->next_index;
    elements[current->next_index].prev_index = current->prev_index;

    if (list->tail == actual_index)
        list->tail = current->prev_index;

    if (list->head == actual_index)
        list->head = current->next_index;

    *current = { (E) 0, list->free, -1 };
    list->free = actual_index;

    return SUCCESS();
}

template <typename E>
stack_trace* linked_list_delete_last(linked_list<E>* list) {
    return linked_list_delete(list, list->tail);
}

template <typename E>
stack_trace* linked_list_delete_first(linked_list<E>* list) {
    return linked_list_delete(list, list->head);
}

template <typename E>
void linked_list_create_graph(FILE* file, linked_list<E>* list) {
    fprintf(file, "digraph { \n" "\t subgraph { \n"
                  "\t\t rank = same; \n"
                  "\t\t node [shape=\"plaintext\"]; \n");

    for (int i = 1; i <= (int) list->capacity; ++ i) {
        const element<E>* el = &list->elements[i];
        fprintf(file, "\t\t "
                R"(node_%03d [label = <<table border="0" cellborder="1" cellspacing="0">
                       <tr> <td port="index" colspan="2"> %d </td> </tr>
                       <tr> <td port="elem"> elem </td> <td> %d </td> </tr>
                       <tr> <td port="prev"> prev </td> <td> %d </td> </tr>
                       <tr> <td port="next"> next </td> <td> %d </td> </tr>
                   </table>>];)" "\n", i, i, el->element, el->prev_index, el->next_index);
    }

    // fprintf(file, "\t\t edge [constraint = true, style = \"invis\"]; \n");
    // for (int i = 1; i < (int) list->capacity; ++ i)
    //     fprintf(file, "\t\t node_%03d -> node_%03d;\n", i, i + 1);

    fprintf(file, "\t\t edge [constraint = false, style = \"solid\"]; \n");
    for (int i = 1; i <= (int) list->capacity; ++ i) {
        const element<E>* el = &list->elements[i];
        if (el->next_index != 0)
            fprintf(file, "\t\t node_%03d:next -> node_%03d; \n", i, el->next_index);

        if (el->prev_index != -1 && el->prev_index != 0)
            fprintf(file, "\t\t node_%03d:prev -> node_%03d; \n", i, el->prev_index);
    }

    fprintf(file, "\t } \n");

    fprintf(file, "\t\t free [label = \"free\", shape=\"rectangle\", style=\"rounded\"]; \n"
                  "\t\t head [label = \"head\", shape=\"rectangle\", style=\"rounded\"]; \n"
                  "\t\t tail [label = \"tail\", shape=\"rectangle\", style=\"rounded\"]; \n");

    if (list->free != 0)
        fprintf(file, "free -> node_%03d;", list->free);

    if (list->head != 0)
        fprintf(file, "head -> node_%03d;", list->head);

    if (list->tail != 0)
        fprintf(file, "tail -> node_%03d;", list->tail);

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

template <typename E>
void linked_list_destroy(linked_list<E>* list) {
    free(list->elements);
}
