#include "var.h"

#include <stdlib.h>

VarType common_type(VarType a, VarType b)
{
        if (a == TYPE_FLOAT || b == TYPE_FLOAT)
                return TYPE_FLOAT;
        if (a == TYPE_LONG || b == TYPE_LONG)
                return TYPE_LONG;
        if (a == TYPE_UINT || b == TYPE_UINT)
                return TYPE_UINT;
        return TYPE_INT;
}

float to_float(const Var* v)
{
        switch (v->type) {
        case TYPE_INT:
                return (float)v->data.i;
        case TYPE_UINT:
                return (float)v->data.ui;
        case TYPE_LONG:
                return (float)v->data.l;
        case TYPE_FLOAT:
                return v->data.f;
        default:
                die(NULL, "cannot cast to float\n");
        }
}

float to_long(const Var* v)
{
        switch (v->type) {
        case TYPE_INT:
                return (long)v->data.i;
        case TYPE_UINT:
                return (long)v->data.ui;
        case TYPE_LONG:
                return v->data.l;
        case TYPE_FLOAT:
                return (long) v->data.f;
        default:
                die(NULL, "cannot cast to long\n");
        }
}

void set_int(Var* v, int val)
{
        v->type = TYPE_INT;
        v->data.i = val;
}

void set_uint(Var* v, unsigned int val)
{
        v->type = TYPE_UINT;
        v->data.ui = val;
}

void set_float(Var* v, float val)
{
        v->type = TYPE_FLOAT;
        v->data.f = val;
}

void set_long(Var* v, long val)
{
        v->type = TYPE_LONG;
        v->data.l = val;
}

void set_string(Var* v, char* val)
{
        v->type = TYPE_STRING;
        v->data.s = val;
}

int as_int(Var v)
{
        if (v.type != TYPE_INT) {
                die("Expected int, got type %d\n", v.type);
        }
        return v.data.i;
}

unsigned int as_uint(Var v)
{
        if (v.type != TYPE_UINT) {
                die("Expected unsigned int, got type %d\n", v.type);
        }
        return v.data.ui;
}

float as_float(Var v)
{
        if (v.type != TYPE_FLOAT) {
                die("Expected float, got type %d\n", v.type);
        }
        return v.data.f;
}

char* as_string(Var v)
{
        if (v.type != TYPE_STRING) {
                die("Expected string, got type %d\n", v.type);
        }
        return v.data.s;
}
