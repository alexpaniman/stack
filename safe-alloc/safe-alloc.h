#pragma once

#include <malloc.h>
#include <string.h>

#include "trace.h"

template <typename E>
stack_trace* safe_calloc(size_t number_of_members, E** allocated_space) {
    E* new_space = (E*) calloc(number_of_members, sizeof(E));

    if (new_space == NULL)
        return FAILURE(RUNTIME_ERROR, "Calloc failed due to %s!"
                       "\n\t" "    number of members: %d"
                       "\n\t" "          member size: %d",
                       "\n\t" "total requested bytes: %d",
                       strerror(errno),
                       number_of_members,  sizeof(E),
                       number_of_members * sizeof(E));

    *allocated_space = new_space; // Successfully allocated
    return SUCCESS();
}

template <typename E>
stack_trace* safe_realloc(E** old_space, size_t number_of_members) {
    E* new_space = (E*) realloc(old_space, number_of_members * sizeof(E));
    if (new_space == NULL)
        return FAILURE(RUNTIME_ERROR, "Realloc failed due to %s!"
                       "\n\t"  "  reallocated pointer: %p" "\n",
                       "\n\t"  "    number of members: %d"
                       "\n\t"  "          member size: %d",
                       "\n\t"  "total requested bytes: %d",
                       strerror(errno), old_space,
                       number_of_members,  sizeof(E),
                       number_of_members * sizeof(E));

    // TODO: Add option to zero out memory 

    *old_space = new_space; // Successfully allocated
    return SUCCESS();
}
