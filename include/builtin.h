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

#define REGISTER_BUILTIN(fn, types, count, ret) \
        static VarType fn##_param_types[count] = types; \
        builtin_register(#fn, fn, fn##_param_types, count, ret)

#define REGISTER_BUILTIN_WNAME(name, fn, types, count, ret) \
        static VarType fn##_param_types[count] = types; \
        builtin_register(name, fn, fn##_param_types, count, ret)


#endif
