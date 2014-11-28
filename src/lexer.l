%{

#include <iostream>
#include "parser.h"

#define SAVE_TOKEN yylval->str = new std::string(yytext, yyleng)
#define YY_USER_ACTION { yylloc->first_line = yylineno; /*yylloc->last_line = yylineno + yylloc->lines(yyleng); /*yylloc->first_column = colnum; yylloc->last_column = colnum + yyleng;*/ }

%}

%option reentrant
%option debug
%option noyywrap
%option never-interactive
%option nounistd
%option yylineno
%option bison-bridge
%option bison-locations

LPAREN "("
RPAREN ")"
LBRACE "{"
RBRACE "}"
SEMICOLON ";"
ADD "+"
MULTIPLY "*"
SUBTRACT "-"
DIVIDE "/"
MOD "%"
ASSIGN "="
COMMA ","
EQ "=="
LT "\<"
GT ">"
LOGICAL_AND "\&\&"
LOGICAL_OR "\|\|"
LOGICAL_NOT "!"

IDENTIFIER [a-z_][a-zA-Z0-9_]*
FUNCTION_REFERENCE :[a-z_][a-zA-Z0-9_]*

NUMBER [0-9]+
WS [ \r\n\t]+
CHAR '[a-z0-9 ]'

STRING \"[a-z0-9 ]+\"
ASSEMBLY_CODE #[a-z0-9_: "]+

%%

{ADD} { return TOKEN_ADD; }
{SUBTRACT} { return TOKEN_SUBTRACT; }
{MULTIPLY} { return TOKEN_MULTIPLY; }
{DIVIDE} { return TOKEN_DIVIDE; }
{MOD} { return TOKEN_MOD; }
{LPAREN} { return TOKEN_LPAREN; }
{RPAREN} { return TOKEN_RPAREN; }
{LBRACE} { return TOKEN_LBRACE; }
{RBRACE} { return TOKEN_RBRACE; }
{SEMICOLON} { return TOKEN_SEMICOLON; }
{ASSIGN} { return TOKEN_ASSIGN; }
{COMMA} { return TOKEN_COMMA; }
{EQ} { return TOKEN_EQ; }
{LT} { return TOKEN_LT; }
{GT} { return TOKEN_GT; }
{LOGICAL_AND} { return TOKEN_LOGICAL_AND; }
{LOGICAL_OR} { return TOKEN_LOGICAL_OR; }
{LOGICAL_NOT} { return TOKEN_LOGICAL_NOT; }
"if" { return TOKEN_IF; }
else { return TOKEN_ELSE; }
var { return TOKEN_VAR; }
def { return TOKEN_DEF; }
return { return TOKEN_RETURN; }
while { return TOKEN_WHILE; }
echo { return TOKEN_ECHO; }
asm { return TOKEN_ASM; }
break { return TOKEN_BREAK; }

{NUMBER} { SAVE_TOKEN; return TOKEN_NUMBER; }
{IDENTIFIER} { SAVE_TOKEN; return TOKEN_IDENTIFIER; }
{STRING} { SAVE_TOKEN; return TOKEN_STRING; }
{ASSEMBLY_CODE} { SAVE_TOKEN; return TOKEN_ASSEMBLY_CODE; }
{CHAR} { SAVE_TOKEN; return TOKEN_CHAR; }
{FUNCTION_REFERENCE} { SAVE_TOKEN; return TOKEN_FUNCTION_REFERENCE; }

{WS} { /* pass */ }

. { std::cout << "Unknown token!" << std::endl; }

%%
