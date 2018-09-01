#include "version.hpp"
#include <sstream>
std::string ric::get_version() {
	std::ostringstream s;
	s << LibraryName << " v" << Version_Major << '.' << Version_Minor << '.' << Version_Patch << '(' << Version_Build << ')';
	return s.str();
}

#include "ric.hpp"
ric::RenderInfoCompiller::RenderInfoCompiller() {}
ric::RenderInfoCompiller::~RenderInfoCompiller() {}