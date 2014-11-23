#include "ast.h"

ASTNodeIdentifier::ASTNodeIdentifier(std::string name) {
	this->name = name;
}

ASTNodeLiteral::ASTNodeLiteral(int16_t x) {
	this->val = x;
}

ASTNodeDeclaration::ASTNodeDeclaration(std::string name) {
	this->var_name = name;
}

ASTNodeBlock::ASTNodeBlock() {
	return;
}

ASTNodeAssignment::ASTNodeAssignment(ASTNodeIdentifier* lhs, ASTNode* rhs) {
	this->lhs = lhs;
	this->rhs = rhs;
}

ASTNodeBinaryOperator::ASTNodeBinaryOperator(EBinaryOperationType op, ASTNode* lhs, ASTNode* rhs) {
	this->op = op;
	this->lhs = lhs;
	this->rhs = rhs;
}

ASTNodeFunction::ASTNodeFunction(ASTNodeFunctionPrototype* prototype, ASTNodeBlock* body) {
	this->prototype = prototype;
	this->body = body;
}

ASTNodeFunctionPrototype::ASTNodeFunctionPrototype(std::string name, std::vector<ASTNodeDeclaration*>* args) {
	this->function_name = name;
	this->args = args;
}

ASTNodeFunctionCall::ASTNodeFunctionCall(std::string name, std::vector<ASTNode*>* args) {
	this->function_name = name;
	this->args = args;
}

ASTNodeIfElse::ASTNodeIfElse(ASTNode* cond, ASTNode* if_true, ASTNode* if_false) {
	this->cond = cond;
	this->if_true = if_true;
	this->if_false = if_false;
}

ASTNodeAssembly::ASTNodeAssembly(std::vector<std::string>* lines) {
	this->lines = lines;
}

ASTNodeEcho::ASTNodeEcho(std::string str) {
	this->str = str;
}

ASTNodeWhileLoop::ASTNodeWhileLoop(ASTNode* cond, ASTNodeBlock* body) {
	this->cond = cond;
	this->body = body;
}

ASTNodeBreak::ASTNodeBreak(int32_t times) {
	this->times = times;
}

ASTNode::~ASTNode() {
	return;
}

ASTNodeIdentifier::~ASTNodeIdentifier() {
	return;
}

ASTNodeLiteral::~ASTNodeLiteral() {
	return;
}

ASTNodeDeclaration::~ASTNodeDeclaration() {
	return;
}

ASTNodeBlock::~ASTNodeBlock() {
	for (std::vector<ASTNode*>::iterator it = this->statements.begin(); it != this->statements.end(); it++) {
		delete *it;
	}
}

ASTNodeAssignment::~ASTNodeAssignment() {
	delete this->lhs;
	delete this->rhs;
}

ASTNodeBinaryOperator::~ASTNodeBinaryOperator() {
	delete this->lhs;
	delete this->rhs;
}

ASTNodeFunctionPrototype::~ASTNodeFunctionPrototype() {
	for (std::vector<ASTNodeDeclaration*>::iterator it = this->args->begin(); it != this->args->end(); it++) {
		delete *it;
	}
	delete this->args;
}

ASTNodeFunction::~ASTNodeFunction() {
	delete this->prototype;
	delete this->body;
}

ASTNodeFunctionCall::~ASTNodeFunctionCall() {
	for (std::vector<ASTNode*>::iterator it = this->args->begin(); it != this->args->end(); it++) {
		delete *it;
	}
	delete this->args;
}

ASTNodeIfElse::~ASTNodeIfElse() {
	delete this->cond;
	delete this->if_true;
	delete this->if_false;
}

ASTNodeAssembly::~ASTNodeAssembly() {
	delete this->lines;
}

ASTNodeEcho::~ASTNodeEcho() {
	return;
}

ASTNodeWhileLoop::~ASTNodeWhileLoop() {
	delete this->cond;
	delete this->body;
}

ASTNodeBreak::~ASTNodeBreak() {
	return;
}

void ASTNodeIdentifier::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeLiteral::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeDeclaration::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeBlock::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeAssignment::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeBinaryOperator::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeIfElse::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeFunctionPrototype::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeFunction::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeFunctionCall::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeAssembly::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeEcho::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeWhileLoop::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

void ASTNodeBreak::accept(IASTNodeVisitor* visitor) {
	visitor->visit(this);
}

// ASTNodeBlock
void ASTNodeBlock::push(ASTNode* node) {
	this->statements.push_back(node);
}
