#include "ast.h"
#include "env.h"
#include "util.h"
#include "ops.h"
#include "func.h"
#include "builtin.h"
#include "arraylist.h"
#include "rec.h"
#include "gc_tri.h"

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
void eval_compound_stmt(Node* node);
void eval_incdec_stmt(Node* node);
void eval_recdef(Node* node);
void eval_fieldassign_stmt(Node* node);

Var eval_funccall(Node* node);
Var eval_assign_expr(Node* node);
Var eval_idxassign_expr(Node* node);
Var eval_compound_expr(Node* node);
Var eval_incdec_expr(Node* node);
Var eval_fieldaccess(Node* node);
Var eval_fieldassign_expr(Node* node);

/* helpers */
void print_var(Node* node, const Var* v);
int var_to_idx(Node* node, Var v);
Var index_store(Node* node, Var container, int idx, Var val);
Var index_load(Node* node, Var container, int idx);
Var load_lvalue(Node* L);
void assign_lvalue(Node* L, Var val);
Var init_var(Node* ctx, VarType type, Node* init_node, const char* recname);

static StmtHandler handlers[NODE_LASTNODE];

void init_handlers(void)
{
        handlers[NODE_NOP]         = eval_nop;
        handlers[NODE_SEQ]         = eval_seq;
        handlers[NODE_PRINT]       = eval_print;
        handlers[NODE_PRINTLN]     = eval_println;
        handlers[NODE_VARDECL]     = eval_vardecl;
        handlers[NODE_ASSIGN]      = eval_assign_stmt;
        handlers[NODE_IF]          = eval_if;
        handlers[NODE_IFELSE]      = eval_ifelse;
        handlers[NODE_FOR]         = eval_for;
        handlers[NODE_WHILE]       = eval_while;
        handlers[NODE_FUNCDEF]     = eval_funcdef;
        handlers[NODE_FUNCCALL]    = eval_funccall_stmt;
        handlers[NODE_IDXASSIGN]   = eval_idxassign_stmt;
        handlers[NODE_ARRAYDECL]   = eval_arraydecl;
        handlers[NODE_COMPOUND]    = eval_compound_stmt;
        handlers[NODE_INCDEC]      = eval_incdec_stmt;
        handlers[NODE_RECDEF]      = eval_recdef;
        handlers[NODE_FIELDASSIGN] = eval_fieldassign_stmt;
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
        CtrlSignal sig;
        env_push();
        sig = eval_with_ctrl(node);
        env_pop();
        return sig;
}

void eval_seq(Node* node)
{
        unsigned int i;
        for (i = 0; i < node->n_children; i++) {
                eval(node->children[i]);
                gc_collect_step();
                /*gc_collect_full();*/
        }
}

void print_var(Node* node, const Var* v)
{
        unsigned int i;
        unsigned int n;
        ArrayList* a;
        RecInst* r;

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
                printf(v->data.b ? "true" : "false");
                break;
        case TYPE_STRING:
                printf("%s", v->data.s->data);
                break;
        case TYPE_ARRAY:
                a = v->data.a;
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
        case TYPE_REC:
                r = v->data.r;
                n = r->def->n_fields;

                printf("{");
                for (i = 0; i < n; i++) {
                        int is_str = (r->fields[i].type == TYPE_STRING);
                        if (is_str)
                                printf("\"");
                        print_var(node, &r->fields[i]);
                        if (is_str)
                                printf("\"");
                        if (i + 1 < n)
                                printf(", ");
                }
                printf("}");
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
        Var* get;
        v.type = node->vartype;

        if ((get = env_get(node->varname)))
                die(node, "'%s' has already been declared as type: '%d'", node->varname, get->type);

        v = init_var(
                node,
                node->vartype,
                node->n_children > 0 ? node->children[0] : NULL,
                node->recname
        );

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
        (void) node;
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

                if (env_get(param->varname))
                        die(node, "variable '%s' already declared elsewhere", param->varname);
                if (arg_val.type != param->vartype) {
                        die(node, "function '%s' argument %d: expected type %d, got %d",
                                node->varname,
                                i + 1,
                                param->vartype,
                                arg_val.type
                        );
                }

                if (param->vartype == TYPE_REC) {
                        RecInst* ri = arg_val.data.r;
                        Var* orig;

                        if (strcmp(ri->def->name, param->recname) != 0) {
                                die(node, "function '%s' argument %d: expected record '%s', got '%s'",
                                        node->varname,
                                        i + 1,
                                        param->recname,
                                        ri->def->name
                                );
                        }

                        if (arg_expr->type != NODE_VAR)
                                die(node, "function '%s': record argument must be a variable", node->varname);

                        orig = env_get(arg_expr->varname);
                        if (!orig)
                                die(node, "undefined variable '%s'", arg_expr->varname);
                        env_set_ptr(param->varname, orig);
                }
                else {
                        env_set(param->varname, arg_val);
                }

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
                set_void(&g_retval);
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
        int out = 0;
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
        String* s;
        char c;
        ArrayList* a;

        switch (container.type) {
        case TYPE_STRING:
                s = container.data.s;
                check_str_bounds(s, idx);
                c = string_get(s, idx);
                set_int(&out, (int)c);
                return out;
        case TYPE_ARRAY:
                a = container.data.a;
                check_arr_bounds(a, idx);
                return a->items[idx];
        default:
                die(node, "cannot index into type %d", container.type);
        }
        return out; /* unreachable */
}

Var index_store(Node* node, Var container, int idx, Var val)
{
        ArrayList* a;
        switch (container.type) {
        case TYPE_STRING:
                if (val.type != TYPE_INT)
                        die(node, "can only assign char to string character");
                string_set(container.data.s, idx, (char)val.data.i);
                return val;
        case TYPE_ARRAY:
                a = container.data.a;
                if (val.type != a->type)
                        die(node, "type mismatch: array holds %d but got %d", a->type, val.type);
                a->items[idx] = val;
                return val;
        default:
                die(node, "cannot index assign into type %d", container.type);
        }

        return val; /* unreachable */
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
        BinOp op = node->op;

        a = eval_expr(node->children[0]);
        b = eval_expr(node->children[1]);

        return do_binop(node, op, a, b);
}

void eval_compound_stmt(Node* node)
{
        (void) eval_compound_expr(node);
}

Var eval_compound_expr(Node* node)
{
        Node* L = node->children[0];
        Var old = load_lvalue(L);
        Var rhs = eval_expr(node->children[1]);
        Var result = do_binop(node, node->op, old, rhs);

        if (L->type == NODE_VAR) {
                Var* v = env_get(L->varname);
                result = implicit_convert(result, v->type);
        }
        else {
                Var container = eval_expr(L->children[0]);
                int is_str = (container.type == TYPE_STRING);
                VarType type = (is_str) ? TYPE_INT : container.data.a->type;
                result = implicit_convert(result, type);
        }

        assign_lvalue(L, result);
        return result;
}

Var eval_idx(Node* node)
{
        Var container = eval_expr(node->children[0]);
        Var v = eval_expr(node->children[1]);
        int idx = var_to_idx(node, v);
        return index_load(node, container, idx);
}

Var load_lvalue(Node* L)
{
        Var* v;
        Var container;
        Var idxv;
        int idx;
        Var dummy = { 0 };

        switch (L->type) {
        case NODE_VAR:
                v = env_get(L->varname);
                if (!v)
                        die(L, "undefined variable '%s'", L->varname);
                return *v;
        case NODE_IDX:
                container = eval_expr(L->children[0]);
                idxv = eval_expr(L->children[1]);
                idx = var_to_idx(L, idxv);
                return index_load(L, container, idx);
        default:
                die(L, "Left hand side is not assignable");
        }

        return dummy; /* unreachable */
}

void assign_lvalue(Node* L, Var val)
{
        Var* v;
        Var container;
        Var idxv;
        int idx;

        switch (L->type) {
        case NODE_VAR:
                v = env_get(L->varname);
                if (!v)
                        die(L, "undefined variable '%s'", L->varname);
                *v = val;
                return;
        case NODE_IDX:
                container = eval_expr(L->children[0]);
                idxv = eval_expr(L->children[1]);
                idx = var_to_idx(L, idxv);
                index_store(L, container, idx, val);
                return;
        default:
                die(L, "Left hand side is not assignable");
        }
}

Var init_var(Node* ctx, VarType type, Node* init_node, const char* recname)
{
        Var v;
        v.type = type;

        if (init_node && init_node->type != NODE_NOP) {
                Var r = eval_expr(init_node);
                r = implicit_convert(r, type);
                if (r.type != type) {
                        die(
                                ctx,
                                "init expr type mismatch for '%s': expected %d got %d",
                                ctx->varname, type, r.type
                        );
                }
                return r;
        }

        switch (type) {
                case TYPE_STRING:
                        v.data.s = string_new("");
                        break;
                case TYPE_ARRAY:
                        v.data.a = arraylist_new(v.type, 0);
                        break;
                case TYPE_REC:
                        v.data.r = rec_new(recname);
                        break;
                default:
                        /* non initialized value (UB) */
                        break;
        }
        return v;
}

Var eval_incdec_expr(Node* node)
{
        Node* L = node->children[0];
        int is_prefix = node->ival;
        Var old = load_lvalue(L);
        Var one;
        Var next;

        set_int(&one, 1);
        next = do_binop(node, node->op, old, one);
        assign_lvalue(L, next);

        return (is_prefix) ? next : old;
}

void eval_incdec_stmt(Node* node)
{
        (void) eval_incdec_expr(node);
}

void eval_recdef(Node* node)
{
        unsigned int n = 0;
        unsigned int i;
        const char** names;
        Var* defs;
        RecDef* rd;
        Node* seq;

        if (node->n_children > 0) {
                seq = node->children[0];
                n = seq->n_children;
        }

        if (n == 0)
                die(node, "record definition must have at least 1 field");

        names = malloc(sizeof(char*) * n);
        defs = malloc(sizeof(Var) * n);

        for (i = 0; i < n; i++) {
                Var v;
                Node* f = seq->children[i];
                names[i] = strdup(f->varname);

                v = init_var(
                        node,
                        f->vartype,
                        (f->n_children==1 && f->children[0]->type!=NODE_NOP) ? f->children[0] : NULL,
                        f->recname
                );

                defs[i] = v;
        }

        rd = recdef_new(node->varname, names, defs, n);
        recdef_register(rd);
        for (i = 0; i < n; i++)
                free((char*)names[i]);
        free(names);
        free(defs);
}

Var eval_fieldaccess(Node* node)
{
        Var container = eval_expr(node->children[0]);
        RecInst* ri;

        if (container.type != TYPE_REC)
                die(node, "cannot access field on non-record, value");

        ri = container.data.r;
        return *rec_get_field(ri, node->varname);
}

Var eval_fieldassign_expr(Node* node)
{
        Var container = eval_expr(node->children[0]);
        Var v = eval_expr(node->children[1]);
        RecInst* ri;

        if (container.type != TYPE_REC)
                die(node, "cannot assign field on non-record, value");

        ri = container.data.r;

        rec_set_field(ri, node->varname, v);
        return v;
}

void eval_fieldassign_stmt(Node* node)
{
        (void) eval_fieldassign_expr(node);
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
        case NODE_FIELDACCESS:
                return eval_fieldaccess(node);
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
        case NODE_FIELDASSIGN:
                return eval_fieldassign_expr(node);
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
        case NODE_INCDEC:
                return eval_incdec_expr(node);
        default:
                die(node, "unhandled expression type: %s", node->type);
        }
        return v;
}
