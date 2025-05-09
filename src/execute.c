#include "ast.h"
#include "env.h"
#include "util.h"
#include "ops.h"
#include "func.h"
#include "builtin.h"
#include "arraylist.h"

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
        CTRL_CONTINUE,
        CTRL_RETURN
} CtrlSignal;

/* function return value */
static Var g_retval;

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
void eval_while(Node* node);
void eval_nop(Node* node);
void eval_funcdef(Node* node);
void eval_funccall_stmt(Node* node);
void eval_idxassign(Node* node);
void eval_arraydecl(Node* node);

Var eval_funccall(Node* node);

/* helpers */
void print_var(Node* node, const Var* v);

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
        { NODE_WHILE, eval_while },
        { NODE_FUNCDEF, eval_funcdef },
        { NODE_FUNCCALL, eval_funccall_stmt },
        { NODE_IDXASSIGN, eval_idxassign },
        { NODE_ARRAYDECL, eval_arraydecl },
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
        case NODE_RETURN:
                if (node->n_children == 0)
                        set_void(&g_retval);
                else
                        g_retval = eval_expr(node->children[0]);
                return CTRL_RETURN;
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

void print_var(Node* node, const Var* v)
{
        switch (v->type) {
        case TYPE_INT:
                printf("%d", v->data.i);
                break;
        case TYPE_UINT:
                printf("%u", v->data.ui);
                break;
        case TYPE_LONG:
                printf("%ld", v->data.l);
                break;
        case TYPE_FLOAT:
                printf("%f", v->data.f);
                break;
        case TYPE_BOOL:
                /* TODO (data == 1) ? "true" : "false" */
                printf(v->data.b ? "true" : "false");
                break;
        case TYPE_STRING:
                printf("%s", v->data.s->data);
                break;
        case TYPE_ARRAY:
                ArrayList* a = v->data.a;
                int i;
                printf("[");
                for (i = 0; i < a->size; i++) {
                        print_var(node, &a->items[i]);
                        if (i + 1 < a->size)
                                printf(", ");
                }
                printf("]");
                break;
        default:
                die(node, "unsupported type in print");
        }
}

void eval_print(Node* node)
{
        Node* args = node->children[0];
        unsigned int i;
        for (i = 0; i < args->n_children; i++) {
                Var v = eval_expr(args->children[i]);
                print_var(node, &v);

                /* spaces between args */
                if (i < args->n_children - 1)
                        printf(" ");
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
        else if (v.type == TYPE_STRING) {
                v.data.s = string_new("");
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

void eval_while(Node* node)
{
        while(as_int(eval_expr(node->children[0]))) {
                CtrlSignal sig = eval_block(node->children[1]);
                if (sig == CTRL_BREAK)
                        break;
                if (sig == CTRL_CONTINUE) {
                        continue;
                }
        }
}

void eval_nop(Node* node)
{
        return;
}

void eval_funcdef(Node* node)
{
        /*
        Node* plist = node->children[0];
        Node* body  = node->children[1];
        int   pcount = plist->n_children;
        int i;

        printf(
                "Parsed function `%s` (returns %d) with %d params:\n",
                node->varname,
                node->vartype,
                pcount
        );

        for (i = 0; i < pcount; i++) {
                Node *param = plist->children[i];
                printf("  - %d %s\n", param->vartype, param->varname);
        }*/
        func_set(node->varname, node);

        /*printf("Body AST:\n");
        print_ast(body, 0);*/
}

void eval_funccall_stmt(Node* node)
{
        (void)eval_funccall(node);
}

Var eval_funccall(Node* node)
{
        Node* func;
        Node* param_list;
        Node* body;
        int expected;
        int given;
        int i;
        CtrlSignal sig;
        Var result;

        if (call_builtin_if_exists(node, &result))
                return result;

        if (!(func = func_get(node->varname)))
                die(node, "undefined function '%s'", node->varname);

        param_list = func->children[0];
        body = func->children[1];

        expected = param_list->n_children;
        given = node->children[0]->n_children;

        if (expected != given)
                die(node, "function '%s' expects %d args, got %d", node->varname, expected, given);

        env_push();
        for (i = 0; i < expected; i++) {
                Node* param = param_list->children[i];
                Node* arg_expr = node->children[0]->children[i];
                Var arg_val = eval_expr(arg_expr);

                if (arg_val.type != param->vartype) {
                        die(node, "function '%s' argument %d: expected type %d, got %d",
                                node->varname, i + 1, param->vartype, arg_val.type);
                }
                env_set(param->varname, arg_val);
        }

        sig = eval_with_ctrl(body);
        if (sig == CTRL_RETURN) {
                if (g_retval.type != func->vartype) {
                        die(node, "function '%s': return type mismatch (expected %d, got %d)",
                                node->varname, func->vartype, g_retval.type);
                }
        }
        else {
                if (func->vartype != TYPE_VOID)
                        die(node, "function '%s': missing return value", node->varname);
                set_void(g_retval);
        }

        env_pop();
        return g_retval;
}

void eval_idxassign(Node* node)
{
        Var container = eval_expr(node->children[0]);
        Var idx = eval_expr(node->children[1]);
        Var val = eval_expr(node->children[2]);
        int i;

        if (idx.type != TYPE_INT)
                die(node, "index must be an integer");

        i = idx.data.i;

        if (container.type == TYPE_STRING) {
                if (val.type != TYPE_INT)
                        die(node, "Can only assign char to a string character");
                string_set(container.data.s, i, (char)val.data.i);
        }
        else if (container.type == TYPE_ARRAY) {
                ArrayList* a = container.data.a;

                if (i < 0 || i >= a->size) {
                        die(
                                node,
                                "array index %d out of bounds (size %d)",
                                i, a->size
                        );
                }

                if (val.type != a->type) {
                        die(
                                node,
                                "Type mismatch: array holds %d but got %d",
                                a->type, val.type
                        );
                }
                a->items[i] = val;
        }
        else {
                die(node, "Cannot index‐assign into type %d", container.type);
        }
}

void eval_arraydecl(Node* node)
{
        Node* dims_node = node->children[0];
        Node* init_node = node->children[1];
        int ndims = dims_node->n_children;
        Var value;

        if (ndims > 0 && init_node->type == NODE_NOP) {
                int* dims = malloc(sizeof(int) * ndims);
                int i;
                for (i = 0; i < ndims; i++)
                        dims[i] = dims_node->children[i]->ival;
                value = build_zero_array(node->vartype, dims, ndims);
                free(dims);
        }
        if (init_node->type != NODE_NOP) {
                value = eval_expr(init_node);
                if (value.type != TYPE_ARRAY) {
                        die(node, "initializer for '%s' must be an array", node->varname);
                }
        }
        else {
                value = build_zero_array(node->vartype, NULL, 0);
        }

        env_set(node->varname, value);
}

Var eval_arraylit(Node* node)
{
        int n = node->n_children;
        VarType type = TYPE_INT;
        ArrayList* arr;
        int i;
        Var out;

        if (n > 0) {
                Var first = eval_expr(node->children[0]);
                type = first.type;
        }

        arr = arraylist_new(type, n);
        for (i = 0; i < n; i++) {
                Var v = eval_expr(node->children[i]);
                if (v.type != type) {
                        die(
                                node,
                                "array literal: element %d has type %d, expected %d",
                                i, v.type, type
                        );
                }
                arraylist_push(arr, v);
        }

        set_array(&out, arr);
        return out;
}

Var eval_binop(Node* node)
{
        Var a;
        Var b;
        VarType type;
        BinOpFunc func;
        BinOp op = get_binop(node->type);

        if (op == -1)
                die(node, "Unhandled expr node type: '%d'", node->type);

        a = eval_expr(node->children[0]);
        b = eval_expr(node->children[1]);
        /*type = common_type(a.type, b.type);*/

        /* overide '+' for string concat */
        if (a.type == TYPE_STRING && b.type == TYPE_STRING && op == OP_ADD) {
                Var result;
                result.type = TYPE_STRING;
                result.data.s = string_concat(a.data.s, b.data.s);
                return result;
        }

        type = coerce(&a, &b);

        func = type_ops[type].ops[op];

        if (!func)
                die(node, "Operator not supported for this type");

        return func(a, b);
}

Var eval_idx(Node* node)
{
        Var container = eval_expr(node->children[0]);
        Var idx = eval_expr(node->children[1]);
        Var v;
        int i;

        if (idx.type != TYPE_INT)
                die(node, "index must be an integer");

        i = idx.data.i;

        if (container.type == TYPE_STRING) {
                char c;
                if (i < 0 || i >= (int)container.data.s->length)
                        die(node, "string index %d out of bounds", i);
                c = string_get(container.data.s, i);
                set_int(&v, (int)c);
                return v;
        }
        else if (container.type == TYPE_ARRAY) {
                ArrayList* a = container.data.a;
                if (i < 0 || i >= a->size)
                        die(node, "array index %d out of bounds (size %d)", i, a->size);
                return a->items[i];
        }
        else {
                die(node, "cannot index type %d", container.type);
        }
}

Var eval_expr(Node* node)
{
        Var v;
        Var* found;

        /* maybe put this in expr dispatch handler */
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
        case NODE_STRING:
                set_string(&v, node->varname);
                return v;
        case NODE_ARRAYLIT:
                return eval_arraylit(node);
        case NODE_CHAR:
                set_int(&v, node->ival);
                return v;
        case NODE_FUNCCALL:
                return eval_funccall(node);
        case NODE_NOT: {
                Var inner = eval_expr(node->children[0]);
                if (inner.type != TYPE_BOOL && inner.type != TYPE_INT) {
                        die(node, "`!` operator requires boolean or integer type");
                }
                set_int(&v, !as_int(inner));
                return v;
        }
        case NODE_AND: {
                Var left = eval_expr(node->children[0]);
                Var right;
                if (!as_int(left)) {
                        set_int(&left, 0);
                        return left;
                }
                right = eval_expr(node->children[1]);
                set_int(&right, as_int(right));
                return right;
        }
        case NODE_OR: {
                Var left = eval_expr(node->children[0]);
                Var right;
                if (as_int(left)) {
                        set_int(&left, 1);
                        return left;
                }
                right = eval_expr(node->children[1]);
                set_int(&right, as_int(right));
                return right;
        }
        case NODE_IDX:
                return eval_idx(node);
        default:
                return eval_binop(node);
        }
}
