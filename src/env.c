/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#include "env.h"
#include "scan.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


Scope* env_stack = NULL;

/* add a new variable scope to the stack */
void env_push(void)
{
        Scope* s = gc_alloc(sizeof(Scope), scan_scope);
        s->table = NULL;
        s->next = env_stack;
        env_stack = s;
}

/* remove a variable scope from the stack */
void env_pop(void)
{
        VarEntry* cur;
        VarEntry* tmp;

        if (!env_stack)
                return;

        HASH_ITER(hh, env_stack->table, cur, tmp) {
                HASH_DEL(env_stack->table, cur);
                free((char*)cur->name);
        }

        env_stack = env_stack->next;
}

Var* env_get(const char* name)
{
        Scope* scope = env_stack;

        while (scope) {
                VarEntry* entry;
                HASH_FIND_STR(scope->table, name, entry);
                if (entry && entry->is_ptr)
                        return entry->alias;
                else if (entry)
                        return &entry->val;
                scope = scope->next;
        }
        return NULL;
}

/* only search top level scope */
Var* env_get_top(const char* name)
{
        VarEntry* entry;

        if (!env_stack)
                return NULL;

        HASH_FIND_STR(env_stack->table, name, entry);
        if (entry && entry->is_ptr)
                return entry->alias;
        else if (entry)
                return &entry->val;
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
                entry = gc_alloc(sizeof(VarEntry), scan_varentry);
                entry->name = strdup(name);
                entry->is_ptr = 0;
                HASH_ADD_KEYPTR(hh, env_stack->table, entry->name, strlen(entry->name), entry);
        }
        entry->val = val;
        entry->alias = NULL;
}

void env_set_ptr(const char* name, Var* target)
{
        VarEntry* entry;

        if (!env_stack) {
                env_push();
        }

        HASH_FIND_STR(env_stack->table, name, entry);
        if (!entry) {
                entry = gc_alloc(sizeof(VarEntry), scan_varentry);
                entry->name = strdup(name);
                entry->is_ptr = 1;
                HASH_ADD_KEYPTR(hh, env_stack->table, entry->name, strlen(entry->name), entry);
        }
        entry->alias = target;
}

void env_clear()
{
        while (env_stack)
                env_pop();
}
