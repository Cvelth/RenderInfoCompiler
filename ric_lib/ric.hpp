#pragma once
#include <istream>
namespace ric {
	class RenderInfoCompiler {
	public:
		void compile(std::istream &source);
	};
}