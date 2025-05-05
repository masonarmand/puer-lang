/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "env.h"
#include "uthash.h"

typedef struct VarEntry {
        const char* name;
        Var val;
        UT_hash_handle hh;
} VarEntry;

typedef struct Scope {
        VarEntry* table;
        struct Scope* next;
} Scope;

static Scope* env_stack = NULL;

/* add a new variable scope to the stack */
void env_push(void)
{
        Scope* s = malloc(sizeof(Scope));
        s->table = NULL;
        s->next = env_stack;
        env_stack = s;
}

/* remove a variable scope from the stack */
void env_pop(void)
{
        VarEntry* cur;
        VarEntry* tmp;
        Scope* tmp_scope;

        if (!env_stack)
                return;

        HASH_ITER(hh, env_stack->table, cur, tmp) {
                HASH_DEL(env_stack->table, cur);
                free((char*)cur->name);
                free(cur);
        }

        tmp_scope = env_stack;
        env_stack = env_stack->next;
        free(tmp_scope);
}

Var* env_get(const char* name)
{
        Scope* scope = env_stack;

        while (scope) {
                VarEntry* entry;
                HASH_FIND_STR(scope->table, name, entry);
                if (entry)
                        return &entry->val;
                scope = scope->next;
        }
        return NULL;
}

void env_set(const char* name, Var val)
{
        VarEntry* entry;

        if (!env_stack) {
                env_push();
        }

        HASH_FIND_STR(env_stack->table, name, entry);
        if (!entry) {
                entry = malloc(sizeof(VarEntry));
                entry->name = strdup(name);
                HASH_ADD_KEYPTR(hh, env_stack->table, entry->name, strlen(entry->name), entry);
        }
        entry->val = val;
}

void env_clear()
{
        while (env_stack)
                env_pop();
}

