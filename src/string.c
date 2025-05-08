#include "puerstring.h"
#include "util.h"

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
        String* s = malloc(sizeof(String));
        s->data = malloc(len + 1);
        memcpy(s->data, cstr, len + 1);
        s->length = len;
        return s;
}

String* string_concat(String* a, String* b)
{
        unsigned int len = a->length + b->length;
        char* new_data = malloc(len + 1);
        String* result;

        memcpy(new_data, a->data, a->length);
        memcpy(new_data + a->length, b->data, b->length);
        new_data[len] = '\0';

        result = malloc(sizeof(String));
        result->data = new_data;
        result->length = len;

        return result;
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

void stfing_free(String* s)
{
        if (!s)
                return;
        free(s->data);
        free(s);
}
