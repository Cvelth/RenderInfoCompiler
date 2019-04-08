#include "extentions.hpp"
#include "primitives.hpp"
#include "transformations.hpp"
bool ric::ExtentionManager::add(std::string const& name) {
	if (name == "primitives")
		return ExtentionManager::add(std::make_unique<Primitives>());
	else if (name == "transformations")
		return ExtentionManager::add(std::make_unique<Transformations>());
	else
		throw Exceptions::ExtentionError("Unsuported Extention");
}
std::set<std::string> const ric::ExtentionManager::names() {
	std::set<std::string> ret;
	for (auto &ext : enabled_extentions) {
		auto &temp = ext->names();
		ret.insert(temp.cbegin(), temp.cend());
	}
	return ret;
}