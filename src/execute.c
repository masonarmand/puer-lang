#include "ast.h"
#include <stdlib.h>
#include <stdio.h>

void eval(Node* node)
{
        switch (node->type) {
        case NODE_SEQ:
                eval(node->children[0]);
                eval(node->children[1]);
                break;
        case NODE_PRINT:
                printf("%d\n", eval_expr(node->children[0]));
                break;
        default:
                fprintf(stderr, "Unhandled stmt type: %d\n", node->type);
                exit(1);
        }
}


int eval_expr(Node* node)
{
        switch (node->type) {
        case NODE_NUM:
                return node->ival;
                break;
        case NODE_ADD:
                return eval_expr(GETCHILD(node, 0)) + eval_expr(GETCHILD(node, 1));
                break;
        case NODE_SUB:
                return eval_expr(GETCHILD(node, 0)) - eval_expr(GETCHILD(node, 1));
                break;
        case NODE_MUL:
                return eval_expr(GETCHILD(node, 0)) * eval_expr(GETCHILD(node, 1));
                break;
        case NODE_DIV:
                return eval_expr(GETCHILD(node, 0)) / eval_expr(GETCHILD(node, 1));
                break;
        default:
                fprintf(stderr, "Unhandled stmt expr type: %d\n", node->type);
                exit(1);
        }
}
