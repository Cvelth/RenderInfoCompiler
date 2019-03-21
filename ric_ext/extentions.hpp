#pragma once
#include <set>
#include "extention.hpp"
class ExtentionManager {
private:
	inline static std::set<Extention> enabled_extentions;
public:
	static std::set<Extention> const& get() { return enabled_extentions; }
	static bool add(Extention const& ext) {
		if (enabled_extentions.find(ext) != enabled_extentions.end())
			return false;
		enabled_extentions.insert(ext);
		return true;
	}
	static bool add(Extention &&ext) {
		if (enabled_extentions.find(ext) != enabled_extentions.end())
			return false;
		enabled_extentions.insert(ext);
		return true;
	}
};

namespace use_extention {
	void primitives();
	void transformations();
}