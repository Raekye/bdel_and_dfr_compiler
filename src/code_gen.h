#ifndef __CODE_GEN_H_
#define __CODE_GEN_H_

#include "ast.h"
#include <map>
#include <deque>
#include <tuple>
#include <iostream>
#include <list>

typedef std::tuple<ASTNodeFunction*, int32_t> CodeGenFunction;

class CodeGen : public IASTNodeVisitor {
public:
	int32_t labels;
	std::map<std::string, int32_t> locals;
	std::map<std::string, CodeGenFunction*> functions;
	std::map<std::string, int32_t> locals_in_registers;
	int32_t current_register;
	std::string main_label;
	int32_t nodes_visited;
	bool will_store;
	int32_t stack_delta;
	std::deque<std::string> loops;

	std::list<ASTNodeFunction*> liberate_me;

	std::string* registers_with_locals;
	int32_t* registers_last_used;

	int32_t num_registers_useable;
	int32_t register_tmp;
	int32_t register_result;
	int32_t register_return_address;
	int32_t register_return_value;

	CodeGen(int32_t, int32_t, int32_t, int32_t, int32_t);
	int32_t next_register();
	void clear();
	void verify();
	ASTNode* parse(std::istream*);
	void gen_program(ASTNode*);
	void stack_pointer(int32_t);
	void commit();

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
	void visit(ASTNodeAssembly*) override;
	void visit(ASTNodeEcho*) override;
	void visit(ASTNodeWhileLoop*) override;
	void visit(ASTNodeBreak*) override;

	static std::map<std::string, int32_t> charcode_from_char;
private:
	static int32_t initialized;
	static int32_t initialize();
};

#endif /* __CODE_GEN_H_ */
