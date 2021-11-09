#ifndef __ASSEMBLER_H
#define __ASSEMBLER_H

#include "commands.h"
#include "lexer.h"
#include "trace.h"

stack_trace* assembly(token* tokens, size_t size, command_t* commands);

#endif
