/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

static const char* node_type_to_str(NodeType t)
{
        switch (t) {
        case NODE_NOP:         return "NODE_NOP";
        case NODE_NUM:         return "NODE_NUM";
        case NODE_FLOAT:       return "NODE_FLOAT";
        case NODE_BINOP:       return "NODE_BINOP";
        case NODE_PRINT:       return "NODE_PRINT";
        case NODE_PRINTLN:     return "NODE_PRINTLN";
        case NODE_SEQ:         return "NODE_SEQ";
        case NODE_VAR:         return "NODE_VAR";
        case NODE_VARDECL:     return "NODE_VARDECL";
        case NODE_ASSIGN:      return "NODE_ASSIGN";
        case NODE_IF:          return "NODE_IF";
        case NODE_IFELSE:      return "NODE_IFELSE";
        case NODE_FOR:         return "NODE_FOR";
        case NODE_BREAK:       return "NODE_BREAK";
        case NODE_CONTINUE:    return "NODE_CONTINUE";
        case NODE_LT:          return "NODE_LT";
        case NODE_GT:          return "NODE_GT";
        case NODE_LE:          return "NODE_LE";
        case NODE_GE:          return "NODE_GE";
        case NODE_EQ:          return "NODE_EQ";
        case NODE_NE:          return "NODE_NE";
        case NODE_FUNCDEF:     return "NODE_FUNCDEF";
        case NODE_FUNCCALL:    return "NODE_FUNCCALL";
        case NODE_RETURN:      return "NODE_RETURN";
        case NODE_IDX:         return "NODE_IDX";
        case NODE_IDXASSIGN:   return "NODE_IDXASSIGN";
        case NODE_ARRAYLIT:    return "NODE_ARRAYLIT";
        case NODE_ARRAYDECL:   return "NODE_ARRAYDECL";
        case NODE_RECDEF:      return "NODE_RECDEF";
        case NODE_FIELDDECL:   return "NODE_FIELDDECL";
        case NODE_FIELDASSIGN: return "NODE_FIELDASSIGN";
        case NODE_FIELDACCESS: return "NODE_FIELDACCESS";
        case NODE_INCDEC:      return "NODE_INCDEC";
        default:               return "UNKNOWN_NODE";
        }
        return "UNKNOWN_NODE"; /* unreachable */
}

Node* node(NodeType type, YYLTYPE loc, unsigned int n_children, ...)
{
        Node* n = malloc(sizeof(Node));
        unsigned int i;
        va_list args;

        n->type = type;
        n->n_children = n_children;
        n->children = malloc(sizeof(Node*) * n_children);
        n->vartype = TYPE_VOID;
        n->varname = NULL;
        n->recname = NULL;
        n->ndims = 0;
        n->lineno = loc.first_line;
        n->column = loc.first_column;

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
        return binop;
}

Node* node_incdec(BinOp op, YYLTYPE loc, Node* child, int is_prefix)
{
        Node* incdec = node(NODE_INCDEC, loc, 1, child);
        incdec->op = op;
        incdec->ival = is_prefix;
        return incdec;
}

Node* node_compound(BinOp op, YYLTYPE loc, Node* lhs, Node* rhs)
{
        Node* compound = node(NODE_COMPOUND, loc, 2, lhs, rhs);
        compound->op = op;
        return compound;
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
        return binop;
}

Node* node_param(VarType type, int isarr, char* varname, YYLTYPE loc)
{
        NodeType ntype = (isarr) ? NODE_ARRAYDECL : NODE_VARDECL;
        Node* d = node(ntype, loc, 0);
        d->varname = varname;
        /*d->vartype = type;*/
        d->vartype = (isarr) ? TYPE_ARRAY : type;

        if (type == TYPE_REC)
                d->recname = g_recname;

        return d;
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
        return n;
}

void print_ast(Node* node, unsigned int depth)
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
        unsigned int i;
        if (!node)
                return;

        for (i = 0; i < node->n_children; i++) {
                free_ast(node->children[i]);
        }

        free(node->children);

        if (node->varname)
                free(node->varname);
        if (node->recname)
                free(node->recname);

        free(node);
}
