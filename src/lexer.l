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
[ \t\r\n]+               ;
[0-9]+                   { yylval.val = atoi(yytext); return NUM; }

"+"                      return ADD;
"-"                      return SUB;
"*"                      return MUL;
"/"                      return DIV;
"print"                  return PRINT;
"println"                return PRINTLN;
";"                      return SEMICOLON;
"("                      return LPAREN;
")"                      return RPAREN;

"int"                    { yylval.vartype = TYPE_INT; return TYPEKEYWORD; }

[a-zA-Z_][a-zA-Z0-9_]*   { yylval.ident = strdup(yytext); return IDENT; }


.                        return *yytext;
%%
