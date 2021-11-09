#include "assembler.h"
#include "registers.h"
#include "trace.h"
#include "lexer.h"

#include <cstdio>
#include <string.h>

struct immediate_constants_table {
    size_t size;
    double* immediate_constants;
};

struct commands_table {
    size_t size;
    command_t* commands;
};

struct assembled_program {
    commands_table commands;

    immediate_constants_table constants;
};

enum argument_type { IMM = 0b001, REG = 0b010, MEM = 0b100 };

struct lexered {
    token* tokens;

    size_t index;
    size_t size;
};

static inline token* get_next_lex(lexered* lex) {
    if (lex->index + 1 < lex->size)
        return &lex->tokens[lex->index ++];

    return NULL;
}

static inline void add_command(assembled_program* prog, command_t command) {
    const size_t ind = prog->commands.size ++;

    if (prog->commands.commands != NULL)
        prog->commands.commands[ind] = command;
}

static inline size_t add_constant(assembled_program* prog, double value) {
    const size_t ind = prog->constants.size ++;
    if (prog->constants.immediate_constants != NULL)
        prog->constants.immediate_constants[ind] = value;

    return prog->constants.size - 1;
}

static inline void add_double_argument(assembled_program* prog,
                                       double value) {

    const size_t const_addr = add_constant(prog, value);
    add_command(prog, (command_t) const_addr);
}

static inline void add_constructed_command(assembled_program* prog,
                                           command_t command,
                                           argument_type arg_type) {

    command_t constructed_command = (command_t) (command << 3);
    constructed_command |= arg_type;

    add_command(prog, constructed_command);
}

stack_trace* parse_argument(lexered* lex, argument_type type, assembled_program* prog) {
    switch (type) {
    case MEM: {
        if (get_next_lex(lex)->type != LSQUARE_BRACKET)
            return FAILURE(RUNTIME_ERROR, "Expected '['!");
            
        const token* number_token = get_next_lex(lex);
        if (number_token->type != NUMBER)
            return FAILURE(RUNTIME_ERROR, "Expected NUMBER!");

        add_double_argument(prog, number_token->number);

        if (get_next_lex(lex)->type != RSQUARE_BRACKET)
            return FAILURE(RUNTIME_ERROR, "Expected ']'!");

        break;
    }

    case IMM: {
        const token* number_token = get_next_lex(lex);
        if (number_token->type != NUMBER)
            return FAILURE(RUNTIME_ERROR, "Expected NUMBER!");

        add_double_argument(prog, number_token->number);

        break;
    }

    case REG: {
        const token* reg_name_token = get_next_lex(lex);
        if (reg_name_token->type != NAME)
            return FAILURE(RUNTIME_ERROR, "Expected '['!");
        // TODO, process reg_name;

        fprintf(stdout, "NOT IMPLEMENTED!");
        return FAILURE(RUNTIME_ERROR, "NOT IMPLEMENTED!!");

        break;
    }

    default:
        fprintf(stdout, "NOT IMPLEMENTED!");
        return FAILURE(RUNTIME_ERROR, "NOT IMPLEMENTED!!");
    }

    return SUCCESS();
}

stack_trace* parse_command(lexered* lex, assembled_program* prog) {
    const token* current = get_next_lex(lex);
    const char* word = current->name.str;

    if (current->type != NAME)
        return FAILURE(RUNTIME_ERROR, "Expected name!");

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("~~~~~~~~~~~~~~> %s", current->name.str);

    #define DEF_COMMAND(enum_name, command_name, arg_type, ...)           \
        if (strncmp(command_name, word, current->name.str_length) == 0) { \
            add_constructed_command(prog, enum_name,                      \
                                    (argument_type) (arg_type));          \
                                                                          \
            parse_argument(lex, (argument_type) (arg_type), prog);        \
                                                                          \
            if (get_next_lex(lex)->type != BR)                            \
                return FAILURE(RUNTIME_ERROR, "Expected new line!");      \
        } else

    #include "commands_defenition.h"
    /* else */ ;

    #undef DEF_COMMAND

    return SUCCESS();
}

#include "malloc.h"

stack_trace* assembly(token* tokens, size_t size, command_t* commands) {
    lexered lex = { .tokens = tokens, .index = 0, .size = size };

    assembled_program prog = {};

    while (lex.index < size)
        parse_command(&lex, &prog);
}
