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
        [NODE_ADD]      = "NODE_ADD",
        [NODE_SUB]      = "NODE_SUB",
        [NODE_MUL]      = "NODE_MUL",
        [NODE_DIV]      = "NODE_DIV",
        [NODE_MOD]      = "NODE_MOD",
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
};

static const char* node_type_to_str(NodeType t)
{
        if ((unsigned)t < sizeof(node_type_names)/sizeof(*node_type_names)
        && node_type_names[t])
                return node_type_names[t];
        return "UNKNOWN_NODE";
}

Node* makeNode(NodeType type, int n_children, ...)
{
        Node* n = malloc(sizeof(Node));
        unsigned int i;

        n->type = type;
        n->n_children = n_children;
        n->children = malloc(sizeof(Node*) * n_children);
        n->vartype = TYPE_VOID;
        n->varname = NULL;
        n->ndims = 0;

        va_list args;
        va_start(args, n_children);
        for (i = 0; i < n_children; i++)
                n->children[i] = va_arg(args, Node*);
        va_end(args);

        return n;
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
