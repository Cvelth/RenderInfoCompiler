#pragma once
#include <string>
#include <istream>
#include <exception>
namespace ric {
	void compile(std::istream &source);

	namespace Exceptions {
		class CompilationError {
		public:
			std::string error;
			size_t line;
			size_t pos;
			CompilationError(std::string const& error, size_t line, size_t pos) 
				: error(error), line(line), pos(pos) {}
			std::string what() const;
		};
	}
}