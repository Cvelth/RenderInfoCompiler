#pragma once
#include <set>
namespace ric {
	class Extention {
	public:
		virtual std::string const name() const abstract;
		virtual std::set<std::string> const names() const abstract;

		bool operator==(Extention const& other) const {
			return name() == other.name();
		}
		bool operator<(Extention const& other) const {
			return name() < other.name();
		}

		virtual ~Extention() {}
	};
}