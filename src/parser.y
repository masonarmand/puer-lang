/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */
%code requires {
#include "var.h"
#include <stdlib.h>
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

%left OR
%left AND
%nonassoc EQ NE
%nonassoc LT GT LE GE
%left ADD SUB
%left MUL DIV MOD
%right NOT UMINUS
%nonassoc IFX
%nonassoc ELSE

%nonassoc ASSIGN
%left LBRACKET RBRACKET


%token <ival> NUM
%token <fval> FLOAT
%token <vartype> TYPEKEYWORD
%token <ident> IDENT
%token <ident> STRING
%token <ival> CHAR

%token SEMICOLON
%token LPAREN RPAREN
%token LBRACE RBRACE
%token LBRACKET RBRACKET
%token ADD SUB MUL DIV
%token LT GT LE GE EQ NE
%token NOT
%token AND OR
%token IF ELSE FOR WHILE
%token BREAK CONTINUE
%token DEF RETURN ARROW COMMA
%token PRINT
%token PRINTLN

/* non terminals */
%type <node> puer code block stmt expr
%type <node> control_stmt opt_stmt opt_expr
%type <node> array_items dims opt_initializer
%type <node> function_def param_list arg_list
%type <vartype> opt_return


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
    | code function_def          { $$ = N(NODE_SEQ, @$, 2, $1, $2); }
    ;

function_def
    : DEF IDENT LPAREN param_list RPAREN opt_return block
    {
        /* children = params + the body of the func */
        Node* fn = N(NODE_FUNCDEF, @$, 2, $4, $7);
        fn->varname = $2; /* func name */
        fn->vartype = $6; /* return type */
        $$ = fn;
    }
    ;

param_list
    : /* empty */                { $$ = N(NODE_SEQ, @$, 0); }
    | TYPEKEYWORD IDENT
    {
        Node* d = N(NODE_VARDECL, @$, 0);
        d->varname = $2;
        d->vartype = $1;
        $$ = N(NODE_SEQ, @$, 1, d);
    }
    | param_list COMMA TYPEKEYWORD IDENT
    {
        /* TODO: probably put this in a function */
        Node* d = N(NODE_VARDECL, @$, 0);
        int old;
        d->varname = $4;
        d->vartype = $3;
        old = $1->n_children;
        $1->n_children = old + 1;
        $1->children = realloc($1->children, sizeof(Node*) * (old + 1));
        $1->children[old] = d;
        $$ = $1;
    }
    ;

opt_return
    : /* empty */                { $$ = TYPE_VOID; }
    | ARROW TYPEKEYWORD          { $$ = $2; }
    ;

/* function args when calling function */
arg_list
    : /*empty */                 { $$ = N(NODE_SEQ, @$, 0); }
    | expr                       { $$ = N(NODE_SEQ, @$, 1, $1); }
    | arg_list COMMA expr
    {
        int old = $1->n_children;
        $1->n_children = old + 1;
        $1->children = realloc($1->children, sizeof(Node*) * (old + 1));
        $1->children[old] = $3;
        $$ = $1;
    }
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
    | WHILE LPAREN expr RPAREN block
                                 {
                                   $$ = N(NODE_WHILE, @$, 2, $3, $5);
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

opt_initializer
    : /* empty */                { $$ = N(NODE_NOP, @$, 0); }
    | '=' expr                   { $$ = $2; }
    ;

dims
    : LBRACKET RBRACKET          { $$ = N(NODE_SEQ, @$, 0); }
    | dims LBRACKET RBRACKET
    {
        Node* seq = $1;
        Node* d = N(NODE_NUM, @$, 0);
        d->ival = -1;
        int old = seq->n_children;

        seq->n_children = old + 1;
        seq->children = realloc(
            seq->children,
            sizeof(Node*) * seq->n_children
        );
        seq->children[old] = d;

        $$ = seq;
    }
    | LBRACKET NUM RBRACKET
    {
        Node* d = N(NODE_NUM, @$, 0);
        d->ival = $2;
        $$ = N(NODE_SEQ, @$, 1, d);
    }
    | dims LBRACKET NUM RBRACKET
    {
        Node* seq = $1;
        Node* d = N(NODE_NUM, @$, 0);
        d->ival = $3;

        int old = seq->n_children;
        seq->n_children = old + 1;
        seq->children = realloc(
            seq->children,
            sizeof(Node*) * seq->n_children
        );
        seq->children[old] = d;

        $$ = seq;
    }
    ;

stmt
    : PRINT LPAREN arg_list RPAREN   { $$ = N(NODE_PRINT, @$, 1, $3); }
    | PRINTLN LPAREN arg_list RPAREN { $$ = N(NODE_PRINTLN, @$, 1, $3); }

    /* variable declarations & assignments */
    | TYPEKEYWORD dims IDENT opt_initializer
    {
        /* child 0 = SEQ of NUMS
         * child 1 = initializer */
        Node* n = N(NODE_ARRAYDECL, @$, 2, $2, $4);
        n->vartype = $1;
        n->varname = $3;
        $$ = n;
    }
    | TYPEKEYWORD IDENT          { $$ = N(NODE_VARDECL, @$, 0); $$->varname = $2; $$->vartype = $1; }
    | TYPEKEYWORD IDENT '=' expr { $$ = N(NODE_VARDECL, @$, 1, $4); $$->varname = $2; $$->vartype = $1; }
    | IDENT '=' expr             { $$ = N(NODE_ASSIGN, @$, 1, $3); $$->varname = $1; }
    | BREAK                      { $$ = N(NODE_BREAK, @$, 0); }
    | CONTINUE                   { $$ = N(NODE_CONTINUE, @$, 0); }
    | RETURN expr                { $$ = N(NODE_RETURN, @$, 1, $2); }
    | RETURN                     { $$ = N(NODE_RETURN, @$, 0); $$->vartype = TYPE_VOID; }
    | expr                       { $$ = $1; }
    | expr LBRACKET expr RBRACKET '=' expr %prec ASSIGN
    {
         $$ = N(NODE_IDXASSIGN, @$, 3, $1, $3, $6);
    }
    ;

array_items
    : expr                       { $$ = N(NODE_ARRAYLIT, @$, 1, $1); }
    | array_items COMMA expr
    {
        Node* a = $1;
        int old = a->n_children;
        a->n_children = old + 1;
        a->children = realloc(a->children, sizeof(Node*) * a->n_children);
        a->children[old] = $3;
        $$ = a;
    }
    ;

expr
    : NUM                        { $$ = N(NODE_NUM, @1, 0); $$->ival = $1; }
    | FLOAT                      { $$ = N(NODE_FLOAT, @1, 0); $$->fval = $1; }
    | IDENT                      { $$ = N(NODE_VAR, @1, 0); $$->varname = $1; }
    | STRING                     { $$ = N(NODE_STRING, @1, 0); $$->varname = $1; }
    | CHAR                       { $$ = N(NODE_CHAR, @1, 0); $$->ival = $1; }
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
    | IDENT LPAREN arg_list RPAREN
    {
        $$ = N(NODE_FUNCCALL, @$, 1, $3);
        $$->varname = $1;
    }
    | NOT expr                   { $$ = N(NODE_NOT, @$, 1, $2); }
    | expr AND expr              { $$ = N(NODE_AND, @$, 2, $1, $3); }
    | expr OR expr               { $$ = N(NODE_OR, @$, 2, $1, $3); }
    | expr LBRACKET expr RBRACKET { $$ = N(NODE_IDX, @$, 2, $1, $3); }
    | LBRACKET RBRACKET          { $$ = N(NODE_ARRAYLIT, @$, 0); }
    | LBRACKET array_items RBRACKET { $$ = $2; }
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
