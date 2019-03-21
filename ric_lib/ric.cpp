#include "version.hpp"
#include <sstream>
#include <fstream>
std::string ric::get_version() {
	std::ostringstream s;
	s << LibraryName << " v" << Version_Major << '.' << Version_Minor << '.' << Version_Patch << '(' << Version_Build << ')';
	return s.str();
}

#include "ric.hpp"
#include "shared.hpp"
ric::Exceptions::CompilationError::CompilationError(std::string const& error, size_t line, size_t pos, std::string const& file)
	: error(error), line(line), pos(pos), file(file) {}
ric::Exceptions::CompilationError::CompilationError(InnerCompilationError const& _error, std::string const& file)
	: error(_error.error), line(_error.line), pos(_error.pos), file(file) {}
std::string ric::Exceptions::CompilationError::what() const {
	std::ostringstream s;
	s << "Error compiling " << file << "\n\t\t" 
		<< "at position " << pos << " of line " << line << ":\n    "
		<< error << "\n";
	return s.str();
}

/*
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
*/
namespace ric {
	std::list<Token> tokenize(std::string const& path);
	//Tree analyze(std::list<Token> const& tokens);
	//void generate(std::ostream &f, Tree tree);
}
void ric::compile(std::string const& path) {
	std::list<ric::Token> tokens;
	Tree tree;
	//try {
		tokens = tokenize(path);
		//tree = analyze(tokens);
		//std::ofstream obj_file;
		//auto pos = path.find_last_of('.');
		//if (pos == path.size())
		//	throw Exceptions::CompilationError("Error determining object file name. Make sure source file isn't corrupted and is properly named.", 0, 0, path);
		//obj_file.open(path.substr(0, pos) + ".rio", std::ios::binary | std::ios::out);
		//if (!obj_file)
		//	throw Exceptions::CompilationError("Unable to open object file for writing. There is a possibility it's already in use.", 0, 0, path);
		//generate(obj_file, tree);
	//} catch (Exceptions::InnerCompilationError &e) {
	//	throw Exceptions::CompilationError(e, path);
	//}
	return;
}
