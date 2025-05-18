/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#ifndef VAR_H
#define VAR_H

#include "puerstring.h"

typedef struct ArrayList ArrayList;
typedef struct RecInst RecInst;

typedef enum VarType {
        TYPE_INT,
        TYPE_UINT,
        TYPE_LONG,
        TYPE_FLOAT,
        TYPE_BOOL,
        TYPE_STRING,
        TYPE_ARRAY,
        TYPE_VOID,
        TYPE_REC,
        TYPE_ANY
} VarType;

typedef struct Var {
        VarType type;

        union {
                int i;
                unsigned int ui;
                long l;
                int b;
                float f;
                String* s;
                ArrayList* a;
                RecInst* r;
        } data;

        int is_const;
} Var;

struct ArrayList {
        VarType type;
        Var* items;
        unsigned int size;
        unsigned int capacity;
};

Var var_clone(const Var* src);
Var implicit_convert(Var in, VarType target);
VarType coerce(Var* a, Var* b);
VarType common_type(VarType a, VarType b);
void cast_to(Var* v, VarType target);
float to_float(const Var* v);
float to_long(const Var* v);
void set_void(Var* v);
void set_bool(Var* v, int val);
void set_int(Var* v, int val);
void set_rec(Var* v, RecInst* val);
void set_uint(Var* v, unsigned int val);
void set_float(Var* v, float val);
void set_long(Var* v, long val);
void set_string(Var* v, const char* val);
void set_array(Var* v, ArrayList* arr);
int as_int(Var v);
int as_bool(Var v);
unsigned int as_uint(Var v);
float as_float(Var v);
char* as_string(Var v);

#endif
