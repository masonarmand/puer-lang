#include "scan.h"
#include "env.h"
#include "puerstring.h"
#include "arraylist.h"
#include "rec.h"
#include "var.h"
#include "ast.h"

#include "uthash.h"


void mark_var(const Var* v, GC_MarkFn mark)
{
        if (!v)
                return;

        switch(v->type) {
        case TYPE_STRING:
                if (v->data.s)
                        mark(v->data.s);
                break;
        case TYPE_ARRAY:
                if (v->data.a)
                        mark(v->data.a);
                break;
        case TYPE_REC:
                if (v->data.r)
                        mark(v->data.r);
                break;
        default:
                break;
        }
}

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
        unsigned int i;
        if (a->items)
                mark(a->items);

        for (i = 0; i < a->size; i++)
                mark_var(&a->items[i], mark);
}

void scan_varentry(void* payload, GC_MarkFn mark)
{
        VarEntry* e = payload;
        mark_var(&e->val, mark);
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

void scan_rec(void* payload, GC_MarkFn mark)
{
        RecInst* ri = payload;
        unsigned int i;

        if (ri->fields)
                mark(ri->fields);

        for (i = 0; i < ri->def->n_fields; i++)
                mark_var(&ri->fields[i], mark);

}
