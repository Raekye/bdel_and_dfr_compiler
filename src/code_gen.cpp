#include "code_gen.h"
#include "parser.h"
#include "lexer.h"
#include <sstream>

std::map<std::string, int32_t> CodeGen::charcode_from_char;
int32_t CodeGen::initialized = CodeGen::initialize();

int32_t CodeGen::initialize() {
	for (int32_t i = 0; i < 10; i++) {
		CodeGen::charcode_from_char[std::to_string(i)] = i;
	}
	for (int32_t i = 'a'; i <= 'z'; i++) {
		CodeGen::charcode_from_char[std::string(1, (char) i)] = i - 'a' + 10;
	}
	CodeGen::charcode_from_char[" "] = 10 + 26;
	return 0;
}

static void code_gen_mov_registers(int32_t, int32_t, std::string);

void code_gen_mov_registers(int32_t a, int32_t b, std::string comment) {
	if (a != b) {
		std::cout << "mov r" << a << " r" << b << " # " << comment << std::endl;
	}
}

CodeGen::CodeGen(int32_t num_registers_useable, int32_t register_tmp, int32_t register_result, int32_t register_return_address, int32_t register_return_value) : labels(0), will_read(true), stack_delta(0), main_label("-"), stack_random_offset(0), in_global_init(false), expected_stack_delta(0) {
	this->num_registers_useable = num_registers_useable;
	this->register_tmp = register_tmp;
	this->register_result = register_result;
	this->register_return_address = register_return_address;
	this->register_return_value = register_return_value;
	this->registers_with_variables = new std::string[this->num_registers_useable];
	this->registers_last_used = new int32_t[this->num_registers_useable];
	this->registers_dirty = new bool[this->num_registers_useable];
	this->clear();
}

CodeGen::~CodeGen() {
	for (std::map<std::string, CodeGenFunction*>::iterator it = this->functions.begin(); it != this->functions.end(); it++) {
		delete it->second;
	}
	delete[] this->registers_with_variables;
	delete[] this->registers_last_used;
	delete[] this->registers_dirty;
	for (std::list<ASTNodeFunction*>::iterator it = this->liberate_me.begin(); it != this->liberate_me.end(); it++) {
		(*it)->prototype = NULL;
		delete *it;
	}
}

void CodeGen::clear() {
	this->locals.clear();
	this->variables_in_registers.clear();
	for (int32_t i = 0; i < this->num_registers_useable; i++) {
		this->registers_with_variables[i] = "-";
		this->registers_last_used[i] = 0;
		this->registers_dirty[i] = false;
	}
	this->nodes_visited = 0;
}

void CodeGen::verify() {
	for (std::map<std::string, CodeGenFunction*>::iterator it = this->functions.begin(); it != this->functions.end(); it++) {
		if (std::get<0>(*(it->second))->body == NULL) {
			throw std::runtime_error("Undefined function");
		}
	}
	if (this->stack_delta != expected_stack_delta) {
		throw std::runtime_error("Generated code stack delta not expected");
	}
	if (!this->in_top_level()) {
		throw std::runtime_error("Verifying, not in top level");
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
	std::cout << std::endl;
	root->accept(this);
	this->verify();
	std::cout << "_after_:" << std::endl;
	this->in_global_init = true;
	for (std::list<ASTNode*>::iterator it = this->global_initializers.begin(); it != this->global_initializers.end(); it++) {
		(*it)->accept(this);
	}
	this->commit();
	std::cout << "literal " << this->globals.size() + 1 << " r" << this->register_tmp << std::endl;
	std::cout << "literal 0 r" << this->register_result << std::endl;
	std::cout << "heap r" << this->register_tmp << " r" << this->register_result << std::endl;
	if (this->main_label != "-") {
		std::cout << "literal :_end_ r" << this->register_return_address << std::endl;
		std::cout << "jump :" << this->main_label << std::endl;
		std::cout << "_end_:" << std::endl;
	}
	std::cout << "eof" << std::endl;
}

void CodeGen::stack_pointer(int32_t n, std::string comment) {
	if (n != 0) {
		std::cout << "stack " << n << " # " << comment << std::endl;
		this->stack_delta += n;
	}
}

void CodeGen::commit(bool include_locals) {
	for (int32_t i = 0; i < this->num_registers_useable; i++) {
		if (this->registers_with_variables[i] != "-") {
			if (this->registers_dirty[i]) {
				int32_t* relative_pos = this->find_local(registers_with_variables[i]);
				if (relative_pos == NULL) {
					std::cout << "literal " << this->globals.at(this->registers_with_variables[i]) << " r" << this->register_result <<std::endl;
					std::cout << "heap r" << i << " r" << this->register_result << " # " << "commit global " << this->registers_with_variables[i] << std::endl;
				} else {
					if (include_locals) {
						std::cout << "store r" << i << " " << (this->locals_size() - *relative_pos) << " # " << "commit local " << this->registers_with_variables[i] << std::endl;
					}
					delete relative_pos;
				}
			}
			this->erase_variable_in_register(i);
		}
	}
}

void CodeGen::push_scope() {
	this->locals.push_back(std::map<std::string, int32_t>());
}

void CodeGen::pop_scope(std::string comment, int32_t delta) {
	for (std::map<std::string, int32_t>::iterator it = this->locals.back().begin(); it != this->locals.back().end(); it++) {
		std::map<std::string, int32_t>::iterator found = this->variables_in_registers.find(it->first);
		if (found != this->variables_in_registers.end()) {
			this->registers_with_variables[found->second] = "-";
			this->variables_in_registers.erase(it->first);
		}
	}
	this->stack_pointer(-(this->locals.back().size() - delta), comment);
	this->locals.pop_back();
}

int32_t* CodeGen::find_local(std::string key) {
	for (std::deque<std::map<std::string, int32_t>>::reverse_iterator it = this->locals.rbegin(); it != this->locals.rend(); it++) {
		std::map<std::string, int32_t>::iterator found = it->find(key);
		if (found != it->end()) {
			int32_t* x = new int32_t;
			*x = found->second;
			return x;
		}
	}
	return NULL;
}

int32_t* CodeGen::find_global(std::string key) {
	std::map<std::string, int32_t>::iterator found = this->globals.find(key);
	if (found == this->globals.end()) {
		return NULL;
	}
	int32_t* x = new int32_t;
	*x = found->second;
	return x;
}

void CodeGen::put_local(std::string key, int32_t x) {
	this->locals.back()[key] = x;
}

int32_t CodeGen::locals_size() {
	int32_t size = this->stack_random_offset;
	for (std::deque<std::map<std::string, int32_t>>::reverse_iterator it = this->locals.rbegin(); it != this->locals.rend(); it++) {
		size += it->size();
	}
	return size;
}

int32_t CodeGen::next_register() {
	if (this->variables_in_registers.size() < this->num_registers_useable) {
		for (int32_t i = 0; i < this->num_registers_useable; i++) {
			if (this->registers_with_variables[i] == "-") {
				return i;
			}
		}
		throw std::runtime_error("Badness");
	}
	int32_t min_index = 0;
	for (int32_t i = 1; i < this->num_registers_useable; i++) {
		if (this->registers_last_used[i] < this->registers_last_used[min_index]) {
			min_index = i;
		}
	}
	if (this->registers_dirty[min_index]) {
		int32_t* relative_pos = this->find_local(this->registers_with_variables[min_index]);
		if (relative_pos == NULL) {
			std::cout << "literal " << this->globals.at(this->registers_with_variables[min_index]) << " r" << this->register_result << std::endl;
			std::cout << "heap r" << min_index << " r" << this->register_result << " # " << "ding global " << this->registers_with_variables[min_index] << std::endl;
		} else {
			std::cout << "store r" << min_index << " " << (this->locals_size() - *relative_pos) << " # " << "ding local " << this->registers_with_variables[min_index] << std::endl;
			delete relative_pos;
		}
	}
	this->erase_variable_in_register(min_index);
	return min_index;
}

bool CodeGen::in_top_level() {
	return this->locals.size() == 0;
}

void CodeGen::put_variable_in_register(std::string key, int32_t reg) {
	this->registers_with_variables[reg] = key;
	this->variables_in_registers[key] = reg;
}

void CodeGen::erase_variable_in_register(int32_t reg) {
	this->variables_in_registers.erase(this->registers_with_variables[reg]);
	this->registers_with_variables[reg] = "-";
	this->registers_dirty[reg] = false;
}

#pragma mark - Assembly code generation

void CodeGen::visit(ASTNodeIdentifier* node) {
	std::map<std::string, int32_t>::iterator registered = this->variables_in_registers.find(node->name);
	if (registered != this->variables_in_registers.end()) {
		this->current_register = registered->second;
		return;
	}
	int32_t* relative_pos = this->find_local(node->name);
	int32_t index = this->next_register();
	if (relative_pos == NULL) {
		std::map<std::string, int32_t>::iterator it = this->globals.find(node->name);
		if (it == this->globals.end()) {
			throw std::runtime_error("Undeclared identifier");
		}
		if (this->will_read) {
			std::cout << "literal " << it->second << " r" << this->register_result << std::endl;
			std::cout << "unheap r" << this->register_result << " r" << index << " # " << "unheap global " << node->name << std::endl;
		}
	} else {
		if (this->will_read) {
			std::cout << "load " << (this->locals_size() - *relative_pos) << " r" << index << " # " << "load " << node->name << ", " << this->locals_size() << ", " << *relative_pos << ", " << this->stack_random_offset << std::endl;
		}
		delete relative_pos;
	}
	this->put_variable_in_register(node->name, index);
	this->registers_last_used[index] = this->nodes_visited;
	this->current_register = index;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeLiteral* node) {
	if (this->in_top_level() && !this->in_global_init) {
		this->global_initializers.push_back(node);
		return;
	}
	int32_t index = this->register_result;
	std::cout << "literal " << node->val << " r" << index << std::endl;
	this->current_register = index;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeDeclaration* node) {
	if (this->in_top_level()) {
		std::map<std::string, int32_t>::iterator it = this->globals.find(node->var_name);
		if (it != this->globals.end()) {
			throw std::runtime_error("Global identifier already declared");
		}
		this->globals[node->var_name] = this->globals.size();
	} else {
		std::map<std::string, int32_t>::iterator found_in_top_scope = this->locals.back().find(node->var_name);
		if (found_in_top_scope != this->locals.back().end()) {
			throw std::runtime_error("Identifier already declared");
		}

		std::map<std::string, int32_t>::iterator it = this->variables_in_registers.find(node->var_name);
		if (it != this->variables_in_registers.end()) {
			if (this->registers_dirty[it->second]) {
				int32_t* relative_pos = this->find_local(node->var_name);
				if (relative_pos == NULL) {
					std::cout << "literal " << this->globals.at(this->registers_with_variables[it->second]) << " r" << this->register_result << std::endl;
					std::cout << "heap r" << it->second << " r" << this->register_result << " # " << "shadow global " << node->var_name << std::endl;
				} else {
					std::cout << "store r" << it->second << " " << (this->locals_size() - *relative_pos) << " # " << "shadow local " << node->var_name << std::endl;
					delete relative_pos;
				}
			}
			this->erase_variable_in_register(it->second);
		}

		this->put_local(node->var_name, this->locals_size());
		this->stack_pointer(1, "var " + node->var_name);
	}
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeBlock* node) {
	for (std::vector<ASTNode*>::iterator it = node->statements.begin(); it != node->statements.end(); it++) {
		(*it)->accept(this);
	}
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeAssignment* node) {
	if (this->in_top_level() && !this->in_global_init) {
		this->global_initializers.push_back(node);
		return;
	}
	int32_t* relative_pos = this->find_local(node->lhs->name);
	if (relative_pos == NULL) {
		std::map<std::string, int32_t>::iterator it = this->globals.find(node->lhs->name);
		if (it == this->globals.end()) {
			throw std::runtime_error("Undeclared identifier");
		}
	}
	delete relative_pos;
	node->rhs->accept(this);
	int32_t reg_b = this->current_register;
	if (false && this->in_top_level()) {
		std::cout << "literal " << this->globals.at(node->lhs->name) << " r" << this->register_tmp << std::endl;
		std::cout << "heap r" << reg_b << " r" << this->register_tmp << std::endl;
	} else {
		this->will_read = false;
		node->lhs->accept(this);
		this->will_read = true;
		this->registers_dirty[this->current_register] = true;
		code_gen_mov_registers(reg_b, this->current_register, "assign to " + node->lhs->name);
	}
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeBinaryOperator* node) {
	if (this->in_top_level() && !this->in_global_init) {
		this->global_initializers.push_back(node);
		return;
	}/*
	if (node->op == eLOGICAL_AND) {
		ASTNodeIfElse f(new ASTNodePhony(node->lhs), new ASTNodePhony(node->rhs), new ASTNodeLiteral(0));
		f.accept(this);
		return;
	} else if (node->op == eLOGICAL_OR) {
		ASTNodeIfElse f(new ASTNodePhony(node->lhs), new ASTNodeLiteral(1), new ASTNodePhony(node->rhs));
		f.accept(this);
		return;
	}*/
	int32_t reg_a = this->register_tmp;
	node->lhs->accept(this);
	this->stack_pointer(1, "binary operation register tmp save");
	this->stack_random_offset += 1;
	std::cout << "store r" << this->current_register << " 1" << std::endl;
	node->rhs->accept(this);
	int32_t reg_b = this->current_register;
	std::cout << "load 1 r" << reg_a << std::endl;
	this->stack_random_offset -= 1;
	this->stack_pointer(-1, "binary operation register tmp restore");
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
			std::cout << "eq r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			break;
		case eLT:
			std::cout << "lt r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			break;
		case eGT:
			std::cout << "gt r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			break;
		case eLOGICAL_AND:
			std::cout << "and r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			break;
		case eLOGICAL_OR:
			std::cout << "or r" << reg_a << " r" << reg_b << " r" << reg_c << std::endl;
			break;
		case eLOGICAL_NOT:
			throw std::runtime_error("Not binary operator");
	}
	this->current_register = reg_c;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeUnaryOperator* node) {
	if (this->in_top_level() && !this->in_global_init) {
		this->global_initializers.push_back(node);
		return;
	}
	int32_t reg_a = this->register_result;
	if (node->op == eLOGICAL_NOT) {
		node->x->accept(this);
		std::cout << "not r" << this->current_register << " r" << reg_a << std::endl;
	} else {
		throw std::runtime_error("Unknown unary operator");
	}
	this->current_register = reg_a;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeFunctionPrototype* node) {
	if (!this->in_top_level()) {
		throw std::runtime_error("Function declared in not top level");
	}
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
	if (!this->in_top_level()) {
		throw std::runtime_error("Function defined in not top level");
	}
	int32_t label = std::get<1>(*(it->second));
	delete it->second;
	it->second = new CodeGenFunction(node, label);
	std::cout << "_" << label << "__" << node->prototype->function_name << ":" <<  std::endl;

	this->push_scope();

	int32_t i = 0;
	for (std::vector<ASTNodeDeclaration*>::iterator it = node->prototype->args->begin(); it != node->prototype->args->end(); it++) {
		this->put_local((*it)->var_name, node->prototype->args->size() - i - 1);
		i++;
	}
	node->body->accept(this);
	code_gen_mov_registers(this->current_register, this->register_return_value, "return");
	this->pop_scope("def " + node->prototype->function_name + "/pop", node->prototype->args->size());
	this->commit();
	std::cout << "jump r" << this->register_return_address << std::endl;


	if (node->prototype->function_name == "main") {
		std::stringstream ss;
		ss << "_" << label << "__" << "main";
		this->main_label = ss.str();
	}

	this->clear();
	std::cout << std::endl;
}

void CodeGen::visit(ASTNodeFunctionCall* node) {
	if (this->in_top_level() && !this->in_global_init) {
		this->global_initializers.push_back(node);
		return;
	}
	std::map<std::string, CodeGenFunction*>::iterator it = this->functions.find(node->function_name);
	if (it == this->functions.end()) {
		throw std::runtime_error("Undeclared function \"" + node->function_name + "\" called");
	}
	ASTNodeFunctionPrototype* prototype = std::get<0>(*(it->second))->prototype;
	if (node->args->size() != prototype->args->size()) {
		throw std::runtime_error("Function called with invalid number of arguments");
	}
	int32_t stack_required = prototype->args->size() + 1;
	this->stack_pointer(stack_required, "call " + node->function_name);
	this->stack_random_offset += stack_required;
	std::cout << "store r" << this->register_return_address << " " << stack_required << std::endl;
	int32_t i = 0;
	for (std::vector<ASTNode*>::iterator it = node->args->begin(); it != node->args->end(); it++) {
		(*it)->accept(this);
		std::cout << "store r" << this->current_register << " " << i + 1 << " # " << "arg " << i << std::endl;
		i++;
	}
	this->commit();
	std::cout << "literal :" << "_" << this->labels << "_call_" << prototype->function_name << " r" << this->register_return_address << std::endl;
	std::cout << "jump :" << "_" << std::get<1>(*(it->second)) << "__" << prototype->function_name << std::endl;
	std::cout << "_" << this->labels << "_call_" << prototype->function_name << ":" << std::endl;
	std::cout << "load " << stack_required << " r" << this->register_return_address << std::endl;
	this->stack_random_offset -= stack_required;
	this->stack_pointer(-stack_required, "call " + node->function_name + "/unstack");
	this->labels++;
	this->current_register = this->register_return_value;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeIfElse* node) {
	if (this->in_top_level() && !this->in_global_init) {
		this->global_initializers.push_back(node);
		return;
	}
	this->commit();
	this->push_scope();
	int32_t reg_c = this->register_result;
	node->cond->accept(this);
	this->commit();
	int32_t l1 = this->labels++;
	int32_t l2 = this->labels++;
	int32_t l3 = this->labels++;
	int32_t l4 = this->labels++;
	int32_t l5 = this->labels++;
	std::string if_true = "_" + std::to_string(l1) + "_branch_";
	std::string if_true_cleanup = "_" + std::to_string(l2) + "_branch_";
	std::string if_false_before = "_" + std::to_string(l3) + "_branch_";
	std::string if_false = "_" + std::to_string(l4) + "_branch_";
	std::string if_merge = "_" + std::to_string(l5) + "_branch_";
	std::cout << "branch r" << this->current_register << " # " << "if " + std::to_string(l1) + "/cond" << std::endl;
	std::cout << "jump :" << if_true << std::endl;
	std::cout << "jump :" << if_false_before << std::endl;
	std::cout << if_true << ":" << " # " << "if " + std::to_string(l1) + "/true" << std::endl;
	this->push_scope();
	node->if_true->accept(this);
	code_gen_mov_registers(this->current_register, reg_c, "if/true eval");
	this->pop_scope("if true/pop_inner");
	this->commit();
	std::cout << "literal :" << if_merge << " r" << this->register_tmp << std::endl;
	std::cout << "jump :" << if_true_cleanup << std::endl;
	std::cout << if_false_before << ": " << " # " << "if " + std::to_string(l1) << "/false_before" << std::endl;
	std::cout << "literal :" << if_false << " r" << this->register_tmp << std::endl;
	std::cout << "jump :" << if_true_cleanup << std::endl;
	std::cout << if_true_cleanup << ": " << " # " << "if " + std::to_string(l1) << "/true_cleanup" << std::endl;
	this->pop_scope("if true/pop");
	//this->commit();
	std::cout << "jump r" << this->register_tmp << std::endl;
	std::cout << if_false << ":" << " # " << "if " + std::to_string(l1) + "/false" << std::endl;
	if (node->if_false != NULL) {
		this->push_scope();
		node->if_false->accept(this);
		code_gen_mov_registers(this->current_register, reg_c, "if/false eval");
		this->pop_scope("if false/pop");
		this->commit();
	}
	std::cout << "jump :" << if_merge << std::endl;
	std::cout << if_merge << ":" << " # " << "if " + std::to_string(l1) + "/merge" <<  std::endl;
	this->current_register = reg_c;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeAssembly* node) {
	if (this->in_top_level() && !this->in_global_init) {
		this->global_initializers.push_back(node);
		return;
	}
	this->commit();
	for (std::vector<std::string>::iterator it = node->lines->begin(); it != node->lines->end(); it++) {
		std::cout << it->substr(1) << std::endl;
	}
	this->current_register = this->register_result;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeEcho* node) {
	if (this->in_top_level() && !this->in_global_init) {
		this->global_initializers.push_back(node);
		return;
	}
	for (int32_t i = 1; i < node->str.length() - 1; i++) {
		std::cout << "literal " << CodeGen::charcode_from_char.at(node->str.substr(i, 1)) << " r" << this->register_result << std::endl;
		ASTNodeLiteral x(CodeGen::charcode_from_char.at(node->str.substr(i, 1)));
		ASTNodeFunctionCall fn_call("io_putchar", new std::vector<ASTNode*>(1, new ASTNodePhony(&x)));
		fn_call.accept(this);
	}
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeWhileLoop* node) {
	if (this->in_top_level() && !this->in_global_init) {
		this->global_initializers.push_back(node);
		return;
	}
	int32_t l1 = this->labels++;
	int32_t l2 = this->labels++;
	int32_t l3 = this->labels++;
	std::string while_begin = "_" + std::to_string(l1) + "_while_";
	std::string while_loop = "_" + std::to_string(l2) + "_while_";
	std::string while_exit = "_" + std::to_string(l3) + "_while_";
	this->commit();
	this->push_scope();
	std::cout << while_begin << ": " << " # " << "while/begin" << std::endl;
	node->cond->accept(this);
	std::cout << "branch r" << this->current_register << std::endl;
	std::cout << "jump :" << while_loop << std::endl;
	std::cout << "jump :" << while_exit << std::endl;
	std::cout << while_loop << ":" << " # " << "while/body" << std::endl;
	node->body->accept(this);
	code_gen_mov_registers(this->current_register, this->register_result, "while eval");
	this->pop_scope("while " + std::to_string(l1) + "/pop");
	this->commit();
	std::cout << "jump :" << while_begin << std::endl;
	std::cout << while_exit << ":" << " # " << "while/exit" << std::endl;
	this->current_register = this->register_result;
	this->nodes_visited++;
}

void CodeGen::visit(ASTNodeBreak* node) {
	throw std::runtime_error("Sorry, need more passes");
	if (this->in_top_level() && !this->in_global_init) {
		this->global_initializers.push_back(node);
		return;
	}
	std::string target = "-";
	std::deque<std::string>::reverse_iterator it = this->loops.rbegin();
	for (int32_t i = 0; i < node->times; i++) {
		if (it == this->loops.rend()) {
			throw std::runtime_error("Out of loops");
		}
		target = this->loops.back();
		it++;
	}
	std::cout << "jump :" << target << std::endl;
}

void CodeGen::visit(ASTNodePhony* node) {
	node->x->accept(this);
}
