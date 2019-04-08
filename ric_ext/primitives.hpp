#pragma once
#include "extention.hpp"
namespace ric {
	class Primitives : public Extention {
	public:
		virtual std::string const name() const override {
			return "Primitives";
		}
		virtual std::set<std::string> const names() const override {
			return {"ellipse", "circle", "rectangle", "square"};
		}
	};
}