#ifndef DEF_COMMAND
#define DEF_COMMAND(...)
#endif

#define STACK_POP __simple_stack_pop(&stk)

DEF_COMMAND(PUSH, "push", IMM, {

})

DEF_COMMAND( POP,  "pop", REG | MEM, {
    STACK_POP
})
