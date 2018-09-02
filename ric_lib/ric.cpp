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
}
void ric::compile(std::string const& path) {
	std::list<ric::Token> tokens;
	try {
		tokens = tokenize(path);
	} catch (Exceptions::InnerCompilationError &e) {
		throw Exceptions::CompilationError(e, path);
	}
	return;
}