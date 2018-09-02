#include "version.hpp"
#include <sstream>
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

namespace ric {
	std::list<Token> tokenize(std::string const& path);
	Syntax analyze(std::list<Token> const& tokens);
}
void ric::compile(std::string const& path) {
	std::list<ric::Token> tokens;
	Syntax syntax;
	try {
		tokens = tokenize(path);
		syntax = analyze(tokens);
	} catch (Exceptions::InnerCompilationError &e) {
		throw Exceptions::CompilationError(e, path);
	}
	return;
}