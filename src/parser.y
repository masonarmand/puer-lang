/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
%code requires {
        #include "var.h"
}
%{
#include <stdio.h>
#include "ast.h"

/* externs */
extern int yylex();
extern int yyparse();
extern FILE* yyin;

/* prototypes */
void yyerror(const char* s);

/* globals */
/* root of ast (abstract syntax tree) */
Node* root;
%}

%union {
        int val;
        char* ident;
        struct Node* node;
        enum VarType vartype;
}

%left ADD SUB
%left MUL DIV
%right UMINUS

%token <val> NUM
%token ADD SUB MUL DIV
%token PRINT
%token PRINTLN
%token SEMICOLON
%token LPAREN RPAREN
%token <vartype> TYPEKEYWORD
%token <ident> IDENT

%type <node> expr stmt code puer

%%
puer:
    code                         { root = $1; }
    ;

code:
    stmt SEMICOLON code          { $$ = makeNode(NODE_SEQ, 2, $1, $3); }
    | stmt SEMICOLON             { $$ = $1; }
    ;

stmt:
    PRINT LPAREN expr RPAREN     { $$ = makeNode(NODE_PRINT, 1, $3); }
    | PRINTLN LPAREN expr RPAREN { $$ = makeNode(NODE_PRINTLN, 1, $3); }

    /* variable declarations & assignments */
    | TYPEKEYWORD IDENT          { $$ = makeNode(NODE_VARDECL, 0); $$->varname = $2; $$->vartype = $1; }
    | TYPEKEYWORD IDENT '=' expr { $$ = makeNode(NODE_VARDECL, 1, $4); $$->varname = $2; $$->vartype = $1; }
    | IDENT '=' expr             { $$ = makeNode(NODE_ASSIGN, 1, $3); $$->varname = $1; }
    ;

expr:
    NUM                          {
                                   $$ = makeNode(NODE_NUM, 0);
                                   $$->ival = $1;
                                 }
    | IDENT                      { $$ = makeNode(NODE_VAR, 0); $$->varname = $1; }
    | expr ADD expr              { $$ = makeNode(NODE_ADD, 2, $1, $3); }
    | expr SUB expr              { $$ = makeNode(NODE_SUB, 2, $1, $3); }
    | expr MUL expr              { $$ = makeNode(NODE_MUL, 2, $1, $3); }
    | expr DIV expr              { $$ = makeNode(NODE_DIV, 2, $1, $3); }
    | SUB expr %prec UMINUS      {
                                   Node* zero = makeNode(NODE_NUM, 0);
                                   zero->ival = 0;
                                   $$ = makeNode(NODE_SUB, 2, zero, $2);
                                 }
    | LPAREN expr RPAREN         { $$ = $2; }
    ;
%%

void yyerror(const char* s)
{
        fprintf(stderr, "Parse error: %s\n", s);
}
