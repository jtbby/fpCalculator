
%{
#include <stdio.h>
#include <unistd.h>
#include "tinysf.h"
#include "symtab.h"
#include "common_structs.h"
#include "common_functions.h"

  int yylex();
  int yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
  }

%}

%code requires {
#include <stdlib.h>
#include "common_structs.h"
}

%union {
  struct number_struct fpval;
  int number;
  char *strval;
};

%expect 2
%token <number> NUMBER 
%token <fpval> FLOAT INF_T NAN_T
%token HELP_T PRINT_T DISPLAY_T EXIT_T SYNERR_T VARIABLE EOL 

%type <number> expression
%type <strval> VARIABLE

%right '='
%left '-' '+'
%left '*' 
%left '(' ')'
%precedence UNARY

%%

start: %empty
  | EXIT_T {printf("%c%sSIC PARVIS MAGNA%s\n", IS_INTERACTIVE?'\r':'\n', CYAN, RST); return 42;}
  | expression EOL {print_prompt("$ ");} start
  | EOL {print_prompt("$ ");} start
  | error EOL {print_prompt("$ ");} start
  ;

expression:  NUMBER           { $$ = $1; }
  | FLOAT                     {
                                $1.fraction = fraction_to_binary($1.conversion.fracorig, $1.conversion.precision);
                                $1.conversion.precision = 0;
                                $1.is_nan = 0;
                                $1.is_infinity = 0;
                                $1.is_negative = 0;
                                $$ = toTinySF(&$1); ECHONOTTY("\r");}
  | VARIABLE                  { $$ = sym_exists($1)?get_value($1):0; 
                                sym_exists($1)?1:print_sym_not_found($1); ECHONOTTY("\r");}
  | INF_T                     { $1.fraction = 0;
                                $1.conversion.precision = 0;
                                $1.is_nan = 0;
                                $1.is_infinity = 1;
                                $1.is_negative = 0;
                                $$ = toTinySF(&$1); ECHONOTTY("\r");}
  | NAN_T                     { $1.fraction = 0;
                                $1.conversion.precision = 0;
                                $1.is_nan = 1;
                                $1.is_infinity = 0;
                                $1.is_negative = 0;
                                $$ = toTinySF(&$1); ECHONOTTY("\r");}
  | '-' expression %prec UNARY{ $$ = negateTinySF($2); }
  | '+' expression %prec UNARY{ $$ = $2; }
  | expression '+' expression { $$ = opTinySF('+', $1, $3); }
  | expression '-' expression { $$ = opTinySF('-', $1, $3); }
  | expression '*' expression { $$ = opTinySF('*', $1, $3); }
  | '(' expression ')'        { $$ = $2; }
  | VARIABLE '=' expression   { $$ = $3; insert_symbol($1, $3); }
  | HELP_T                    { $$ = 0; print_help(); }
  | PRINT_T '(' VARIABLE ')'  { struct number_struct *num = calloc(1, sizeof(Number_s));
                                sym_exists($3)?toNumber(num, get_value($3)):0;
                                int precision = 0;
                                long result = binary_to_fraction(num->fraction, &precision);
                                ECHOTTY("\n");
                                sym_exists($3)?print_num($3, num, precision, result):print_sym_not_found($3);
                                free(num);
                                $$ = get_value($3);
                              }
  | PRINT_T '(' expression ')'  { struct number_struct *num = calloc(1, sizeof(Number_s));
                                toNumber(num, $3);
                                int precision = 0;
                                long result = binary_to_fraction(num->fraction, &precision);
                                ECHOTTY("\n");
                                print_num("Value", num, precision, result);
                                free(num);
                                $$ = $3;
                              }
  | DISPLAY_T '(' VARIABLE ')'  {
                                $$ = get_value($3);
                                ECHOTTY("\n");
                                sym_exists($3)?print_value("TinySF Value in Binary = ", $$):print_sym_not_found($3);
                              }
  | DISPLAY_T '(' expression ')'  {
                                $$ = $3;
                                ECHOTTY("\n");
                                print_value("TinySF Value in Binary = ", $$);
                              }
  | SYNERR_T                  { printf("\rCOMMAND Error\n"); }
  ;

%%

extern int yylex();
extern int yyparse();
extern int yyerror();
extern FILE *yyin;

