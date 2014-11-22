%{

#include <iostream>
#include "parser.h"
#include "lexer.h"
#include "ast.h"

void yyerror(YYLTYPE* llocp, ASTNode**, yyscan_t scanner, const char *s) {
	std::cerr << "YYERROR: " << s << ", " << llocp->first_line << ", " << llocp->first_column << ", " << llocp->last_line << ", " << llocp->last_column << std::endl;
}

%}

%code requires {

#include <vector>

class ASTNode;
class ASTNodeBlock;
class ASTNodeIdentifier;
class ASTNodeDeclaration;
class ASTNodeFunctionPrototype;

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

}

%define api.pure full
/*%define parse.error verbose*/
%error-verbose
%locations
%lex-param { yyscan_t yyscanner }
%parse-param { ASTNode** root }
%parse-param { yyscan_t yyscanner }

%union {
	ASTNode* node;
	ASTNodeBlock* block;
	ASTNodeIdentifier* identifier;
	ASTNodeFunctionPrototype* function_prototype;
	ASTNodeDeclaration* declaration;
	std::vector<ASTNodeDeclaration*>* identifier_list;
	std::vector<ASTNode*>* args_list;
	std::string* str;
}

%right TOKEN_ASSIGN
%left TOKEN_EQ TOKEN_LT TOKEN_GT
%left TOKEN_ADD TOKEN_SUBTRACT
%left TOKEN_MULTIPLY TOKEN_DIVIDE
%left TOKEN_POW
%left TOKEN_RPAREN

%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_ASSIGN TOKEN_LBRACE TOKEN_RBRACE
%token TOKEN_ADD TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_SUBTRACT
%token TOKEN_COMMA TOKEN_IF TOKEN_ELSE TOKEN_VAR TOKEN_DEF TOKEN_RETURN
%token TOKEN_SEMICOLON
%token <str> TOKEN_NUMBER TOKEN_IDENTIFIER

%type <node> program expr number binary_operator_expr assignment_expr function_call_expr function_expr if_else_expr top_level_expr
%type <block> stmts function_list
%type <identifier> identifier
%type <function_prototype> function_prototype_expr
%type <declaration> declaration_expr
%type <identifier_list> identifier_list
%type <args_list> args_list

%start program

%%

program
	: function_list { *root = $1; }
	;

function_list
	: top_level_expr {
		$$ = new ASTNodeBlock();
		$$->push($1);
	}
	| function_list top_level_expr { $$->push($2); }
	;

stmts
	: expr TOKEN_SEMICOLON {
		$$ = new ASTNodeBlock();
		$$->push($1);
	}
	| stmts expr TOKEN_SEMICOLON { $$->push($2); }
	;

top_level_expr
	: function_expr { $$ = $1; }
	| function_prototype_expr TOKEN_SEMICOLON { $$ = $1; }
	;

expr
	: TOKEN_LPAREN expr TOKEN_RPAREN { $$ = $2; }
	| number { $$ = $1; }
	| identifier { $$ = $1; }
	| declaration_expr { $$ = $1; }
	| assignment_expr { $$ = $1; }
	| if_else_expr { $$ = $1; }
	| function_call_expr { $$ = $1; }
	| binary_operator_expr { $$ = $1; }
	| declaration_expr TOKEN_ASSIGN expr {
		std::cout << "here" << std::endl;
		ASTNodeBlock* block = new ASTNodeBlock();
		block->push($1);
		block->push(new ASTNodeAssignment(new ASTNodeIdentifier($1->var_name), $3));
		$$ = block;
	}
	;

identifier
	: TOKEN_IDENTIFIER {
		$$ = new ASTNodeIdentifier(*$1);
		delete $1;
	}
	;

number
	: TOKEN_NUMBER {
		$$ = new ASTNodeLiteral(std::stoi(*$1));
		delete $1;
	}
	;

assignment_expr
	: identifier TOKEN_ASSIGN expr { $$ = new ASTNodeAssignment($1, $3); }
	;

declaration_expr
	: TOKEN_VAR TOKEN_IDENTIFIER {
		$$ = new ASTNodeDeclaration(*$2);
		delete $2;
	}
	;

if_else_expr
	: TOKEN_IF TOKEN_LPAREN expr[cond] TOKEN_RPAREN TOKEN_LBRACE stmts[if_true] TOKEN_RBRACE TOKEN_ELSE TOKEN_LBRACE stmts[if_false] TOKEN_RBRACE {
		$$ = new ASTNodeIfElse($cond, $if_true, $if_false);
	}
	;

binary_operator_expr
	: expr TOKEN_ADD expr { $$ = new ASTNodeBinaryOperator(eADD, $1, $3); }
	| expr TOKEN_SUBTRACT expr { $$ = new ASTNodeBinaryOperator(eSUBTRACT, $1, $3); }
	| expr TOKEN_DIVIDE expr { $$ = new ASTNodeBinaryOperator(eDIVIDE, $1, $3); }
	| expr TOKEN_MULTIPLY expr { $$ = new ASTNodeBinaryOperator(eMULTIPLY, $1, $3); }
	| expr TOKEN_EQ expr { $$ = new ASTNodeBinaryOperator(eEQ, $1, $3); }
	| expr TOKEN_LT expr { $$ = new ASTNodeBinaryOperator(eLT, $1, $3); }
	| expr TOKEN_GT expr { $$ = new ASTNodeBinaryOperator(eGT, $1, $3); }
	;

function_prototype_expr
	: TOKEN_DEF TOKEN_IDENTIFIER TOKEN_LPAREN identifier_list TOKEN_RPAREN {
		$$ = new ASTNodeFunctionPrototype(*$2, $4);
		delete $2;
	}
	;

function_expr
	: function_prototype_expr TOKEN_LBRACE stmts TOKEN_RBRACE {
		$$ = new ASTNodeFunction($1, $3);
	}
	;

function_call_expr
	: TOKEN_IDENTIFIER TOKEN_LPAREN args_list TOKEN_RPAREN {
		$$ = new ASTNodeFunctionCall(*$1, $3);
		delete $1;
	}
	;

identifier_list
	: declaration_expr {
		$$ = new std::vector<ASTNodeDeclaration*>();
		$$->push_back($1);
	}
	| identifier_list TOKEN_COMMA declaration_expr {
		$$->push_back($3);
	}
	| { $$ = new std::vector<ASTNodeDeclaration*>(); }
	;

args_list
	: expr {
		$$ = new std::vector<ASTNode*>();
		$$->push_back($1);
	}
	| args_list TOKEN_COMMA expr {
		$$->push_back($3);
	}
	| { $$ = new std::vector<ASTNode*>(); }
	;
