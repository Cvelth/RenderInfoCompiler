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
		new_line,
		identificator,
		number,
		color_literal,
		directive,
		keyword,
		datatype,
		arithmetic,
		block,
		index,
		args,
		comma,
		file,
		extention,
		object,
		namespace_
	};
	struct Token {
		TokenType type;
		std::string value;
		size_t line, pos;
		Token(TokenType type, std::string value, size_t line, size_t pos)
			: type(type), value(value), line(line), pos(pos) {}
		Token(std::string value, size_t line, size_t pos) 
			: Token(TokenType::unknown, value, line, pos) {}
		std::string* operator->() { return &value; }
		std::string const* operator->() const { return &value; }
	};
}

#include <map>
namespace ric {
	extern std::map<std::string, bool> mode_parameters;
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
	using Tree = std::shared_ptr<Node>;
}

/*
#include <list>
namespace ric {
	enum class DataType {
		palette, color, object, primitive
	};
}

namespace mml { class transformation3d; }
namespace ric::library {
	Primitive ellipse(double aspect_ratio, bool is_filled, size_t points = 30, size_t numbers_per_vertex = 2);
	Primitive circle(bool is_filled, size_t points = 30, size_t numbers_per_vertex = 2);
	Primitive rectangle(double aspect_ratio, bool is_filled, size_t numbers_per_vertex = 2);
	Primitive square(bool is_filled, size_t numbers_per_vertex = 2);

	std::unique_ptr<mml::transformation3d> translation(double x, double y, double z);
	std::unique_ptr<mml::transformation3d> rotation(double a, double x, double y, double z);
	std::unique_ptr<mml::transformation3d> scaling(double x, double y, double z);
}
*/