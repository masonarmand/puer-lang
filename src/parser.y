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
%left EQ NE
%left LT GT LE GE
%right UMINUS
%nonassoc IFX
%nonassoc ELSE

%token <val> NUM
%token ADD SUB MUL DIV
%token LT GT LE GE EQ NE
%token IF ELSE FOR
%token PRINT
%token PRINTLN
%token SEMICOLON
%token LPAREN RPAREN
%token LBRACE RBRACE
%token BREAK CONTINUE
%token <vartype> TYPEKEYWORD
%token <ident> IDENT

%type <node> expr control_stmt stmt block code puer opt_stmt opt_expr

%%
puer
    : code                       { root = $1; }
    ;

block
    : stmt SEMICOLON             { $$ = $1; }
    | control_stmt               { $$ = $1; }
    | LBRACE code RBRACE         { $$ = $2; }

code
    : /* empty */                { $$ = makeNode(NODE_NOP, 0); }
    | code stmt SEMICOLON        { $$ = makeNode(NODE_SEQ, 2, $1, $2); }
    | code control_stmt          { $$ = makeNode(NODE_SEQ, 2, $1, $2); }
    ;

control_stmt
    : IF LPAREN expr RPAREN block %prec IFX
                                 {
                                   $$ = makeNode(NODE_IF, 2, $3, $5);
                                 }
    | IF LPAREN expr RPAREN block ELSE block
                                 {
                                   $$ = makeNode(NODE_IFELSE, 3, $3, $5, $7);
                                 }
    | FOR LPAREN opt_stmt SEMICOLON opt_expr SEMICOLON opt_stmt RPAREN block
                                 {
                                   $$ = makeNode(NODE_FOR, 4, $3, $5, $7, $9);
                                 }
    ;

/* optional statements. ex: for things like for(;;)*/
opt_stmt
    : /* empty */                { $$ = makeNode(NODE_NOP, 0); }
    | stmt                       { $$ = $1; }
    ;

/* optional conditions */
opt_expr
    : /* empty */                { $$ = makeNode(NODE_NOP, 0); }
    | expr                       { $$ = $1; }
    ;

stmt
    : PRINT LPAREN expr RPAREN   { $$ = makeNode(NODE_PRINT, 1, $3); }
    | PRINTLN LPAREN opt_expr RPAREN { $$ = makeNode(NODE_PRINTLN, 1, $3); }

    /* variable declarations & assignments */
    | TYPEKEYWORD IDENT          { $$ = makeNode(NODE_VARDECL, 0); $$->varname = $2; $$->vartype = $1; }
    | TYPEKEYWORD IDENT '=' expr { $$ = makeNode(NODE_VARDECL, 1, $4); $$->varname = $2; $$->vartype = $1; }
    | IDENT '=' expr             { $$ = makeNode(NODE_ASSIGN, 1, $3); $$->varname = $1; }
    | BREAK                      { $$ = makeNode(NODE_BREAK, 0); }
    | CONTINUE                   { $$ = makeNode(NODE_CONTINUE, 0); }
    ;

expr
    :expr LT expr                { $$ = makeNode(NODE_LT, 2, $1, $3); }
    | expr GT expr               { $$ = makeNode(NODE_GT, 2, $1, $3); }
    | expr LE expr               { $$ = makeNode(NODE_LE, 2, $1, $3); }
    | expr GE expr               { $$ = makeNode(NODE_GE, 2, $1, $3); }
    | expr EQ expr               { $$ = makeNode(NODE_EQ, 2, $1, $3); }
    | expr NE expr               { $$ = makeNode(NODE_NE, 2, $1, $3); }
    | NUM                        {
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
