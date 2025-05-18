#include "rec.h"
#include "scan.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

/* so lexer can distinguish user types from builtin types */
typedef struct RecName {
        char* name;
        UT_hash_handle hh;
} RecName;

static RecName* recnames = NULL;
static RecDef* recdefs = NULL;

void recdef_free(RecDef* rd);

RecDef* recdef_new(const char* name, const char** field_names, const Var* fields, unsigned int n_fields)
{
        unsigned int i;
        RecDef* rd = calloc(1, sizeof(RecDef));
        rd->name = strdup(name);
        rd->n_fields = n_fields;
        rd->fields = malloc(sizeof(Var) * n_fields);
        rd->index_map = NULL;

        for (i = 0; i < n_fields; i++) {
                FieldIndex* fi = malloc(sizeof(FieldIndex));
                rd->fields[i] = fields[i];
                fi->name = strdup(field_names[i]);
                fi->idx = i;
                HASH_ADD_KEYPTR(hh, rd->index_map, fi->name, strlen(fi->name), fi);
        }

        return rd;
}

void recdef_register(RecDef* rd)
{
        HASH_ADD_KEYPTR(hh, recdefs, rd->name, strlen(rd->name), rd);
}

RecDef* recdef_find(const char* name)
{
        RecDef* rd;
        HASH_FIND_STR(recdefs, name, rd);
        return rd;
}

void recdef_clear(void)
{
        RecDef* cur;
        RecDef* tmp;

        HASH_ITER(hh, recdefs, cur, tmp) {
                recdef_free(cur);
        }
}

void recdef_free(RecDef* rd)
{
        unsigned int i;
        FieldIndex* fi;
        FieldIndex* tmp;

        HASH_DEL(recdefs, rd);

        HASH_ITER(hh, rd->index_map, fi, tmp) {
                HASH_DEL(rd->index_map, fi);
                free(fi->name);
                free(fi);
        }

        free(rd->fields);
        free(rd->name);
        free(rd);
}

RecInst* rec_new(const char* recdef_name)
{
        RecDef* rd = recdef_find(recdef_name);
        RecInst* ri;
        if (!rd)
                die(NULL, "unknown record '%s'", recdef_name);

        ri = gc_alloc(sizeof(RecInst), scan_rec);
        ri->def = rd;
        ri->fields = gc_alloc(sizeof(Var) * rd->n_fields, scan_raw);
        memcpy(ri->fields, rd->fields, sizeof(Var) * rd->n_fields);
        return ri;
}

Var* rec_get_field(RecInst* ri, const char* field_name)
{
        FieldIndex* fi = NULL;
        HASH_FIND_STR(ri->def->index_map, field_name, fi);
        if (!fi)
                die(NULL, "record has no field '%s'", field_name);
        return &ri->fields[fi->idx];
}

void rec_set_field(RecInst* ri, const char* field_name, Var val)
{
        Var* v = rec_get_field(ri, field_name);
        if (v->type != val.type)
                die(NULL, "type error: Can't assign record field: %s to type %d (field is of type %d)", field_name, val.type, v->type);
        *v = val;
}

int is_rec_name(const char* name)
{
        RecName* r;
        HASH_FIND_STR(recnames, name, r);
        return (r != NULL);
}

void recname_register(const char* name)
{
        RecName* r;
        HASH_FIND_STR(recnames, name, r);
        if (!r) {
                r = malloc(sizeof(RecName));
                r->name = strdup(name);
                HASH_ADD_KEYPTR(hh, recnames, r->name, strlen(r->name), r);
        }
}

void recname_clear(void)
{
        RecName* cur;
        RecName* tmp;
        HASH_ITER(hh, recnames, cur, tmp) {
                HASH_DEL(recnames, cur);
                free(cur->name);
                free(cur);
        }
}
