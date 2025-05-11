#include "puerstring.h"
#include "util.h"
#include "scan.h"

#include <string.h>
#include <stdlib.h>

void check_str_bounds(String* s, int index)
{
        if (index < 0 || (unsigned int) index >= s->length)
                die(NULL, "index out of bounds for index: %d in string: '%s'", s->length, s->data);
}

String* string_new(const char* cstr)
{
        unsigned int len = strlen(cstr);
        String* s = gc_alloc(sizeof(String), scan_string);
        s->data = gc_alloc(len + 1, scan_raw);
        memcpy(s->data, cstr, len + 1);
        s->length = len;
        return s;
}

String* string_concat(String* a, String* b)
{
        unsigned int len = a->length + b->length;
        char* new_data = gc_alloc(len + 1, scan_raw);
        String* result;

        memcpy(new_data, a->data, a->length);
        memcpy(new_data + a->length, b->data, b->length);
        new_data[len] = '\0';

        result = gc_alloc(sizeof(String), scan_string);
        result->data = new_data;
        result->length = len;

        return result;
}

String* string_clone(const String* s)
{
        return string_new(s->data);
}

char string_get(String* s, int index)
{
        check_str_bounds(s, index);
        return s->data[index];
}

void string_set(String* s, int index, char c)
{
        check_str_bounds(s, index);
        s->data[index] = c;
}

int string_len(String* s)
{
        return (int)s->length;
}

void string_free(String* s)
{
        if (!s)
                return;
        free(s->data);
        free(s);
}
