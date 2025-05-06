/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
#ifndef VAR_H
#define VAR_H

typedef enum VarType {
        TYPE_INT,
        TYPE_UINT,
        TYPE_LONG,
        TYPE_FLOAT,
        TYPE_BOOL,
        TYPE_STRING
} VarType;

typedef struct Var {
        VarType type;

        union {
                int i;
                unsigned int ui;
                long l;
                int b;
                float f;
                char* s;
        } data;

        int is_const;
} Var;

VarType common_type(VarType a, VarType b);
float to_float(const Var* v);
float to_long(const Var* v);
void set_int(Var* v, int val);
void set_uint(Var* v, unsigned int val);
void set_float(Var* v, float val);
void set_long(Var* v, long val);
void set_string(Var* v, char* val);
int as_int(Var v);
unsigned int as_uint(Var v);
float as_float(Var v);
char* as_string(Var v);

#endif
