#include "util.h"
#include <stdlib.h>

void die(Node* node, const char* fmt, ...)
{
        fprintf(stderr, "Error");
        if (node) {
                fprintf(stderr, " at line %d, column %d", node->lineno, node->column);
        }
        fprintf(stderr, ": ");
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        fprintf(stderr, "\n");

        exit(1);
}
