#include "ast.h"
#include "builtin.h"
#include "func.h"
#include "puerlib.h"
#include <stdio.h>
#include <limits.h>

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

        if (!yyparse()) {
                /*print_ast(root, 0);*/
                gc_init();
                init_puerlib();
                eval(root);

                /* cleanup */
                free_ast(root);
                env_clear();
                builtin_clear();
                func_clear();
                gc_collect_full();
        }
        fclose(yyin);
        yylex_destroy();

        return 0;
}
