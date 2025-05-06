#include "ast.h"
#include "env.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>

#define DEFINE_OPS(opname, op) \
	static float f_##opname(float a, float b) { return a op b; } \
        static long long l_##opname(long long a, long long b) { return a op b; } \
        static unsigned u_##opname(unsigned a, unsigned b) { return a op b; } \
        static int i_##opname(int a, int b) { return a op b; }

#define DEFINE_CMPS(cmpname, op) \
    static int f_##cmpname(float a, float b) { return a op b; } \
    static int l_##cmpname(long long a, long long b) { return a op b; } \
    static int u_##cmpname(unsigned a, unsigned b) { return a op b; } \
    static int i_##cmpname(int a, int b) { return a op b; }

DEFINE_CMPS(lt, <)
DEFINE_CMPS(gt, >)
DEFINE_CMPS(le, <=)
DEFINE_CMPS(ge, >=)
DEFINE_CMPS(eq, ==)
DEFINE_CMPS(ne, !=)
DEFINE_OPS(add, +)
DEFINE_OPS(sub, -)
DEFINE_OPS(mul, *)
DEFINE_OPS(div, /)


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
        /* TODO properly handle print for all var types */
        printf("%d", as_int(eval_expr(node->children[0])));
}

void eval_println(Node* node)
{
        /* TODO properly handle println for all var types */
        if (node->children[0]->type == NODE_NOP) {
                printf("\n");
                return;
        }
        printf("%d\n", as_int(eval_expr(node->children[0])));
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

Var eval_arith(Node* node,
                float (*f_op)(float, float),
                long long (*l_op)(long long, long long),
                unsigned (*u_op)(unsigned, unsigned),
                int (*i_op)(int, int))
{
        Var a = eval_expr(node->children[0]);
        Var b = eval_expr(node->children[1]);
        Var result;
        VarType type = common_type(a.type, b.type);

        switch (type) {
        case TYPE_FLOAT:
                set_float(&result, f_op(to_float(&a), to_float(&b)));
                break;
        case TYPE_LONG:
                set_long(&result, l_op(to_long(&a), to_long(&b)));
                break;
        case TYPE_UINT:
                set_uint(&result, u_op(a.data.ui, b.data.ui));
                break;
        case TYPE_INT:
                set_int(&result, i_op(a.data.i, b.data.i));
                break;
        default:
                die(node, "unsupported type in arithmetic expression");
        }
        return result;
}

Var eval_cmp(Node* node,
        int (*f_cmp)(float, float),
        int (*l_cmp)(long long, long long),
        int (*u_cmp)(unsigned, unsigned),
        int (*i_cmp)(int, int))
{
        Var a = eval_expr(node->children[0]);
        Var b = eval_expr(node->children[1]);
        int result;
        VarType type = common_type(a.type, b.type);

        switch (type) {
        case TYPE_FLOAT:
                result = f_cmp(to_float(&a), to_float(&b));
                break;
        case TYPE_LONG:
                result = l_cmp(to_long(&a), to_long(&b));
                break;
        case TYPE_UINT:
                result = u_cmp(a.data.ui, b.data.ui);
                break;
        case TYPE_INT:
                result = i_cmp(a.data.i, b.data.i);
                break;
        default: die(node, "unsupported type in comparison expression");
        }

        Var out;
        set_int(&out, result);
        return out;
}

Var eval_expr(Node* node)
{
        Var v;

        switch (node->type) {
        case NODE_NOP:
                set_int(&v, 1);
                return v;
        case NODE_NUM:
                set_int(&v, node->ival);
                return v;
        case NODE_VAR:
                Var* found = env_get(node->varname);
                if (!found)
                        die(node, "undefined variable '%s'", node->varname);
                return *found;
        case NODE_ADD:
                return eval_arith(node, f_add, l_add, u_add, i_add);
        case NODE_SUB:
                return eval_arith(node, f_sub, l_sub, u_sub, i_sub);
        case NODE_MUL:
                return eval_arith(node, f_mul, l_mul, u_mul, i_mul);
        case NODE_DIV:
                return eval_arith(node, f_div, l_div, u_div, i_div);

        case NODE_LT:
                return eval_cmp(node, f_lt, l_lt, u_lt, i_lt);
        case NODE_GT:
                return eval_cmp(node, f_gt, l_gt, u_gt, i_gt);
        case NODE_LE:
                return eval_cmp(node, f_le, l_le, u_le, i_le);
        case NODE_GE:
                return eval_cmp(node, f_ge, l_ge, u_ge, i_ge);
        case NODE_EQ:
                return eval_cmp(node, f_eq, l_eq, u_eq, i_eq);
        case NODE_NE:
                return eval_cmp(node, f_ne, l_ne, u_ne, i_ne);

        default:
                die(node, "Unhandled expr node type: %d", node->type);
        }
}
