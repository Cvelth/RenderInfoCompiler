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
		library,
		identificator,
		number,
		color_literal,
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

namespace ric {
	struct use_parameters {
		static bool double_precision; // = false.
	};
}