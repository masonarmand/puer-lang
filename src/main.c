#include "ast.h"
#include <stdio.h>

extern int yyparse();
extern FILE* yyin;
extern Node* root;

extern int yylineno;
extern int yycolumn;


int main(int argc, char** argv)
{
        if (argc > 1)
                yyin = fopen(argv[1], "r");
        if (!yyin) {
                /* yyin = stdin; */
                fprintf(stderr, "Error: could not open input file \n");
                return 1;
        }

        yylineno = 1;
        yycolumn = 1;

        if (yyparse() == 0) {
                /*print_ast(root, 0);*/
                eval(root);
        }

        return 0;
}
