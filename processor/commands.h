typedef unsigned char command_t;

enum commands {
    #define DEF_COMMAND(name, ...) \
        name,

    #include "commands_defenition.h"

    #undef DEF_COMMAND
};
