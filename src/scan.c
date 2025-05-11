#include "scan.h"
#include "env.h"
#include "puerstring.h"
#include "arraylist.h"
#include "var.h"
#include "ast.h"

#include "uthash.h"


void scan_raw(void* payload, GC_MarkFn mark)
{
        (void) payload;
        (void) mark;
}

void scan_string(void* payload, GC_MarkFn mark)
{
        String* s = payload;
        if (s->data)
                mark(s->data);
}

void scan_arraylist(void* payload, GC_MarkFn mark)
{
        ArrayList* a = payload;
        int i;
        if (a->items)
                mark(a->items);
        for (i = 0; i < a->size; i++) {
                Var* v = &a->items[i];
                if (v->type == TYPE_STRING && v->data.s)
                        mark(v->data.s);
                if (v->type == TYPE_ARRAY  && v->data.a)
                        mark(v->data.a);
        }
}

void scan_varentry(void* payload, GC_MarkFn mark)
{
        VarEntry* e = payload;
        Var* v = &e->val;

        if (v->type == TYPE_STRING && v->data.s)
                mark(v->data.s);
        if (v->type == TYPE_ARRAY  && v->data.a)
                mark(v->data.a);
}

void scan_scope(void* payload, GC_MarkFn mark)
{
        Scope* s = payload;
        VarEntry* cur;
        VarEntry* tmp;
        mark(s->next);
        HASH_ITER(hh, s->table, cur, tmp) {
                mark(cur);
        }
}
