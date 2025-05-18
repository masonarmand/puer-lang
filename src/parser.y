/*
 * Author: Mason Armand
 * Date Created: May 2nd, 2025
 * Last Modified: May 2nd, 2025
 */

%code requires {
#include "var.h"
#include "rec.h"
#include <stdlib.h>
}

%locations
%defines

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
char* g_recname = NULL;
%}

%union {
        int ival;
        int bval;
        float fval;
        char* ident;
        struct Node* node;
        enum VarType vartype;
}

%right '=' ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%right '.'
%nonassoc RET
%right ARROW
%left OR
%left AND
%nonassoc EQ NE
%nonassoc LT GT LE GE
%left ADD SUB
%left MUL DIV MOD

%right INC DEC NOT UMINUS

%nonassoc POSTFIX '[' ']' '(' ')'

%nonassoc IFX
%nonassoc ELSE

%token <ival> NUM
%token <bval> TRUE FALSE
%token <fval> FLOAT
%token <vartype> TYPE
%token <ident> IDENT
%token <ident> STRING
%token <ival> CHAR

%token '='
%token ';'
%token '(' ')'
%token '{' '}'
%token '[' ']'
%token ADD SUB MUL DIV
%token ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token LT GT LE GE EQ NE
%token NOT
%token AND OR
%token INC DEC
%token IF ELSE FOR WHILE
%token BREAK CONTINUE
%token DEF RETURN ARROW ','
%token REC
%token PRINT
%token PRINTLN

/* non terminals */
%type <node> puer top_code code block stmt expr
%type <node> control_stmt opt_stmt opt_expr stmt_end top_stmt_end
%type <node> array_items dims opt_init
%type <node> function_def param param_list arg_list
%type <node> vardecl varassign
%type <node> rec_def rec_field_list rec_field
%type <vartype> opt_return


%%
puer
    : top_code                             { root = $1; }
    ;

top_code
    : /* empty */                          { $$ = node(NODE_SEQ, @$, 0); }
    | top_code top_stmt_end                { $$ = node_append($1, $2); }
    ;

top_stmt_end
    : function_def                         { $$ = $1; }
    | rec_def                              { $$ = $1; }
    | stmt_end                             { $$ = $1; }
    ;

block
    : '{' code '}'                         { $$ = $2; }
    ;

code
    : /* empty */                          { $$ = node(NODE_SEQ, @$, 0); }
    | code stmt_end                        { $$ = node_append($1, $2); }
    ;

stmt_end
   : stmt ';'                              { $$ = $1; }
   | control_stmt                          { $$ = $1; }
   | block                                 { $$ = $1; }
   ;

stmt
    : PRINT '(' arg_list ')'               { $$ = node(NODE_PRINT, @$, 1, $3); }
    | PRINTLN '(' arg_list ')'             { $$ = node(NODE_PRINTLN, @$, 1, $3); }
    | BREAK                                { $$ = node(NODE_BREAK, @$, 0); }
    | CONTINUE                             { $$ = node(NODE_CONTINUE, @$, 0); }
    | RETURN expr                          { $$ = node(NODE_RETURN, @$, 1, $2); }
    | RETURN                               { $$ = node(NODE_RETURN, @$, 0); settype($$, TYPE_VOID); }
    | vardecl                              { $$ = $1; }
    | expr                                 { $$ = $1; }
    ;

expr
    : NUM                                  { $$ = node(NODE_NUM, @1, 0); $$->ival = $1; }
    | TRUE                                 { $$ = node(NODE_BOOL, @1, 0); $$->ival = $1; }
    | FALSE                                { $$ = node(NODE_BOOL, @1, 0); $$->ival = $1; }
    | FLOAT                                { $$ = node(NODE_FLOAT, @1, 0); $$->fval = $1; }
    | IDENT                                { $$ = node(NODE_VAR, @1, 0); setname($$, $1); }
    | STRING                               { $$ = node(NODE_STRING, @1, 0); setname($$, $1); }
    | CHAR                                 { $$ = node(NODE_CHAR, @1, 0); $$->ival = $1; }
    | expr '.' IDENT         %prec POSTFIX { $$ = node(NODE_FIELDACCESS, @$, 1, $1); setname($$, $3); }
    | expr LT expr                         { $$ = node_binop(OP_LT, @$, $1, $3); }
    | expr GT expr                         { $$ = node_binop(OP_GT, @$, $1, $3); }
    | expr LE expr                         { $$ = node_binop(OP_LE, @$, $1, $3); }
    | expr GE expr                         { $$ = node_binop(OP_GE, @$, $1, $3); }
    | expr EQ expr                         { $$ = node_binop(OP_EQ, @$, $1, $3); }
    | expr NE expr                         { $$ = node_binop(OP_NE, @$, $1, $3); }
    | expr ADD expr                        { $$ = node_binop(OP_ADD, @$, $1, $3); }
    | expr SUB expr                        { $$ = node_binop(OP_SUB, @$, $1, $3); }
    | expr MUL expr                        { $$ = node_binop(OP_MUL, @$, $1, $3); }
    | expr DIV expr                        { $$ = node_binop(OP_DIV, @$, $1, $3); }
    | expr MOD expr                        { $$ = node_binop(OP_MOD, @$, $1, $3); }

    | expr AND expr                        { $$ = node(NODE_AND, @$, 2, $1, $3); }
    | expr OR expr                         { $$ = node(NODE_OR, @$, 2, $1, $3); }

    | expr INC               %prec POSTFIX { $$ = node_incdec(OP_ADD, @$, $1, 0); }
    | expr DEC               %prec POSTFIX { $$ = node_incdec(OP_SUB, @$, $1, 0); }
    | IDENT '(' arg_list ')' %prec POSTFIX { $$ = node(NODE_FUNCCALL, @$, 1, $3); setname($$, $1); }
    | expr '[' expr ']'      %prec POSTFIX { $$ = node(NODE_IDX, @$, 2, $1, $3); }

    | INC expr                             { $$ = node_incdec(OP_ADD, @$, $2, 1); }
    | DEC expr                             { $$ = node_incdec(OP_SUB, @$, $2, 1); }
    | NOT expr                             { $$ = node(NODE_NOT, @$, 1, $2); }
    | SUB expr               %prec UMINUS  { $$ = node_uminus($2, @$); }

    | expr ADD_ASSIGN expr                 { $$ = node_compound(OP_ADD, @$, $1, $3); }
    | expr SUB_ASSIGN expr                 { $$ = node_compound(OP_SUB, @$, $1, $3); }
    | expr MUL_ASSIGN expr                 { $$ = node_compound(OP_MUL, @$, $1, $3); }
    | expr DIV_ASSIGN expr                 { $$ = node_compound(OP_DIV, @$, $1, $3); }
    | expr MOD_ASSIGN expr                 { $$ = node_compound(OP_MOD, @$, $1, $3); }

    | '(' expr ')'                         { $$ = $2; }
    | '[' ']'                              { $$ = node(NODE_ARRAYLIT, @$, 0); }
    | '[' array_items ']'                  { $$ = $2; }
    | varassign                            { $$ = $1; }
    ;

vardecl
    : TYPE dims IDENT opt_init        {
        $$ = node(NODE_ARRAYDECL, @$, 2, $2, $4);
        setvar($$, $1, $3);
        if ($1 == TYPE_REC)
                $$->recname = g_recname;
    }
    | TYPE IDENT opt_init             {
        $$ = node(NODE_VARDECL, @$, 1, $3);
        setvar($$, $1, $2);
        if ($1 == TYPE_REC)
                $$->recname = g_recname;
    }
    ;

rec_def
    : REC IDENT '{' rec_field_list '}' ';' {
        $$ = node(NODE_RECDEF, @$, 1, $4);
        setname($$, $2);
        recname_register($2);
    }
    ;

rec_field_list
    : /* empty */                          { $$ = node(NODE_SEQ, @$, 0); }
    | rec_field_list rec_field             { $$ = node_append($1, $2); }
    ;

rec_field
    : vardecl ';'                          { $$ = $1; }
    ;

function_def
    : DEF IDENT '(' param_list ')' opt_return block { $$ = node(NODE_FUNCDEF, @$, 2, $4, $7); setvar($$, $6, $2); };

param_list
    : /* empty */                          { $$ = node(NODE_SEQ, @$, 0); }
    | param                                { $$ = $1; }
    | param_list ',' param                 { $$ = node_append($1, $3); }
    ;

param
    : TYPE IDENT                           { $$ = node_param($1, $2, @$); }
    | TYPE dims IDENT {
        Node* arrdecl = node(NODE_ARRAYDECL, @$, 2, $2, node(NODE_NOP, @$, 0));
        setvar(arrdecl, TYPE_ARRAY, $3);
        $$ = node_param_append(arrdecl, $1, $3, @$);
    }

opt_return
    : /* empty */                          { $$ = TYPE_VOID; }
    | ARROW TYPE                 %prec RET { $$ = $2; }
    | ARROW TYPE dims %prec RET            { $$ = TYPE_ARRAY; free_ast($3); }
    ;

arg_list
    : /*empty */                           { $$ = node(NODE_SEQ, @$, 0); }
    | expr                                 { $$ = node(NODE_SEQ, @$, 1, $1); }
    | arg_list ',' expr                    { $$ = node_append($1, $3); }
    ;

control_stmt
    : IF '(' expr ')' stmt_end %prec IFX                      { $$ = node(NODE_IF, @$, 2, $3, $5); }
    | IF '(' expr ')' stmt_end ELSE stmt_end                  { $$ = node(NODE_IFELSE, @$, 3, $3, $5, $7); }
    | FOR '(' opt_stmt ';' opt_expr ';' opt_stmt ')' stmt_end { $$ = node(NODE_FOR, @$, 4, $3, $5, $7, $9); }
    | WHILE '(' expr ')' stmt_end                             { $$ = node(NODE_WHILE, @$, 2, $3, $5); }
    ;

/* optional statements. ex: for things like for(;;)*/
opt_stmt
    : /* empty */                          { $$ = node(NODE_NOP, @$, 0); }
    | stmt                                 { $$ = $1; }
    ;

/* optional conditions */
opt_expr
    : /* empty */                          { $$ = node(NODE_NOP, @$, 0); }
    | expr                                 { $$ = $1; }
    ;

opt_init
    : /* empty */                          { $$ = node(NODE_NOP, @$, 0); }
    | '=' expr                             { $$ = $2; }
    ;

dims
    : '[' ']'                              { $$ = node(NODE_SEQ, @$, 0); }
    | dims '[' ']'                         { $$ = node_append_type($1, NODE_NOP, @$); }
    | '[' expr ']'                         { $$ = node(NODE_SEQ, @$, 1, $2); }
    | dims '[' expr ']'                    { $$ = node_append($1, $3); }
    ;

varassign
    : IDENT '=' expr             %prec '=' { $$ = node(NODE_ASSIGN, @$, 1, $3); setname($$, $1); }
    | expr '[' expr ']' '=' expr %prec '=' { $$ = node(NODE_IDXASSIGN, @$, 3, $1, $3, $6); }
    | expr '.' IDENT '=' expr    %prec '=' { $$ = node(NODE_FIELDASSIGN, @$, 2, $1, $5); setname($$, $3); }
    ;

array_items
    : expr                                 { $$ = node(NODE_ARRAYLIT, @$, 1, $1); }
    | array_items ',' expr                 { node_append($1, $3); }
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
                        int i;
                        fprintf(stderr, "%s", buf);

                        for (i = 0; i <= yylloc.first_column; i++)
                                fputc(buf[i] == '\t' ? '\t' : ' ', stderr);
                        fprintf(stderr, "^\n");
                }
        }
}
