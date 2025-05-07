/*
 * Author: Mason Armand
 * Date Created: May 5th, 2025
 */

#include "func.h"
#include "uthash.h"
#include <string.h>

typedef struct Func {
        const char* name;
        Node* ast;
        UT_hash_handle hh;
} Func;

static Func* table = NULL;

void func_set(const char* name, Node* ast)
{
        Func* entry;
        HASH_FIND_STR(table, name, entry);
        if (!entry) {
                entry = malloc(sizeof(Func));
                entry->name = strdup(name);
                HASH_ADD_KEYPTR(hh, table, entry->name, strlen(entry->name), entry);
        }
        entry->ast = ast;
}

Node* func_get(const char* name)
{
        Func* entry;
        HASH_FIND_STR(table, name, entry);
        return entry ? entry->ast : NULL;
}

void func_clear(void)
{
        Func* cur;
        Func* tmp;

        HASH_ITER(hh, table, cur, tmp) {
                HASH_DEL(table, cur);
                free((char*)cur->name);
                free(cur);
        }
}
