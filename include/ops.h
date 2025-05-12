#ifndef OPS_H
#define OPS_H

#include "var.h"
#include "ast.h"

typedef Var (*BinOpFunc)(Var a, Var b);

typedef struct {
        BinOpFunc ops[NUM_OPS];
} TypeOps;

extern TypeOps type_ops[];

BinOp get_binop(NodeType type);

#endif
