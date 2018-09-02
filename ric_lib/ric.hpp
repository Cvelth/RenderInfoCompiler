#pragma once
#include <string>
#include <exception>
namespace ric {
	void compile(std::string const& path);

	namespace Exceptions {
		class InnerCompilationError;

		class CompilationError {
		public:
			std::string error;
			size_t line;
			size_t pos;
			std::string file;
			CompilationError(std::string const& error, size_t line, size_t pos, std::string const& file);
			CompilationError(InnerCompilationError const& error, std::string const& file);
			std::string what() const;
		};
	}
}