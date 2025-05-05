/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#ifndef AST_H
#define AST_H

#include <stdarg.h>
#include "var.h"

#define GETCHILD(node, n) ((node)->children[(n)])

typedef enum {
        NODE_NOP,
        NODE_NUM,
        NODE_ADD,
        NODE_SUB,
        NODE_MUL,
        NODE_DIV,
        NODE_PRINT,
        NODE_PRINTLN,
        NODE_SEQ,

        NODE_VAR,
        NODE_VARDECL,
        NODE_ASSIGN,

        /* control flow */
        NODE_IF,
        NODE_IFELSE,
        NODE_FOR,
        NODE_BREAK,
        NODE_CONTINUE,

        /* conditions */
        NODE_LT,
        NODE_GT,
        NODE_LE,
        NODE_GE,
        NODE_EQ,
        NODE_NE
} NodeType;

typedef struct Node {
        NodeType type;
        struct Node** children;
        int n_children;
        int ival; /* for NODE_NUM */
        char* varname;
        VarType vartype;
} Node;

/* ast.c */
Node* makeNode(NodeType type, int n_children, ...);
void print_ast(Node* node, int depth);

/* execute.c */
void eval(Node* node);

#endif
