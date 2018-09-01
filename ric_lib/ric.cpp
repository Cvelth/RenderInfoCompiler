#include "version.hpp"
#include <sstream>
std::string ric::get_version() {
	std::ostringstream s;
	s << LibraryName << " v" << Version_Major << '.' << Version_Minor << '.' << Version_Patch << '(' << Version_Build << ')';
	return s.str();
}

#include "ric.hpp"
#include <list>
namespace ric {
	enum class TokenType {
		unknown,
		semicolon,
		comma,
		block,
		index,
		reserved,
		identificator,
		number,
		color_literal
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
const std::string separators = " \t";
const std::string s_tokens = ";,{}[]/*=";
const std::list<std::string> reserved = {
	"palette", "color", "object", "primitive"
};

namespace ric {
	void parse_line(std::list<Token> &tokens, bool &is_commentary, std::string line, size_t c) {
		for (int i = 0; i < line.size() - 1; i++)
			if (!is_commentary && line.at(i) == '/' && line.at(i + 1) == '*') {
				line = line.substr(0, i);
				is_commentary = true;
				if (line.size() == 0)
					return;
			} else if (is_commentary && line.at(i) == '*' && line.at(i + 1) == '/') {
				line = line.substr(i + 2);
				is_commentary = false;
				if (line.size() == 0)
					return;
			} else if (!is_commentary && line.at(i) == '/' && line.at(i + 1) == '/') {
				line = line.substr(0, i);
				if (line.size() == 0)
					return;
			}
			if (!is_commentary) {
				if (line.at(0) == '#') {
					//handle precompiler directive.
				} else
					tokens.push_back(Token(TokenType::unknown, line, c, 0));
			}
	}

	bool split_tokens(std::list<Token> &tokens, std::list<Token>::iterator &it, int i) {
		for (auto &s : separators)
			if (it->value.at(i) == s) {
				Token temp(TokenType::unknown, it->value.substr(0, i), it->line, it->pos);
				it->value = it->value.substr(i + 1);
				it->pos += i + 1;
				tokens.insert(it, temp);
				return true;
			}
		for (auto &s : s_tokens)
			if (it->value.at(i) == s) {
				Token temp(TokenType::unknown, it->value.substr(0, i), it->line, it->pos);
				Token s_temp(TokenType::unknown, it->value.substr(i, 1), it->line, it->pos + i);
				it->value = it->value.substr(i + 1);
				it->pos += i + 1;
				tokens.insert(it, temp);
				tokens.insert(it, s_temp);
				return true;
			}
		return false;
	}

	bool is_operator(std::list<Token>::iterator &it) {
		if (it->value.size() == 1)
			switch (it->value[0]) {
				case ';': it->type = TokenType::semicolon; return true;
				case ',': it->type = TokenType::comma; return true;
				case '{': case '}': it->type = TokenType::block; return true;
				case '[': case ']': it->type = TokenType::index; return true;
			}
		return false;
	}
	bool is_reserved(std::list<Token>::iterator &it) {
		for (auto r : reserved)
			if (it->value == r) {
				it->type = TokenType::reserved;
				return true;
			}
		return false;
	}
	bool is_number(std::string const& s) {
		for (int i = 0; i < s.size(); i++)
			if (!(isdigit(s[i]) || s[i] == '.'))
				return false;
		return true;
	}
	bool is_number(std::list<Token>::iterator &it) {
		bool ret = is_number(it->value);
		if (ret) it->type = TokenType::number;
		return ret;
	}
	bool is_identificator(std::string const& s) {
		if (s.size() == 0 || !isalpha(s[0]))
			return false;
		for (int i = 1; i < s.size(); i++)
			if (!(isalnum(s[i]) || s[i] == '_'))
				return false;
		return true;
	}
	bool is_identificator(std::list<Token>::iterator &it) {
		bool ret = is_identificator(it->value);
		if (ret) it->type = TokenType::identificator;
		return ret;
	}
	bool is_color_literal(std::string const& s) {
		if (s.size() < 2 || s[0] != '0' || s[1] != 'x')
			return false;
		for (int i = 1; i < s.size(); i++)
			if (!(isalnum(s[i])))
				return false;
		return true;
	}
	bool is_color_literal(std::list<Token>::iterator &it) {
		bool ret = is_color_literal(it->value);
		if (ret)
			it->type = TokenType::color_literal;
		return ret;
	}

	std::list<Token> tokenize(std::istream &source) {
		std::list<Token> tokens;

		bool is_commentary = false;
		size_t c = 0;
		std::string temp;
		while (std::getline(source, temp))
			if (temp.size() != 0)
				parse_line(tokens, is_commentary, temp, c++);

		for (auto it = tokens.begin(); it != tokens.end(); it++)
			if (it->type == TokenType::unknown)
				for (int i = 0; i < it->value.size(); i++)
					if (split_tokens(tokens, it, i))
						i = -1;

		for (auto it = tokens.begin(); it != tokens.end();) {
			if (it->type == TokenType::unknown)
				if (it->value.size() == 0)
					if (it != tokens.begin()) {
						auto del = it--;
						tokens.erase(del);
					} else {
						tokens.pop_front();
						it = tokens.begin();
						continue;
					}
			it++;
		}

		for (auto it = tokens.begin(); it != tokens.end(); it++)
			if (it->type == TokenType::unknown) {
				if (it->value.size() == 1)
					if (is_operator(it))
						continue;
				if (is_reserved(it))
					continue;
				if (is_number(it))
					continue;
				if (is_identificator(it))
					continue;
				if (is_color_literal(it))
					continue;
			}
		return tokens;
	}
}

void ric::compile(std::istream &source) {
	auto tokens = tokenize(source);
}