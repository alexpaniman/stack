#include "linked-list.h"
#include "test-framework.h"

#include "simple-stack.h"
#include "trace.h"
#include <stdlib.h>

typedef char* frame_t;

simple_stack<frame_t> frame_stack_create(void) {
    simple_stack<frame_t> stack;
    simple_stack_create(&stack);

    return stack;
}

template <typename E>
void frame(linked_list<E>* list, simple_stack<frame_t>* frames) {
    char* name = linked_list_call_graphviz(list);
    simple_stack_push(frames, name);
}

void show(simple_stack<frame_t>* frames) {
    char* buffer = (char*)
        calloc(frames->next_index * MAX_TMP_NAME_SIZE, sizeof(char));

    strcat(buffer, "sxiv ");

    for (int i = 0; i < (int) frames->next_index; ++ i) {
        strcat(buffer, frames->elements[i]);
        strcat(buffer, " ");

        free(frames->elements[i]);
    }

    system(buffer);

    free(buffer), buffer = NULL;
}


#define ASSERT_CONTENT(list, type, ...)                                   \
    do {                                                                  \
        type __expected_content[] = { __VA_ARGS__ };                      \
                                                                          \
        element_index_t count = 0;                                        \
        for (element<type>* current = linked_list_head(list);             \
                current != linked_list_end (list);                        \
                current =  linked_list_next(list, current))               \
            ASSERT_EQUAL(__expected_content[count ++], current->element); \
    } while(0)


TEST(populate_linked_list_with_different_values) {
    linked_list<int> list = {};
    linked_list_create(&list, 10);

    element_index_t places[5];
    for (int i = 0; i < 5; ++ i)
        linked_list_push_back(&list, i, &places[i]);
    ASSERT_CONTENT(&list, int, 0, 1, 2, 3, 4);

    linked_list_delete(&list, places[1]);
    ASSERT_CONTENT(&list, int, 0, 2, 3, 4);

    linked_list_delete(&list, places[0]);
    ASSERT_CONTENT(&list, int, 2, 3, 4);

    linked_list_push_front(&list, 6, &places[0]);
    ASSERT_CONTENT(&list, int, 6, 2, 3, 4);

    linked_list_linealize(&list);
    print_text_dump(&list);

    linked_list_insert_after(&list, 1, places[0], &places[1]);
    ASSERT_CONTENT(&list, int, 6, 1, 2, 3, 4);

    linked_list_pop_back(&list);
    ASSERT_CONTENT(&list, int, 1, 2, 3, 4);

    linked_list_pop_front(&list);
    ASSERT_CONTENT(&list, int, 2, 3, 4);

    linked_list_destroy(&list);
}

TEST(test_linked_list) {
    linked_list<int> list;
    linked_list_create(&list, 10);

    simple_stack<frame_t> frames = frame_stack_create();

    for (int i = 0; i < 9; ++ i) {
        frame(&list, &frames);

        if (i != 10)
            linked_list_push_back(&list, i, NULL);
    }

    frame(&list, &frames);

    // int place2 = 0;
    // trace_print_stack_trace(stdout, linked_list_insert_after(&list, 25, 1, &place2));
    // frame(&list, &frames);

    // linked_list_linearize(&list);
    // frame(&list, &frames);

    // trace_print_stack_trace(stdout, linked_list_delete(&list, place2));
    // frame(&list, &frames);

    // trace_print_stack_trace(stdout, linked_list_delete_last(&list));
    // frame(&list, &frames);


    show(&frames);

    simple_stack_destruct(&frames);

    linked_list_destroy(&list);
}

TEST_MAIN()
