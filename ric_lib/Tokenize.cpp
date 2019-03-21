#include "shared.hpp"
#include "ric.hpp"

#include <set>
const std::set<char> s_tokens{';', ',', '{', '}', '[', ']', '(', ')', '/', '*', '+', '-', '='};
const std::set<std::string> directives {
	"enable", "disable", "include", "use"
};
const std::set<std::string> keywords {
	"virtual", "namespace"
};
const std::set<std::string> datatypes {
	"palette", "color", "object", "primitive"
};
#include "ric_ext/extentions.hpp"

std::string current_path = "";

std::map<std::string, bool> ric::mode_parameters = {
	std::pair("double_precision", false),
	std::pair("color_alpha", false)
};

#include <sstream>
namespace ric {
	void parse_line(std::list<Token> &tokens, bool &is_commentary, std::string line, size_t c) {
		size_t p_shift = 0;
		for (int i = 0; i < line.size() - 1; i++)
			if (!is_commentary && line.at(i) == '/' && line.at(i + 1) == '*') {
				tokens.emplace_back(line.substr(0, i), c, p_shift);
				is_commentary = true; i++;
			} else if (is_commentary && line.at(i) == '*' && line.at(i + 1) == '/') {
				line = line.substr(i + 2);
				p_shift = i + 2; i = 0;
				is_commentary = false;
				if (line.size() == 0)
					return;
			} else if (!is_commentary && line.at(i) == '/' && line.at(i + 1) == '/') {
				line = line.substr(0, i);
				if (line.size() == 0)
					return;
			}
			if (!is_commentary && line.size() != 0)
				tokens.emplace_back(line, c, p_shift);
	}

	bool split_tokens(std::list<Token> &tokens, std::list<Token>::iterator &it, int i) {
		for (auto &s : {' ', '\t'})
			if (it->value.at(i) == s) {
				Token temp(it->value.substr(0, i), it->line, it->pos);
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
				case ',': it->type = TokenType::comma; return true;
				case '{': case '}': it->type = TokenType::block; return true;
				case '[': case ']': it->type = TokenType::index; return true;
				case '(': case ')': it->type = TokenType::args; return true;
	
				case '+': case '-': case '*': case '/': case '=':
					it->type = TokenType::arithmetic; return true;
			}
		return false;
	}
	bool is_reserved(std::list<Token>::iterator &it) {
		if (directives.find(it->value) != directives.end()) {
			it->type = TokenType::directive;
			return true;
		} else if (datatypes.find(it->value) != datatypes.end()) {
			it->type = TokenType::datatype;
			return true;
		} else if (keywords.find(it->value) != keywords.end()) {
			it->type = TokenType::keyword;
			return true;
		}
		return false;
	}
	bool is_number(std::string const& s) {
		bool has_point = false;
		for (int i = 0; i < s.size(); i++) {
			if (s[i] == '.')
				if (has_point)
					return false;
				else
					has_point = true;
			else if (!(isdigit(s[i])))
				return false;
		}
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
	bool is_color_literal(std::string const& s, size_t line, size_t pos) {
		if (s.size() < 2 || s[0] != '0' || s[1] != 'x')
			return false;
		if (s.size() > 10)
			throw Exceptions::InnerCompilationError("Color literal seems to be broken: " + s + "\n    "
													"Literal cannot consist of more than 8 digits.", line, pos);
		for (int i = 2; i < s.size(); i++)
			if ((s[i] < '0' || s[i] > '9') && (s[i] < 'A' || s[i] > 'F') && (s[i] < 'a' || s[i] > 'f'))
				throw Exceptions::InnerCompilationError("Color literal seems to be broken: " + s + "\n    '"
														+ s[i] + "' is not a valid hex-digit.", line, pos);
		return true;
	}
	bool is_color_literal(std::list<Token>::iterator &it) {
		bool ret = is_color_literal(it->value, it->line, it->pos);
		if (ret)
			it->type = TokenType::color_literal;
		return ret;
	}
	bool is_file_name(std::list<Token>::iterator &it) {
		if (it->value.size() > 2 && (it->value.front() == '\"' && it->value.back() == '\"')) {
			it->type = TokenType::file;
			it->value = it->value.substr(1, it->value.size() - 2);
			return true;
		} else
			return false;
	}
	bool is_extention_name(std::list<Token>::iterator &it) {
		if (it->value.size() > 2 && (it->value.front() == '<' && it->value.back() == '>')) {
			it->type = TokenType::extention;
			it->value = it->value.substr(1, it->value.size() - 2);
			return true;
		} else
			return false;
	}

	bool is_double_new_line(std::list<Token>::iterator &it, std::list<Token> &list) {
		if (it == --list.end())
			return false;
		if (it->type != TokenType::new_line)
			return false;
		auto temp = it; ++temp;
		return (temp)->type == TokenType::new_line;
	}
}

#include "ric_ext/extentions.hpp"
namespace ric {
	std::list<Token> tokenize(std::string const& path);
	std::list<Token> process_directive(std::list<Token> &tokens) {
		if (tokens.size() != 3)
			throw Exceptions::InnerCompilationError("Junk information at the end of '" + tokens.front().value + "'-directive.",
													(--tokens.end())->line, (--tokens.end())->pos);
		auto value = *(++tokens.begin());
		if (tokens.front().value == "enable") {
			if (value.type != TokenType::identificator)
				throw Exceptions::InnerCompilationError("Only parameter names are accepted by 'enable'-directive.",
														value.line, value.pos);
			ric::mode_parameters.insert_or_assign(value.value, true);
			return {};
		} else if (tokens.front().value == "disable") {
			if (value.type != TokenType::identificator)
				throw Exceptions::InnerCompilationError("Only parameter names are accepted by 'disable'-directive.",
														value.line, value.pos);
			ric::mode_parameters.insert_or_assign(value.value, false);
			return {};
		} else if (tokens.front().value == "include") {
			if(value.type != TokenType::file)
				throw Exceptions::InnerCompilationError("Only filenames are accepted by 'include'-directive.",
														value.line, value.pos);
			try {
				auto included = tokenize(current_path + '/' + value.value);
				return included;
			} catch (Exceptions::InnerCompilationError &e) {
				throw Exceptions::CompilationError(e, current_path + '/' + value.value);
			}

			return tokenize(current_path + '/' + value.value);
		} else if (tokens.front().value == "use") {
			if (value.type != TokenType::extention)
				throw Exceptions::InnerCompilationError("Only extention names are accepted by 'use'-directive.",
														value.line, value.pos);
			try {
				if (!ExtentionManager::add(value.value))
					throw Exceptions::InnerCompilationError("Extention cannot be used.",
					(--tokens.end())->line, (--tokens.end())->pos);
			} catch (Exceptions::ExtentionError &e) {
				throw Exceptions::InnerCompilationError("Extention error: " + e.error, value.line, value.pos);
			}
			return {};
		} else
			throw Exceptions::InnerCompilationError("Unsupported directive.",
													value.line, value.pos);
	}
	std::list<Token> process_directives(std::list<Token> &tokens) {
		for (auto it = tokens.begin(); it != tokens.end();) {
			if (it->type == TokenType::directive) {
				std::list<Token> params;
				auto it2 = it;
				while (it2->type != TokenType::new_line)
					params.push_back(*it2++);
				params.push_back(*it2++);
				auto res = process_directive(params);
				if (it != tokens.begin()) {
					auto del = it--;
					tokens.erase(del, it2);
				} else {
					tokens.erase(tokens.begin(), it2);
					it = tokens.begin();
				}
				tokens.insert(it, res.begin(), res.end());
			} else
				it++;
		}
		return tokens;
	}
}

#include <fstream>
namespace ric {
	std::list<Token> tokenize(std::string const& path) {
		std::ifstream source;
		source.open(path);
		if (!source)
			throw Exceptions::CompilationError("Unable to open file.", 0, 0, path);

		auto pos1 = path.find_last_of('/'), pos2 = path.find_last_of('\\');
		auto pos = std::max(pos1, pos2);
		if (pos != path.size())
			current_path = path.substr(0, pos);
		else
			current_path = "";

		std::list<Token> tokens;

		bool is_commentary = false;
		size_t line = 0;
		std::string temp;
		while (std::getline(source, temp)) {
			if (temp.size() != 0)
				parse_line(tokens, is_commentary, temp, line);
			line++;
		}
		if (is_commentary)
			throw Exceptions::InnerCompilationError("Looks like there is an unclosed commentary at the end of the file.",
													line, 0);

		for (auto it = ++tokens.begin(); it != tokens.end(); it++)
			tokens.insert(it, Token(TokenType::new_line, "", it->line, -1));
		tokens.emplace_back(TokenType::new_line, "", tokens.back().line, -1);
		for (auto it = tokens.begin(); it != tokens.end(); it++)
			if (it->type == TokenType::unknown)
				for (int i = 0; i < it->value.size(); i++)
					if (split_tokens(tokens, it, i))
						i = -1;

		for (auto it = tokens.begin(); it != tokens.end(); it++)
			if (it->value == ";")
				it->type = TokenType::new_line;
		
		for (auto it = tokens.begin(); it != tokens.end();) {
			if (it->type == TokenType::unknown && it->value.size() == 0)
				if (it != tokens.begin()) {
					auto del = it--;
					tokens.erase(del);
				} else {
					tokens.pop_front();
					it = tokens.begin();
				}
			else
				it++;
		}
		for (auto it = tokens.begin(); it != tokens.end();) {
			if (is_double_new_line(it, tokens))
				if (it != tokens.begin()) {
					auto del = it--;
					tokens.erase(del);
				} else {
					tokens.pop_front();
					it = tokens.begin();
				}
			else
				it++;
		}
		
		for (auto it = tokens.begin(); it != tokens.end(); it++)
			if (it->type == TokenType::unknown) {
				if (it->value.size() == 1 && is_operator(it))
					continue;
				else if (is_reserved(it))
					continue;
				else if (is_number(it))
					continue;
				else if (is_identificator(it))
					continue;
				else if (is_color_literal(it))
					continue;
				else if (is_file_name(it))
					continue;
				else if (is_extention_name(it))
					continue;
				else
					throw Exceptions::InnerCompilationError("Unknown Token: " + it->value, it->line, it->pos);
			}

		process_directives(tokens);
		
		return tokens;
	}
}