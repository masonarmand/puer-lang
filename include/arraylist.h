/*
 * Author: Mason Armand
 * Date Created: May 8th, 2025
 */

#ifndef ARRAYLIST_H
#define ARRAYLIST_H

ArrayList* arraylist_new(VarType type, int initial_capacity);
void arraylist_grow(ArrayList* a);
void arraylist_push(ArrayList* a, Var v);
Var build_zero_array(VarType base, int* dims, int ndims);

#endif
