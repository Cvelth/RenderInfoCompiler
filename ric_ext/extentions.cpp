#include "extentions.hpp"

#include "primitives.hpp"
void use_extention::primitives() {
	ExtentionManager::add(std::make_unique<Primitives>());
}

#include "transformations.hpp"
void use_extention::transformations() {
	ExtentionManager::add(std::make_unique<Transformations>());
}
