#pragma once
#include <string>
namespace ric::Exceptions {
	class InnerCompilationError {
	public:
		std::string error;
		size_t line;
		size_t pos;
		InnerCompilationError(std::string const& error, size_t line, size_t pos)
			: error(error), line(line), pos(pos) {}
	};
}
#include <list>
namespace ric {
	enum class TokenType {
		unknown,
		semicolon,
		comma,
		arithmetic,
		block,
		index,
		bracket,
		reserved,
		datatype,
		library,
		identificator,
		number,
		color_literal,
		object,
		namespace_
	};
	struct Token {
		TokenType type;
		std::string value;
		size_t line, pos;
		Token(TokenType type, std::string value, size_t line, size_t pos)
			: type(type), value(value), line(line), pos(pos) {}
		std::string* operator->() { return &value; }
		std::string const* operator->() const { return &value; }
	};
}
#include <memory>
namespace ric {
	struct Node : Token {
		std::shared_ptr<Node> left;
		std::shared_ptr<Node> right;
		Node(TokenType type, std::string const& value, size_t line, size_t pos, std::shared_ptr<Node> left = nullptr, std::shared_ptr<Node> right = nullptr)
			: Token(type, value, line, pos), left(left), right(right) {}
		Node(TokenType type, size_t line, size_t pos, std::shared_ptr<Node> left = nullptr, std::shared_ptr<Node> right = nullptr)
			: Token(type, "", line, pos), left(left), right(right) {}
		Node(Token const& token, std::shared_ptr<Node> left = nullptr, std::shared_ptr<Node> right = nullptr)
			: Node(token.type, token.value, token.line, token.pos, left, right) {}

		bool operator==(Node const& other) const {
			return value == other.value && type == other.type;
		}
		bool operator<(Node const& other) const {
			return value < other.value;
		}
	};
}
#include <list>
namespace ric {
	enum class DataType {
		palette, color, object, primitive
	};
	using Tree = std::shared_ptr<Node>;
}

namespace ric {
	struct use_parameters {
		static bool double_precision; // = false.
		static bool alpha_colors; // = false.
	};
	double number(std::string const& s);
	std::string number(double const& s);
}

#include <vector>
#include <map>
namespace ric {
	class AbstactObject {
	private:
		bool const m_is_virtual;
	public:
		AbstactObject(bool is_virtual = false) : m_is_virtual(is_virtual) {}
		bool is_virtual() const { return m_is_virtual; }
	};
	struct Color : public AbstactObject {
		uint8_t data[4];
		Color(bool is_virtual = false, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 1) : data{r,g,b,a}, AbstactObject(is_virtual) {}
		uint8_t const* operator*() const { return data; }
		uint8_t* operator*() { return data; }
		uint8_t const& operator[](size_t index) const { return data[index]; }
		uint8_t& operator[](size_t index) { return data[index]; }
		uint8_t const& b() const { return data[0]; }
		uint8_t const& g() const { return data[1]; }
		uint8_t const& r() const { return data[2]; }
		uint8_t const& a() const { return data[3]; }
		uint8_t& b() { return data[0]; }
		uint8_t& g() { return data[1]; }
		uint8_t& r() { return data[2]; }
		uint8_t& a() { return data[3]; }
	};
	struct Palette : public AbstactObject {
		std::vector<Color> data;
		using AbstactObject::AbstactObject;
		auto const& operator*() const { return data; }
		auto& operator*() { return data; }
		auto const* operator->() const { return &data; }
		auto* operator->() { return &data; }
	};

	enum class primitive_type {
		points, lines, line_strip, line_loop, triangles, triangle_strip, triangle_fan, quads, quad_strip, polygon
	};
	primitive_type convert_to_primitive_type(Tree tree);
	uint8_t convert(primitive_type const&);
	struct Primitive : public AbstactObject {
		primitive_type type;
		size_t vertices_per_instance;
		std::vector<double> data;
		Primitive(primitive_type type, size_t vertices_per_instance) : type(type), vertices_per_instance(vertices_per_instance), AbstactObject(true) {}
		auto const& operator*() const { return data; }
		auto& operator*() { return data; }
		auto const* operator->() const { return &data; }
		auto* operator->() { return &data; }
	};
	class Object : public AbstactObject {
		using ColorStorage = std::vector<Color>;
		using ColorIterator = size_t;
		using DataType = std::vector<std::pair<ColorIterator, Primitive>>;

		ColorStorage colors;
		ColorIterator current_color;
		DataType data;
	public:
		using AbstactObject::AbstactObject;
		void color(Color const& color) { colors.push_back(color); current_color = colors.size() - 1; }
		Color const& color() const { return colors[current_color]; }
		void primitive(Primitive const& primitive) { data.push_back(std::make_pair(current_color, primitive)); }
		auto const& operator*() const { return data; }
		auto const& operator->() const { return data; }
		auto const& operator[](ColorIterator const& index) const { return colors[index]; }
	};

	struct ObjectFile {
		std::map<std::string, Color> colors;
		std::map<std::string, Palette> palettes;
		std::map<std::string, Primitive> primitives;
		std::map<std::string, Object> objects;
	};
}