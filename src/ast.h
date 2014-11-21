#ifndef __AST_H_
#define __AST_H_

#include <vector>
#include <tuple>
#include <cstdint>

class CodeGen;

enum tagEBinaryOperationType {
	eADD = 1,
	eSUBTRACT = 3,
	eMULTIPLY = 5,
	eDIVIDE = 7,
	eEQ = 0,
	eLT = 2,
	eGT = 4,
};

typedef enum tagEBinaryOperationType EBinaryOperationType;

class IASTNodeVisitor;

class ASTNode {
public:
	virtual void accept(IASTNodeVisitor*) = 0;

	virtual ~ASTNode() = 0;
};

class ASTNodeIdentifier : public ASTNode {
public:
	std::string name;

	ASTNodeIdentifier(std::string);
	virtual void accept(IASTNodeVisitor*) override;

	virtual ~ASTNodeIdentifier();
};

class ASTNodeLiteral : public ASTNode {
public:
	int16_t val;

	ASTNodeLiteral(int16_t);
	virtual void accept(IASTNodeVisitor*) override;

	virtual ~ASTNodeLiteral();
};

class ASTNodeDeclaration : public ASTNode {
public:
	std::string var_name;

	ASTNodeDeclaration(std::string);
	virtual void accept(IASTNodeVisitor*) override;

	virtual ~ASTNodeDeclaration();
};

class ASTNodeBlock : public ASTNode {
public:
	std::vector<ASTNode*> statements;

	ASTNodeBlock();
	virtual void accept(IASTNodeVisitor*) override;
	void push(ASTNode*);

	virtual ~ASTNodeBlock();
};

class ASTNodeAssignment : public ASTNode {
public:
	ASTNodeIdentifier* lhs;
	ASTNode* rhs;

	ASTNodeAssignment(ASTNodeIdentifier*, ASTNode*);
	virtual void accept(IASTNodeVisitor*) override;

	virtual ~ASTNodeAssignment();
};

class ASTNodeBinaryOperator : public ASTNode {
public:
	EBinaryOperationType op;
	ASTNode* lhs;
	ASTNode* rhs;

	ASTNodeBinaryOperator(EBinaryOperationType, ASTNode*, ASTNode*);
	virtual void accept(IASTNodeVisitor*) override;

	virtual ~ASTNodeBinaryOperator();
};

class ASTNodeFunctionPrototype : public ASTNode {
public:
	std::string function_name;
	std::vector<ASTNodeDeclaration*>* args;

	ASTNodeFunctionPrototype(std::string, std::vector<ASTNodeDeclaration*>*);
	virtual void accept(IASTNodeVisitor*) override;

	virtual ~ASTNodeFunctionPrototype();
};

class ASTNodeFunction : public ASTNode {
public:
	ASTNodeFunctionPrototype* prototype;
	ASTNodeBlock* body;

	ASTNodeFunction(ASTNodeFunctionPrototype*, ASTNodeBlock*);
	virtual void accept(IASTNodeVisitor*) override;

	virtual ~ASTNodeFunction();
};

class ASTNodeFunctionCall : public ASTNode {
public:
	std::string function_name;
	std::vector<ASTNode*>* args;

	ASTNodeFunctionCall(std::string, std::vector<ASTNode*>*);
	virtual void accept(IASTNodeVisitor*) override;

	virtual ~ASTNodeFunctionCall();
};

class ASTNodeIfElse : public ASTNode {
public:
	ASTNode* cond;
	ASTNode* if_true;
	ASTNode* if_false;

	ASTNodeIfElse(ASTNode* cond, ASTNode* if_true, ASTNode* if_false);
	virtual void accept(IASTNodeVisitor*) override;

	virtual ~ASTNodeIfElse();
};

class IASTNodeVisitor {
public:
	virtual void visit(ASTNodeIdentifier*) = 0;
	virtual void visit(ASTNodeLiteral*) = 0;
	virtual void visit(ASTNodeDeclaration*) = 0;
	virtual void visit(ASTNodeBlock*) = 0;
	virtual void visit(ASTNodeAssignment*) = 0;
	virtual void visit(ASTNodeBinaryOperator*) = 0;
	virtual void visit(ASTNodeFunctionPrototype*) = 0;
	virtual void visit(ASTNodeFunction*) = 0;
	virtual void visit(ASTNodeFunctionCall*) = 0;
	virtual void visit(ASTNodeIfElse*) = 0;
};

#endif /* __AST_H_ */
