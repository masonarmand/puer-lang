#ifndef SCAN_H
#define SCAN_H

#include "gc_tri.h"

void scan_raw(void* payload, GC_MarkFn mark);
void scan_string(void* payload, GC_MarkFn mark);
void scan_arraylist(void* payload, GC_MarkFn mark);
void scan_varentry(void* payload, GC_MarkFn mark);
void scan_scope(void* payload, GC_MarkFn mark);
void scan_rec(void* payload, GC_MarkFn mark);

#endif
