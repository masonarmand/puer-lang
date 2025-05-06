/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
%code requires {
#include "var.h"
}

%locations

%{
#include <stdio.h>
#include "ast.h"

/* externs */
extern int yylex();
extern int yyparse();
extern FILE* yyin;

/* prototypes */
void yyerror(const char* s);

/* macros */
#define N(T, LOC, ...)  \
    ({ Node* _tmp = makeNode((T), __VA_ARGS__); set_loc(_tmp, (LOC)); _tmp; })


/* globals */
/* root of ast (abstract syntax tree) */
Node* root;
%}

%code provides {
#include <stdarg.h>
#include "ast.h"

static Node* set_loc(Node* n, YYLTYPE loc)
{
        n->lineno  = loc.first_line;
        n->column  = loc.first_column;
        return n;
}


}

%union {
        int ival;
        float fval;
        char* ident;
        struct Node* node;
        enum VarType vartype;
}

%left ADD SUB MUL DIV MOD
%left EQ NE
%left LT GT LE GE
%right UMINUS
%nonassoc IFX
%nonassoc ELSE

%token <ival> NUM
%token <fval> FLOAT
%token <vartype> TYPEKEYWORD
%token <ident> IDENT

%token SEMICOLON
%token LPAREN RPAREN
%token LBRACE RBRACE
%token ADD SUB MUL DIV
%token LT GT LE GE EQ NE
%token IF ELSE FOR
%token BREAK CONTINUE
%token DEF RETURN ARROW COMMA
%token PRINT
%token PRINTLN

%type <node> expr control_stmt stmt block code puer opt_stmt opt_expr
%type <node> funcdef param_list param return_stmt call


%%
puer
    : code                       { root = $1; }
    ;

block
    : stmt SEMICOLON             { $$ = $1; }
    | control_stmt               { $$ = $1; }
    | LBRACE code RBRACE         { $$ = $2; }

code
    : /* empty */                { $$ = N(NODE_NOP, @$, 0); }
    | code stmt SEMICOLON        { $$ = N(NODE_SEQ, @$, 2, $1, $2); }
    | code control_stmt          { $$ = N(NODE_SEQ, @$, 2, $1, $2); }
    ;

control_stmt
    : IF LPAREN expr RPAREN block %prec IFX
                                 {
                                   $$ = N(NODE_IF, @$, 2, $3, $5);
                                 }
    | IF LPAREN expr RPAREN block ELSE block
                                 {
                                   $$ = N(NODE_IFELSE, @$, 3, $3, $5, $7);
                                 }
    | FOR LPAREN opt_stmt SEMICOLON opt_expr SEMICOLON opt_stmt RPAREN block
                                 {
                                   $$ = N(NODE_FOR, @$, 4, $3, $5, $7, $9);
                                 }
    ;

/* optional statements. ex: for things like for(;;)*/
opt_stmt
    : /* empty */                { $$ = N(NODE_NOP, @$, 0); }
    | stmt                       { $$ = $1; }
    ;

/* optional conditions */
opt_expr
    : /* empty */                { $$ = N(NODE_NOP, @$, 0); }
    | expr                       { $$ = $1; }
    ;

stmt
    : PRINT LPAREN expr RPAREN   { $$ = N(NODE_PRINT, @$, 1, $3); }
    | PRINTLN LPAREN opt_expr RPAREN { $$ = N(NODE_PRINTLN, @$, 1, $3); }

    /* variable declarations & assignments */
    | TYPEKEYWORD IDENT          { $$ = N(NODE_VARDECL, @$, 0); $$->varname = $2; $$->vartype = $1; }
    | TYPEKEYWORD IDENT '=' expr { $$ = N(NODE_VARDECL, @$, 1, $4); $$->varname = $2; $$->vartype = $1; }
    | IDENT '=' expr             { $$ = N(NODE_ASSIGN, @$, 1, $3); $$->varname = $1; }
    | BREAK                      { $$ = N(NODE_BREAK, @$, 0); }
    | CONTINUE                   { $$ = N(NODE_CONTINUE, @$, 0); }
    ;

expr
    : NUM                        { $$ = N(NODE_NUM, @1, 0); $$->ival = $1; }
    | FLOAT                      { $$ = N(NODE_FLOAT, @1, 0); $$->fval = $1; }
    | IDENT                      { $$ = N(NODE_VAR, @1, 0); $$->varname = $1; }
    | expr LT expr               { $$ = N(NODE_LT, @$, 2, $1, $3); }
    | expr GT expr               { $$ = N(NODE_GT, @$, 2, $1, $3); }
    | expr LE expr               { $$ = N(NODE_LE, @$, 2, $1, $3); }
    | expr GE expr               { $$ = N(NODE_GE, @$, 2, $1, $3); }
    | expr EQ expr               { $$ = N(NODE_EQ, @$, 2, $1, $3); }
    | expr NE expr               { $$ = N(NODE_NE, @$, 2, $1, $3); }
    | expr ADD expr              { $$ = N(NODE_ADD, @$, 2, $1, $3); }
    | expr SUB expr              { $$ = N(NODE_SUB, @$, 2, $1, $3); }
    | expr MUL expr              { $$ = N(NODE_MUL, @$, 2, $1, $3); }
    | expr DIV expr              { $$ = N(NODE_DIV, @$, 2, $1, $3); }
    | expr MOD expr              { $$ = N(NODE_MOD, @$, 2, $1, $3); }
    | SUB expr %prec UMINUS      {
                                   Node* zero = N(NODE_NUM, @$, 0);
                                   zero->ival = 0;
                                   $$ = N(NODE_SUB, @$, 2, zero, $2);
                                 }
    | LPAREN expr RPAREN         { $$ = $2; }
    ;
%%

#include <string.h>

extern YYLTYPE yyloc;
extern char* yytext;

void yyerror(const char* s)
{
        fprintf(stderr, "Parse error: %s\n", s);

        fprintf(
                stderr, "Parse error at line %d, column %d: %s\n",
                yylloc.first_line,
                yylloc.first_column, s
        );

        if (yyin) {
                int line = 1;
                int c;
                char buf[512];

                fseek(yyin, 0, SEEK_SET);

                while (line < yylloc.first_line && (c = fgetc(yyin)) != EOF) {
                        if (c == '\n')
                                line++;
                }

                if (fgets(buf, sizeof(buf), yyin)) {
                        unsigned int i;
                        fprintf(stderr, "%s", buf);

                        for (i = 0; i <= yylloc.first_column; i++)
                                fputc(buf[i] == '\t' ? '\t' : ' ', stderr);
                        fprintf(stderr, "^\n");
                }
        }
}
