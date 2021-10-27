#include "trace.h"
#include "ansi-colors.h"

#include <cstdio>
#include <cstdlib>

stack_trace* __trace_create_success() {
    stack_trace* new_trace = (stack_trace*)
        calloc(1, sizeof(*new_trace));

    new_trace->trace = NULL;
    new_trace->latest_error = { .error_code = SUCCESS };

    return new_trace;
}

stack_trace* __trace_create_failure(stack_trace* cause, int code,
                                    const char* message, occurance occured) {

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

int trace_error_code(stack_trace* trace) {
    return trace->latest_error.error_code;
}

bool trace_is_success(stack_trace* trace) {
    return trace_error_code(trace) == SUCCESS;
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
    fprintf(stream, COLOR_BLUE "In %s:%d %s:\n" COLOR_RESET, occurance->file,
            occurance->line, occurance->function);
}

void trace_print_stack_trace(FILE* stream, stack_trace* trace) {
    if (trace == NULL || stream == NULL || trace_is_success(trace))
        return;

    __trace_print_occurance(stream, &trace->latest_error.occured);

    fprintf(stream, "==> " COLOR_RED "Error occured: \n" COLOR_RESET);

    __trace_print_description_indented(stream,
        trace->latest_error.description, "    ");

    fprintf(stream, "\n");

    int trace_depth = 1;

    stack_trace* current_trace = trace;
    while ((current_trace = current_trace->trace) != NULL) {
        if (trace_is_success(current_trace))
            break;

        fprintf(stream, "    | " COLOR_BLUE "Depth %d" COLOR_RESET " | ", trace_depth ++);

        __trace_print_occurance(stream,
            &current_trace->latest_error.occured);

        fprintf(stream, "    | ==> " COLOR_RED "Caused error:" COLOR_RESET " \n");

        __trace_print_description_indented(stream,
            current_trace->latest_error.description, "    |    ");

        fprintf(stream, "\n");
    }
}

void trace_destruct(stack_trace* trace) {
    if (trace == NULL)
        return;

    trace_destruct(trace->trace);
    free(trace), trace = NULL;
}

int main(void) {
    stack_trace* trace = FAILURE(RUNTIME_ERROR, "My error, everything is dead");

    trace = PASS_FAILURE(trace, RUNTIME_ERROR, "Passed error");

    trace = PASS_FAILURE(trace, RUNTIME_ERROR, "Triple error");

    trace_print_stack_trace(stderr, trace);

    trace_destruct(trace);
}
