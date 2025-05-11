/*
 * Author: Mason Armand
 * Date Created: May 9th, 2025
 */
#include "gc_tri.h"
#include "util.h"
#include "env.h"
#include <stdlib.h>
#include "scan.h"

#define GC_SWEEP_SLICE_SIZE  20

#define HEADER_OF(p) ( (GC_Header*)( (p) ) - 1 )
#define PAYLOAD_OF(h) ( (void*)( (h) + 1 ) )

typedef struct GC_Header {
        struct GC_Header* prev;
        struct GC_Header* next;
        struct GC_Header* gray_next;

        GC_ScanFn scan;
        int marked;
        /* payload*/
} GC_Header;

static GC_Header* heap_head = NULL;
static GC_Header* gray_head = NULL;
static GC_Header* sweep_ptr = NULL;

/* mark bit flips each gc cycle */
static int current_mark_bit = 0;
static int gc_cycle_in_progress = 0;

static void mark_obj(void* payload)
{
        GC_Header* h;
        if (!payload)
                return;
        h = HEADER_OF(payload);
        if (h->marked != current_mark_bit) {
                h->marked = current_mark_bit;
                h->gray_next = gray_head;
                gray_head = h;
        }
}

static void scan_children(GC_Header* h)
{
        if (h->scan) {
                void* payload = PAYLOAD_OF(h);
                h->scan(payload, mark_obj);
        }
}

static void gc_begin_cycle(void)
{
        current_mark_bit = !current_mark_bit;
        gray_head = NULL;
        gc_cycle_in_progress = 1;
        mark_obj(env_stack);
}

/* public gc api */

void gc_init(void)
{
        heap_head = NULL;
        gray_head = NULL;
        current_mark_bit = 0;
}

void* gc_alloc(size_t size, GC_ScanFn scan)
{
        GC_Header* h = malloc(sizeof(GC_Header) + size);
        void* payload;
        if (!h)
                die(NULL, "Out of Memory Error");
        h->marked = 0;
        h->scan = scan;
        h->prev = NULL;
        h->next = heap_head;
        if (heap_head)
                heap_head->prev = h;
        heap_head = h;

        payload = PAYLOAD_OF(h);

        if (gc_cycle_in_progress)
                mark_obj(payload);

        return payload;
}

void gc_mark_root(void* payload)
{
        mark_obj(payload);
}

/* returns 1 (true), if work still remaining */
int gc_step(void)
{
        GC_Header* h;
        if (!gray_head)
                return 0;
        h = gray_head;
        gray_head = h->gray_next;
        scan_children(h);
        return gray_head != NULL;
}

/*
 * sweep up to max
 * if max <= 0, sweep entire heap
 * returns number of objects freed
 */
size_t gc_sweep(int max)
{
        GC_Header* h = heap_head;
        size_t freed = 0;

        while (h) {
                GC_Header* next = h->next;
                int is_white = (h->marked != current_mark_bit);
                int in_budget = (max <= 0 || freed < max);

                if (is_white && in_budget) {
                        /*
                        void* payload = PAYLOAD_OF(h);
                        if (h->scan == scan_arraylist) {
                                fprintf(stderr, "[GC] free ArrayList   @ %p (items @ %p)\n",
                                payload, ((ArrayList*)payload)->items);
                        }
                        else if (h->scan == scan_raw) {
                                fprintf(stderr, "[GC] free raw buffer  @ %p\n", payload);
                        }
                        else if (h->scan == scan_varentry) {
                                fprintf(stderr, "[GC] free VarEntry    @ %p\n", payload);
                        }
                        */
                        if (h->prev)
                                h->prev->next = h->next;
                        else
                                heap_head = h->next;
                        if (h->next)
                                h->next->prev = h->prev;
                        free(h);
                        freed++;
                }
                h = next;
        }
        return freed;
}

int gc_collect_step(void)
{
        if(!gc_cycle_in_progress)
                gc_begin_cycle();

        if (gc_step())
                return 1;
        if (gc_sweep_slice(GC_SWEEP_SLICE_SIZE) > 0)
                return 1;

        gc_cycle_in_progress = 0;
        return 0;
}

void gc_collect_full(void)
{
        gc_begin_cycle();
        while (gc_step()) {}
        gc_sweep_all();
        gc_cycle_in_progress = 0;
}
