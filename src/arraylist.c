#include "var.h"
#include "scan.h"
#include "arraylist.h"
#include "puerstring.h"
#include <string.h>

ArrayList* arraylist_new(VarType type, int initial_capacity)
{
        ArrayList* a = gc_alloc(sizeof(ArrayList), scan_arraylist);
        a->type = type;
        a->size = 0;
        a->capacity = initial_capacity > 0 ? initial_capacity : 1;
        a->items = gc_alloc(sizeof(Var) * a->capacity, scan_raw);
        return a;
}

ArrayList* arraylist_clone(const ArrayList* src)
{
        ArrayList* dst = arraylist_new(src->type, src->size);
        int i;
        for (i = 0; i < src->size; i++) {
                Var child_copy = var_clone(&src->items[i]);
                arraylist_push(dst, child_copy);
        }
        return dst;
}

void arraylist_grow(ArrayList* a)
{
        /* TODO
         * algorithm for determining new capacity
         * instead of just doubling it
         */
        int new_cap = a->capacity *= 2;
        Var* new_items = gc_realloc(
                a->items,
                sizeof(Var) * new_cap,
                scan_raw
        );

        a->items = new_items;
        a->capacity = new_cap;
}

void arraylist_push(ArrayList* a, Var v)
{
        if (a->size >= a->capacity) {
                arraylist_grow(a);
        }
        a->items[a->size++] = v;
}

void check_arr_bounds(ArrayList* a, int index)
{
        if (index < 0 || (unsigned int) index >= a->size)
                die(NULL, "index out of bounds for index: %d in array", index);
}

Var build_zero_array_1d(VarType base, int size)
{
        ArrayList* a = arraylist_new(base, size);
        Var out;
        int i;

        for (i = 0; i < size; i++) {
                Var elt;
                elt.type = base;
                switch (base) {
                case TYPE_INT:
                        elt.data.i = 0;
                        break;
                case TYPE_UINT:
                        elt.data.ui = 0;
                        break;
                case TYPE_LONG:
                        elt.data.l = 0;
                        break;
                case TYPE_FLOAT:
                        elt.data.f = 0.0f;
                        break;
                case TYPE_STRING:
                        elt.data.s = string_new("");
                        break;
                default:
                        die(NULL, "unsupported array base type %d", base);
                }
                arraylist_push(a, elt);
        }

        set_array(&out, a);
        return out;
}

static Var build_zero_array_nd(VarType base, int* dims, int ndims)
{
        ArrayList* a;
        Var out;
        int i;

        if (ndims == 1)
                return build_zero_array_1d(base, dims[0]);

        a = arraylist_new(TYPE_ARRAY, dims[0]);
        for (i = 0; i < dims[0]; i++) {
                Var child = build_zero_array_nd(base, dims + 1, ndims - 1);
                arraylist_push(a, child);
        }

        set_array(&out, a);
        return out;
}

Var build_zero_array(VarType base, int* dims, int ndims)
{
        if (ndims <= 0)
                return build_zero_array_1d(base, 0);
        return build_zero_array_nd(base, dims, ndims);
}
