#pragma once
#include "extention.hpp"
class Transformations : public Extention {
public:
	virtual std::string const name() const override {
		return "Transformations";
	}
	virtual std::set<std::string> const names() const override {
		return {"translate", "scale", "rotate"};
	}
};