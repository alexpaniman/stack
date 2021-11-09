#ifndef __CPU_H
#define __CPU_H

#include "commands.h"

typedef struct {
    int* ip;
    command_t* commands;
} cpu;

void process(cpu* processor);

#endif    
