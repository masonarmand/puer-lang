/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#ifndef AST_H
#define AST_H

#include "var.h"
#include <stdarg.h>

#define GETCHILD(node, n) ((node)->children[(n)])

typedef enum {
        NODE_NOP,
        NODE_NUM,
        NODE_FLOAT,
        NODE_STRING,
        NODE_CHAR,
        NODE_ADD,
        NODE_SUB,
        NODE_MUL,
        NODE_DIV,
        NODE_MOD,
        NODE_NOT,
        NODE_AND,
        NODE_OR,
        NODE_PRINT,
        NODE_PRINTLN,
        NODE_SEQ,

        NODE_VAR,
        NODE_VARDECL,
        NODE_ASSIGN,

        NODE_IDX,
        NODE_IDXASSIGN,

        /* control flow */
        NODE_IF,
        NODE_IFELSE,
        NODE_FOR,
        NODE_WHILE,
        NODE_BREAK,
        NODE_CONTINUE,

        /* conditions */
        NODE_LT,
        NODE_GT,
        NODE_LE,
        NODE_GE,
        NODE_EQ,
        NODE_NE,

        /* functions */
        NODE_FUNCDEF,
        NODE_FUNCCALL,
        NODE_RETURN
} NodeType;

typedef struct {
        char* name;
        VarType type;
} FunctionParam;

typedef struct Node {
        NodeType type;
        struct Node** children;
        int n_children;

        /* file metadata */
        int lineno;
        int column;


        int ival; /* for NODE_NUM */
        float fval; /* for NODE_FLOAT */

        char* varname; /* function names, variable names, identifiers */
        VarType vartype; /* VARDECL, PARAM, FUNCDECL type */
        FunctionParam* params;
        int n_params;
} Node;

/* ast.c */
Node* makeNode(NodeType type, int n_children, ...);
void print_ast(Node* node, int depth);

/* execute.c */
void eval(Node* node);
Var eval_expr(Node* node);

#endif
