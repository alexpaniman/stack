#include "trace.h"
#include "ansi-colors.h"

#include <cstdio>
#include <cstdlib>

stack_trace* __trace_create_success() {
    // We only ever need single instance of
    // success, because success has no cause
    static stack_trace success_trace = {
        .latest_error = { .error_code = SUCCESS },
        .trace = NULL
    };

    return &success_trace;
}

int trace_error_code(stack_trace* trace) {
    return trace->latest_error.error_code;
}

bool trace_is_success(stack_trace* trace) {
    return trace_error_code(trace) == SUCCESS;
}

stack_trace* __trace_create_failure(stack_trace* cause, int code,
                                    const char* message, occurance occured) {

    if (cause != NULL && trace_is_success(cause))
        return FAILURE(LOGIC_ERROR, "Error can't be caused by success!");

    stack_trace* new_trace = (stack_trace*)
        calloc(1, sizeof(*new_trace));

    new_trace->trace = cause;
    new_trace->latest_error = error {
        .error_code = code,
        .description = message,
        .occured = occured
    };

    return new_trace;
}

static void __trace_print_description_indented(FILE* stream, const char* string,
                                               const char* indentation) {

    fprintf(stream, "%s", indentation);

    for (int i = 0; string[i] != '\0'; ++ i) {
        char symbol = string[i];

        fprintf(stream, "%c", symbol);

        if (symbol == '\n')
            fprintf(stream, "%s", indentation);
    }

    fprintf(stream, "\n");
}

static void __trace_print_occurance(FILE* stream, occurance* occurance) {
    fprintf(stream, TEXT_INFO("In %s:%d %s:") "\n", occurance->file,
            occurance->line, occurance->function);
}

void trace_print_stack_trace(FILE* stream, stack_trace* trace) {
    if (trace == NULL || stream == NULL || trace_is_success(trace))
        return;

    __trace_print_occurance(stream, &trace->latest_error.occured);

    fprintf(stream, "==> " TEXT_ERROR("Error occured: ") "\n"
            COLOR_WARNING /* Color for description */);

    __trace_print_description_indented(stream,
        trace->latest_error.description, TAB);

    fprintf(stream, COLOR_RESET "\n");

    int trace_depth = 1;

    stack_trace* current_trace = trace;
    while ((current_trace = current_trace->trace) != NULL) {
        if (trace_is_success(current_trace))
            break;

        fprintf(stream, TAB "| " TEXT_SUCCESS("Depth %d") " | ",
                trace_depth ++);

        __trace_print_occurance(stream,
            &current_trace->latest_error.occured);

        fprintf(stream, TAB "| ==> " TEXT_ERROR("Caused error:") " \n");

        __trace_print_description_indented(stream,
            current_trace->latest_error.description,
            TAB COLOR_RESET "|" COLOR_WARNING TAB);

        fprintf(stream, COLOR_RESET "\n");
    }
}

void trace_destruct(stack_trace* trace) {
    if (trace == NULL || trace == SUCCESS())
        return;

    trace_destruct(trace->trace);
    free(trace), trace = NULL;
}

// int main(void) {
//     stack_trace* traced0 = FAILURE(RUNTIME_ERROR, "Fail!");

//     stack_trace* traced1 = PASS_FAILURE(traced0, RUNTIME_ERROR, "Fail!");

//     stack_trace* traced2 = PASS_FAILURE(traced1, RUNTIME_ERROR, "Fail!");

//     stack_trace* traced3 = PASS_FAILURE(traced2, RUNTIME_ERROR, "Fail!");

//     trace_print_stack_trace(stderr, traced3);

//     trace_destruct(traced3);
// }
