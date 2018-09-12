#include "shared.hpp"
ric::Primitive ric::library::ellipse(double aspect_ratio, bool is_filled, size_t points, size_t numbers_per_vertex) {
	//To be implemeted.
}
ric::Primitive ric::library::circle(bool is_filled, size_t points, size_t numbers_per_vertex) {
	return ellipse(1.0, is_filled, points, numbers_per_vertex);
}
ric::Primitive ric::library::rectangle(double aspect_ratio, bool is_filled, size_t numbers_per_vertex) {
	//To be implemented.
}
ric::Primitive ric::library::square(bool is_filled, size_t numbers_per_vertex) {
	return rectangle(1.0, is_filled, numbers_per_vertex);
}