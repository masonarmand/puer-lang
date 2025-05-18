/*
 * Author: Mason Armand
 * Date Created: May 8th, 2025
 */

#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include "var.h"

ArrayList* arraylist_new(VarType type, int initial_capacity);
ArrayList* arraylist_clone(const ArrayList* src);
void arraylist_grow(ArrayList* a);
void arraylist_push(ArrayList* a, Var v);
void check_arr_bounds(ArrayList* a, int index);
Var build_zero_array(VarType base, const char* recname, int* dims, int ndims);

#endif
