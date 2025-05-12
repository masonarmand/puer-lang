/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

static const char* node_type_names[] = {
        [NODE_NOP]      = "NODE_NOP",
        [NODE_NUM]      = "NODE_NUM",
        [NODE_FLOAT]    = "NODE_FLOAT",
        [NODE_BINOP]    = "NODE_BINOP",
        [NODE_PRINT]    = "NODE_PRINT",
        [NODE_PRINTLN]  = "NODE_PRINTLN",
        [NODE_SEQ]      = "NODE_SEQ",
        [NODE_VAR]      = "NODE_VAR",
        [NODE_VARDECL]  = "NODE_VARDECL",
        [NODE_ASSIGN]   = "NODE_ASSIGN",
        [NODE_IF]       = "NODE_IF",
        [NODE_IFELSE]   = "NODE_IFELSE",
        [NODE_FOR]      = "NODE_FOR",
        [NODE_BREAK]    = "NODE_BREAK",
        [NODE_CONTINUE] = "NODE_CONTINUE",
        [NODE_LT]       = "NODE_LT",
        [NODE_GT]       = "NODE_GT",
        [NODE_LE]       = "NODE_LE",
        [NODE_GE]       = "NODE_GE",
        [NODE_EQ]       = "NODE_EQ",
        [NODE_NE]       = "NODE_NE",
        [NODE_FUNCDEF]  = "NODE_FUNCDEF",
        [NODE_FUNCCALL] = "NODE_FUNCCALL",
        [NODE_RETURN]   = "NODE_RETURN",
        [NODE_IDX]      = "NODE_IDX",
        [NODE_IDXASSIGN]= "NODE_IDXASSIGN",
        [NODE_ARRAYLIT] = "NODE_ARRAYLIT",
        [NODE_ARRAYDECL]= "NODE_ARRAYDECL",
};

static const char* node_type_to_str(NodeType t)
{
        if ((unsigned)t < sizeof(node_type_names)/sizeof(*node_type_names)
        && node_type_names[t])
                return node_type_names[t];
        return "UNKNOWN_NODE";
}

Node* node(NodeType type, YYLTYPE loc, int n_children, ...)
{
        Node* n = malloc(sizeof(Node));
        unsigned int i;

        n->type = type;
        n->n_children = n_children;
        n->children = malloc(sizeof(Node*) * n_children);
        n->vartype = TYPE_VOID;
        n->varname = NULL;
        n->ndims = 0;
        n->lineno = loc.first_line;
        n->column = loc.first_column;

        va_list args;
        va_start(args, n_children);
        for (i = 0; i < n_children; i++)
                n->children[i] = va_arg(args, Node*);
        va_end(args);

        return n;
}

Node* node_binop(BinOp op, YYLTYPE loc, Node* lhs, Node* rhs)
{
        Node* binop = node(NODE_BINOP, loc, 2, lhs, rhs);
        binop->op = op;
}

Node* node_compound(BinOp op, YYLTYPE loc, Node* lhs, Node* rhs)
{
        Node* compound = node(NODE_COMPOUND, loc, 2, lhs, rhs);
        compound->op = op;
}

void setvar(Node* node, VarType type, char* varname)
{
        node->vartype = type;
        node->varname = varname;
}

void setname(Node* node, char* varname)
{
        node->varname = varname;
}

void settype(Node* node, VarType type)
{
        node->vartype = type;
}

Node* node_uminus(Node* n, YYLTYPE loc)
{
        Node* zero = node(NODE_NUM, loc, 0);
        Node* binop = node(NODE_BINOP, loc, 2, zero, n);
        zero->ival = 0;
        binop->op = OP_SUB;

}

Node* node_param(VarType type, char* varname, YYLTYPE loc)
{
        Node* d = node(NODE_VARDECL, loc, 0);
        d->varname = varname;
        d->vartype = type;
        return node(NODE_SEQ, loc, 1, d);
}

Node* node_param_append(Node* list, VarType type, char* varname, YYLTYPE loc)
{
        Node* param = node(NODE_VARDECL, loc, 0);
        param->varname = varname;
        param->vartype = type;
        return node_append(list, param);
}

Node* node_append(Node* list, Node* child)
{
        int old = list->n_children;

        list->n_children = old + 1;
        list->children = realloc(list->children, sizeof(Node*) * list->n_children);
        list->children[old] = child;

        return list;
}

Node* node_append_type(Node* list, NodeType type, YYLTYPE loc)
{
        Node* n = node(type, loc, 0);
        node_append(list, n);
}

void print_ast(Node* node, int depth)
{
        unsigned int i;

        for (i = 0; i < depth; i++)
                printf("  ");

        printf("node type: %s\n", node_type_to_str(node->type));
        for (i = 0; i < node->n_children; i++)
                print_ast(node->children[i], depth + 1);
}

void free_ast(Node* node)
{
        int i;
        if (!node)
                return;

        for (i = 0; i < node->n_children; i++) {
                free_ast(node->children[i]);
        }

        free(node->children);

        if (node->varname) {
                free(node->varname);
        }

        free(node);
}
