#include "code_gen.h"
#include "parser.h"
#include "lexer.h"
#include <sstream>

CodeGen::CodeGen(int32_t num_registers_useable, int32_t register_tmp, int32_t register_result, int32_t register_return_address, int32_t register_return_value) {
	this->labels = 0;
	this->num_registers_useable = num_registers_useable;
	this->register_tmp = register_tmp;
	this->register_result = register_result;
	this->register_return_address = register_return_address;
	this->register_return_value = register_return_value;
	this->registers_with_locals = new std::string[this->num_registers_useable];
	this->registers_last_used = new int32_t[this->num_registers_useable];
	this->clear();
}

CodeGen::~CodeGen() {
	for (std::map<std::string, CodeGenFunction*>::iterator it = this->functions.begin(); it != this->functions.end(); it++) {
		delete it->second;
	}
	delete[] this->registers_with_locals;
	delete[] this->registers_last_used;
	for (std::list<ASTNodeFunction*>::iterator it = this->liberate_me.begin(); it != this->liberate_me.end(); it++) {
		(*it)->prototype = NULL;
		delete *it;
	}
}

void CodeGen::clear() {
	this->locals.clear();
	this->locals_in_registers.clear();
	for (int32_t i = 0; i < this->num_registers_useable; i++) {
		registers_with_locals[i] = "-";
		registers_last_used[i] = 0;
	}
	this->nodes_visited = 0;
}

void CodeGen::verify() {
	for (std::map<std::string, CodeGenFunction*>::iterator it = this->functions.begin(); it != this->functions.end(); it++) {
		if (std::get<0>(*(it->second))->body == NULL) {
			throw std::runtime_error("Undefined function");
		}
	}
	if (this->main_label == "") {
		throw std::runtime_error("No main function");
	}
}

ASTNode* CodeGen::parse(std::istream* in) {
	std::string code;
	ASTNode* root = NULL;
	std::string line;
	while (std::getline(*in, line)) {
		code += line + "\n";
	}
	yyscan_t scanner;
	YY_BUFFER_STATE buffer;
	yylex_init(&scanner);
	buffer = yy_scan_string(code.c_str(), scanner);
	yyparse(&root, scanner);
	yy_delete_buffer(buffer, scanner);
	yylex_destroy(scanner);
	return root;
}

void CodeGen::gen_program(ASTNode* root) {
	std::cout << "jump :_after_" << std::endl;
	root->accept(this);
	this->verify();
	std::cout << "_after_:" << std::endl;
	std::cout << "literal :_end_ r" << this->register_return_address << std::endl;
	std::cout << "jump :" << this->main_label << std::endl;
	std::cout << "_end_:" << std::endl;
	std::cout << "eof" << std::endl;
}

int32_t CodeGen::next_register() {
	if (this->locals_in_registers.size() < this->num_registers_useable) {
		for (int32_t i = 0; i < this->num_registers_useable; i++) {
			if (registers_with_locals[i] == "-") {
				return i;
			}
		}
		throw std::runtime_error("Badness");
	}
	int32_t min_index = 0;
	for (int32_t i = 1; i < this->num_registers_useable; i++) {
		if (registers_last_used[i] < registers_last_used[min_index]) {
			min_index = i;
		}
	}
	// save
	std::cout << "store r" << min_index << " " << (this->locals.size() - this->locals.at(registers_with_locals[min_index])) << std::endl;
	registers_with_locals[min_index] = "-";
	return min_index;
}

void CodeGen::visit(ASTNodeIdentifier* node) {
	std::map<std::string, int32_t>::iterator declared = this->locals.find(node->name);
	if (declared == this->locals.end()) {
		throw std::runtime_error("Undeclared identifier");
	}
	std::map<std::string, int32_t>::iterator registered = this->locals_in_registers.find(node->name);
	if (registered != this->locals_in_registers.end()) {
		this->current_register = registered->second;
		return;
	}
	int32_t index = this->next_register();
	std::cout << "load " << (this->locals.size() - declared->second) << " r" << index << std::endl;
	this->locals_in_registers[node->name] = index;
	this->registers_with_locals[index] = node->name;
	this->registers_last_used[index] = this->nodes_visited;
	this->current_register = index;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeLiteral* node) {
	int32_t index = this->next_register();
	std::cout << "literal " << node->val << " r" << index << std::endl;
	this->current_register = index;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeDeclaration* node) {
	std::map<std::string, int32_t>::iterator declared = this->locals.find(node->var_name);
	if (declared != this->locals.end()) {
		throw std::runtime_error("Identifier already declared");
	}
	int32_t num_locals = this->locals.size();
	this->locals[node->var_name] = num_locals;
	std::cout << "stack 1" << std::endl;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeBlock* node) {
	for (std::vector<ASTNode*>::iterator it = node->statements.begin(); it != node->statements.end(); it++) {
		(*it)->accept(this);
	}
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeAssignment* node) {
	std::map<std::string, int32_t>::iterator declared = this->locals.find(node->lhs->name);
	if (declared == this->locals.end()) {
		throw std::runtime_error("Undeclared identifier");
	}
	node->lhs->accept(this);
	int32_t reg_a = this->current_register;
	node->rhs->accept(this);
	std::cout << "mov r" << this->current_register << " r" << reg_a << std::endl;
	this->current_register = reg_a;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeBinaryOperator* node) {
	node->lhs->accept(this);
	std::cout << "mov r" << this->current_register << " r" << this->register_tmp << std::endl;
	int32_t reg_a = this->register_tmp;
	node->rhs->accept(this);
	int32_t reg_b = this->current_register;
	int32_t reg_c = this->register_result;
	switch (node->op) {
		case eADD:
			std::cout << "add r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			break;
		case eSUBTRACT:
			std::cout << "sub r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			break;
		case eMULTIPLY:
			std::cout << "mul r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			break;
		case eDIVIDE:
			std::cout << "div r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			break;
		case eEQ:
			std::cout << "cmp r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			break;
		case eLT:
			std::cout << "cmp r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			std::cout << "inc r" << reg_c << std::endl;
			break;
		case eGT:
			std::cout << "cmp r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			std::cout << "dec r" << reg_c << std::endl;
			break;
	}
	this->current_register = reg_c;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeFunctionPrototype* node) {
	std::map<std::string, CodeGenFunction*>::iterator it = this->functions.find(node->function_name);
	if (it != this->functions.end()) {
		throw std::runtime_error("Function already declared");
	}
	ASTNodeFunction* fn = new ASTNodeFunction(node, NULL);
	this->liberate_me.push_back(fn);
	this->functions[node->function_name] = new CodeGenFunction(fn, this->labels++);
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeFunction* node) {
	std::map<std::string, CodeGenFunction*>::iterator it = this->functions.find(node->prototype->function_name);
	if (it != this->functions.end()) {
		if (std::get<0>(*(it->second))->body != NULL) {
			throw std::runtime_error("Function already declared");
		}
	} else {
		node->prototype->accept(this);
		it = this->functions.find(node->prototype->function_name);
	}
	int32_t label = std::get<1>(*(it->second));
	delete it->second;
	it->second = new CodeGenFunction(node, label);
	std::cout << "_" << label << "__" << node->prototype->function_name << ":" <<  std::endl;

	int32_t i = 0;
	for (std::vector<ASTNodeDeclaration*>::iterator it = node->prototype->args->begin(); it != node->prototype->args->end(); it++) {
		this->locals[(*it)->var_name] = -i - 1;
		i++;
	}
	node->body->accept(this);
	std::cout << "stack -" << this->locals.size() << std::endl;
	std::cout << "mov r" << this->current_register << " r" << this->register_return_value << std::endl;
	std::cout << "jump r" << this->register_return_address << std::endl;

	if (node->prototype->function_name == "main") {
		std::stringstream ss;
		ss << "_" << label << "__" << "main";
		this->main_label = ss.str();
	}

	this->clear();
}

void CodeGen::visit(ASTNodeFunctionCall* node) {
	std::map<std::string, CodeGenFunction*>::iterator it = this->functions.find(node->function_name);
	if (it == this->functions.end()) {
		throw std::runtime_error("Undeclared function called");
	}
	ASTNodeFunctionPrototype* prototype = std::get<0>(*(it->second))->prototype;
	if (node->args->size() != prototype->args->size()) {
		throw std::runtime_error("Function called with invalid number of arguments");
	}
	std::cout << "stack " << prototype->args->size() << std::endl;
	int32_t i = 0;
	for (std::vector<ASTNode*>::iterator it = node->args->begin(); it != node->args->end(); it++) {
		(*it)->accept(this);
		std::cout << "store r" << this->current_register << " " << i + 1 << std::endl;
		i++;
	}
	std::cout << "literal :" << "_" << this->labels << "_call_" << prototype->function_name << " r" << this->register_return_address << std::endl;
	std::cout << "supermandive" << std::endl;
	std::cout << "jump :" << "_" << std::get<1>(*(it->second)) << "__" << prototype->function_name << std::endl;
	std::cout << "_" << this->labels << "_call_" << prototype->function_name << ":" << std::endl;
	// function return already does this
	//std::cout << "stack -" << prototype->args->size() << std::endl;
	std::cout << "getup" << std::endl;
	this->labels++;
	this->current_register = this->register_return_value;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeIfElse* node) {
	int32_t reg_c = this->register_result;
	node->cond->accept(this);
	std::cout << "branch r" << this->current_register << std::endl;
	int32_t l1 = this->labels++;
	int32_t l2 = this->labels++;
	int32_t l3 = this->labels++;
	std::cout << "jump :_" << l1 << "_branch_" << std::endl;
	std::cout << "jump :_" << l2 << "_branch_" << std::endl;
	std::cout << "_" << l1 << "_branch_:" << std::endl;
	node->if_true->accept(this);
	std::cout << "mov r" << this->current_register << " r" << reg_c << std::endl;
	std::cout << "jump :_" << l3 << "_merge_" << std::endl;
	std::cout << "_" << l2 << "_branch_:" << std::endl;
	node->if_false->accept(this);
	std::cout << "mov r" << this->current_register << " r" << reg_c << std::endl;
	std::cout << "jump :_" << l3 << "_merge_" << std::endl;
	std::cout << "_" << l3 << "_merge_:" << std::endl;
	this->current_register = reg_c;
	this->nodes_visited++;
}
