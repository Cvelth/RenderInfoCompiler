#include "shared.hpp"
#include <cmath>
#define M_PI 3.14159265358979323846

ric::Primitive ric::library::ellipse(double aspect_ratio, bool is_filled, size_t points, size_t numbers_per_vertex) {
	Primitive ret(is_filled ? primitive_type::triangle_fan : primitive_type::line_strip, numbers_per_vertex);

	double const step = M_PI / points * 2;
	if (aspect_ratio < 1.0 && aspect_ratio > 0.0) {
		for (double f = 0.0; f < M_PI * 2.0; f += step) {
			ret->push_back(aspect_ratio * cos(f));
			ret->push_back(sin(f));
		}
		ret->push_back(aspect_ratio * cos(0.0));
		ret->push_back(sin(0.0));
	} else if (aspect_ratio >= 1.0) {
		for (double f = 0.0; f < M_PI * 2.0; f += step) {
			ret->push_back(cos(f));
			ret->push_back(aspect_ratio * sin(f));
		}
		ret->push_back(cos(0.0));
		ret->push_back(aspect_ratio * sin(0.0));
	} else 
		throw Exceptions::LibraryCompilationError("Unsupported aspect_ratio value.");

	return ret;
}
ric::Primitive ric::library::circle(bool is_filled, size_t points, size_t numbers_per_vertex) {
	return ellipse(1.0, is_filled, points, numbers_per_vertex);
}
ric::Primitive ric::library::rectangle(double aspect_ratio, bool is_filled, size_t numbers_per_vertex) {
	Primitive ret(is_filled ? primitive_type::triangle_strip : primitive_type::line_loop, numbers_per_vertex);

	if (aspect_ratio < 1.0 && aspect_ratio > 0.0) {
		ret->push_back(-aspect_ratio); ret->push_back(-1.f);
		ret->push_back(+aspect_ratio); ret->push_back(-1.f);
		ret->push_back(-aspect_ratio); ret->push_back(+1.f);
		ret->push_back(+aspect_ratio); ret->push_back(+1.f);
	} else if (aspect_ratio >= 1.0) {
		ret->push_back(-1.f); ret->push_back(-1.f / aspect_ratio);
		ret->push_back(-1.f); ret->push_back(+1.f / aspect_ratio);
		ret->push_back(+1.f); ret->push_back(-1.f / aspect_ratio);
		ret->push_back(+1.f); ret->push_back(+1.f / aspect_ratio);
	} else
		throw Exceptions::LibraryCompilationError("Unsupported aspect_ratio value.");

	return ret;
}
ric::Primitive ric::library::square(bool is_filled, size_t numbers_per_vertex) {
	return rectangle(1.0, is_filled, numbers_per_vertex);
}

#include "mgl/math/transformation.hpp"
std::unique_ptr<mgl::math::transformation3d> ric::library::translation(double x, double y, double z) {
	auto ret = mgl::math::translation<double>(mgl::math::basic_vector<double, 3u>{x, y, z});
	return std::unique_ptr<mgl::math::transformation3d>(new mgl::math::transformation3d(ret));
}
std::unique_ptr<mgl::math::transformation3d> ric::library::rotation(double a, double x, double y, double z) {
	auto ret = mgl::math::rotation<double>(a, mgl::math::basic_vector<double, 3u>{x, y, z});
	return std::unique_ptr<mgl::math::transformation3d>(new mgl::math::transformation3d(ret));
}
std::unique_ptr<mgl::math::transformation3d> ric::library::scaling(double x, double y, double z) {
	auto ret = mgl::math::scaling<double>(mgl::math::basic_vector<double, 3u>{x, y, z});
	return std::unique_ptr<mgl::math::transformation3d>(new mgl::math::transformation3d(ret));
}