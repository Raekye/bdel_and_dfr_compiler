#ifndef __CODE_GEN_H_
#define __CODE_GEN_H_

#include "ast.h"

class CodeGen : public IASTNodeVisitor {
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
};

#endif /* __CODE_GEN_H_ */
