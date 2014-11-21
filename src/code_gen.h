#ifndef __CODE_GEN_H_
#define __CODE_GEN_H_

#include "ast.h"
#include <map>
#include <deque>
#include <tuple>
#include <iostream>

#define NUM_REGISTERS_USEABLE 13

typedef std::tuple<ASTNodeFunction*, int32_t> CodeGenFunction;

class CodeGen : public IASTNodeVisitor {
public:
	int32_t labels;
	std::map<std::string, int32_t> locals;
	std::map<std::string, CodeGenFunction*> functions;
	std::map<std::string, int32_t> locals_in_registers;
	std::string registers_with_locals[NUM_REGISTERS_USEABLE];
	int32_t registers_last_used[NUM_REGISTERS_USEABLE];
	int32_t current_register;
	std::string main_label;
	int32_t nodes_visited;

	CodeGen();
	int32_t next_register();
	void clear();
	void verify();
	ASTNode* parse(std::istream*);
	void gen_program(ASTNode*);

	~CodeGen();

	void visit(ASTNodeIdentifier*) override;
	void visit(ASTNodeLiteral*) override;
	void visit(ASTNodeDeclaration*) override;
	void visit(ASTNodeBlock*) override;
	void visit(ASTNodeAssignment*) override;
	void visit(ASTNodeBinaryOperator*) override;
	void visit(ASTNodeFunctionPrototype*) override;
	void visit(ASTNodeFunction*) override;
	void visit(ASTNodeFunctionCall*) override;
	void visit(ASTNodeIfElse*) override;

	static const int32_t REGISTER_RESULT = 13;
	static const int32_t REGISTER_RETURN_ADDRESS = 14;
	static const int32_t REGISTER_RETURN_VALUE = 15;
};

#endif /* __CODE_GEN_H_ */
