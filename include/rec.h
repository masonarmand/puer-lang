#ifndef REC_H
#define REC_H

#include "var.h"
#include "uthash.h"

typedef struct FieldIndex {
        char* name;
        unsigned int idx;
        UT_hash_handle hh;
} FieldIndex;

typedef struct RecDef {
        char* name;
        unsigned int n_fields;
        Var* fields;
        FieldIndex *index_map;
        UT_hash_handle hh;
} RecDef;

struct RecInst {
        RecDef* def;
        Var* fields;
};

RecDef* recdef_new(const char* name, const char** field_names, const Var* fields, unsigned int n_fields);
RecDef* recdef_find(const char* name);
RecInst* rec_new(const char* recdef_name);
Var* rec_get_field(RecInst* ri, const char* field_name);
void rec_set_field(RecInst* ri, const char* field_name, Var val);
void recdef_register(RecDef* rd);
void recdef_clear(void);

int is_rec_name(const char* name);
void recname_register(const char* name);
void recname_clear(void);

#endif
