#include "lexer.h"

#include "simple-stack.h"
#include "trace.h"

#include <ctype.h>
#include <math.h>

stack_trace* tokenize(const char* text, simple_stack<token>* tokens) {
    simple_stack_create(tokens);

    const char* beg_name = NULL; int name_length = 0;

    const char* beg_num  = NULL; int  num_length = 0;

    char symbol = '\0'; int text_index = 0;

    while ((symbol = text[text_index ++]) != L'\0') {
        // Skip comments
        if (symbol == ';')
            while ((symbol = text[text_index ++]) != L'\0')
                if (symbol == '\n')
                    break;
        
        bool recognised = true;

        if (!isalnum(symbol) && symbol != '_' && symbol != '.') {
            if        (name_length > 0) {
                substring name = { beg_name, (size_t) name_length };

                simple_stack_push(tokens, {
                    .type = NAME,
                    .name = name
                });

            } else if ( num_length > 0) {
                double number = NAN;
                char*  endptr = NULL;

                number = strtod(beg_num, &endptr);
                if (endptr != beg_num + num_length)
                    return FAILURE(RUNTIME_ERROR, "Unrecognised number!");

                simple_stack_push(tokens, token {
                        .type   = NUMBER,
                        .number = number
                });
            }

            beg_name = NULL, name_length = 0;
            beg_num  = NULL,  num_length = 0;

            recognised = false;
        }

        switch (symbol) {
            #define SYMBOL_TOKEN(symbol, type)           \
                case symbol:                             \
                    simple_stack_push(tokens, { type }); \
                    break

            SYMBOL_TOKEN( ':',           COLON);
            SYMBOL_TOKEN( ',',           COMMA);
            SYMBOL_TOKEN( '[', LSQUARE_BRACKET);
            SYMBOL_TOKEN( ']', RSQUARE_BRACKET);
            SYMBOL_TOKEN('\n',              BR);

            #undef SIMPLE_TOKEN

        default:
            // Skip whitespace
            if (isspace(symbol))
                break;

            if (!recognised) {
                // printf("~~~~~~~~> '%c'", symbol);
                return FAILURE(RUNTIME_ERROR, "Unrecognised token!");
            }

            if (isdigit(symbol)) {
                if (beg_num == NULL)
                    beg_num  = &text[text_index - 1];
            }
            else {
                if (beg_name == NULL)
                    beg_name = &text[text_index - 1];
            }

            name_length += beg_name != NULL? 1 : 0;
             num_length += beg_num  != NULL? 1 : 0;

            continue;
        }
    }

    return SUCCESS();
}
