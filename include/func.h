#ifndef FUNC_H
#define FUNC_H

#include "ast.h"

void func_set(const char* name, Node* ast);
Node* func_get(const char* name);
void func_clear(void);

#endif
