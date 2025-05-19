/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#ifndef ENV_H
#define ENV_H

#include "var.h"
#include "uthash.h"

/* typedefs exposed for scan.c */
typedef struct VarEntry {
        const char* name;
        Var val;
        Var* alias;
        int is_ptr;
        UT_hash_handle hh;
} VarEntry;

typedef struct Scope {
        VarEntry* table;
        struct Scope* next;
} Scope;

extern Scope* env_stack;


void env_push(void);
void env_pop(void);
Var* env_get(const char* name);
Var* env_get_top(const char* name);
void env_set(const char* name, Var val);
void env_set_ptr(const char* name, Var* target);
void env_clear();

#endif
