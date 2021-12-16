#pragma once

#include <cstdio>
#include <cstddef>
#include <setjmp.h>
#include <errno.h>

/**
 * Error codes meant primarily for #error.
 * 
 * If you want to make your own error with unique code,
 * you should create second enum like #error_codes in
 * which first value will be +1 bigger then the last
 * value in #error_codes, so they don't overlap.
 */
enum error_codes {
    SUCCESS,      //!< Successfully finished

    LOGIC_ERROR,  //!< Bug that causes program to operate incorrectly,
                  //!< but not to terminate abnormally

    RUNTIME_ERROR //!< Error that occurs due to illegal operation in 
                  //!< a program, that causes it to terminate
};

/**
 * Struct that describes where #error happend
 * It's usually constructed via #__LINE__, #__FUNCTION__ and #__ macro
 */
struct occurance {
    int line;             //!< Line number on which error occured
    const char* file;     //!< Full name of file in which error occured

    const char* function; //!< Full name of function in which error occured 
};

/**
 * Error description. You can use it in conjuction with
 * #error_codes to describe your own errors.
 *
 * If you want to make error with unique error code, you
 * should look into documentation for #error_codes
 *
 * @note There's error code #SUCCESS
 */
struct error {
    int error_code;    //!< Error code from #error_codes or extended from it enum

    char* description; //!< Error description in english
    occurance occured;
};

/**
 * Represents linked list of errors that caused each
 * other in chronological order.
 */
struct stack_trace {
    error latest_error; //!< Last error in chronological order

    stack_trace *trace; //!< Linked list of errors that cause it,
                        //!< #NULL if doesn't have cause
};


stack_trace* __trace_create_success();
stack_trace* __trace_create_failure(stack_trace* cause,
    int code, occurance occured, const char* message, ...);


#define __TRACE_CREATE_OCCURANCE()         \
    (occurance { .line = __LINE__,         \
                 .file = __FILE__,         \
                 .function = __FUNCTION__ })


#define SUCCESS() __trace_create_success()

#define FAILURE(            code, ...)                                         \
    __trace_create_failure( NULL, code, __TRACE_CREATE_OCCURANCE(), __VA_ARGS__)

#define PASS_FAILURE(cause, code, ...)                                         \
    __trace_create_failure(cause, code, __TRACE_CREATE_OCCURANCE(), __VA_ARGS__)


bool trace_is_success(stack_trace* trace);

void trace_print_stack_trace(FILE* stream, stack_trace* trace);

void trace_destruct(stack_trace* trace);


#define TRY                                                       \
    do {                                                          \
        stack_trace* __trace = ({

#define CATCH(impl)                                               \
        ;   });                                                   \
        if (!trace_is_success(__trace)) {                         \
            impl                                                  \
        }                                                         \
    } while(false)

#define FAIL(...)                                                 \
    CATCH({                                                       \
        return PASS_FAILURE(__trace, RUNTIME_ERROR, __VA_ARGS__); \
    })                                                            \

#define THROW(...)                                                \
    CATCH({                                                       \
        stack_trace* __current_trace =                            \
            PASS_FAILURE(__trace, RUNTIME_ERROR, __VA_ARGS__);    \
                                                                  \
        trace_print_stack_trace(stderr, __current_trace);         \
        /* Free memory before aborting! */                        \
        trace_destruct(__current_trace);                          \
        /* This should be avoided in favour of FAIL */            \
        abort(); /* Dangerous way of handling errors */           \
    })


extern thread_local jmp_buf finally_return_addr;

#define FINALIZER(name, impl)                                     \
    jmp_buf finalizer_##name = {};                                \
    if (setjmp(finalizer_##name) != 0) {                          \
        impl                                                      \
        longjmp(finally_return_addr, -1);                         \
    }

#define CALL_FINALIZER(finalizer)                                 \
    do {                                                          \
        if (setjmp(finally_return_addr) == 0)                     \
            longjmp(finalizer_##finalizer, -1);                   \
    } while(false)                                                \


#define FINALIZE_AND_FAIL(finalizer, ...)                         \
    CATCH({                                                       \
        CALL_FINALIZER(finalizer);                                \
        return PASS_FAILURE(__trace, RUNTIME_ERROR, __VA_ARGS__); \
    })
