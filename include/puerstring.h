#ifndef PUERSTRING_H
#define PUERSTRING_H

typedef struct {
        char* data;
        unsigned int length;
} String;

String* string_new(const char* cstr);
String* string_concat(String* a, String* b);
String* string_clone(const String* s);
char string_get(String* s, int index);
void string_set(String* s, int index, char c);
int string_len(String* s);
void string_free(String* s);

#endif
