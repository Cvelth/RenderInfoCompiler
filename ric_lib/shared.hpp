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
	};
	double number(std::string const& s);
	std::string number(double const& s);
}