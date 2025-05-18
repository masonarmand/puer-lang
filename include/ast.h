/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#ifndef AST_H
#define AST_H

#include "var.h"
#include <stdarg.h>

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
        OP_AND,
        OP_OR,
        NUM_OPS
} BinOp;

typedef enum {
        NODE_NOP,
        NODE_NUM,
        NODE_BOOL,
        NODE_FLOAT,
        NODE_STRING,
        NODE_CHAR,

        NODE_AND,
        NODE_OR,

        NODE_INCDEC,
        NODE_BINOP,
        NODE_COMPOUND,
        NODE_NOT,
        NODE_PRINT,
        NODE_PRINTLN,
        NODE_SEQ,

        NODE_VAR,
        NODE_VARDECL,
        NODE_ASSIGN,

        NODE_IDX,
        NODE_IDXASSIGN,
        NODE_ARRAYLIT,
        NODE_ARRAYDECL,

        NODE_RECDEF,
        NODE_FIELDDECL,
        NODE_FIELDASSIGN,
        NODE_FIELDACCESS,

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
        NODE_RETURN,

        /* for getting enum size */
        NODE_LASTNODE
} NodeType;

typedef struct Node {
        NodeType type;
        BinOp op;
        struct Node** children;
        int n_children;

        /* file metadata */
        int lineno;
        int column;

        int ndims;

        int ival; /* for NODE_NUM */
        float fval; /* for NODE_FLOAT */

        char* recname; /* for record vardecl */
        char* varname; /* function names, variable names, identifiers */
        VarType vartype; /* VARDECL, PARAM, FUNCDECL type */
} Node;

#include "parser.tab.h"

/* ast.c */
Node* node(NodeType type, YYLTYPE loc, int n_children, ...);
Node* node_binop(BinOp op, YYLTYPE loc, Node* lhs, Node* rhs);
Node* node_incdec(BinOp op, YYLTYPE loc, Node* child, int is_prefix);
Node* node_compound(BinOp op, YYLTYPE loc, Node* lhs, Node* rhs);
Node* node_uminus(Node* n, YYLTYPE loc);
Node* node_param(VarType type, char* varname, YYLTYPE loc);
Node* node_param_append(Node* list, VarType type, char* varname, YYLTYPE loc);
Node* node_append(Node* list, Node* child);
Node* node_append_type(Node* list, NodeType type, YYLTYPE loc);

void setvar(Node* node, VarType type, char* varname);
void setname(Node* node, char* varname);
void settype(Node* node, VarType type);

void print_ast(Node* node, int depth);
void free_ast(Node* node);

/* execute.c */
void eval(Node* node);
Var eval_expr(Node* node);
void init_handlers(void);


#endif
