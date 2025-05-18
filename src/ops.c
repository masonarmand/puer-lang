/*
 * Author: Mason Armand
 * Date Created: May 5th, 2025
 *
 * Arithmetic and comparison operations for all numeric types in puer.
 */

#include "ops.h"
#include "util.h"

#define DEFINE_BINOP_FN(name, type, ctype, field, op) \
        static Var name##_##type(Var a, Var b) { \
                Var out; \
                set_##type(&out, (ctype)(a.data.field op b.data.field)); \
                return out; \
        }

#define DEFINE_CMP_FN(name, type, field, op) \
        static Var name##_##type(Var a, Var b) { \
                Var out; \
                set_bool(&out, (a.data.field op b.data.field)); \
                return out; \
        }

#define DEFINE_MOD_FN(type, ctype, field) \
        static Var mod_##type(Var a, Var b) { \
                Var out; \
                if (b.data.field == 0) die(NULL, "modulo by zero"); \
                set_##type(&out, (ctype)(a.data.field % b.data.field)); \
                return out; \
        }

/* INT */
DEFINE_BINOP_FN(add, int, int, i, +)
DEFINE_BINOP_FN(sub, int, int, i, -)
DEFINE_BINOP_FN(mul, int, int, i, *)
DEFINE_BINOP_FN(div, int, int, i, /)
DEFINE_MOD_FN(int, int, i)
DEFINE_CMP_FN(lt, int, i, <)
DEFINE_CMP_FN(gt, int, i, >)
DEFINE_CMP_FN(le, int, i, <=)
DEFINE_CMP_FN(ge, int, i, >=)
DEFINE_CMP_FN(eq, int, i, ==)
DEFINE_CMP_FN(ne, int, i, !=)

/* UNSIGNED INT */
DEFINE_BINOP_FN(add, uint, unsigned, ui, +)
DEFINE_BINOP_FN(sub, uint, unsigned, ui, -)
DEFINE_BINOP_FN(mul, uint, unsigned, ui, *)
DEFINE_BINOP_FN(div, uint, unsigned, ui, /)
DEFINE_MOD_FN(uint, unsigned, ui)
DEFINE_CMP_FN(lt, uint, ui, <)
DEFINE_CMP_FN(gt, uint, ui, >)
DEFINE_CMP_FN(le, uint, ui, <=)
DEFINE_CMP_FN(ge, uint, ui, >=)
DEFINE_CMP_FN(eq, uint, ui, ==)
DEFINE_CMP_FN(ne, uint, ui, !=)

/* LONG */
DEFINE_BINOP_FN(add, long, long, l, +)
DEFINE_BINOP_FN(sub, long, long, l, -)
DEFINE_BINOP_FN(mul, long, long, l, *)
DEFINE_BINOP_FN(div, long, long, l, /)
DEFINE_MOD_FN(long, long, l)
DEFINE_CMP_FN(lt, long, l, <)
DEFINE_CMP_FN(gt, long, l, >)
DEFINE_CMP_FN(le, long, l, <=)
DEFINE_CMP_FN(ge, long, l, >=)
DEFINE_CMP_FN(eq, long, l, ==)
DEFINE_CMP_FN(ne, long, l, !=)

/* FLOAT */
DEFINE_BINOP_FN(add, float, float, f, +)
DEFINE_BINOP_FN(sub, float, float, f, -)
DEFINE_BINOP_FN(mul, float, float, f, *)
DEFINE_BINOP_FN(div, float, float, f, /)
/* float mod unsupported */
DEFINE_CMP_FN(lt, float, f, <)
DEFINE_CMP_FN(gt, float, f, >)
DEFINE_CMP_FN(le, float, f, <=)
DEFINE_CMP_FN(ge, float, f, >=)
DEFINE_CMP_FN(eq, float, f, ==)
DEFINE_CMP_FN(ne, float, f, !=)

/* BOOL */
DEFINE_CMP_FN(eq,   bool, b, ==)
DEFINE_CMP_FN(ne,   bool, b, !=)

TypeOps type_ops[] = {
        /* TYPE_INT = 0 */
        { { add_int, sub_int, mul_int, div_int, mod_int, lt_int, gt_int, le_int, ge_int, eq_int, ne_int } },
        /* TYPE_UINT = 1 */
        { { add_uint, sub_uint, mul_uint, div_uint, mod_uint, lt_uint, gt_uint, le_uint, ge_uint, eq_uint, ne_uint } },
        /* TYPE_LONG = 2 */
        { { add_long, sub_long, mul_long, div_long, mod_long, lt_long, gt_long, le_long, ge_long, eq_long, ne_long } },
        /* TYPE_FLOAT = 3 */
        { { add_float, sub_float, mul_float, div_float, 0, lt_float, gt_float, le_float, ge_float, eq_float, ne_float } },
        /* TYPE_BOOL = 4 */
        { { 0, 0, 0, 0, 0, 0, 0, 0, 0, eq_bool, ne_bool, 0, 0 } },
};
