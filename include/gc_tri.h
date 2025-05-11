#ifndef GC_TRI_H
#define GC_TRI_H

#include <stddef.h>

/* callback to mark child pointer */
typedef void (*GC_MarkFn)(void* payload);

/* scan callback for scanning children of heap object */
typedef void (*GC_ScanFn)(void* payload, GC_MarkFn mark);

void gc_init(void);
void* gc_alloc(size_t size, GC_ScanFn scan);
void* gc_realloc(void* ptr, size_t new_size, GC_ScanFn scan);
int gc_collect_step(void);

void gc_mark_root(void* payload);
int gc_step(void);
size_t gc_sweep(int max);

#define gc_sweep_all() gc_sweep(0)
#define gc_sweep_slice(n) gc_sweep(n)

#endif
