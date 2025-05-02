#include "ast.h"
#include <stdio.h>

extern int yyparse();
extern FILE* yyin;
extern Node* root;

int main(int argc, char** argv)
{
        if (argc > 1)
                yyin = fopen(argv[1], "r");
        if (!yyin)
                yyin = stdin;

        if (yyparse() == 0) {
                /* print_ast(root, 0); */
                eval(root);
        }

        return 0;
}
