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
void eval_assign_stmt(Node* node);
void eval_if(Node* node);
void eval_ifelse(Node* node);
void eval_for(Node* node);
void eval_while(Node* node);
void eval_nop(Node* node);
void eval_funcdef(Node* node);
void eval_funccall_stmt(Node* node);
void eval_idxassign_stmt(Node* node);
void eval_arraydecl(Node* node);
void eval_compount_stmt(Node* node);

Var eval_funccall(Node* node);
Var eval_assign_expr(Node* node);
Var eval_idxassign_expr(Node* node);
Var eval_compound_expr(Node* node);

/* helpers */
void print_var(Node* node, const Var* v);
int var_to_idx(Node* node, Var v);
Var index_store(Node* node, Var container, int idx, Var val);
Var index_load(Node* node, Var container, int idx);

static StmtHandler handlers[NODE_LASTNODE];

void init_handlers(void)
{
        handlers[NODE_NOP]       = eval_nop;
        handlers[NODE_SEQ]       = eval_seq;
        handlers[NODE_PRINT]     = eval_print;
        handlers[NODE_PRINTLN]   = eval_println;
        handlers[NODE_VARDECL]   = eval_vardecl;
        handlers[NODE_ASSIGN]    = eval_assign_stmt;
        handlers[NODE_IF]        = eval_if;
        handlers[NODE_IFELSE]    = eval_ifelse;
        handlers[NODE_FOR]       = eval_for;
        handlers[NODE_WHILE]     = eval_while;
        handlers[NODE_FUNCDEF]   = eval_funcdef;
        handlers[NODE_FUNCCALL]  = eval_funccall_stmt;
        handlers[NODE_IDXASSIGN] = eval_idxassign_stmt;
        handlers[NODE_ARRAYDECL] = eval_arraydecl;
}

void eval(Node* node)
{
        StmtHandler h = handlers[node->type];
        if (!h)
                die(node, "unhandled stmt type: %d", node->type);
        h(node);
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
        if (as_bool(eval_expr(node->children[0])))
                return eval_block(node->children[1]);
        return CTRL_NONE;
}

CtrlSignal eval_ifelse_ctrl(Node* node)
{
        if (as_bool(eval_expr(node->children[0])))
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
        int i;
        for (i = 0; i < node->n_children; i++) {
                eval(node->children[i]);
                gc_collect_step();
                /*gc_collect_full();*/
        }
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
                        int is_str = (a->items[i].type == TYPE_STRING);
                        if (is_str)
                                printf("\"");
                        print_var(node, &a->items[i]);
                        if (is_str)
                                printf("\"");
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
                Var result = eval_expr(node->children[0]);
                result = implicit_convert(result, node->vartype);

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

void eval_assign_stmt(Node* node)
{
        (void) eval_assign_expr(node);
}

Var eval_assign_expr(Node* node)
{
        Var* v = env_get(node->varname);
        Var result;
        if (!v) {
                die(node, "assignment to undeclared variable '%s'", node->varname);
        }

        result = eval_expr(node->children[0]);
        result = implicit_convert(result, v->type);
        if (result.type != v->type)
                die(node, "Type error: cannot assign to variable '%s'", node->varname);
        v->data = result.data;
        return result;
}


void eval_if(Node* node)
{
        if (as_bool(eval_expr(node->children[0])))
                eval_block(node->children[1]);
}

void eval_ifelse(Node* node)
{
        if (as_bool(eval_expr(node->children[0])))
                eval_block(node->children[1]);
        else
                eval_block(node->children[2]);
}

void eval_for(Node* node)
{
        /* for init */
        env_push();
        eval(node->children[0]);

        while (as_bool(eval_expr(node->children[1]))) {
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
        while(as_bool(eval_expr(node->children[0]))) {
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
                if (func->vartype == TYPE_ARRAY || func->vartype == TYPE_STRING)
                        g_retval = var_clone(&g_retval);
        }
        else {
                if (func->vartype != TYPE_VOID)
                        die(node, "function '%s': missing return value", node->varname);
                set_void(g_retval);
        }

        env_pop();
        return g_retval;
}

void eval_idxassign_stmt(Node* node)
{
        (void) eval_idxassign_expr(node);
}

Var eval_idxassign_expr(Node* node)
{
        Var container = eval_expr(node->children[0]);
        Var v = eval_expr(node->children[1]);
        int idx = var_to_idx(node, v);
        Var val = eval_expr(node->children[2]);

        return index_store(node, container, idx, val);
}

void eval_arraydecl(Node* node)
{
        Node* dims_node = node->children[0];
        Node* init_node = node->children[1];
        int ndims = dims_node->n_children;
        Var value;

        /* TODO split into two functions */
        if (ndims == 0) {
                if (init_node->type != NODE_NOP) {
                        value = eval_expr(init_node);
                        if (value.type != TYPE_ARRAY)
                                die(node, "initializer for '%s' must be an array", node->varname);
                }
                else {
                        ArrayList* a = arraylist_new(node->vartype, 0);
                        value.type = TYPE_ARRAY;
                        value.data.a = a;
                }
        }
        else {
                int* sizes = malloc(sizeof(int) * ndims);
                int i;
                for (i = 0; i < ndims; i++) {
                        Node* dim_expr = dims_node->children[i];
                        if (dim_expr->type == NODE_NOP) {
                                sizes[i] = 0;
                        }
                        else {
                                Var v = eval_expr(dim_expr);
                                if (v.type != TYPE_INT)
                                        die(node, "array dimension %d is not an integer", i);
                                sizes[i] = v.data.i;
                        }
                }
                value = build_zero_array(node->vartype, sizes, ndims);
                free(sizes);
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

int var_to_idx(Node* node, Var v)
{
        int out;
        switch (v.type) {
        case TYPE_INT:
                out = v.data.i;
                break;
        case TYPE_UINT:
                out = (int) v.data.ui;
                break;
        case TYPE_LONG:
                out = (int) v.data.l;
                break;
        default:
                die(node, "index can't be of type: '%d'", v.type);

        }
        if (out < 0)
                die(node, "index must be positive");
        return out;
}

Var index_load(Node* node, Var container, int idx)
{
        Var out;

        switch (container.type) {
        case TYPE_STRING:
                String* s = container.data.s;
                char c;
                check_str_bounds(s, idx);
                c = string_get(s, idx);
                set_int(&out, (int)c);
                return out;
        case TYPE_ARRAY:
                ArrayList* a = container.data.a;
                check_arr_bounds(a, idx);
                return a->items[idx];
        default:
                die(node, "cannot index into type %d", container.type);
        }
}

Var index_store(Node* node, Var container, int idx, Var val)
{
        switch (container.type) {
        case TYPE_STRING:
                if (val.type != TYPE_INT)
                        die(node, "can only assign char to string character");
                string_set(container.data.s, idx, (char)val.data.i);
                return val;
        case TYPE_ARRAY:
                ArrayList* a = container.data.a;
                if (val.type != a->type)
                        die(node, "type mismatch: array holds %d but got %d", a->type, val.type);
                a->items[idx] = val;
                return val;
        default:
                die(node, "cannot index assign into type %d", container.type);
        }
}

Var do_binop(Node* at, BinOp op, Var a, Var b)
{
        VarType type;
        BinOpFunc func;

        if (a.type == TYPE_STRING && b.type == TYPE_STRING && op == OP_ADD) {
                Var result;
                result.type = TYPE_STRING;
                result.data.s = string_concat(a.data.s, b.data.s);
                return result;
        }

        type = coerce(&a, &b);
        func = type_ops[type].ops[op];
        if (!func)
                die(at, "Operator not supported for this type");

        return func(a, b);
}

Var eval_binop(Node* node)
{
        Var a;
        Var b;
        VarType type;
        BinOpFunc func;
        BinOp op = node->op;

        a = eval_expr(node->children[0]);
        b = eval_expr(node->children[1]);

        return do_binop(node, op, a, b);
}

void eval_compount_stmt(Node* node)
{
        (void) eval_compound_expr(node);
}

Var eval_compound_expr(Node* node)
{
        Node* L = node->children[0];
        Node* R = node->children[1];
        Var rhs;
        Var result;
        BinOp op = node->op;
        Var old;
        Var* v;
        Var container;
        int idx;

        switch (L->type) {
        case NODE_VAR:
                v = env_get(L->varname);
                if (!v)
                        die(node, "undeclared variable '%s'", L->varname);
                old = *v;
                break;
        case NODE_IDX:
                Var idxv = eval_expr(L->children[1]);

                container = eval_expr(L->children[0]);
                idx = var_to_idx(node, idxv);
                old = index_load(node, container, idxv.data.i);
                break;
        default:
                die(node, "invalid LHS in compound assign");
                break;
        }

        rhs = eval_expr(R);
        result = do_binop(node, op, old, rhs);

        if (L->type == NODE_VAR) {
                result = implicit_convert(result, v->type);
                env_set(L->varname, result);
        }
        else {
                int is_str = container.type == TYPE_STRING;
                VarType type = (is_str ? TYPE_INT : container.data.a->type);
                result = implicit_convert(result, type);
                index_store(node, container, idx, result);
        }
}

Var eval_idx(Node* node)
{
        Var container = eval_expr(node->children[0]);
        Var v = eval_expr(node->children[1]);
        int idx = var_to_idx(node, v);
        return index_load(node, container, idx);
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
        case NODE_BOOL:
                set_bool(&v, node->ival);
                return v;
        case NODE_FUNCCALL:
                return eval_funccall(node);
        case NODE_ASSIGN:
                return eval_assign_expr(node);
        case NODE_IDXASSIGN:
                return eval_idxassign_expr(node);
        case NODE_NOT: {
                Var inner = eval_expr(node->children[0]);
                if (inner.type != TYPE_BOOL && inner.type != TYPE_INT) {
                        die(node, "`!` operator requires boolean or integer type");
                }
                set_bool(&v, !as_int(inner));
                return v;
        }
        case NODE_AND: {
                Var left = eval_expr(node->children[0]);
                Var right;
                if (!as_bool(left)) {
                        set_bool(&left, 0);
                        return left;
                }
                right = eval_expr(node->children[1]);
                set_bool(&right, as_bool(right));
                return right;
        }
        case NODE_OR: {
                Var left = eval_expr(node->children[0]);
                Var right;
                if (as_bool(left)) {
                        set_bool(&left, 1);
                        return left;
                }
                right = eval_expr(node->children[1]);
                set_bool(&right, as_bool(right));
                return right;
        }
        case NODE_IDX:
                return eval_idx(node);
        case NODE_BINOP:
                return eval_binop(node);
        case NODE_COMPOUND:
                return eval_compound_expr(node);
        default:
                die(node, "unhandled expression type: %s", node->type);
        }
}
