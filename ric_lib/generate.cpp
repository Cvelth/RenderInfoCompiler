#include "shared.hpp"
#include "objects.hpp"
#include "ric.hpp"
#include <algorithm>
#include <sstream>
#define Unimplemented_Feature(tree) throw Exceptions::InnerCompilationError("The feature is not implemented.", (tree)->line, (tree)->pos)

namespace ric {
	ObjectMap& process_node(Tree node, ObjectMap &objects);
	ObjectMap& process_assignment_node(Tree node, ObjectMap &objects);
	ObjectMap& process_object_node(Tree node, ObjectMap &objects);

	Color process_color(Tree tree, ObjectMap &file, bool is_virtual = false);
	Palette process_palette(Tree tree, ObjectMap &file, bool is_virtual = false);
	Primitive process_primitive(Tree tree, ObjectMap &file, Tree params);
	Object process_object(Tree tree, ObjectMap &file, bool is_virtual = false, Color *default_color = nullptr);

	std::string current_namespace;
	ObjectMap generate_object_map(Tree tree) {
		current_namespace = "";
		ObjectMap ret;
		process_node(tree, ret);
		return ret;
	}
	void generate(std::ostream &file, Tree tree) {
		auto file_data = generate_object_map(tree);
		return;
	}

	enum class DataType {
		palette, color, object, primitive
	};
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
	std::string convert(DataType type) {
		switch (type) {
			case DataType::color: return "color";
			case DataType::palette: return "palette";
			case DataType::primitive: return "primitive";
			case DataType::object: return "object";
			default:
				throw Exceptions::InnerCompilationError("Unsupported DataType.", -1, -1);
		}
	}

	template <typename Type>
	Type get_identificator(Tree node, std::map<std::string, Type> map) {
		if (node->type != TokenType::identificator)
			throw Exceptions::InnerCompilationError("'" + node->value + "' is not a valid identificator.", node->line, node->pos);
		if (auto it = map.find(node->value); it != map.end())
			return it->second;
		else if (auto it = map.find(current_namespace + "::" + node->value); it != map.end())
			return it->second;
		else
			throw Exceptions::InnerCompilationError("'" + node->value + "' is not a variable.", node->line, node->pos);
	}
	auto get_color(Tree node, ObjectMap &objects) { return get_identificator(node, objects.colors); }
	auto get_palette(Tree node, ObjectMap &objects) { return get_identificator(node, objects.palettes); }
	auto get_primitive(Tree node, ObjectMap &objects) { return get_identificator(node, objects.primitives); }
	auto get_object(Tree node, ObjectMap &objects) { return get_identificator(node, objects.objects); }

	template <class List>
	List& parse_with_separator(Tree tree, List &list, TokenType separator) {
		if (tree) {
			if (tree->type != separator)
				list.push_back(tree);
			else {
				if (tree->left)
					parse_with_separator(tree->left, list, separator);
				if (tree->right)
					parse_with_separator(tree->right, list, separator);
			}
		}
		return list;
	}
	std::list<Tree> parse_with_separator(Tree tree, TokenType separator) {
		std::list<Tree> ret;
		return parse_with_separator(tree, ret, separator);
	}

	ObjectMap& process_node(Tree node, ObjectMap &objects) {
		if (!node)
			return objects;
		switch (node->type) {
			case TokenType::new_line:
				process_node(node->left, objects);
				process_node(node->right, objects);
				return objects;
			case TokenType::namespace_:
				current_namespace = node->value;
				process_node(node->right, objects);
				return objects;
			case TokenType::arithmetic:
				if (node->value == "=")
					return process_assignment_node(node, objects);
				else
					throw Exceptions::InnerCompilationError("Operator" + node->value + " is not expected here.", node->line, node->pos);
			case TokenType::object:
				return process_object_node(node, objects);

			case TokenType::comma:
				throw Exceptions::InnerCompilationError("Comma is not expected here.", node->line, node->pos);
			case TokenType::block:
				throw Exceptions::InnerCompilationError("{}-block is not expected here.", node->line, node->pos);
			case TokenType::index:
				throw Exceptions::InnerCompilationError("[]-block is not expected here.", node->line, node->pos);
			case TokenType::args:
				throw Exceptions::InnerCompilationError("()-block is not expected here.", node->line, node->pos);
			case TokenType::identificator:
				throw Exceptions::InnerCompilationError("Identificator " + node->value + " is not expected here.", node->line, node->pos);
			case TokenType::number:
				throw Exceptions::InnerCompilationError("Number " + node->value + " is not expected here.", node->line, node->pos);
			case TokenType::color_literal:
				throw Exceptions::InnerCompilationError("Color " + node->value + " is not expected here.", node->line, node->pos);

			case TokenType::keyword:
			case TokenType::datatype:
			case TokenType::extention:
			case TokenType::file:
				throw Exceptions::InnerCompilationError(node->value + " is not expected here.", node->line, node->pos);

			case TokenType::unknown:
			case TokenType::directive:
			default:
				throw Exceptions::InnerCompilationError("Unknown tree node '" + node->value + "' was encountered.", node->line, node->pos);
		}
	}
	ObjectMap& process_assignment_node(Tree node, ObjectMap &objects) {
		if (!node->left)
			throw Exceptions::InnerCompilationError("There's nothing to the left side of operator=.", node->line, node->pos);
		if (!node->right)
			throw Exceptions::InnerCompilationError("There's nothing to the right side of operator=.", node->line, node->pos);
		if (node->left->type != TokenType::object)
			throw Exceptions::InnerCompilationError(node->value + " was found to the left side of operator= instead of lvalue.", node->left->line, node->left->pos);
		if (node->left->left->type != TokenType::datatype)
			throw Exceptions::InnerCompilationError(node->value + " was found to the left side of operator= instead of datatype.", node->left->line, node->left->pos);
		if (!node->left->left)
			throw Exceptions::InnerCompilationError("Object type is unsupported: '" + node->left->value + "'.", node->left->line, node->left->pos);

		switch (auto type = convert_to_DataType(node->left->left)) {
			case DataType::color:
				objects.add(node->left->value, process_color(node->right, objects, node->left->left->left
															 && node->left->left->left->type == TokenType::keyword
															 && node->left->left->left->value == "virtual"));
				return objects;
			case DataType::palette:
			case DataType::primitive:
			case DataType::object:
				Unimplemented_Feature(node);
			default:
				throw Exceptions::InnerCompilationError("Unsupported object type.", node->left->left->line, node->left->left->pos);
		}
	}
	ObjectMap& process_object_node(Tree node, ObjectMap &objects) {
		if (node->type != TokenType::object)
			throw Exceptions::InnerCompilationError("'" + node->value + "' is found instead of an object.", node->line, node->pos);
		if (!node->left)
			throw Exceptions::InnerCompilationError("DataType is expected before '" + node->value + "'.", node->line, node->pos);
		Tree type_node;
		if (node->left->type == TokenType::datatype)
			type_node = node->left;
		else if ((node->left->type == TokenType::index || node->left->type == TokenType::args)
				 && node->left->left && node->left->left->type == TokenType::datatype)
			type_node = node->left->left;
		else
			throw Exceptions::InnerCompilationError("DataType is expected before '" + node->value + "'.", node->line, node->pos);

		switch (auto type = convert_to_DataType(type_node)) {
			case DataType::color:
				if (!node->right || (node->right->type != TokenType::color_literal && node->right->type != TokenType::identificator && node->right->type != TokenType::index))
					if (node->right->type == TokenType::new_line)
						throw Exceptions::InnerCompilationError("Only one color can be used on color initialization.", node->right->line, node->right->pos);
					else
						throw Exceptions::InnerCompilationError("Color was expected but '" + node->right->value + " is found instead.", node->right->line, node->right->pos);
				objects.add(node->value, process_color(node->right, objects, type_node->left
													   && type_node->left->type == TokenType::keyword
													   && type_node->left->value == "virtual"));
				return objects;
			case DataType::palette:
				if (node->left->type != TokenType::datatype)
					throw Exceptions::InnerCompilationError("DataType is expected before '" + node->value + "'.", node->line, node->pos);
				objects.add(node->value, process_palette(node->right, objects, type_node->left
														 && type_node->left->type == TokenType::keyword
														 && type_node->left->value == "virtual"));
				return objects;
			case DataType::primitive:
				if (node->value == "")
					throw Exceptions::InnerCompilationError("Unnamed primitive outside of an object was ignored.", node->line, node->pos);
				objects.add(node->value, process_primitive(node->right, objects, node->left->right));
				return objects;
			case DataType::object:
				if (node->left->type == TokenType::datatype) {
					objects.add(node->value, process_object(node->right, objects, type_node->left
															&& type_node->left->type == TokenType::keyword
															&& type_node->left->value == "virtual"));
				} else if (node->left->type == TokenType::index) {
					if (!node->left->right || node->left->right->type != TokenType::identificator)
						throw Exceptions::InnerCompilationError("Palette name is expected inside index brackets on object creation.", node->line, node->pos);
					Palette palette = get_palette(node->left->right, objects);
					size_t counter = 0;
					for (auto color : *palette)
						objects.add(node->value + "[" + std::to_string(counter++) + "]", process_object(node->right, objects, type_node->left
																										&& type_node->left->type == TokenType::keyword
																										&& type_node->left->value == "virtual",
																										&color));
				} else
					throw Exceptions::InnerCompilationError("DataType is expected before '" + node->value + "'.", node->line, node->pos);
				return objects;
			default:
				throw Exceptions::InnerCompilationError("Unsupported datatype.", node->line, node->pos);
		}
	}

	Color process_color(Tree node, ObjectMap &objects, bool is_virtual) {
		if (!node)
			throw Exceptions::InnerCompilationError("Color cannot be empty.", node->line, node->pos);

		switch (node->type) {
			case TokenType::identificator:
				return get_color(node, objects);
			case TokenType::index:
				if (!node->right || node->right->type != TokenType::number)
					throw Exceptions::InnerCompilationError("Only numeric indices are supported.", node->line, node->pos);
				if (auto index = std::stod(node->right->value); index == size_t(index)) {
					Color ret(is_virtual);
					return ret = get_palette(node->left, objects)->at(size_t(index));
				} else
					throw Exceptions::InnerCompilationError("Index should be an unsigned integer.", node->line, node->pos);
			case TokenType::block:
			case TokenType::args:
				if (!node->right)
					throw Exceptions::InnerCompilationError("Color literal or variable name was expected.", node->right->line, node->right->pos);
				else {
					Color ret(is_virtual);
					return ret = process_color(node->right, objects);
				}

			case TokenType::color_literal:
			{
				Color ret(is_virtual);
				std::string literal = node->value.substr(2);
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
						ret[i] = uint8_t(t);
						if (auto s = literal.size(); s > 2)
							literal = literal.substr(0, literal.size() - 2);
						else
							literal = "";
					} else
						ret[i] = 0;
				}
				return ret;
			}

			case TokenType::comma:
				throw Exceptions::InnerCompilationError("Comma is not expected here.", node->line, node->pos);
			case TokenType::number:
				throw Exceptions::InnerCompilationError("Number " + node->value + " is not expected here.", node->line, node->pos);
			case TokenType::new_line:
				throw Exceptions::InnerCompilationError("NewLine (or semicolon) is not expected here.", node->line, node->pos);
			case TokenType::arithmetic:
				throw Exceptions::InnerCompilationError("Operator '" + node->value + "' is not expected here.", node->line, node->pos);
			case TokenType::object:
				throw Exceptions::InnerCompilationError("Object is not expected here.", node->line, node->pos);
			case TokenType::namespace_:
				throw Exceptions::InnerCompilationError("Namespace is not expected here.", node->line, node->pos);

			case TokenType::keyword:
			case TokenType::extention:
			case TokenType::datatype:
				throw Exceptions::InnerCompilationError(node->value + " is not expected here.", node->line, node->pos);

			case TokenType::unknown:
			default:
				throw Exceptions::InnerCompilationError("Unknown node node '" + node->value + "' was encountered.", node->line, node->pos);
		}
	}
	Palette process_palette(Tree node, ObjectMap &objects, bool is_virtual) {
		auto list = parse_with_separator(node, TokenType::comma);
		if (list.size() > std::numeric_limits<uint16_t>::max())
			throw Exceptions::InnerCompilationError("To many elements in a palette. There should be no more than 65535", node->line, node->pos);
		Palette ret(is_virtual);
		/*
		if (list.size() == 1 && list.front()->type == TokenType::new_line)
			if (list.front()->left != nullptr || list.front()->right == nullptr)
				return process_palette(list.front()->left, objects, is_virtual);
			else if (list.front()->left == nullptr || list.front()->right != nullptr)
				return process_palette(list.front()->right, objects, is_virtual);
			else
				throw Exceptions::InnerCompilationError("NewLine (or semicolon) is not expected here.", node->line, node->pos);
		*/
		for (auto it : list) {
			switch (it->type) {
				case TokenType::color_literal:
				case TokenType::identificator:
				case TokenType::index:
				case TokenType::block:
				case TokenType::args:
					ret->push_back(process_color(it, objects, true));
					break;
				case TokenType::extention:
				case TokenType::arithmetic:
					Unimplemented_Feature(node);

				case TokenType::comma:
					throw Exceptions::InnerCompilationError("Comma is not expected here.", node->line, node->pos);
				case TokenType::number:
					throw Exceptions::InnerCompilationError("Number " + node->value + " is not expected here.", node->line, node->pos);
				case TokenType::new_line:
					throw Exceptions::InnerCompilationError("NewLine (or semicolon) is not expected here.", node->line, node->pos);
				case TokenType::namespace_:
					throw Exceptions::InnerCompilationError("Namespace is not expected here.", node->line, node->pos);
				case TokenType::object:
					throw Exceptions::InnerCompilationError("Object is not expected here.", node->line, node->pos);

				case TokenType::keyword:
				case TokenType::datatype:
					throw Exceptions::InnerCompilationError(node->value + " is not expected here.", node->line, node->pos);

				case TokenType::unknown:
				default:
					throw Exceptions::InnerCompilationError("Unsupported member of a palette: '" + it->value + "'.", it->line, it->pos);
			}
		}
		return ret;
	}
	Primitive process_primitive(Tree node, ObjectMap &objects, Tree params) {
		auto parameters = parse_with_separator(params, TokenType::comma);
		if (parameters.size() != 2)
			throw Exceptions::InnerCompilationError("Primitive expects two parameters: drawing mode and number of vertices per instance.", params->line, params->pos);

		auto number_value = std::stod(parameters.back()->value);
		if (number_value != size_t(number_value))
			throw Exceptions::InnerCompilationError("Number of vertices per primitive instance should be an unsigned integer.", node->line, node->pos);
		Primitive ret(convert_to_primitive_type(parameters.front()), size_t(number_value));

		std::list<std::vector<double>> values;
		for (auto row : parse_with_separator(node, TokenType::new_line)) {
			values.push_back(std::vector<double>());
			for (auto it : parse_with_separator(row, TokenType::comma)) {
				if (it->type != TokenType::number)
					throw Exceptions::InnerCompilationError("Unsupported value in primitive body: '" + it->value + "'.", it->line, it->pos);
				values.back().push_back(std::stod(it->value));
			}
		}

		for (auto row : values) {
			std::copy(row.begin(), row.end(), std::back_inserter(*ret));
			if (row.size() != number_value) {
				for (size_t i = 0; i < number_value - row.size() - 1; i++)
					ret->push_back(0.0);
				ret->push_back(1.0);
			}
		}
		return ret;
	}
	/*
	std::unique_ptr<Color> process_color_binary_operator(Tree node, ObjectFile &objects);
	std::unique_ptr<Palette> process_palette_binary_operator(Tree node, ObjectFile &objects);
	std::unique_ptr<Primitive> process_primitive_binary_operator(Tree node, ObjectFile &objects);
	std::unique_ptr<Object> process_object_binary_operator(Tree node, ObjectFile &objects);
	std::unique_ptr<mml::transformation3d> process_transformation_binary_operator(Tree node, ObjectFile &objects);
	void throw_unsupported_operator_error(Tree node, ObjectFile &objects);
	*/
	Object process_object(Tree node, ObjectMap &objects, bool is_virtual, Color *default_color) {
		auto list = parse_with_separator(node, TokenType::new_line);
		Object ret(is_virtual);
		if (default_color)
			ret.color(default_color);
		for (auto it : list) {
			switch (it->type) {
				case TokenType::identificator:
					try {
						ret.color(get_color(it, objects));
						break;
					} catch (Exceptions::InnerCompilationError&) {}
					try {
						ret.primitive(get_primitive(it, objects));
						break;
					} catch (Exceptions::InnerCompilationError&) {}
					try {
						Object object = get_object(it, objects);
						for (auto it : *object) {
							ret.color(object[it.first]);
							ret.primitive(it.second);
						}
						break;
					} catch (Exceptions::InnerCompilationError&) {}
					throw Exceptions::InnerCompilationError("Unsupported indentificator: '" + it->value + "'.", it->line, it->pos);
				case TokenType::index:
				case TokenType::color_literal:
					ret.color(process_color(it, objects, true));
					break;
				case TokenType::object:
				{
					if (!it->left)
						throw Exceptions::InnerCompilationError("Datatype expected to the left of '" + it->value + "'.", it->line, it->pos);
					Tree type_node;
					if (it->left->type == TokenType::datatype)
						type_node = it->left;
					else if ((it->left->type == TokenType::index || it->left->type == TokenType::args)
							 && it->left->left && it->left->left->type == TokenType::datatype)
						type_node = it->left->left;
					else
						throw Exceptions::InnerCompilationError("DataType is expected before '" + it->value + "'.", it->line, it->pos);

					switch (auto type = convert_to_DataType(type_node)) {
						case DataType::color:
							ret.color(process_color(it->right, objects, true));
							break;
						case DataType::primitive:
							ret.primitive(process_primitive(it->right, objects, it->left->right));
							break;
						case DataType::object:
							Unimplemented_Feature(node);
						case DataType::palette:
							throw Exceptions::InnerCompilationError("Palette objects cannot be placed inside other objects.", node->left->line, node->left->pos);
						default:
							throw Exceptions::InnerCompilationError("Unsupported object type.", node->left->line, node->left->pos);
					}
				}
				break;

				case TokenType::arithmetic:
					if (!it->left)
						throw Exceptions::InnerCompilationError("There's nothing to the left side of operator" + it->value + ".", it->line, it->pos);
					if (!it->right)
						throw Exceptions::InnerCompilationError("There's nothing to the right side of operator" + it->value + ".", it->line, it->pos);
					if (it->value == "=") {
						if (it->left->type != TokenType::object)
							throw Exceptions::InnerCompilationError(it->value + " was found to the left side of operator= instead of lvalue.", it->left->line, it->left->pos);
						if (it->left->left->type != TokenType::datatype)
							throw Exceptions::InnerCompilationError(it->value + " was found to the left side of operator= instead of datatype.", it->left->line, it->left->pos);
						if (!it->left->left)
							throw Exceptions::InnerCompilationError("Object type is unsupported: '" + it->left->value + "'.", it->left->line, it->left->pos);
						switch (auto type = convert_to_DataType(it->left->left)) {
							case DataType::color:
								ret.color(process_color(it->right, objects, true));
								break;
							case DataType::palette:
							case DataType::primitive:
							case DataType::object:
								Unimplemented_Feature(node);
							default:
								throw Exceptions::InnerCompilationError("Unsupported object type.", it->left->left->line, it->left->left->pos);
						}
					} else {
						Unimplemented_Feature(node);
					}
					break;
				case TokenType::extention:
					Unimplemented_Feature(node);
					/*
					try {
						ret.primitive(process_library_primitive(it));
					} catch (Exceptions::LibraryCompilationError &e) {
						throw Exceptions::InnerCompilationError(e.error, it->line, it->pos);
					}
					*/
					break;

				case TokenType::comma:
					throw Exceptions::InnerCompilationError("Comma is not expected here.", node->line, node->pos);
				case TokenType::number:
					throw Exceptions::InnerCompilationError("Number " + node->value + " is not expected here.", node->line, node->pos);
				case TokenType::new_line:
					throw Exceptions::InnerCompilationError("NewLine (or semicolon) is not expected here.", node->line, node->pos);
				case TokenType::namespace_:
					throw Exceptions::InnerCompilationError("Namespace is not expected here.", node->line, node->pos);
				case TokenType::block:
					throw Exceptions::InnerCompilationError("{}-block is not expected here.", node->line, node->pos);
				case TokenType::args:
					throw Exceptions::InnerCompilationError("()-block is not expected here.", node->line, node->pos);

				case TokenType::keyword:
				case TokenType::datatype:
					throw Exceptions::InnerCompilationError(node->value + " is not expected here.", node->line, node->pos);

				case TokenType::unknown:
				default:
					throw Exceptions::InnerCompilationError("Unsupported member of an object: '" + it->value + "'.", it->line, it->pos);
			}
		}
		return ret;
	}
}

ric::primitive_type ric::convert_to_primitive_type(Tree tree) {
	if (tree->type != TokenType::identificator)
		throw Exceptions::InnerCompilationError("Unsupported primitive type.", tree->line, tree->pos);

	if (tree->value == "points")
		return primitive_type::points;
	else if (tree->value == "lines")
		return primitive_type::lines;
	else if (tree->value == "line_strip")
		return primitive_type::line_strip;
	else if (tree->value == "line_loop")
		return primitive_type::line_loop;
	else if (tree->value == "triangles")
		return primitive_type::triangles;
	else if (tree->value == "triangle_strip")
		return primitive_type::triangle_strip;
	else if (tree->value == "triangle_fan")
		return primitive_type::triangle_fan;
	else if (tree->value == "quads")
		return primitive_type::quads;
	else if (tree->value == "quad_strip")
		return primitive_type::quad_strip;
	else if (tree->value == "polygon")
		return primitive_type::polygon;
	else
		throw Exceptions::InnerCompilationError("Unsupported primitive type.", tree->line, tree->pos);
}
uint8_t ric::convert(primitive_type const& s) {
	return uint8_t(s);
}

/*
#include "shared.hpp"
#include "ric.hpp"
#include <algorithm>
#include <sstream>
#define Unimplemented_Feature throw Exceptions::InnerCompilationError("The feature is not implemented.", tree->line, tree->pos)
namespace ric {
	std::string current_namespace;

	Primitive process_library_primitive(Tree tree) {
		if (!tree || tree->type != TokenType::library)
			throw Exceptions::InnerCompilationError("'" + tree->value + "' is not a library identificator.", tree->line, tree->pos);
		auto parameters = parse_with_separator(tree->right, TokenType::comma);
		if (tree->value == "ellipse") {
			switch (parameters.size()) {
				case 0:
					throw Exceptions::InnerCompilationError("Impossible to call ellipse() withous aspect_ratio parameter.", tree->line, tree->pos);
				case 1:
					if (parameters.front()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported aspect_ratio parameter.", tree->line, tree->pos);
					return library::ellipse(std::stod(parameters.front()->value), false);
				case 2:
					if (parameters.front()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported aspect_ratio parameter.", tree->line, tree->pos);
					if (parameters.back()->type == TokenType::number)
						return library::ellipse(std::stod(parameters.front()->value), false, std::stoull(parameters.back()->value));
					else if (parameters.back()->type == TokenType::identificator && parameters.back()->value == "filled")
						return library::ellipse(std::stod(parameters.front()->value), true);
					else
						throw Exceptions::InnerCompilationError("Unsupported parameters.", tree->line, tree->pos);
				case 3:
					if (parameters.front()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported aspect_ratio parameter.", tree->line, tree->pos);
					if (parameters.back()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported last parameter.", tree->line, tree->pos);
					if ((*(++parameters.begin()))->type == TokenType::number)
						return library::ellipse(std::stod(parameters.front()->value), false, std::stoull((*(++parameters.begin()))->value), std::stoull(parameters.back()->value));
					else if ((*(++parameters.begin()))->type == TokenType::identificator && (*(++parameters.begin()))->value == "filled")
						return library::ellipse(std::stod(parameters.front()->value), true, std::stoull(parameters.back()->value));
					else
						throw Exceptions::InnerCompilationError("Unsupported parameters.", tree->line, tree->pos);
				case 4:
					if (parameters.front()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported aspect_ratio parameter.", tree->line, tree->pos);
					if ((*(++parameters.begin()))->type != TokenType::identificator || (*(++parameters.begin()))->value != "filled")
						throw Exceptions::InnerCompilationError("Unsupported is_filled parameter.", tree->line, tree->pos);
					if ((*(++++parameters.begin()))->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported points parameter.", tree->line, tree->pos);
					if (parameters.back()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported numbers_per_vertex parameter.", tree->line, tree->pos);
					return library::ellipse(std::stod(parameters.front()->value), true, std::stoull((*(++parameters.begin()))->value), std::stoull(parameters.back()->value));
				default:
					throw Exceptions::InnerCompilationError("Too many parameters in ellipse() call.", tree->line, tree->pos);
			}
		} else if (tree->value == "circle") {
			switch (parameters.size()) {
				case 0:
					return library::circle(false);
				case 1:
					if (parameters.back()->type == TokenType::number)
						return library::circle(false, std::stoull(parameters.back()->value));
					else if (parameters.back()->type == TokenType::identificator && parameters.back()->value == "filled")
						return library::circle(true);
					else
						throw Exceptions::InnerCompilationError("Unsupported parameters.", tree->line, tree->pos);
				case 2:
					if (parameters.back()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported last parameter.", tree->line, tree->pos);
					if (parameters.front()->type == TokenType::number)
						return library::circle(false, std::stoull(parameters.front()->value), std::stoull(parameters.back()->value));
					else if (parameters.front()->type == TokenType::identificator && parameters.front()->value == "filled")
						return library::circle(true, std::stoull(parameters.back()->value));
					else
						throw Exceptions::InnerCompilationError("Unsupported parameters.", tree->line, tree->pos);
				case 3:
					if (parameters.front()->type != TokenType::identificator || (*(++parameters.begin()))->value != "filled")
						throw Exceptions::InnerCompilationError("Unsupported is_filled parameter.", tree->line, tree->pos);
					if ((*(++parameters.begin()))->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported points parameter.", tree->line, tree->pos);
					if (parameters.back()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported numbers_per_vertex parameter.", tree->line, tree->pos);
					return library::circle(true, std::stoull((*(++parameters.begin()))->value), std::stoull(parameters.back()->value));
				default:
					throw Exceptions::InnerCompilationError("Too many parameters in circle() call.", tree->line, tree->pos);
			}
		} else if (tree->value == "rectangle") {
			switch (parameters.size()) {
				case 0:
					throw Exceptions::InnerCompilationError("Impossible to call rectangle() withous aspect_ratio parameter.", tree->line, tree->pos);
				case 1:
					if (parameters.front()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported aspect_ratio parameter.", tree->line, tree->pos);
					return library::rectangle(std::stod(parameters.front()->value), false);
				case 2:
					if (parameters.front()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported aspect_ratio parameter.", tree->line, tree->pos);
					if (parameters.back()->type == TokenType::number)
						return library::rectangle(std::stod(parameters.front()->value), false, std::stoull(parameters.back()->value));
					else if (parameters.back()->type == TokenType::identificator && parameters.back()->value == "filled")
						return library::rectangle(std::stod(parameters.front()->value), true);
					else
						throw Exceptions::InnerCompilationError("Unsupported parameters.", tree->line, tree->pos);
				case 3:
					if (parameters.front()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported aspect_ratio parameter.", tree->line, tree->pos);
					if ((*(++parameters.begin()))->type != TokenType::identificator || (*(++parameters.begin()))->value != "filled")
						throw Exceptions::InnerCompilationError("Unsupported is_filled parameter.", tree->line, tree->pos);
					if (parameters.back()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported numbers_per_vertex parameter.", tree->line, tree->pos);
					return library::rectangle(std::stod(parameters.front()->value), true, std::stoull(parameters.back()->value));
				default:
					throw Exceptions::InnerCompilationError("Too many parameters in rectangle() call.", tree->line, tree->pos);
			}
		} else if (tree->value == "square") {
			switch (parameters.size()) {
				case 0:
					return library::square(false);
				case 1:
					if (parameters.back()->type == TokenType::number)
						return library::square(false, std::stoull(parameters.back()->value));
					else if (parameters.back()->type == TokenType::identificator && parameters.back()->value == "filled")
						return library::square(true);
					else
						throw Exceptions::InnerCompilationError("Unsupported parameters.", tree->line, tree->pos);
				case 3:
					if (parameters.front()->type != TokenType::identificator || (*(++parameters.begin()))->value != "filled")
						throw Exceptions::InnerCompilationError("Unsupported is_filled parameter.", tree->line, tree->pos);
					if (parameters.back()->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Unsupported numbers_per_vertex parameter.", tree->line, tree->pos);
					return library::square(true, std::stoull(parameters.back()->value));
				default:
					throw Exceptions::InnerCompilationError("Too many parameters in square() call.", tree->line, tree->pos);
			}
		} else
			throw Exceptions::InnerCompilationError("'" + tree->value + "' cannot be used here.", tree->line, tree->pos);
	}
	std::unique_ptr<mml::transformation3d> process_library_transformation(Tree tree) {
		Unimplemented_Feature;
	}

	

/*

#define write_i(name, size) write(reinterpret_cast<char*>(&name), size)
#define write_s(name) write(name.c_str(), name.size() + 1)

namespace ric {

	void print_color_value(std::ostream &file, Tree tree, Names &names) {
		std::string literal;
		switch (tree->type) {
			case TokenType::color_literal:
				literal = tree->value.substr(2);
				break;
			case TokenType::identificator:
				literal = get_name(names, DataType::color, tree)->value.substr(2);
				break;
			case TokenType::index:
			{
				if (!tree->right || tree->right->type != TokenType::number)
					throw Exceptions::InnerCompilationError("Only numeric indices are supported.", tree->line, tree->pos);
				literal = separator_at_index(get_name(names, DataType::palette, tree->left), 
											 TokenType::comma, size_t(number(tree->right->value)))->value.substr(2);
				break;
			}
			case TokenType::block:
			case TokenType::bracket:
				if (!tree->right)
					throw Exceptions::InnerCompilationError("Literal or color name was expected.", tree->right->line, tree->right->pos);
				return print_color_value(file, tree->right, names);
			default:
				throw Exceptions::InnerCompilationError("Color value was expected. '" + tree->value + "' was met instead.", tree->line, tree->pos);
		}
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
	}
	bool print_operator(std::ostream &file, Tree tree, Names &names) {
		if (tree->type != TokenType::arithmetic)
			throw Exceptions::InnerCompilationError("Unexpected node:'" + tree->value + "'. Operator was expected.", tree->line, tree->pos);
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
					names.at(DataType::color).insert(std::make_pair(tree->left->value, tree->right));
					if (!tree->left->left->left || tree->left->left->left->type != TokenType::reserved || tree->left->left->left->value != "virtual") {
						file.write_i(type, 1);
						file.write_s(tree->left->value);
						print_color_value(file, tree->right, names);
					}
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
	void print_palette(std::ostream &file, Tree tree, Names &names) {
		auto list = parse_with_separator(tree->right, TokenType::comma);
		auto list_size = list.size();
		if (list_size > std::numeric_limits<uint16_t>::max())
			throw Exceptions::InnerCompilationError("To many members of the palette. There should be no more than 65535", tree->line, tree->pos);
		file.write_i(list_size, 2);
		for (auto it : list) {
			switch (it->type) {
				case TokenType::color_literal:
				case TokenType::identificator:
				case TokenType::index:
				case TokenType::block:
				case TokenType::bracket:
					print_color_value(file, it, names);
					break;
				case TokenType::library:
					Unimplemented_Feature;
					break;
				default: 
					throw Exceptions::InnerCompilationError("Unsupported member of a palette: '" + it->value + "'.", it->line, it->pos);
			}
		}
	}
	void print_object(std::ostream &file, Tree tree, Names &names, Tree current_color = nullptr) {
		if (current_color && current_color->type != TokenType::color_literal)
			throw Exceptions::InnerCompilationError("Color was expected, but '" + current_color->value + "' is found instead.", current_color->line, current_color->pos);
		auto list = parse_with_separator(tree->right, TokenType::semicolon);
		auto list_size = list.size();
		if (list_size > std::numeric_limits<uint16_t>::max())
			throw Exceptions::InnerCompilationError("To many members of the object. There should be no more than 65535", tree->line, tree->pos);
		file.write_i(list_size, 2);
		for (auto it : list) {
			switch (it->type) {
				case TokenType::color_literal:
					current_color = it;
					break;
				case TokenType::identificator:
					current_color = get_name(names, DataType::color, it);
					break;
				case TokenType::index:
				{
					if (!it->right || it->right->type != TokenType::number)
						throw Exceptions::InnerCompilationError("Only numeric indices are supported.", it->line, it->pos);
					current_color = separator_at_index(get_name(names, DataType::palette, it->left),
													   TokenType::comma, size_t(number(it->right->value)));
					break;
				}
				case TokenType::block:
				case TokenType::bracket:
					if (!it->right)
						throw Exceptions::InnerCompilationError("Literal or color name was expected.", it->right->line, it->right->pos);
					list.push_back(it->right);
					break;
				case TokenType::object:
					if (it->left->type == TokenType::datatype)
						if (auto type = convert_to_DataType(it->left); type == DataType::color) {
							names.at(DataType::color).insert(std::make_pair(tree->left->value, tree->right));
							if (!tree->left->left->left || tree->left->left->left->type != TokenType::reserved || tree->left->left->left->value != "virtual") {
								file.write_i(type, 1);
								file.write_s(tree->left->value);
								print_color_value(file, tree->right, names);
							}
						} else
							throw Exceptions::InnerCompilationError("This DataType is unsupported.", it->left->line, it->left->pos);
					else if (it->left->type == TokenType::bracket) {
						if (!it->left->left || it->left->left->type != TokenType::datatype)
							throw Exceptions::InnerCompilationError("DataType was expected.", it->left->line, it->left->pos);
						if (auto type = convert_to_DataType(it->left->left); type == DataType::primitive) {
							if (tree->value)
							names.at(DataType::primitive).insert(std::make_pair)
						}
					} else
						throw Exceptions::InnerCompilationError("Unsupported object type.", it->left->line, it->left->pos);
					break;
				case TokenType::arithmetic:

					break;
				case TokenType::library:
					Unimplemented_Feature;
					break;
				default: 
					throw Exceptions::InnerCompilationError("Unsupported member of an object: '" + it->value + "'.", it->line, it->pos);
			}
		}
	}
	bool print_objects(std::ostream &file, Tree tree, Names &names) {
		if (tree->type != TokenType::object)
			throw Exceptions::InnerCompilationError("Unexpected node:'" + tree->value + "'. object was expected.", tree->line, tree->pos);
		if (!tree->left)
			throw Exceptions::InnerCompilationError("DataType is expected before '" + tree->value + "'.", tree->line, tree->pos);
		Tree type_node;
		if (tree->left->type == TokenType::datatype)
			type_node = tree->left;
		else if (tree->left->type == TokenType::index && tree->left->left && tree->left->left->type == TokenType::datatype)
			type_node = tree->left->left;
		else
			throw Exceptions::InnerCompilationError("DataType is expected before '" + tree->value + "'.", tree->line, tree->pos);
		switch (auto type = convert_to_DataType(type_node)) {
			case DataType::color:
				if (!tree->right || (tree->right->type != TokenType::color_literal && tree->right->type != TokenType::identificator))
					if (tree->right->type == TokenType::semicolon)
						throw Exceptions::InnerCompilationError("Only one color can be used on color initialization.", tree->right->line, tree->right->pos);
					else
						throw Exceptions::InnerCompilationError("Color was expected but '" + tree->right->value + " is found instead.", tree->right->line, tree->right->pos);
				names.at(DataType::color).insert(std::make_pair(tree->value, tree->right));
				if (!tree->left->left || tree->left->left->type != TokenType::reserved || tree->left->left->value != "virtual") {
					file.write_i(type, 1);
					file.write_s(tree->value);
					print_color_value(file, tree->right, names);
				}
				break;
			case DataType::palette:
				if (tree->left->type != TokenType::datatype)
					throw Exceptions::InnerCompilationError("DataType is expected before '" + tree->value + "'.", tree->line, tree->pos);
				names.at(DataType::palette).insert(std::make_pair(tree->value, tree->right));
				if (!tree->left->left || tree->left->left->type != TokenType::reserved || tree->left->left->value != "virtual") {
					file.write_i(type, 1);
					file.write_s(tree->value);
					print_palette(file, tree, names);
				}
				break;
			case DataType::primitive:
				Unimplemented_Feature;
				break;
			case DataType::object:
				if (tree->left->type == TokenType::datatype) {
					names.at(DataType::object).insert(std::make_pair(tree->value, tree->right));
					if (!tree->left->left || tree->left->left->type != TokenType::reserved || tree->left->left->value != "virtual") {
						file.write_i(type, 1);
						file.write_s(tree->value);
						print_object(file, tree, names);
					}
				} else if (tree->left->type == TokenType::index) {
					if (!tree->left->right || tree->left->right->type != TokenType::identificator)
						throw Exceptions::InnerCompilationError("Identificator is expected in index-brackets.", tree->left->line, tree->left->pos);
					auto palette = get_name(names, DataType::palette, tree->left->right);
					Unimplemented_Feature;
				} else
					throw Exceptions::InnerCompilationError("DataType is expected before '" + tree->value + "'.", tree->line, tree->pos);
				break;
		}
		return true;
	}
	void print(std::ostream &file, Tree tree, Names &names) {
		switch (tree->type) {
			case TokenType::semicolon: break;
			case TokenType::arithmetic:
				if (print_operator(file, tree, names))
					return;
				else
					break;
			case TokenType::object:
				if (print_objects(file, tree, names))
					return;
				else
					break;
			case TokenType::namespace_:
				current_namespace = tree->value;
				return print(file, tree->right, names);
			default:
				throw Exceptions::InnerCompilationError("Unknown tree node '" + tree->value + "' was encountered.", tree->line, tree->pos);
		}

		if (tree->left) print(file, tree->left, names);
		if (tree->right) print(file, tree->right, names);
	}
	void generate(std::ostream &file, Tree tree) {
		file.write("rio", 3);
		uint8_t v = 1;
		file.write_i(v, 1);
		v = 0;
		file.write_i(v, 1);

		current_namespace = "";
		Names names;
		if (tree) print(file, tree, initialize_names(names));
	}
}*/