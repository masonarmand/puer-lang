/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
%{
#include "parser.tab.h"
#include <stdio.h>
%}

%%
[ \t\r\n]+      ;
[0-9]+          { yylval.val = atoi(yytext); return NUM; }
"+"             return ADD;
"-"             return SUB;
"*"             return MUL;
"/"             return DIV;
"print"         return PRINT;
";"             return SEMICOLON;
"("             return LPAREN;
")"             return RPAREN;
.               return *yytext;
%%
