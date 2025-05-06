#include "ast.h"
#include "env.h"
#include "util.h"
#include "ops.h"

#include <stdlib.h>
#include <stdio.h>

typedef void (*StmtHandler)(Node*);

typedef struct {
        NodeType type;
        StmtHandler handler;
} StmtDispatch;

typedef enum {
        CTRL_NONE,
        CTRL_BREAK,
        CTRL_CONTINUE
} CtrlSignal;

Var eval_expr(Node* node);

CtrlSignal eval_with_ctrl(Node* node);
CtrlSignal eval_if_ctrl(Node* node);
CtrlSignal eval_ifelse_ctrl(Node* node);
CtrlSignal eval_block(Node* node);

void eval_seq(Node* node);
void eval_print(Node* node);
void eval_println(Node* node);
void eval_vardecl(Node* node);
void eval_assign(Node* node);
void eval_if(Node* node);
void eval_ifelse(Node* node);
void eval_for(Node* node);
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
        { NODE_FOR, eval_for },
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

        die(node, "unhandled stmt type: %d", node->type);
}

/* for handling break & continue in loops */
CtrlSignal eval_with_ctrl(Node* node)
{
        unsigned int i;

        switch (node->type) {
        case NODE_SEQ:
                for (i = 0; i < node->n_children; i++) {
                        CtrlSignal sig = eval_with_ctrl(node->children[i]);
                        if (sig != CTRL_NONE)
                                return sig;
                }
                return CTRL_NONE;
        case NODE_BREAK:
                return CTRL_BREAK;
        case NODE_CONTINUE:
                return CTRL_CONTINUE;
        case NODE_IF:
                return eval_if_ctrl(node);
        case NODE_IFELSE:
                return eval_ifelse_ctrl(node);
        default:
                eval(node);
                return CTRL_NONE;
        }
}

CtrlSignal eval_if_ctrl(Node* node)
{
        if (as_int(eval_expr(node->children[0])))
                return eval_block(node->children[1]);
        return CTRL_NONE;
}

CtrlSignal eval_ifelse_ctrl(Node* node)
{
        if (as_int(eval_expr(node->children[0])))
                return eval_block(node->children[1]);
        else
                return eval_block(node->children[2]);
}

CtrlSignal eval_block(Node* node)
{
        env_push();
        CtrlSignal sig = eval_with_ctrl(node);
        env_pop();
        return sig;
}

void eval_seq(Node* node)
{
        eval(node->children[0]);
        eval(node->children[1]);
}

void eval_print(Node* node)
{
        Var v = eval_expr(node->children[0]);
        switch (v.type) {
        case TYPE_INT:
                printf("%d", v.data.i);
                break;
        case TYPE_UINT:
                printf("%u", v.data.ui);
                break;
        case TYPE_LONG:
                printf("%ld", v.data.l);
                break;
        case TYPE_FLOAT:
                printf("%f", v.data.f);
                break;
        case TYPE_BOOL:
                /* TODO (data == 1) ? "true" : "false" */
                break;
        case TYPE_STRING:
                printf("%s", v.data.s);
                break;
        default:
                die(node, "unsupported type in print");
        }
}

void eval_println(Node* node)
{
        if (node->children[0]->type == NODE_NOP) {
                printf("\n");
                return;
        }
        eval_print(node);
        printf("\n");
}

void eval_vardecl(Node* node)
{
        Var v;
        v.type = node->vartype;

        /* if var is initialized with a value */
        if (node->n_children > 0) {
                Var result = eval_expr(GETCHILD(node, 0));

                if (result.type != node->vartype) {
                        die(node, "init expr type mismatch for '%s'", node->varname);
                }
                v = result;
        }

        env_set(node->varname, v);
}

void eval_assign(Node* node)
{
        Var* v = env_get(node->varname);
        Var result;
        if (!v) {
                die(node, "assignment to undeclared variable '%s'", node->varname);
        }

        result = eval_expr(GETCHILD(node, 0));
        if (result.type != v->type)
                die(node, "Type error: cannot assign to variable '%s'", node->varname);
        v->data = result.data;
}

void eval_if(Node* node)
{
        if (as_int(eval_expr(node->children[0])))
                eval_block(node->children[1]);
}

void eval_ifelse(Node* node)
{
        if (as_int(eval_expr(node->children[0])))
                eval_block(node->children[1]);
        else
                eval_block(node->children[2]);
}

void eval_for(Node* node)
{
        /* for init */
        env_push();
        eval(node->children[0]);

        while (as_int(eval_expr(node->children[1]))) {
                /* for body */
                CtrlSignal sig = eval_block(node->children[3]);
                if (sig == CTRL_BREAK)
                        break;
                if (sig == CTRL_CONTINUE) {
                        eval(node->children[2]);
                        continue;
                }

                /* for incr */
                eval(node->children[2]);

        }
        env_pop();
}

void eval_nop(Node* node)
{
        return;
}

Var eval_expr(Node* node)
{
        Var v;
        Var* found;

        switch (node->type) {
        case NODE_NOP:
                set_int(&v, 1);
                return v;
        case NODE_VAR:
                found = env_get(node->varname);
                if (!found)
                        die(node, "undefined variable '%s'", node->varname);
                return *found;
        case NODE_NUM:
                set_int(&v, node->ival);
                return v;
        case NODE_FLOAT:
                set_float(&v, node->fval);
                return v;
        default: {
                Var a;
                Var b;
                VarType type;
                BinOpFunc func;
                BinOp op = get_binop(node->type);

                if (op == -1)
                        die(node, "Unhandled expr node type: '%d'", node->type);

                a = eval_expr(node->children[0]);
                b = eval_expr(node->children[1]);
                type = common_type(a.type, b.type);
                func = type_ops[type].ops[op];

                if (!func)
                        die(node, "Operator not supported for this type");

                return func(a, b);
        }
        }
}
