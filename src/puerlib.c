/*
 * Author: Mason Armand
 * Date Created: May 8th, 2025
 *
 * Builtin functions in Puer.
 */
#include "builtin.h"
#include "puerlib.h"
#include "var.h"
#include "puerstring.h"
#include "scan.h"
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


Var getch(Node* node, Var* argv)
{
        /* implementation from conio.h */

        struct termios oldattr, newattr;
        int ch;
        tcgetattr( STDIN_FILENO, &oldattr );
        newattr = oldattr;
        newattr.c_lflag &= ~( ICANON | ECHO );
        tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
        ch = getchar();
        tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );


        Var out;
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
        fputs("\x1b[2J\x1b[H", stdout);
        fflush(stdout);
        set_void(&out);
        return out;
}

Var testlib(Node* node, Var* argv)
{
        Var out;
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
        set_void(&out);
        gc_collect_full();
        return out;
}

void init_puerlib(void)
{
        REGISTER_BUILTIN(input, TYPE_STRING, TYPE_STRING);
        REGISTER_BUILTIN(testlib, TYPE_VOID);
        REGISTER_BUILTIN(getch, TYPE_INT);
        REGISTER_BUILTIN(clear, TYPE_VOID);
        REGISTER_BUILTIN(len, TYPE_INT, TYPE_ANY);
        REGISTER_BUILTIN(append, TYPE_VOID, TYPE_ANY, TYPE_ANY);
        REGISTER_BUILTIN(gc_collect, TYPE_VOID);
}
