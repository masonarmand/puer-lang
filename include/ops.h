#ifndef OPS_H
#define OPS_H

#include "var.h"
#include "ast.h"

typedef enum {
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_LT,
        OP_GT,
        OP_LE,
        OP_GE,
        OP_EQ,
        OP_NE,
        NUM_OPS
} BinOp;

typedef Var (*BinOpFunc)(Var a, Var b);

typedef struct {
        BinOpFunc ops[NUM_OPS];
} TypeOps;

extern TypeOps type_ops[];

BinOp get_binop(NodeType type);

#endif
