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
ASSIGN "="
COMMA ","
EQ "=="
LT "<"
GT ">"

IDENTIFIER [a-z_][a-zA-Z0-9_]*

NUMBER [0-9]+
WS [ \r\n\t]+

STRING \"[a-z0-9 ]+\"
ASSEMBLY_CODE %[a-z0-9_: ]+

%%

{ADD} { return TOKEN_ADD; }
{SUBTRACT} { return TOKEN_SUBTRACT; }
{MULTIPLY} { return TOKEN_MULTIPLY; }
{DIVIDE} { return TOKEN_DIVIDE; }
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

{WS} { /* pass */ }

. { std::cout << "Unknown token!" << std::endl; }

%%
