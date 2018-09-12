#include "shared.hpp"
#include "ric.hpp"
#include <algorithm>
#include <sstream>
#define Unimplemented_Feature throw Exceptions::InnerCompilationError("The feature is not implemented.", tree->line, tree->pos)
namespace ric {
	std::string current_namespace;

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

	Color get_color(Tree tree, ObjectFile &file) {
		if (tree->type != TokenType::identificator)
			throw Exceptions::InnerCompilationError(tree->value + " is not a valid identificator.", tree->line, tree->pos);
		if (auto iterator = std::find_if(file.colors.begin(), file.colors.end(), [&tree](std::pair<std::string, Color> t) -> bool {
			return t.first == tree->value || t.first == current_namespace + "::" + tree->value;
		}); iterator != file.colors.end())
			return iterator->second;
		else
			throw Exceptions::InnerCompilationError("'" + tree->value + "' is not a color variable.", tree->line, tree->pos);
	}
	Palette get_palette(Tree tree, ObjectFile &file) {
		if (tree->type != TokenType::identificator)
			throw Exceptions::InnerCompilationError(tree->value + " is not a valid identificator.", tree->line, tree->pos);
		if (auto iterator = std::find_if(file.palettes.begin(), file.palettes.end(), [&tree](std::pair<std::string, Palette> t) -> bool {
			return t.first == tree->value || t.first == current_namespace + "::" + tree->value;
		}); iterator != file.palettes.end())
			return iterator->second;
		else
			throw Exceptions::InnerCompilationError("'" + tree->value + "' is not a palette variable.", tree->line, tree->pos);
	}
	Primitive get_primitive(Tree tree, ObjectFile &file) {
		if (tree->type != TokenType::identificator)
			throw Exceptions::InnerCompilationError(tree->value + " is not a valid identificator.", tree->line, tree->pos);
		if (auto iterator = std::find_if(file.primitives.begin(), file.primitives.end(), [&tree](std::pair<std::string, Primitive> t) -> bool {
			return t.first == tree->value || t.first == current_namespace + "::" + tree->value;
		}); iterator != file.primitives.end())
			return iterator->second;
		else
			throw Exceptions::InnerCompilationError("'" + tree->value + "' is not a primitive variable.", tree->line, tree->pos);
	}
	Object get_object(Tree tree, ObjectFile &file) {
		if (tree->type != TokenType::identificator)
			throw Exceptions::InnerCompilationError(tree->value + " is not a valid identificator.", tree->line, tree->pos);
		if (auto iterator = std::find_if(file.objects.begin(), file.objects.end(), [&tree](std::pair<std::string, Object> t) -> bool {
			return t.first == tree->value || t.first == current_namespace + "::" + tree->value;
		}); iterator != file.objects.end())
			return iterator->second;
		else
			throw Exceptions::InnerCompilationError("'" + tree->value + "' is not a objects variable.", tree->line, tree->pos);
	}

	Color process_color(Tree tree, ObjectFile &file, bool is_virtual = false) {
		if (!tree)
			throw Exceptions::InnerCompilationError("Color cannot be empty.", tree->line, tree->pos);

		switch (tree->type) {
			case TokenType::identificator:
				return get_color(tree, file);
			case TokenType::index:
				if (!tree->right || tree->right->type != TokenType::number)
					throw Exceptions::InnerCompilationError("Only numeric indices are supported.", tree->line, tree->pos);
				if (auto index = number(tree->right->value); index == size_t(index))
					return get_palette(tree->left, file)->at(size_t(index));
				else
					throw Exceptions::InnerCompilationError("Index should be an unsigned integer.", tree->line, tree->pos);
			case TokenType::block:
			case TokenType::bracket:
				if (!tree->right)
					throw Exceptions::InnerCompilationError("Literal or color name was expected.", tree->right->line, tree->right->pos);
				return process_color(tree->right, file);

			case TokenType::color_literal: {
				Color ret(is_virtual);
				std::string literal = tree->value.substr(2);
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
				throw Exceptions::InnerCompilationError("Comma is not expected here.", tree->line, tree->pos);
			case TokenType::reserved:
				throw Exceptions::InnerCompilationError(tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::datatype:
				throw Exceptions::InnerCompilationError(tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::library:
				throw Exceptions::InnerCompilationError(tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::number:
				throw Exceptions::InnerCompilationError("Number " + tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::semicolon:
				throw Exceptions::InnerCompilationError("Semicolon is not expected here.", tree->line, tree->pos);
			case TokenType::arithmetic:
				throw Exceptions::InnerCompilationError("Operator '" + tree->value + "' is not expected here.", tree->line, tree->pos);
			case TokenType::object:
				throw Exceptions::InnerCompilationError("Object is not expected here.", tree->line, tree->pos);
			case TokenType::namespace_:
				throw Exceptions::InnerCompilationError("Namespace is not expected here.", tree->line, tree->pos);
			case TokenType::unknown:
			default:
				throw Exceptions::InnerCompilationError("Unknown tree node '" + tree->value + "' was encountered.", tree->line, tree->pos);
		}
	}

	void process_operator_node(Tree tree, ObjectFile &file) {
		if (tree->type != TokenType::arithmetic)
			throw Exceptions::InnerCompilationError(tree->value + " was found instead of an operator.", tree->line, tree->pos);
		if (tree->value.size() != 1u)
			throw Exceptions::InnerCompilationError("Operator " + tree->value + " is not supported.", tree->line, tree->pos);
		switch (tree->value[0]) {
			case '=':
				if (!tree->left)
					throw Exceptions::InnerCompilationError("There's nothing to the left side of operator=.", tree->line, tree->pos);
				if (!tree->right)
					throw Exceptions::InnerCompilationError("There's nothing to the right side of operator=.", tree->line, tree->pos);
				if (tree->left->type != TokenType::object)
					throw Exceptions::InnerCompilationError(tree->value + " was found to the left side of operator= instead of lvalue.", tree->left->line, tree->left->pos);
				if (tree->left->left->type != TokenType::datatype)
					throw Exceptions::InnerCompilationError(tree->value + " was found to the left side of operator= instead of datatype.", tree->left->line, tree->left->pos);
				if (!tree->left->left)
					throw Exceptions::InnerCompilationError("Object type is unsupported: '" + tree->left->value + "'.", tree->left->line, tree->left->pos);
				switch (auto type = convert_to_DataType(tree->left->left)) {
					case DataType::color:
						file.colors.insert(std::make_pair(tree->left->value, process_color(tree->right, file, 
																								tree->left->left->left 
																								&& tree->left->left->left->type == TokenType::reserved 
																								&& tree->left->left->left->value == "virtual")));
						break;
					case DataType::palette:
					case DataType::primitive:
					case DataType::object:
						Unimplemented_Feature;
					default:
						throw Exceptions::InnerCompilationError("Unsupported object type.", tree->left->left->line, tree->left->left->pos);
				}
				break;
			case '+':
			case '-':
			case '*':
			case '/':
				Unimplemented_Feature;
			default:
				throw Exceptions::InnerCompilationError("Operator " + tree->value + " is not supported.", tree->line, tree->pos);
		}
	}
	void process_object_node(Tree tree, ObjectFile &file) {
		if (tree->type != TokenType::object)
			throw Exceptions::InnerCompilationError("'" + tree->value + "' is found instead of an object.", tree->line, tree->pos);
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
				if (!tree->right || (tree->right->type != TokenType::color_literal && tree->right->type != TokenType::identificator && tree->right->type != TokenType::index))
					if (tree->right->type == TokenType::semicolon)
						throw Exceptions::InnerCompilationError("Only one color can be used on color initialization.", tree->right->line, tree->right->pos);
					else
						throw Exceptions::InnerCompilationError("Color was expected but '" + tree->right->value + " is found instead.", tree->right->line, tree->right->pos);
				file.colors.insert(std::make_pair(tree->value, process_color(tree->right, file,
																				  tree->left->left
																				  && tree->left->left->type == TokenType::reserved
																				  && tree->left->left->value == "virtual")));
				break;
			case DataType::palette:
			case DataType::primitive:
			case DataType::object:
				Unimplemented_Feature;
		}
	}

	void process_tree_node_type(Tree tree, ObjectFile &file) {
		if (!tree)
			return;
		switch (tree->type) {
			case TokenType::semicolon:
				process_tree_node_type(tree->left, file);
				process_tree_node_type(tree->right, file);
				break;
			case TokenType::namespace_:
				current_namespace = tree->value;
				process_tree_node_type(tree->right, file);
				break;
			case TokenType::arithmetic:
				process_operator_node(tree, file);
				break;
			case TokenType::object:
				process_object_node(tree, file);
				break;

			case TokenType::comma:
				throw Exceptions::InnerCompilationError("Comma is not expected here.", tree->line, tree->pos);
			case TokenType::block:
				throw Exceptions::InnerCompilationError("{}-block is not expected here.", tree->line, tree->pos);
			case TokenType::index:
				throw Exceptions::InnerCompilationError("[]-block is not expected here.", tree->line, tree->pos);
			case TokenType::bracket:
				throw Exceptions::InnerCompilationError("()-block is not expected here.", tree->line, tree->pos);
			case TokenType::reserved:
				throw Exceptions::InnerCompilationError(tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::datatype:
				throw Exceptions::InnerCompilationError(tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::library:
				throw Exceptions::InnerCompilationError(tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::identificator:
				throw Exceptions::InnerCompilationError("Identificator " + tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::number:
				throw Exceptions::InnerCompilationError("Number " + tree->value + " is not expected here.", tree->line, tree->pos);
			case TokenType::color_literal:
				throw Exceptions::InnerCompilationError("Color " + tree->value + " is not expected here.", tree->line, tree->pos);

			case TokenType::unknown:
			default:
				throw Exceptions::InnerCompilationError("Unknown tree node '" + tree->value + "' was encountered.", tree->line, tree->pos);
		}
	}
	ObjectFile generate_object_file(Tree tree) {
		current_namespace = "";
		ObjectFile file;
		process_tree_node_type(tree, file);
		return file;
	}
	void generate(std::ostream &file, Tree tree) {
		auto file_data = generate_object_file(tree);


	}
}

/*
#include <ostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>

#define Unimplemented_Feature throw Exceptions::InnerCompilationError("Unimplemented feature", tree->line, tree->pos)
#define write_i(name, size) write(reinterpret_cast<char*>(&name), size)
#define write_s(name) write(name.c_str(), name.size() + 1)

namespace ric {
	std::string current_namespace;

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
	Names& initialize_names(Names &names) {
		names.insert(std::make_pair(DataType::color, std::set<std::pair<std::string, Tree>>()));
		names.insert(std::make_pair(DataType::palette, std::set<std::pair<std::string, Tree>>()));
		names.insert(std::make_pair(DataType::primitive, std::set<std::pair<std::string, Tree>>()));
		names.insert(std::make_pair(DataType::object, std::set<std::pair<std::string, Tree>>()));
		return names;
	}
	Tree get_name(Names &names, DataType type, Tree tree) {
		if (tree->type != TokenType::identificator)
			throw Exceptions::InnerCompilationError("Identificator was expected, but '" + tree->value + "' is found instead.", tree->line, tree->pos);
		if (auto iterator = std::find_if(names[type].begin(), names[type].end(), [&tree](std::pair<std::string, Tree> t) -> bool {
			return t.first == tree->value || t.first == current_namespace + "::" + tree->value;
		}); iterator != names[type].end())
			return iterator->second;
		else
			throw Exceptions::InnerCompilationError("'" + tree->value + " is not a " + convert(type) + " name.", tree->line, tree->pos);
	}
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
	Tree separator_at_index(Tree tree, TokenType separator, size_t index) {
		std::vector<Tree> ret;
		return parse_with_separator(tree, ret, separator).at(index);
	}

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