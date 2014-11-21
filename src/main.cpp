#include <iostream>

#include "code_gen.h"
#include <iostream>

int main() {
	CodeGen cg;
	ASTNode* node = cg.parse(&std::cin);
	if (node == NULL) {
		std::cout << "Unable to parse" << std::endl;
		return 1;
	}
	cg.gen_program(node);
	return 0;
}
