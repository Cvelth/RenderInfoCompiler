#include "ric.hpp"
#include "shared.hpp"
namespace ric {
	Object process_binary_operator(Tree tree, ObjectFile &file) {
		if (tree->value.size != 1)
			throw Exceptions::InnerCompilationError("Operator" + tree->value + " is not supported.", tree->line, tree->pos);
		switch (tree->value[0]) {
			case '+':
				return process_operand(tree->left, file) + process_operand(tree->right, file);
			case '-':
				return process_operand(tree->left, file) - process_operand(tree->right, file);
			case '*':
				return process_operand(tree->left, file) * process_operand(tree->right, file);
			case '/':
				return process_operand(tree->left, file) / process_operand(tree->right, file);
			default:
				throw Exceptions::InnerCompilationError("Operator" + tree->value + " is not supported.", tree->line, tree->pos);
		}
	}
}