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

void ric::compile(std::istream &source) {
	std::list<Token> tokens;

	bool is_commentary = false;
	size_t c = 0;
	std::string temp;
	while (source) {
		std::getline(source, temp);
		if (temp.size() == 0) {
			c++; continue;
		}
		for (int i = 0; i < temp.size() - 1; i++)
			if (!is_commentary && temp.at(i) == '/' && temp.at(i + 1) == '*') {
				temp = temp.substr(0, i);
				is_commentary = true;
				if (temp.size() == 0)
					goto wout;
			} else if (is_commentary && temp.at(i) == '*' && temp.at(i + 1) == '/') {
				temp = temp.substr(i + 2);
				is_commentary = false;
				if (temp.size() == 0)
					goto wout;
			} else if (!is_commentary && temp.at(i) == '/' && temp.at(i + 1) == '/') {
				temp = temp.substr(0, i);
				if (temp.size() == 0)
					goto wout;
			}
		if (!is_commentary) {
			if (temp.at(0) == '#') {
				//handle precompiler directive.
			} else
				tokens.push_back(Token(TokenType::unknown, temp, c, 0));
		}
		wout: c++;
	}

	for (auto it = tokens.begin(); it != tokens.end(); it++)
		if (it->type == TokenType::unknown)
			for (int i = 0; i < it->value.size(); i++) {
				for (auto &s : separators)
					if (it->value.at(i) == s) {
						Token temp(TokenType::unknown, it->value.substr(0, i), it->line, it->pos);
						it->value = it->value.substr(i + 1);
						it->pos += i + 1;
						tokens.insert(it, temp);
						i = -1; goto out;
					}
				for (auto &s : s_tokens)
					if (it->value.at(i) == s) {
						Token temp(TokenType::unknown, it->value.substr(0, i), it->line, it->pos);
						Token s_temp(TokenType::unknown, it->value.substr(i, 1), it->line, it->pos + i);
						it->value = it->value.substr(i + 1);
						it->pos += i + 1;
						tokens.insert(it, temp);
						tokens.insert(it, s_temp);
						i = -1; goto out;
					}
				out: continue;
			}

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
		if (it->type == TokenType::unknown)
			if (it->value.size() == 1)
				switch (it->value[0]) {
					case ';': it->type = TokenType::semicolon; break;
					case ',': it->type = TokenType::comma; break;
					case '{': case '}': it->type = TokenType::block; break;
					case '[': case ']': it->type = TokenType::index; break;
				}
	for (auto it = tokens.begin(); it != tokens.end(); it++)
		if (it->type == TokenType::unknown)
			for (auto r : reserved)
				if (it->value == r)
					it->type = TokenType::reserved;

	for (auto it = tokens.begin(); it != tokens.end(); it++)
		if (it->type == TokenType::unknown) {
			bool number = true;
			for (int i = 0; i < it->value.size(); i++)
				if (!(isdigit(it->value[i]) || it->value[i] == '.')) {
					number = false; break;
				}
			if (number)
				it->type = TokenType::number;
		}

	for (auto it = tokens.begin(); it != tokens.end(); it++)
		if (it->type == TokenType::unknown) {
			bool identificator = it->value.size() > 0 && isalpha(it->value[0]);
			if (identificator)
				for (int i = 1; i < it->value.size(); i++)
					if (!(isalnum(it->value[i]) || it->value[i] == '_')) {
						identificator = false; break;
					}
			if (identificator)
				it->type = TokenType::identificator;
		}

	for (auto it = tokens.begin(); it != tokens.end(); it++)
		if (it->type == TokenType::unknown) {
			bool literal = it->value.size() > 1 && it->value[0] == '0' && it->value[1] == 'x';
			if (literal)
				for (int i = 1; i < it->value.size(); i++)
					if (!(isalnum(it->value[i]) || it->value[i] == '.')) {
						literal = false; break;
					}
			if (literal)
				it->type = TokenType::color_literal;
		}
}