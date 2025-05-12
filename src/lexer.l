/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
%{
#include "parser.tab.h"
#include <stdio.h>

/*
 * function for updating line number and column in parser
 * from: https://stackoverflow.com/a/22125500
 */
static void update_loc(YYLTYPE* loc, char* txt)
{
        int i;

        loc->first_line = loc->last_line;
        loc->first_column = loc->last_column;

        for(i = 0; txt[i] != '\0'; i++) {
                if(txt[i] == '\n') {
                        loc->last_line++;
                        loc->last_column = 0;
                }
                else {
                        loc->last_column++;
                }
        }
}

int yycolumn = 1;
#define YY_USER_ACTION update_loc(&yylloc, yytext);

%}

%option yylineno

%%
"//".*                   ;
\n                       { yylineno++; yycolumn = 1; }
[ \t\r]+                 { yycolumn += yyleng; }

[0-9]+\.[0-9]+           { yylval.fval = atof(yytext); return FLOAT; }
[0-9]+                   { yylval.ival = atoi(yytext); return NUM; }


\"([^\\\"]|\\.)*\" {
        yylval.ident = strdup(yytext + 1);
        yylval.ident[strlen(yylval.ident) - 1] = '\0';
        return STRING;
}

'[^\\']' {
        yylval.ival = yytext[1];
        return CHAR;
}

'\\n'    { yylval.ival = '\n'; return CHAR; }
'\\t'    { yylval.ival = '\t'; return CHAR; }
'\\r'    { yylval.ival = '\r'; return CHAR; }
'\\\\'   { yylval.ival = '\\'; return CHAR; }
'\\\''   { yylval.ival = '\''; return CHAR; }

"="                      return '=';
"\\+="                   return ADD_ASSIGN;
"-="                     return SUB_ASSIGN;
"*="                     return MUL_ASSIGN;
"%="                     return MOD_ASSIGN;
"/="                     return DIV_ASSIGN;

"+"                      return ADD;
"-"                      return SUB;
"*"                      return MUL;
"/"                      return DIV;
"%"                      return MOD;

"!"                      return NOT;
"&&"                     return AND;
"||"                     return OR;


"=="                     return EQ;
"!="                     return NE;
"<="                     return LE;
">="                     return GE;
"<"                      return LT;
">"                      return GT;

"if"                     return IF;
"else"                   return ELSE;
"for"                    return FOR;
"while"                  return WHILE;
"break"                  return BREAK;
"continue"               return CONTINUE;

"def"                    return DEF;
"return"                 return RETURN;
"->"                     return ARROW;
","                      return ',';

"print"                  return PRINT;
"println"                return PRINTLN;
";"                      return ';';
"("                      return '(';
")"                      return ')';
"{"                      return '{';
"}"                      return '}';
"["                      return '[';
"]"                      return ']';

"int"                    { yylval.vartype = TYPE_INT; return TYPE; }
"bool"                   { yylval.vartype = TYPE_BOOL; return TYPE; }
"uint"                   { yylval.vartype = TYPE_UINT; return TYPE; }
"long"                   { yylval.vartype = TYPE_LONG; return TYPE; }
"float"                  { yylval.vartype = TYPE_FLOAT; return TYPE; }
"str"                    { yylval.vartype = TYPE_STRING; return TYPE; }

"true"                   { yylval.bval = 1; return TRUE; }
"false"                  { yylval.bval = 0; return FALSE; }

[a-zA-Z_][a-zA-Z0-9_]*   { yylval.ident = strdup(yytext); return IDENT; }


.                        return *yytext;

%%
