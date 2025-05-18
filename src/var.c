#include "var.h"
#include "arraylist.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>


Var var_clone(const Var* src)
{
        Var out;
        out.type = src->type;

        switch (src->type) {
        case TYPE_ARRAY:
                out.data.a = arraylist_clone(src->data.a);
                break;
        case TYPE_STRING:
                out.data.s = string_clone(src->data.s);
                break;
        default:
                out.data = src->data;
                break;
        }
        return out;
}

Var implicit_convert(Var in, VarType target)
{
        if (in.type == target)
                return in;

        if (target == TYPE_BOOL && in.type == TYPE_INT) {
                set_bool(&in, as_bool(in));
                return in;
        }

        if (target == TYPE_INT && in.type == TYPE_BOOL) {
                set_int(&in, as_int(in));
        }

        die(NULL, "cannot convert type %d to %d", in.type, target);
        return in; /* unreachable */
}

VarType coerce(Var* a, Var* b)
{
        VarType type = common_type(a->type, b->type);
        cast_to(a, type);
        cast_to(b, type);
        return type;
}

VarType common_type(VarType a, VarType b)
{
        if (a == b)
                return a;
        if (a == TYPE_FLOAT || b == TYPE_FLOAT)
                return TYPE_FLOAT;
        if (a == TYPE_LONG || b == TYPE_LONG)
                return TYPE_LONG;
        if (a == TYPE_UINT || b == TYPE_UINT)
                return TYPE_UINT;
        if (a == TYPE_BOOL || b == TYPE_BOOL)
                return TYPE_BOOL;
        return TYPE_INT;
}

void cast_to(Var* v, VarType target)
{
        if (v->type == target)
                return;

        switch (target) {
        case TYPE_FLOAT:
                switch (v->type) {
                case TYPE_INT:
                        v->data.f = (float)v->data.i;
                        break;
                case TYPE_UINT:
                        v->data.f = (float)v->data.ui;
                        break;
                case TYPE_LONG:
                        v->data.f = (float)v->data.l;
                        break;
                default: die(NULL, "cannot cast to float");
                }
                v->type = TYPE_FLOAT;
                break;
        case TYPE_INT:
                switch (v->type) {
                case TYPE_FLOAT:
                        v->data.i = (int)v->data.f;
                        break;
                case TYPE_UINT:
                        v->data.i = (int)v->data.ui;
                        break;
                case TYPE_LONG:
                        v->data.i = (int)v->data.l;
                        break;
                default: die(NULL, "cannot cast to int");
                }
                v->type = TYPE_INT;
                break;
        case TYPE_BOOL:
                switch (v->type) {
                case TYPE_INT:
                        v->data.b = v->data.i;
                        break;
                case TYPE_FLOAT:
                        v->data.b = (int)v->data.f;
                        break;
                case TYPE_UINT:
                        v->data.b = (int)v->data.ui;
                        break;
                case TYPE_LONG:
                        v->data.b = (int)v->data.l;
                        break;
                default: die(NULL, "cannot cast to bool");
                }
                v->data.b = !!(v->data.b);
                v->type = TYPE_BOOL;
                break;
        default:
                die(NULL, "unsupported coercion to type %d", target);
        }
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

        return 0.0f; /* unreachable */
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

        return 0.0f; /* unreachable */
}

void set_void(Var* v)
{
        v->type = TYPE_VOID;
}

void set_rec(Var* v, RecInst* val)
{
        v->type = TYPE_REC;
        v->data.r = val;
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


void set_bool(Var* v, int val)
{
        val = !!val;
        v->type = TYPE_BOOL;
        v->data.b = val;
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

void set_string(Var* v, const char* val)
{
        v->type = TYPE_STRING;
        v->data.s = string_new(val);
}

void set_array(Var* v, ArrayList* arr)
{
        v->type = TYPE_ARRAY;
        v->data.a = arr;
}

int as_int(Var v)
{
        if (v.type == TYPE_BOOL) {
                return v.data.b;
        }
        if (v.type != TYPE_INT) {
                die(NULL, "Expected int, got type %d\n", v.type);
        }

        return v.data.i;
}

int as_bool(Var v)
{
        if (v.type != TYPE_BOOL && v.type != TYPE_INT) {
                die(NULL, "Expected bool, got type %d\n", v.type);
        }
        if (v.type == TYPE_BOOL)
                return v.data.b;
        if (v.type == TYPE_INT) {
                return !!(v.data.i);
        }

        return 0; /* unreachable */
}

unsigned int as_uint(Var v)
{
        if (v.type != TYPE_UINT) {
                die(NULL, "Expected unsigned int, got type %d\n", v.type);
        }
        return v.data.ui;
}

float as_float(Var v)
{
        if (v.type != TYPE_FLOAT) {
                die(NULL, "Expected float, got type %d\n", v.type);
        }
        return v.data.f;
}
