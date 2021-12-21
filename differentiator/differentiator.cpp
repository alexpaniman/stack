#include "hash-table.h"
#include "default-hash-functions.h"
#include "safe-alloc.h"
#include "trace.h"
#include <cassert>

enum operator_type { SUM, SUB, MUL, DIV, POW };

// Names for all expression types
hash_table<int, const char*> operator_names =
    HASH_TABLE(int, const char*, int_hash,
               PAIR(SUM , "+"), PAIR(SUB , "-"),
               PAIR(MUL , "*"), PAIR(DIV , "/"),
               PAIR(POW , "^"));

struct expression_node;

struct binary_operator {
    operator_type type;
    expression_node *left, *right;
};

enum expression_type { CONST, BINARY_OPERATOR };

struct expression_node {
    expression_type type;

    union {
        binary_operator op;
        double number;
    };
};

struct variable {
    const char* name;
};

expression_node* expression_node_create() {
    expression_node* node = NULL;
    TRY safe_calloc(1, &node)
        THROW("Memory allocation for node failed!");

    return node;
}

#define OPERATOR(name, type_name)                                 \
    static inline expression_node* name(expression_node *left,    \
                                        expression_node *right) { \
                                                                  \
        expression_node* node = expression_node_create();         \
        *node = { .type = BINARY_OPERATOR,                        \
                  .op   = { (type_name), (left), (right) } };     \
                                                                  \
        return node;                                              \
    }

OPERATOR(sub, SUB)
OPERATOR(sum, SUM)
OPERATOR(mul, MUL)
OPERATOR(div, DIV)
OPERATOR(pow, POW)

static inline expression_node* c(double constant) {
    expression_node* node = expression_node_create();        
    *node = { .type = CONST, .number = constant };  
                                                             
    return node;                                             
}

expression_node* expression_copy(expression_node* source) {
    expression_node* new_node = expression_node_create();

    // Copy everything
    *new_node = *source;
    if (source->type == CONST)
        return new_node;

    // It's an operator
    // We need a deep copy of its left and right expressions

    new_node->op.left  = expression_copy(source->op.left );
    new_node->op.right = expression_copy(source->op.right);

    return new_node;
}

expression_node* expression_derive(expression_node* source, variable var) {
    #define DL expression_derive(source->op.left , var)
    #define DR expression_derive(source->op.right, var)
    #define CL expression_copy  (source->op.left )
    #define CR expression_copy  (source->op.right)

    switch (source->op.type) {
    case SUM: return sum(DL, /* + */ DR);

    case SUB: return sub(DL, /* - */ DR);

    case MUL: return sum(mul(CL, /* x */ DR), /* + */ mul(CR, /* x */ DL));

    case POW: return mul(pow(CL,     /* ^ */ sub(CR, /* - */ c(1.0))),
                         sum(mul(CL, /* x */ mul(DR, /* x */ ln(CL))),
                             mul(DL,           /* x */          CR)));

    case DIV: return div(sub(mul(CL, /* x */ DR), mul(CR, /* x */ DL)),
                         /* / */ pow(DR, /* ^ */ c(2.0)));
    default: assert(0);
    }

    #undef DR
    #undef DL
    #undef CL
    #undef CR
}
