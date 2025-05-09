/*
 * Author: Mason Armand
 * Date Created: May 8th, 2025
 */
#ifndef BUILTIN_H
#define BUILTIN_H

#include "ast.h"
#include "var.h"

typedef Var (*BuiltinFn)(Node *node, Var *args);

void builtin_register(const char* name, BuiltinFn fn, VarType* param_types, int n_params, VarType return_type);

int call_builtin_if_exists(Node* node, Var* out);

#define REGISTER_BUILTIN(fn, ret, ...) \
        static VarType fn##_param_types[] = { __VA_ARGS__ }; \
        builtin_register( \
                #fn, fn, \
                fn##_param_types, \
                sizeof(fn##_param_types) / sizeof(fn##_param_types[0]), \
                ret \
        )



#endif
