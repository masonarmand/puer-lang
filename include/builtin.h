/*
 * Author: Mason Armand
 * Date Created: May 8th, 2025
 */
#ifndef BUILTIN_H
#define BUILTIN_H

#include "ast.h"
#include "var.h"

typedef Var (*BuiltinFn)(Node *node, Var *args);

void builtin_register(const char* name, BuiltinFn fn, VarType ret_type, int n_params, ...);
void builtin_clear(void);

int call_builtin_if_exists(Node* node, Var* out);


#endif
