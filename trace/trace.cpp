#include "trace.h"
#include "ansi-colors.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


static stack_trace __trace_stack_trace_reserved_space_in_case_calloc_fails;
static char __trace_error_message_reserved_space_in_case_calloc_fails[256];

stack_trace* __trace_create_failure(stack_trace* cause, int code, occurance occured,
                                    const char* format, ...) {

    if (cause != NULL && trace_is_success(cause))
        return FAILURE(LOGIC_ERROR, "Error can't be caused by success!");

    stack_trace* new_trace = (stack_trace*) calloc(1, sizeof(*new_trace));

    if (new_trace == NULL) {
        fprintf(stderr, "Lost and unwrapped error: %s\n", format);
        format = "Trace allocation failed, actual error was lost";

        // We could be unable to notify user about actual error with
        // trace mechanism, that's a big problem, let's show message:
        perror(format);

        // calloc failed, but we still need a way to notify user
        // about error, so we will use static space for that:
        new_trace = &__trace_stack_trace_reserved_space_in_case_calloc_fails;
    }

    va_list  vprintf_args;
    va_start(vprintf_args, format);

    #pragma clang diagnostic push

    // This function is intended for use only with string
    // literals, so disabling warning should be ok:
    #pragma clang diagnostic ignored "-Wformat-nonliteral"

    va_list args_copy_for_size_calculation;
    va_copy(args_copy_for_size_calculation, vprintf_args);

    // Calculate buffer size first, works since C99 
    int buffer_size = vsnprintf(NULL, 0, format, args_copy_for_size_calculation) + 1;
    #pragma clang diagnostic pop

    if (buffer_size < 0) {
        fprintf(stderr, "Lost and unwrapped error: %s\n", format);

        // vsprintf failed, we need to notify user about that:
        format = "Message size calculation failed, actual error was lost!";

        // We could be unable to notify user about actual error with
        // trace mechanism, that's a big problem, let's show message:
        fprintf(stderr, "%s", format);

        // Update buffer size, format is just a string now, so it's size will be just:
        buffer_size = (int) strlen(format);
    }

    char* message_buffer = (char*) calloc((size_t) buffer_size, sizeof(*message_buffer));

    if (message_buffer == NULL) {
        fprintf(stderr, "Lost and unwrapped error: %s\n", format);
        format = "Trace message allocation failed, actual error was lost";

        // We could be unable to notify user about error with
        // trace mechanism, that's a big problem, let's show message:
        perror("Trace message allocation failed, actual error was lost");

        message_buffer = __trace_error_message_reserved_space_in_case_calloc_fails;
    }

    #pragma clang diagnostic push

    // This function is intended for use only with string
    // literals, so disabling warning should be ok:
    #pragma clang diagnostic ignored "-Wformat-nonliteral"
    int written_byte = vsnprintf(message_buffer, (size_t) buffer_size,
                                 format, vprintf_args);
    #pragma clang diagnostic pop

    // Final message with const qualifier
    const char* message = message_buffer;

    if (written_byte < 0) {
        fprintf(stderr, "Lost and unwrapped error: %s\n", format);
        message = "Error message construction failed, actual error was lost!";

        // We are unable to notify our user about actual error with
        // trace mechanism, that's a big problem, let's show message:
        fprintf(stderr, "%s", message);
    }

    new_trace->trace = cause;
    new_trace->latest_error = error {
        .error_code = code,
        .description = message_buffer,
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

void trace_throw(FILE* stream, stack_trace* trace) {
    const bool is_success = trace_is_success(trace);

    trace_print_stack_trace(stream, trace);
    trace_destruct(trace);

    if (!is_success)
        abort(); // Use only for testing purposes
}

void trace_destruct(stack_trace* trace) {
    if (trace == NULL || trace == SUCCESS())
        return;

    trace_destruct(trace->trace);

    free((char*) trace->latest_error.description);
    free(trace), trace = NULL;
}

thread_local jmp_buf finally_return_addr = {};
