#include "util.h"
#include <stdlib.h>

void die(Node* node, const char* fmt, ...)
{
        va_list args;
        fprintf(stderr, "Error");
        if (node) {
                fprintf(stderr, " at line %d, column %d", node->lineno, node->column);
        }
        fprintf(stderr, ": ");
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        fprintf(stderr, "\n");

        exit(1);
}
