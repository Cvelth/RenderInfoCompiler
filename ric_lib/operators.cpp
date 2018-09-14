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
	void throw_unsupported_operator_error(Tree tree, ObjectFile &file) {
		if (auto left = process_color_operand(tree->left, file); left) {
			if (auto right = process_color_operand(tree->right, file); right) {
				//Color * Color -> Color
			} else if (auto right = process_palette_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Color, Palette).", tree->line, tree->pos);
			} else if (auto right = process_primitive_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Color, Primitive).", tree->line, tree->pos);
			} else if (auto right = process_object_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Color, Object).", tree->line, tree->pos);
			} else if (auto right = process_transformation_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Color, Transformation).", tree->line, tree->pos);
			} else if (auto right = process_number_operand(tree->right, file); right) {
				//Color * Number -> Color
			} else
				throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
		} else if (auto left = process_palette_operand(tree->left, file); left) {
			if (auto right = process_color_operand(tree->right, file); right) {
				//Palette * Color -> Palette
			} else if (auto right = process_palette_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Palette, Palette).", tree->line, tree->pos);
			} else if (auto right = process_primitive_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Palette, Primitive).", tree->line, tree->pos);
			} else if (auto right = process_object_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Palette, Object).", tree->line, tree->pos);
			} else if (auto right = process_transformation_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Palette, Transformation).", tree->line, tree->pos);
			} else if (auto right = process_number_operand(tree->right, file); right) {
				//Palette * Number -> Palette
			} else
				throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
		} else if (auto left = process_primitive_operand(tree->left, file); left) {
			if (auto right = process_color_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Primitive, Color).", tree->line, tree->pos);
			} else if (auto right = process_palette_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Primitive, Palette).", tree->line, tree->pos);
			} else if (auto right = process_primitive_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Primitive, Primitive).", tree->line, tree->pos);
			} else if (auto right = process_object_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Primitive, Object).", tree->line, tree->pos);
			} else if (auto right = process_transformation_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Primitive, Transformation). "
														"Did you mean operator" + tree->value + "(Transformation, Primitive)?", tree->line, tree->pos);
			} else if (auto right = process_number_operand(tree->right, file); right) {
				//Primitive * Number -> Primitive
			} else
				throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
		} else if (auto left = process_object_operand(tree->left, file); left) {
			if (auto right = process_color_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Object, Color).", tree->line, tree->pos);
			} else if (auto right = process_palette_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Object, Palette).", tree->line, tree->pos);
			} else if (auto right = process_primitive_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Object, Primitive).", tree->line, tree->pos);
			} else if (auto right = process_object_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Object, Object).", tree->line, tree->pos);
			} else if (auto right = process_transformation_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Object, Transformation). "
														"Did you mean operator" + tree->value + "(Transformation, Object)?", tree->line, tree->pos);
			} else if (auto right = process_number_operand(tree->right, file); right) {
				//Object * Number -> Object
			} else
				throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
		} else if (auto left = process_transformation_operand(tree->left, file); left) {
			if (auto right = process_color_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Transformation, Color).", tree->line, tree->pos);
			} else if (auto right = process_palette_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Transformation, Palette).", tree->line, tree->pos);
			} else if (auto right = process_primitive_operand(tree->right, file); right) {
				//Transformation * Primitive -> Primitive
			} else if (auto right = process_object_operand(tree->right, file); right) {
				//Transformation * Object -> Object
			} else if (auto right = process_transformation_operand(tree->right, file); right) {
				//Transformation * Transformation -> Transformation
			} else if (auto right = process_number_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Transformation, Number).", tree->line, tree->pos);
			} else
				throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
		} else if (auto left = process_number_operand(tree->left, file); left) {
			if (auto right = process_color_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Number, Color). "
														"Did you mean operator" + tree->value + "(Color, Number)?", tree->line, tree->pos);
			} else if (auto right = process_palette_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Number, Palette). "
														"Did you mean operator" + tree->value + "(Palette, Number)?", tree->line, tree->pos);
			} else if (auto right = process_primitive_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Number, Primitive). "
														"Did you mean operator" + tree->value + "(Primitive, Number)?", tree->line, tree->pos);
			} else if (auto right = process_object_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Number, Object). "
														"Did you mean operator" + tree->value + "(Object, Number)?", tree->line, tree->pos);
			} else if (auto right = process_transformation_operand(tree->right, file); right) {
				throw Exceptions::InnerCompilationError("There is no operator" + tree->value + "(Number, Transformation). "
														"Did you mean operator" + tree->value + "(Transformation, Number)?", tree->line, tree->pos);
			} else if (auto right = process_number_operand(tree->right, file); right) {
				Unimplemented_Feature;
			} else
				throw Exceptions::InnerCompilationError("Unsupported identificator to the right of operator" + tree->value + ".", tree->line, tree->pos);
		} else
			throw Exceptions::InnerCompilationError("Unsupported identificator to the left of operator" + tree->value + ".", tree->line, tree->pos);
	}

	Color operator+(Color const& a, Color const& b) {
		return Color(true, a.r() + b.r(), a.g() + b.g(), a.b() + b.b(), a.a() + b.a());
	}
	Color operator-(Color const& a, Color const& b) {
		return Color(true, a.r() - b.r(), a.g() - b.g(), a.b() - b.b(), a.a() - b.a());
	}
	Color operator*(Color const& a, Color const& b) {
		return Color(true, a.r() * b.r(), a.g() * b.g(), a.b() * b.b(), a.a() * b.a());
	}
	Color operator/(Color const& a, Color const& b) {
		return Color(true, a.r() / b.r(), a.g() / b.g(), a.b() / b.b(), a.a() / b.a());
	}
	Color operator+(Color const& a, uint8_t const& b) {
		return Color(true, a.r() + b, a.g() + b, a.b() + b, a.a() + b);
	}
	Color operator-(Color const& a, uint8_t const& b) {
		return Color(true, a.r() - b, a.g() - b, a.b() - b, a.a() - b);
	}
	Color operator*(Color const& a, uint8_t const& b) {
		return Color(true, a.r() * b, a.g() * b, a.b() * b, a.a() * b);
	}
	Color operator/(Color const& a, uint8_t const& b) {
		return Color(true, a.r() / b, a.g() / b, a.b() / b, a.a() / b);
	}
	Palette operator+(Palette const& a, Color const& b) {
		Palette ret = a;
		for (auto &it : *ret)
			it = it + b;
		return ret;
	}
	Palette operator-(Palette const& a, Color const& b) {
		Palette ret = a;
		for (auto &it : *ret)
			it = it - b;
		return ret;
	}
	Palette operator*(Palette const& a, Color const& b) {
		Palette ret = a;
		for (auto &it : *ret)
			it = it * b;
		return ret;
	}
	Palette operator/(Palette const& a, Color const& b) {
		Palette ret = a;
		for (auto &it : *ret)
			it = it / b;
		return ret;
	}

	Primitive operator+(Primitive const& a, double const& b) {
		Primitive ret = a;
		for (auto &it : *ret)
			it += b;
		return ret;
	}
	Primitive operator-(Primitive const& a, double const& b) {
		Primitive ret = a;
		for (auto &it : *ret)
			it -= b;
		return ret;
	}
	Primitive operator*(Primitive const& a, double const& b) {
		Primitive ret = a;
		for (auto &it : *ret)
			it *= b;
		return ret;
	}
	Primitive operator/(Primitive const& a, double const& b) {
		Primitive ret = a;
		for (auto &it : *ret)
			it /= b;
		return ret;
	}

	Primitive operator*(mgl::math::transformation3d const& a, Primitive const& b) {
		Primitive ret = b;
		for (size_t i = 0; i < ret->size(); i += ret.vertices_per_instance) {
			mgl::math::basic_vector<double, 4u> tmp{0,0,0,1};
			for (size_t j = 0; j < ret.vertices_per_instance; j++)
				tmp[j] = ret->at(i + j);
			tmp = a * tmp;
			for (size_t j = 0; j < ret.vertices_per_instance; j++)
				ret->at(i + j) = tmp[j];
		}
		return ret;
	}

	Object operator+(Object const& a, double const& b) {
		Object ret = a;
		for (auto &it : *ret)
			it.second = it.second + b;
		return ret;
	}
	Object operator-(Object const& a, double const& b) {
		Object ret = a;
		for (auto &it : *ret)
			it.second = it.second - b;
		return ret;
	}
	Object operator*(Object const& a, double const& b) {
		Object ret = a;
		for (auto &it : *ret)
			it.second = it.second * b;
		return ret;
	}
	Object operator/(Object const& a, double const& b) {
		Object ret = a;
		for (auto &it : *ret)
			it.second = it.second / b;
		return ret;
	}

	template <typename A, typename B>
	auto operation(A a, B b, Tree tree) -> decltype(a + b) {
		switch (tree->value[0]) {
			case '+': return a + b;
			case '-': return a - b;
			case '*': return a * b;
			case '/': return a / b;
			default: throw Exceptions::InnerCompilationError("Operator" + tree->value + " is not supported or is not expected here.", tree->line, tree->pos);
		}
	}

	std::unique_ptr<Color> process_color_binary_operator(Tree tree, ObjectFile &file) {
		if (tree->value.size() != 1)
			throw Exceptions::InnerCompilationError("Operator" + tree->value + " is not supported.", tree->line, tree->pos);
		check_operand(tree->left); check_operand(tree->right);

		if (auto left = process_color_operand(tree->left, file); left)
			if (auto right = process_color_operand(tree->right, file); right)
				return std::unique_ptr<Color>(new Color(operation(*left, *right, tree)));
			else if (auto right = process_number_operand(tree->right, file); right)
				return std::unique_ptr<Color>(new Color(operation(*left, uint8_t(*right), tree)));
		return nullptr;
	}
	std::unique_ptr<Palette> process_palette_binary_operator(Tree tree, ObjectFile &file) {
		if (tree->value.size() != 1)
			throw Exceptions::InnerCompilationError("Operator" + tree->value + " is not supported.", tree->line, tree->pos);
		check_operand(tree->left); check_operand(tree->right);

		if (auto left = process_palette_operand(tree->left, file); left) {
			if (auto right = process_color_operand(tree->right, file); right) {
				return std::unique_ptr<Palette>(new Palette(operation(*left, *right, tree)));
			} else if (auto right = process_number_operand(tree->right, file); right) {
				return std::unique_ptr<Palette>(new Palette(operation(*left, *right, tree)));
			}
		}
		return nullptr;
	}
	std::unique_ptr<Primitive> process_primitive_binary_operator(Tree tree, ObjectFile &file) {
		if (tree->value.size() != 1)
			throw Exceptions::InnerCompilationError("Operator" + tree->value + " is not supported.", tree->line, tree->pos);
		check_operand(tree->left); check_operand(tree->right);

		if (auto left = process_primitive_operand(tree->left, file); left) {
			if (auto right = process_number_operand(tree->right, file); right) {
				return std::unique_ptr<Primitive>(new Primitive(operation(*left, *right, tree)));
			}
		} else if (auto left = process_transformation_operand(tree->left, file); left) {
			if (auto right = process_primitive_operand(tree->right, file); right) {
				if (tree->value[0] == '*') {
					if (right->vertices_per_instance > 4)
						throw Exceptions::InnerCompilationError("Operator" + tree->value + "(Transformation, Primitive) is not supported for primitives with more than 4 values per vertex.", tree->line, tree->pos);
					return std::unique_ptr<Primitive>(new Primitive(*left * *right));
				} else
					throw Exceptions::InnerCompilationError("Operator" + tree->value + "(Transformation, Primitive) is not supported.", tree->line, tree->pos);
			}
		}
		return nullptr;
	}
	std::unique_ptr<Object> process_object_binary_operator(Tree tree, ObjectFile &file) {
		if (tree->value.size() != 1)
			throw Exceptions::InnerCompilationError("Operator" + tree->value + " is not supported.", tree->line, tree->pos);
		check_operand(tree->left); check_operand(tree->right);

		if (auto left = process_object_operand(tree->left, file); left) {
			if (auto right = process_number_operand(tree->right, file); right) {
				return std::unique_ptr<Object>(new Object(operation(*left, *right, tree)));
			}
		} else if (auto left = process_transformation_operand(tree->left, file); left) {
			if (auto right = process_object_operand(tree->right, file); right) {
				if (tree->value[0] == '*') {
					auto ret = *right;
					for (auto &it : *ret) {
						it.second = *left * it.second;
					}
					return std::unique_ptr<Object>(new Object(ret));
				} else
					throw Exceptions::InnerCompilationError("Operator" + tree->value + "(Transformation, Primitive) is not supported.", tree->line, tree->pos);
			}
		}
		return nullptr;
	}
	std::unique_ptr<mgl::math::transformation3d> process_transformation_binary_operator(Tree tree, ObjectFile &file) {
		if (tree->value.size() != 1)
			throw Exceptions::InnerCompilationError("Operator" + tree->value + " is not supported.", tree->line, tree->pos);
		check_operand(tree->left); check_operand(tree->right);

		if (auto left = process_transformation_operand(tree->left, file); left)
			if (auto right = process_transformation_operand(tree->right, file); right)
				return std::unique_ptr<mgl::math::transformation3d>(new mgl::math::transformation3d(*left * *right));
		return nullptr;
	}
}