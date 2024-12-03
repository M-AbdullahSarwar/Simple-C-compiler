%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "IR.h"
    
    extern int yyparse();
    extern int yylex();
    extern FILE *yyin;
    
    extern int yylineno;
    extern char* yytext;  
    
    void yyerror(const char *err);
%}

%debug

%union {
    char *identifier;
    double double_literal;
    char *string_literal;
}

%token tok_printd tok_prints tok_void tok_double tok_return
%token <identifier> tok_identifier
%token <double_literal> tok_double_literal
%token <string_literal> tok_string_literal

%type <double_literal> term expression function_call

%left '+' '-' 
%left '*' '/'
%left '(' ')'

%start root

%%

root:   /* empty */
    | statement root
    | function_definition root
    ; 

statement:
      prints
    | printd
    | assignment
    | function_call ';'
    | return_statement
    ;

prints: tok_prints '(' tok_string_literal ')' ';'   { wrapPrints($3); }

printd: tok_printd '(' expression ')' ';'  { wrapPrintd($3); }

term:   tok_identifier                    { $$ = getValueForIdentifier($1); }
    | tok_double_literal                  { $$ = $1; }
    | function_call                       { $$ = $1; }
    ;

assignment:  tok_identifier '=' expression ';'  { wrapAssignment($1, $3); }

expression: term
    | expression '+' expression           { $$ = performBinaryOperation($1, $3, '+'); }
    | expression '-' expression           { $$ = performBinaryOperation($1, $3, '-'); }
    | expression '/' expression           { $$ = performBinaryOperation($1, $3, '/'); }
    | expression '*' expression           { $$ = performBinaryOperation($1, $3, '*'); }
    | '(' expression ')'                  { $$ = $2; }
    ;	   

function_definition:
    tok_void tok_identifier '(' ')' '{' { startFunctionDefinition($2, false); } statement_list '}'
    { endFunctionDefinition(); }
    | tok_double tok_identifier '(' ')' '{' { startFunctionDefinition($2, true); } statement_list '}'
    { endFunctionDefinition(); }
    ;

statement_list:
    /* empty */
    | statement statement_list
    ;

function_call:
    tok_identifier '(' ')'                { $$ = callFunction($1); }
    ;

return_statement:
    tok_return expression ';'             { wrapReturn($2); }
    ;

%%

void yyerror(const char *err) {
    fprintf(stderr, "Syntax error at line %d: %s\n", yylineno, err);
    fprintf(stderr, "Last token read: %s\n", yytext);
}

int main(int argc, char** argv) {
    if (argc > 1) {
        FILE *fp = fopen(argv[1], "r");
        yyin = fp;
    } 
    if (yyin == NULL) { 
        yyin = stdin;
    }
    
    int parserResult = yyparse();
    
    return EXIT_SUCCESS;
}