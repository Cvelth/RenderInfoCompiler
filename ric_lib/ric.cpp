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

bool ric::use_parameters::double_precision = false;

double ric::number(std::string const& s) {
	std::istringstream iss(s);
	double r;
	iss >> r;
	return r;
}
std::string ric::number(double const& s) {
	std::ostringstream oss;
	oss << s;
	return oss.str();
}

namespace ric {
	std::list<Token> tokenize(std::string const& path);
	Tree analyze(std::list<Token> const& tokens);
	void generate(std::ostream &f, Tree tree);
}
void ric::compile(std::string const& path) {
	std::list<ric::Token> tokens;
	Tree tree;
	try {
		tokens = tokenize(path);
		tree = analyze(tokens);
		std::ofstream obj_file;
		auto pos = path.find_last_of('.');
		if (pos == path.size())
			throw Exceptions::CompilationError("Error determining object file name. Make sure source file isn't corrupted and is prprly named.", 0, 0, path);
		obj_file.open(path.substr(0, pos) + ".rio", std::ios::binary | std::ios::out);
		if (!obj_file)
			throw Exceptions::CompilationError("Unable to open object file for writing. It's possibly already in use.", 0, 0, path);
		generate(obj_file, tree);
	} catch (Exceptions::InnerCompilationError &e) {
		throw Exceptions::CompilationError(e, path);
	}
	return;
}
