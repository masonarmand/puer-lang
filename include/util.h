#ifndef UTIL_H
#define UTIL_H

#include "ast.h"
#include <stdarg.h>
#include <stdio.h>

void die(Node* node, const char* fmt, ...);

#endif
