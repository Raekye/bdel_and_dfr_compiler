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
	std::map<std::string, int32_t> globals;
	std::deque<std::map<std::string, int32_t>> locals;
	std::map<std::string, CodeGenFunction*> functions;
	std::map<std::string, int32_t> variables_in_registers;
	int32_t current_register;
	std::string main_label;
	int32_t nodes_visited;
	bool will_read;
	int32_t stack_delta;
	std::deque<std::string> loops;
	int32_t stack_random_offset;
	std::list<ASTNode*> global_initializers;
	bool in_global_init;
	int32_t expected_stack_delta;

	std::list<ASTNodeFunction*> liberate_me;

	std::string* registers_with_variables;
	int32_t* registers_last_used;
	bool* registers_dirty;

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
	void stack_pointer(int32_t, std::string);
	void commit(bool = true);
	void push_scope();
	void pop_scope(std::string, int32_t = 0);
	int32_t* find_local(std::string);
	int32_t* find_global(std::string);
	void put_local(std::string, int32_t);
	int32_t locals_size();
	bool in_top_level();
	void put_variable_in_register(std::string, int32_t);
	void erase_variable_in_register(int32_t);

	~CodeGen();

	void visit(ASTNodeIdentifier*) override;
	void visit(ASTNodeLiteral*) override;
	void visit(ASTNodeDeclaration*) override;
	void visit(ASTNodeBlock*) override;
	void visit(ASTNodeAssignment*) override;
	void visit(ASTNodeBinaryOperator*) override;
	void visit(ASTNodeUnaryOperator*) override;
	void visit(ASTNodeFunctionPrototype*) override;
	void visit(ASTNodeFunction*) override;
	void visit(ASTNodeFunctionCall*) override;
	void visit(ASTNodeIfElse*) override;
	void visit(ASTNodeAssembly*) override;
	void visit(ASTNodeEcho*) override;
	void visit(ASTNodeWhileLoop*) override;
	void visit(ASTNodeBreak*) override;
	void visit(ASTNodePhony*) override;
	void visit(ASTNodeFunctionReference*) override;
	void visit(ASTNodeString*) override;

	static std::map<std::string, int32_t> charcode_from_char;
private:
	static int32_t initialized;
	static int32_t initialize();
};

#endif /* __CODE_GEN_H_ */
