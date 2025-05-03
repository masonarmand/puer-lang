/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */

#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "uthash.h"

typedef struct VarEntry {
        const char* name;
        Var val;
        UT_hash_handle hh;
} VarEntry;

static VarEntry* env = NULL;

Var* env_get(const char* name)
{
        VarEntry* entry;
        HASH_FIND_STR(env, name, entry);
        return entry ? &entry->val : NULL;
}

void env_set(const char* name, Var val)
{
        VarEntry* entry;
        HASH_FIND_STR(env, name, entry);
        if (!entry) {
                entry = malloc(sizeof(VarEntry));
                entry->name = strdup(name);
                HASH_ADD_KEYPTR(hh, env, entry->name, strlen(entry->name), entry);
        }
        entry->val = val;
}

void env_clear()
{
        VarEntry* cur;
        VarEntry* tmp;
        HASH_ITER(hh, env, cur, tmp) {
                HASH_DEL(env, cur);
                free((char*)cur->name);
                free(cur);
        }
}

