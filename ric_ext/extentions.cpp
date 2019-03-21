#include "extentions.hpp"

#include "primitives.hpp"
void use_extention::primitives() {
	ExtentionManager::add(Primitives());
}

#include "transformations.hpp"
void use_extention::transformations() {
	ExtentionManager::add(Transformations());
}
