#include "shared.hpp"
namespace ric {
	Tree parse_tree(std::list<Tree> source);

	std::list<Tree> convert_to_nodes(std::list<Token> const& source) {
		std::list<Tree> ret;
		for (auto it : source)
			ret.push_back(std::make_shared<Node>(it));
		return ret;
	}

	inline bool check_type(TokenType const& type) {
		return type == TokenType::block || type == TokenType::index || type == TokenType::bracket;
	}
	std::list<Tree> parse_brackets(std::list<Tree> source, TokenType current = TokenType::unknown) {
		for (auto it = source.begin(); it != source.end(); it++) {
			if (check_type((*it)->type))
				if ((*it)->value == "(" || (*it)->value == "[" || (*it)->value == "{") {
					auto temp = it; temp++;
					auto nodes = parse_brackets({temp, source.end()}, (*it)->type);
					source = {source.begin(), it};
					source.insert(source.end(), nodes.begin(), nodes.end());
					it = source.begin();
				} else if (check_type(current) && ((*it)->value == ")" || (*it)->value == "]" || (*it)->value == "}")) {
					Tree ret;
					if ((*it)->value == ")" && current == TokenType::bracket)
						ret = std::make_shared<Node>(TokenType::bracket, "", (*it)->line, (*it)->pos);
					else if ((*it)->value == "}" && current == TokenType::block)
						ret = std::make_shared<Node>(TokenType::block, "", (*it)->line, (*it)->pos);
					else if ((*it)->value == "]" && current == TokenType::index)
						ret = std::make_shared<Node>(TokenType::index, "", (*it)->line, (*it)->pos);
					else
						throw Exceptions::InnerCompilationError("Strange error while parsing brackets that should have never happened.", (*it)->line, (*it)->pos);

					if (it == source.begin())
						ret->right = nullptr;
					else
						ret->right = parse_tree(std::list<Tree>{ source.begin(), it });

					source = {++it, source.end()};
					source.push_front(ret);
					return source;
				}
		}
		return source;
	}

	Tree parse_objects(std::list<Tree> source) {
		if (source.size() == 0)
			return nullptr;
		else if (source.size() == 1)
			return source.front();
		else {
			for (auto it = source.begin(); it != source.end(); it++) {
				if ((*it)->type == TokenType::semicolon) {
					auto ret = it;
					if (it != source.begin())
						(*it)->left = parse_tree(std::list<Tree>{source.begin(), it});
					if (it != --source.end())
						(*it)->right = parse_tree(std::list<Tree>{++it, source.end()});
					return *ret;
				}
			}
			for (auto it = source.begin(); it != source.end(); it++) {
				if ((*it)->type == TokenType::comma) {
					auto ret = it;
					if (it != source.begin())
						(*it)->left = parse_tree(std::list<Tree>{source.begin(), it});
					if (it != --source.end())
						(*it)->right = parse_tree(std::list<Tree>{++it, source.end()});
					return *ret;
				}
			}
			for (auto it = source.begin(); it != source.end(); it++)
				if ((*it)->type == TokenType::arithmetic && (*it)->value != ",") {
					auto ret = it;
					(*it)->left = parse_tree(std::list<Tree>{source.begin(), it});
					(*it)->right = parse_tree(std::list<Tree>{++it, source.end()});
					return *ret;
				} else if ((*it)->type == TokenType::reserved && (*it)->value == "virtual") {
					if (auto temp = it++; (*it)->type == TokenType::datatype) {
						(*it)->left = *temp;
						source.erase(temp);
					}
				} else {

				}
		}
		if (source.size() == 4) {
			Tree t[4];
			size_t i = 0;
			for (auto it : source)
				t[i++] = it;
			if (t[0]->type == TokenType::datatype && t[1]->type == TokenType::identificator
				&& t[2]->type == TokenType::index && t[3]->type == TokenType::block
				&& t[2]->right->type == TokenType::identificator) 
			{
				t[2]->left = t[0];
				t[1]->left = t[2];
				t[1]->right = t[3]->right;
				t[1]->type = TokenType::object;
				return t[1];
			}
		}
		if (source.size() == 3)
			if (source.front()->type == TokenType::datatype && source.back()->type == TokenType::block) {
				if ((*++source.begin())->type == TokenType::identificator) {
					auto ret = *++source.begin();
					ret->left = source.front();
					ret->right = (*--source.end())->right;
					ret->type = TokenType::object;
					return ret;
				} else if ((*++source.begin())->type == TokenType::bracket) {
					(*++source.begin())->left = source.front();
					return std::make_shared<Node>(TokenType::object, (*++source.begin())->line, (*++source.begin())->pos,
												  *++source.begin(), source.back()->right);
				}
			} else if (source.front()->type == TokenType::reserved && source.front()->value == "namespace"
					   && (*++source.begin())->type == TokenType::identificator && source.back()->type == TokenType::block) {
				auto ret = *++source.begin();
				ret->right = source.back()->right;
				ret->type = TokenType::namespace_;
				return ret;
			}
		if (source.size() == 2)
			if (source.front()->type == TokenType::library && source.back()->type == TokenType::bracket) {
				source.front()->right = source.back()->right;
				return source.front();
			} else if (source.front()->type == TokenType::identificator && source.back()->type == TokenType::index) {
				source.back()->left = source.front();
				return source.front();
			} else if (source.front()->type == TokenType::datatype && source.back()->type == TokenType::identificator) {
				source.back()->left = source.front();
				source.back()->type = TokenType::object;
				return source.back();
			}
		throw Exceptions::InnerCompilationError("Undocumentated error.", source.front()->line, source.front()->pos);
	}

	Tree parse_tree(std::list<Tree> source) {
		if (source.size() != 0)
			return parse_objects(parse_brackets(source));
		else
			return nullptr;
	}

	void successful_simplification(Tree tree, double value) {
		tree->type = TokenType::number;
		tree->left = nullptr;
		tree->right = nullptr;
		tree->value = number(value);
	}

	void simplify_number(Tree tree) {
		if (tree->value.size() != 1)
			throw Exceptions::InnerCompilationError("Broken operator" + tree->value, tree->line, tree->pos);
		if (!tree->right)
			throw Exceptions::InnerCompilationError("There is nothing to the right of operator" + tree->value, tree->line, tree->pos);
		else if (tree->right->type != TokenType::number)
			return;
		auto right = number(tree->right->value);
		if (!tree->left)
			switch (tree->value[0]) {
				case '+': return successful_simplification(tree, +right);
				case '-': return successful_simplification(tree, -right);
				default:  throw Exceptions::InnerCompilationError("There is nothing to the left of operator" + tree->value, tree->line, tree->pos);
			} 
		else if (tree->left->type != TokenType::number)
			return;
		auto left = number(tree->left->value);
		switch (tree->value[0]) {
			case '+': return successful_simplification(tree, left + right);
			case '-': return successful_simplification(tree, left - right);
			case '*': return successful_simplification(tree, left * right);
			case '/': return successful_simplification(tree, left / right); 
			case '=': throw Exceptions::InnerCompilationError("Rvalue at the left side of operator=.", tree->line, tree->pos);
			default:  throw Exceptions::InnerCompilationError("Unsupported operator" + tree->value, tree->line, tree->pos);
		}
	}

	Tree clean_tree(Tree &tree) {
		if (tree->type == TokenType::semicolon)
			if (!tree->left)
				if (!tree->right)
					tree = nullptr;
				else
					tree = tree->right;
			else
				if (!tree->right)
					tree = tree->left;

		if (tree->type == TokenType::comma)
			if (!(tree->left && tree->right))
				throw Exceptions::InnerCompilationError("Unexpected comma.", tree->line, tree->pos);

		if (tree->type == TokenType::arithmetic)
			simplify_number(tree);

		if (tree->left)
			clean_tree(tree->left);
		if (tree->right)
			clean_tree(tree->right);
		return tree;
	}

	Tree analyze(std::list<Token> const& tokens) {
		Tree g;
		return clean_tree(g = parse_tree(convert_to_nodes(tokens)));
	}
}