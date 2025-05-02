/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

Node* makeNode(NodeType type, int n_children, ...)
{
        Node* n = malloc(sizeof(Node));
        unsigned int i;

        n->type = type;
        n->n_children;
        n->children = malloc(sizeof(Node*) * n_children);

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

        printf("node type: %d\n", node->type);
        for (i = 0; i < node->n_children; i++)
                print_ast(node->children[i], depth + 1);
}
