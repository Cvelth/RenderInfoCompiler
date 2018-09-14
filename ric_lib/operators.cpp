#include "ric.hpp"
#include "shared.hpp"
#include "mgl/math/transformation.hpp"
#include <sstream>
#define Unimplemented_Feature throw Exceptions::InnerCompilationError("The feature is not implemented.", tree->line, tree->pos)
namespace ric {
	Color get_color(Tree tree, ObjectFile &file);
	Palette get_palette(Tree tree, ObjectFile &file);
	Primitive get_primitive(Tree tree, ObjectFile &file);
	Object get_object(Tree tree, ObjectFile &file);

	Color process_color(Tree tree, ObjectFile &file, bool is_virtual = false);
	Palette process_palette(Tree tree, ObjectFile &file, bool is_virtual = false);
	Primitive process_primitive(Tree tree, ObjectFile &file, Tree params);
	Object process_object(Tree tree, ObjectFile &file, bool is_virtual = false, Color *default_color = nullptr);

	std::unique_ptr<mgl::math::transformation3d> process_library_transformation(Tree tree);

	std::unique_ptr<Color> process_color_operand(Tree tree, ObjectFile &file) {
		switch (tree->type) {
			case TokenType::identificator:
				try {
					return std::unique_ptr<Color>(new Color(get_color(tree, file)));
				} catch (Exceptions::InnerCompilationError) {}
				return nullptr;
			case TokenType::index:
				try {
					return std::unique_ptr<Color>(new Color(get_palette(tree, file)[size_t(number(tree->right->value))]));
				} catch (Exceptions::InnerCompilationError) {}
				return nullptr;
			case TokenType::color_literal:
				return std::unique_ptr<Color>(new Color(process_color(tree, file, true)));
			case TokenType::object:
				if ((tree->left->type == TokenType::datatype && tree->left->value == "color")
					|| ((tree->left->type == TokenType::index || tree->left->type == TokenType::bracket)
						&& tree->left->left && tree->left->left->type == TokenType::datatype && tree->left->left->value == "color")) {
					return std::unique_ptr<Color>(new Color(process_color(tree->right, file, true)));
				} else
					return nullptr;
			case TokenType::bracket:
				return process_color_operand(tree->right, file);
			default:
				return nullptr;
		}
	}
	std::unique_ptr<Palette> process_palette_operand(Tree tree, ObjectFile &file) {
		switch (tree->type) {
			case TokenType::identificator:
				try {
					return std::unique_ptr<Palette>(new Palette(get_palette(tree, file)));
				} catch (Exceptions::InnerCompilationError) {}
				return nullptr;
			case TokenType::object:
				if ((tree->left->type == TokenType::datatype && tree->left->value == "palette")
					|| ((tree->left->type == TokenType::index || tree->left->type == TokenType::bracket)
						&& tree->left->left && tree->left->left->type == TokenType::datatype && tree->left->left->value == "palette")) {
					return std::unique_ptr<Palette>(new Palette(process_palette(tree->right, file, true)));
				} else
					return nullptr;
			case TokenType::bracket:
				return process_palette_operand(tree->right, file);
			default:
				return nullptr;
		}
	}
	std::unique_ptr<Primitive> process_primitive_operand(Tree tree, ObjectFile &file) {
		switch (tree->type) {
			case TokenType::identificator:
				try {
					return std::unique_ptr<Primitive>(new Primitive(get_primitive(tree, file)));
				} catch (Exceptions::InnerCompilationError) {}
				return nullptr;
			case TokenType::object:
				if ((tree->left->type == TokenType::datatype && tree->left->value == "primitive")
					|| ((tree->left->type == TokenType::index || tree->left->type == TokenType::bracket)
						&& tree->left->left && tree->left->left->type == TokenType::datatype && tree->left->left->value == "primitive")) {
					return std::unique_ptr<Primitive>(new Primitive(process_primitive(tree->right, file, tree->left->right)));
				} else
					return nullptr;
			case TokenType::bracket:
				return process_primitive_operand(tree->right, file);
			default:
				return nullptr;
		}
	}
	std::unique_ptr<Object> process_object_operand(Tree tree, ObjectFile &file) {
		switch (tree->type) {
			case TokenType::identificator:
				try {
					return std::unique_ptr<Object>(new Object(get_object(tree, file)));
				} catch (Exceptions::InnerCompilationError) {}
				return nullptr;
			case TokenType::index:
				try {
					std::ostringstream s;
					s << tree->value << "[" << size_t(number(tree->right->value)) << "]";
					return std::unique_ptr<Object>(new Object(get_object(std::make_shared<Node>(tree->type, s.str(), tree->line, tree->pos, tree->left, tree->right), file)));
				} catch (Exceptions::InnerCompilationError) {}
				return nullptr;
			case TokenType::object:
				if ((tree->left->type == TokenType::datatype && tree->left->value == "object")
					|| ((tree->left->type == TokenType::index || tree->left->type == TokenType::bracket)
						&& tree->left->left && tree->left->left->type == TokenType::datatype && tree->left->left->value == "object")) {
					return std::unique_ptr<Object>(new Object(process_object(tree->right, file, true)));
				} else
					return nullptr;
			case TokenType::bracket:
				return process_object_operand(tree->right, file);
			default:
				return nullptr;
		}
	}
	std::unique_ptr<mgl::math::transformation3d> process_transformation_operand(Tree tree, ObjectFile &file) {
		switch (tree->type) {
			case TokenType::library:
				return process_library_transformation(tree);
			case TokenType::bracket:
				return process_transformation_operand(tree->right, file);
			default:
				return nullptr;
		}
	}

	std::unique_ptr<double> process_number_operand(Tree tree, ObjectFile &file) {
		switch (tree->type) {
			case TokenType::number:
				return std::make_unique<double>(number(tree->value));
			case TokenType::bracket:
				return process_number_operand(tree->right, file);
			default:
				return nullptr;
		}
	}

	void check_operand(Tree tree) {
		if (!tree->left)
			throw Exceptions::InnerCompilationError("There's nothing to the left side of operator" + tree->value + ".", tree->line, tree->pos);
		if (!tree->right)
			throw Exceptions::InnerCompilationError("There's nothing to the right side of operator" + tree->value + ".", tree->line, tree->pos);
		switch (tree->type) {
			case TokenType::arithmetic:
				if (!tree->left)
					throw Exceptions::InnerCompilationError("There's nothing to the left side of operator" + tree->value + ".", tree->line, tree->pos);
				if (!tree->right)
					throw Exceptions::InnerCompilationError("There's nothing to the right side of operator" + tree->value + ".", tree->line, tree->pos);
				return;

			case TokenType::bracket:
			case TokenType::identificator:
			case TokenType::index:
			case TokenType::library:
			case TokenType::number:
			case TokenType::color_literal:
			case TokenType::object:
				return;

			case TokenType::comma:
				throw Exceptions::InnerCompilationError("Comma is not expected here.", tree->line, tree->pos);
			case TokenType::reserved:
				throw Exceptions::InnerCompilationError(tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::datatype:
				throw Exceptions::InnerCompilationError(tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::semicolon:
				throw Exceptions::InnerCompilationError("Semicolon is not expected here.", tree->line, tree->pos);
			case TokenType::namespace_:
				throw Exceptions::InnerCompilationError("Namespace is not expected here.", tree->line, tree->pos);
			case TokenType::block:
				throw Exceptions::InnerCompilationError("{}-block is not expected here.", tree->line, tree->pos);

			case TokenType::unknown:
			default:
				throw Exceptions::InnerCompilationError("Unsupported operand.", tree->line, tree->pos);
		}
	}
	Object process_binary_operator(Tree tree, ObjectFile &file) {
		if (tree->value.size() != 1)
			throw Exceptions::InnerCompilationError("Operator" + tree->value + " is not supported.", tree->line, tree->pos);
		check_operand(tree->left); check_operand(tree->right);

		switch (tree->type) {
			case TokenType::arithmetic:
				if (!tree->left)
					throw Exceptions::InnerCompilationError("There's nothing to the left side of operator" + tree->value + ".", tree->line, tree->pos);
				if (!tree->right)
					throw Exceptions::InnerCompilationError("There's nothing to the right side of operator" + tree->value + ".", tree->line, tree->pos);
				return process_binary_operator(tree, file);
			default:
			{
				if (auto left = process_color_operand(tree->left, file); left) {
					if (auto right = process_color_operand(tree->right, file); right) {
						//Color * Color
					} else if (auto right = process_palette_operand(tree->right, file); right) {
						//Color * Palette
					} else if (auto right = process_primitive_operand(tree->right, file); right) {
						//Color * Primitive
					} else if (auto right = process_object_operand(tree->right, file); right) {
						//Color * Object
					} else if (auto right = process_transformation_operand(tree->right, file); right) {
						//Color * Transformation
					} else if (auto right = process_number_operand(tree->right, file); right) {
						//Color * Number
					} else
						throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
				} else if (auto left = process_palette_operand(tree->left, file); left) {
					if (auto right = process_color_operand(tree->right, file); right) {
						//Palette * Color
					} else if (auto right = process_palette_operand(tree->right, file); right) {
						//Palette * Palette
					} else if (auto right = process_primitive_operand(tree->right, file); right) {
						//Palette * Primitive
					} else if (auto right = process_object_operand(tree->right, file); right) {
						//Palette * Object
					} else if (auto right = process_transformation_operand(tree->right, file); right) {
						//Palette * Transformation
					} else if (auto right = process_number_operand(tree->right, file); right) {
						//Palette * Number
					} else
						throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
				} else if (auto left = process_primitive_operand(tree->left, file); left) {
					if (auto right = process_color_operand(tree->right, file); right) {
						//Primitive * Color
					} else if (auto right = process_palette_operand(tree->right, file); right) {
						//Primitive * Palette
					} else if (auto right = process_primitive_operand(tree->right, file); right) {
						//Primitive * Primitive
					} else if (auto right = process_object_operand(tree->right, file); right) {
						//Primitive * Object
					} else if (auto right = process_transformation_operand(tree->right, file); right) {
						//Primitive * Transformation
					} else if (auto right = process_number_operand(tree->right, file); right) {
						//Primitive * Number
					} else
						throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
				} else if (auto left = process_object_operand(tree->left, file); left) {
					if (auto right = process_color_operand(tree->right, file); right) {
						//Object * Color
					} else if (auto right = process_palette_operand(tree->right, file); right) {
						//Object * Palette
					} else if (auto right = process_primitive_operand(tree->right, file); right) {
						//Object * Primitive
					} else if (auto right = process_object_operand(tree->right, file); right) {
						//Object * Object
					} else if (auto right = process_transformation_operand(tree->right, file); right) {
						//Object * Transformation
					} else if (auto right = process_number_operand(tree->right, file); right) {
						//Object * Number
					} else
						throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
				} else if (auto left = process_transformation_operand(tree->left, file); left) {
					if (auto right = process_color_operand(tree->right, file); right) {
						//Transformation * Color
					} else if (auto right = process_palette_operand(tree->right, file); right) {
						//Transformation * Palette
					} else if (auto right = process_primitive_operand(tree->right, file); right) {
						//Transformation * Primitive
					} else if (auto right = process_object_operand(tree->right, file); right) {
						//Transformation * Object
					} else if (auto right = process_transformation_operand(tree->right, file); right) {
						//Transformation * Transformation
					} else if (auto right = process_number_operand(tree->right, file); right) {
						//Transformation * Number
					} else
						throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
				} else if (auto left = process_number_operand(tree->left, file); left) {
					if (auto right = process_color_operand(tree->right, file); right) {
						//Number * Color
					} else if (auto right = process_palette_operand(tree->right, file); right) {
						//Number * Palette
					} else if (auto right = process_primitive_operand(tree->right, file); right) {
						//Number * Primitive
					} else if (auto right = process_object_operand(tree->right, file); right) {
						//Number * Object
					} else if (auto right = process_transformation_operand(tree->right, file); right) {
						//Number * Transformation
					} else if (auto right = process_number_operand(tree->right, file); right) {
						//Number * Number
					} else
						throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
				} else
					throw Exceptions::InnerCompilationError("Unsupported identificator to the left of operator" + tree->value + ".", tree->line, tree->pos);
			}
		}		

		/*
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
				throw Exceptions::InnerCompilationError("Operator" + tree->value + " is not supported or is not expected here.", tree->line, tree->pos);
		}
		*/
	}
}