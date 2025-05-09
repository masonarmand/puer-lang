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
        char* buf = malloc(bufcap);
        int c;
        Var out;
        String* prompt = argv[0].data.s;

        fputs(prompt->data, stdout);
        fflush(stdout);

        while ((c = fgetc(stdin)) != EOF && c != '\n') {
                if (len + 1 >= bufcap) {
                        bufcap += chunk;
                        buf = realloc(buf, bufcap);
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

Var arrlen(Node* node, Var* argv)
{
        Var arg = argv[0];
        Var out;
        set_int(&out, arg.data.a->size);
        return out;
}

Var puer_strlen(Node* node, Var* argv)
{
        Var arg = argv[0];
        Var out;
        set_int(&out, arg.data.s->length);
        return out;
}

void init_puerlib(void)
{
        REGISTER_BUILTIN(input, { TYPE_STRING }, 1, TYPE_STRING);
        REGISTER_BUILTIN(testlib, {}, 0, TYPE_VOID);
        REGISTER_BUILTIN(getch, {}, 0, TYPE_INT);
        REGISTER_BUILTIN(clear, {}, 0, TYPE_VOID);
        REGISTER_BUILTIN(arrlen, { TYPE_ARRAY  }, 1, TYPE_INT);
        REGISTER_BUILTIN_WNAME("strlen", puer_strlen, { TYPE_STRING }, 1, TYPE_INT);
}
