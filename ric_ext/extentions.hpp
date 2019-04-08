#pragma once
#include <set>
#include <memory>
#include <algorithm>
#include "extention.hpp"
namespace ric {
	namespace Exceptions {
		class ExtentionError {
		public:
			std::string error;
			ExtentionError(std::string const& error) : error(error) {}
		};
	}
	class ExtentionManager {
	private:
		inline static std::set<std::unique_ptr<Extention>> enabled_extentions;
	public:
		static std::set<std::unique_ptr<Extention>> const& get() { return enabled_extentions; }
		static bool add(std::unique_ptr<Extention> &&ext) {
			if (std::find_if(enabled_extentions.begin(), enabled_extentions.end(), [&ext](std::unique_ptr<Extention> const& o) {return ext->name() == o->name(); }) != enabled_extentions.end())
				return false;
			enabled_extentions.insert(std::move(ext));
			return true;
		}
		static bool add(std::string const& name);
		static std::set<std::string> const names();
	};
}