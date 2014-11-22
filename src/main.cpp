#include <iostream>

#include "code_gen.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
	CodeGen cg(12, 12, 13, 14, 15);
	std::istream* in = &std::cin;
	ASTNode* node = NULL;
	if (argc > 1) {
		std::ifstream in(argv[1]);
		node = cg.parse(&in);
	} else {
		node = cg.parse(&std::cin);
	}
	if (node == NULL) {
		std::cout << "Unable to parse" << std::endl;
		return 1;
	}
	cg.gen_program(node);
	delete node;
	return 0;
}
