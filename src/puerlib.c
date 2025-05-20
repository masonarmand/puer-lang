/*
 * Author: Mason Armand
 * Date Created: May 8th, 2025
 *
 * Builtin functions in Puer.
 */
#include "builtin.h"
#include "puerlib.h"
#include "var.h"
#include "arraylist.h"
#include "puerstring.h"
#include "scan.h"
#include "util.h"

#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


Var getch(Node* node, Var* argv)
{
        Var out;
        /* implementation from conio.h */

        struct termios oldattr, newattr;
        int ch;
        tcgetattr( STDIN_FILENO, &oldattr );
        newattr = oldattr;
        newattr.c_lflag &= ~( ICANON | ECHO );
        tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
        ch = getchar();
        tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );


        (void) argv;
        (void) node;
        set_int(&out, ch);
        return out;
}

Var input(Node* node, Var* argv)
{
        unsigned int chunk = 64;
        unsigned int bufcap = chunk;
        unsigned int len = 0;
        char* buf = gc_alloc(bufcap, scan_raw);
        int c;
        Var out;
        String* prompt = argv[0].data.s;

        fputs(prompt->data, stdout);
        fflush(stdout);

        while ((c = fgetc(stdin)) != EOF && c != '\n') {
                if (len + 1 >= bufcap) {
                        bufcap += chunk;
                        buf = gc_realloc(buf, bufcap, scan_raw);
                        if (!buf)
                                die(node, "Error: Out of memory in input()");
                }
                buf[len++] = (char) c;
        }

        buf[len] = '\0';
        set_string(&out, buf);

        return out;
}

Var clear(Node* node, Var* argv)
{
        Var out;
        (void) argv;
        (void) node;
        fputs("\x1b[2J\x1b[H", stdout);
        fflush(stdout);
        set_void(&out);
        return out;
}

Var testlib(Node* node, Var* argv)
{
        Var out;
        (void) argv;
        (void) node;
        fputs("Puer Standard Library !!\n", stdout);
        fflush(stdout);
        set_void(&out);
        return out;
}

Var len(Node* node, Var* argv)
{
        Var container = argv[0];
        Var out;
        switch(container.type) {
        case TYPE_ARRAY:
                set_int(&out, container.data.a->size);
                break;
        case TYPE_STRING:
                set_int(&out, container.data.s->length);
                break;
        default:
                die(node, "Expected type array or string for len()");
        }
        return out;
}

Var append(Node* node, Var* argv)
{
        Var a = argv[0];
        Var v = argv[1];
        Var out;
        ArrayList* arr;

        if (a.type != TYPE_ARRAY)
                die(node, "append: first argument must be an array");

        arr = a.data.a;
        if (v.type != arr->type) {
                die(
                        node,
                        "append: element type mismatch (array holds %d, got %d)",
                        arr->type, v.type
                );
        }

        arraylist_push(arr, v);
        set_void(&out);
        return out;
}

Var gc_collect(Node* node, Var* argv)
{
        Var out;
        (void) argv;
        (void) node;
        set_void(&out);
        gc_collect_full();
        return out;
}

Var randrange(Node* node, Var* argv)
{
        Var out;
        int min = argv[0].data.i;
        int max = argv[1].data.i;

        int range = max - min;
        int num = (rand() % range) + min;
        (void) node;

        set_int(&out, num);
        return out;
}

Var puer_abs(Node* node, Var* argv)
{
        Var out;
        int val = argv[0].data.i;
        (void) node;
        set_int(&out, abs(val));
        return out;
}

void init_puerlib(void)
{
        srand(time(NULL));

        builtin_register("input",      input,      TYPE_STRING, 1, TYPE_STRING);
        builtin_register("testlib",    testlib,    TYPE_VOID,   0);
        builtin_register("getch",      getch,      TYPE_INT,    0);
        builtin_register("clear",      clear,      TYPE_VOID,   0);
        builtin_register("len",        len,        TYPE_INT,    1, TYPE_ANY);
        builtin_register("append",     append,     TYPE_VOID,   2, TYPE_ANY, TYPE_ANY);
        builtin_register("gc_collect", gc_collect, TYPE_VOID,   0);
        builtin_register("randrange",  randrange,  TYPE_INT,    2, TYPE_INT, TYPE_INT);
        builtin_register("abs",        puer_abs,   TYPE_INT,    1, TYPE_INT);
}
