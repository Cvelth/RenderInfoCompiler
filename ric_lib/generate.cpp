#include "shared.hpp"
#include "ric.hpp"
#include <ostream>
#include <sstream>
#include <iomanip>

#define Unimplemented_Feature throw Exceptions::InnerCompilationError("Unimplemented feature", tree->line, tree->pos)
#define write_i(name, size) write(reinterpret_cast<char*>(&name), size)
#define write_s(name) write(name.c_str(), name.size() + 1)

namespace ric {
	DataType convert_to_DataType(Tree tree) {
		if (tree->type != TokenType::datatype)
			throw Exceptions::InnerCompilationError("Unexpected node:'" + tree->value + "'. DataType was expected.", tree->line, tree->pos);
		if (tree->value == "color")
			return DataType::color;
		else if (tree->value == "palette")
			return DataType::palette;
		else if (tree->value == "primitive")
			return DataType::primitive;
		else if (tree->value == "object")
			return DataType::object;
		else
			throw Exceptions::InnerCompilationError("Unsupported DataType.", tree->line, tree->pos);
	}
	void print_color_value(std::ostream &file, Tree tree) {
		if (tree->type == TokenType::color_literal) {
			auto literal = tree->value.substr(2);
			for (int i = 0; i < 4; i++) {
				if (literal.size() > 0) {
					std::string temp;
					if (auto s = literal.size(); s > 2)
						temp = literal.substr(s - 2);
					else
						temp = literal;
					std::istringstream iss(temp);
					uint16_t t;
					iss >> std::hex >> t;
					file.write_i(t, 1);
					if (auto s = literal.size(); s > 2)
						literal = literal.substr(0, literal.size() - 2);
					else
						literal = "";
				} else {
					uint8_t t = 0;
					file.write_i(t, 1);
				}
			}
		} else
			throw Exceptions::InnerCompilationError("Color value was expected. '" + tree->value + "' was met instead.", tree->line, tree->pos);

	}
	bool print_operator(std::ostream &file, Tree tree) {
		if (tree->type != TokenType::arithmetic)
			throw Exceptions::InnerCompilationError("Unexpected node:'" + tree->value + "'. DataType ws expected.", tree->line, tree->pos);
		if (tree->value == "=") {
			if (!tree->left)
				throw Exceptions::InnerCompilationError("There's nothing to the left side of operator=.", tree->line, tree->pos);
			if (!tree->right)
				throw Exceptions::InnerCompilationError("There's nothing to the right side of operator=.", tree->line, tree->pos);
			if (tree->left->type != TokenType::object)
				throw Exceptions::InnerCompilationError("lvalue was expected to the left side of operator=. '" + tree->value + "' was found instead.", tree->left->line, tree->left->pos);
			if (!tree->left->left)
				throw Exceptions::InnerCompilationError("Object of unknown type was encountered: '" + tree->left->value + "'.", tree->left->line, tree->left->pos);
			switch (auto type = convert_to_DataType(tree->left->left)) {
				case DataType::color:
					file.write_i(type, 1);
					file.write_s(tree->left->value);
					print_color_value(file, tree->right);
					break;
				case DataType::palette:
					Unimplemented_Feature;
					break;
				case DataType::primitive:
					Unimplemented_Feature;
					break;
				case DataType::object:
					Unimplemented_Feature;
					break;
				default:
					throw Exceptions::InnerCompilationError("Unsupported object type.", tree->left->left->line, tree->left->left->pos);
			}
			return true;
		} else {
			Unimplemented_Feature;
			return false;
		}
	}
	void print(std::ostream &file, Tree tree) {
		switch (tree->type) {
			case TokenType::semicolon: break;
			case TokenType::arithmetic:
				if (print_operator(file, tree))
					return;
				else
					break;
			default:
				throw Exceptions::InnerCompilationError("Unknown tree node '" + tree->value + "' was encountered.", tree->line, tree->pos);
		}

		if (tree->left) print(file, tree->left);
		if (tree->right) print(file, tree->right);
	}
	void generate(std::ostream &file, Tree tree) {
		file.write("rio", 3);
		uint8_t v = 1;
		file.write_i(v, 1);
		v = 0;
		file.write_i(v, 1);

		if (tree) print(file, tree);
	}
}