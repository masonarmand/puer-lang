/*
 * Author: Mason Armand
 * Date Created: May 8th, 2025
 */
#include "builtin.h"
#include "util.h"
#include "uthash.h"
#include <stdlib.h>
#include <string.h>

typedef struct Builtin {
        const char* name;
        VarType* param_types;
        unsigned int n_params;
        VarType return_type;
        BuiltinFn fn;
        UT_hash_handle hh;
} Builtin;

static Builtin* builtin_table = NULL;

void builtin_register(const char* name, BuiltinFn fn, VarType ret_type, int n_params, ...)
{
        Builtin* b;
        VarType* param_types = NULL;
        if (n_params > 0) {
                va_list ap;
                int i;

                param_types = malloc(sizeof(VarType) * n_params);
                va_start(ap, n_params);
                for (i = 0; i < n_params; i++)
                        param_types[i] = va_arg(ap, VarType);
                va_end(ap);
        }

        HASH_FIND_STR(builtin_table, name, b);
        if (b) {
                fprintf(stderr, "Builtin function '%s' is already registered.\n", name);
                return;
        }

        b = malloc(sizeof(Builtin));
        b->name = strdup(name);
        b->param_types = param_types;
        b->n_params = n_params;
        b->return_type = ret_type;
        b->fn = fn;
        HASH_ADD_KEYPTR(hh, builtin_table, b->name, strlen(b->name), b);
}

Builtin* builtin_get(const char* name)
{
        Builtin* b;
        HASH_FIND_STR(builtin_table, name, b);
        return b;
}

void builtin_clear(void)
{
        Builtin* cur;
        Builtin* tmp;
        HASH_ITER(hh, builtin_table, cur, tmp) {
                HASH_DEL(builtin_table, cur);
                free((char*)cur->name);
                free(cur);
        }
}

int call_builtin_if_exists(Node* node, Var* out)
{
        Builtin* b = builtin_get(node->varname);
        Var result;
        Var* argv;
        Node* argv_nodes;
        unsigned int i;

        if (!b)
                return 0;

        argv_nodes = node->children[0];
        if (argv_nodes->n_children != b->n_params) {
                die(
                        node,
                        "function '%s' expects %d args, got %d",
                        b->name, b->n_params, argv_nodes->n_children
                );
        }

        argv = malloc(sizeof(Var) * b->n_params);
        for (i = 0; i < b->n_params; i++) {
                argv[i] = eval_expr(argv_nodes->children[i]);
                if (argv[i].type != b->param_types[i] && b->param_types[i] != TYPE_ANY) {
                        die(
                                node,
                                "function '%s' arg %d: expected type %d, got %d",
                                b->name, i+1, b->param_types[i], argv[i].type
                        );
                }
        }

        result = b->fn(node, argv);
        if (result.type != b->return_type) {
                die(
                        node,
                        "function '%s': return type mismatch (expected %d, got %d)",
                        b->name, b->return_type, result.type
                );
        }

        free(argv);
        *out = result;
        return 1;
}
