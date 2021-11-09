#include "linked-list.h"
#include "test-framework.h"

#include "simple-stack.h"
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

TEST(test_linked_list) {
    linked_list<int> list;
    linked_list_create(&list, 10);

    simple_stack<frame_t> frames = frame_stack_create();

    for (int i = 0; i < 5; ++ i) {
        if (i != 10)
            linked_list_insert(&list, i, NULL);

    }

    frame(&list, &frames);

    int place2 = 0;
    trace_print_stack_trace(stdout, linked_list_insert_after(&list, 25, 2, &place2));
    frame(&list, &frames);

    // trace_print_stack_trace(stdout, linked_list_linearize(&list));
    // frame(&list, &frames);

    trace_print_stack_trace(stdout, linked_list_delete(&list, place2));
    frame(&list, &frames);

    trace_print_stack_trace(stdout, linked_list_delete_last(&list));
    frame(&list, &frames);


    show(&frames);

    simple_stack_destruct(&frames);

    linked_list_destroy(&list);
}

TEST_MAIN()
