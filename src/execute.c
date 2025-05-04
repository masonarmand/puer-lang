#include "ast.h"
#include "env.h"

#include <stdlib.h>
#include <stdio.h>

typedef void (*StmtHandler)(Node*);

typedef struct {
        NodeType type;
        StmtHandler handler;
} StmtDispatch;

void eval_seq(Node* node);
void eval_print(Node* node);
void eval_println(Node* node);
void eval_vardecl(Node* node);
void eval_assign(Node* node);
void eval_if(Node* node);
void eval_ifelse(Node* node);
void eval_nop(Node* node);

static StmtDispatch handlers[] = {
        { NODE_NOP, eval_nop },
        { NODE_SEQ, eval_seq },
        { NODE_PRINT, eval_print },
        { NODE_PRINTLN, eval_println },
        { NODE_VARDECL, eval_vardecl },
        { NODE_ASSIGN, eval_assign },
        { NODE_IF, eval_if },
        { NODE_IFELSE, eval_ifelse },
        { -1, NULL },
};

void eval(Node* node)
{
        unsigned int i;
        for (i = 0; handlers[i].handler; i++) {
                if (handlers[i].type == node->type) {
                        handlers[i].handler(node);
                        return;
                }
        }

        fprintf(stderr, "Unhandled stmt type: %d\n", node->type);
        exit(1);
}

void eval_seq(Node* node)
{
        eval(node->children[0]);
        eval(node->children[1]);
}

void eval_print(Node* node)
{
        printf("%d", eval_expr(node->children[0]));
}

void eval_println(Node* node)
{
        printf("%d\n", eval_expr(node->children[0]));
}

void eval_vardecl(Node* node)
{
        Var v;
        v.data.i = 0;
        v.type = node->vartype;

        /* if var is initialized with a value */
        if (node->n_children > 0) {
                int result = eval_expr(GETCHILD(node, 0));

                switch (v.type) {
                case TYPE_INT:
                        v.data.i = result;
                        break;
                default:
                        fprintf(stderr, "init expr type mismatch for %s\n", node->varname);
                        exit(1);
                }
        }

        env_set(node->varname, v);
}

void eval_assign(Node* node)
{
        Var* v = env_get(node->varname);
        if (!v) {
                fprintf(stderr, "assignment to undeclared variable '%s'\n", node->varname);
                exit(1);
        }

        /* TODO: support more than just int type */
        int result = eval_expr(GETCHILD(node, 0));

        switch (v->type) {
        case TYPE_INT:
                v->data.i = result;
                break;
        default:
                fprintf(stderr, "Type error: cannot assign to variable '%s'\n", node->varname);
                exit(1);
        }
}

void eval_if(Node* node)
{
        if (eval_expr(node->children[0]))
                eval(node->children[1]);
}

void eval_ifelse(Node* node)
{
        if (eval_expr(node->children[0]))
                eval(node->children[1]);
        else
                eval(node->children[2]);
}

void eval_nop(Node* node)
{
        return;
}


int eval_expr(Node* node)
{
        switch (node->type) {
        case NODE_LT:
                return eval_expr(GETCHILD(node, 0)) < eval_expr(GETCHILD(node, 1));
        case NODE_GT:
                return eval_expr(GETCHILD(node, 0)) > eval_expr(GETCHILD(node, 1));
        case NODE_LE:
                return eval_expr(GETCHILD(node, 0)) <= eval_expr(GETCHILD(node, 1));
        case NODE_GE:
                return eval_expr(GETCHILD(node, 0)) >= eval_expr(GETCHILD(node, 1));
        case NODE_EQ:
                return eval_expr(GETCHILD(node, 0)) == eval_expr(GETCHILD(node, 1));
        case NODE_NE:
                return eval_expr(GETCHILD(node, 0)) != eval_expr(GETCHILD(node, 1));
        case NODE_NUM:
                return node->ival;
        case NODE_VAR:
                Var* v = env_get(node->varname);
                if (!v) {
                        fprintf(stderr, "undefined variable: '%s'\n", node->varname);
                        exit(1);
                }
                return v->data.i;
        case NODE_ADD:
                return eval_expr(GETCHILD(node, 0)) + eval_expr(GETCHILD(node, 1));
        case NODE_SUB:
                return eval_expr(GETCHILD(node, 0)) - eval_expr(GETCHILD(node, 1));
        case NODE_MUL:
                return eval_expr(GETCHILD(node, 0)) * eval_expr(GETCHILD(node, 1));
        case NODE_DIV:
                return eval_expr(GETCHILD(node, 0)) / eval_expr(GETCHILD(node, 1));
        default:
                fprintf(stderr, "Unhandled stmt expr type: %d\n", node->type);
                exit(1);
        }
}
