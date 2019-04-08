#pragma once
#include <vector>
#include <map>
#include "shared.hpp"
namespace ric {
	class AbstactObject {
	private:
		bool const m_is_virtual;
	public:
		AbstactObject(bool is_virtual = false) : m_is_virtual(is_virtual) {}
		bool is_virtual() const { return m_is_virtual; }
	};
	struct Color : public AbstactObject {
		uint8_t data[4];
		Color(bool is_virtual = false, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 1) : data{r,g,b,a}, AbstactObject(is_virtual) {}
		Color& operator=(Color const& other) {
			for (int i = 0; i < 4; i++)
				data[i] = other.data[i];
			return *this;
		}
		uint8_t const* operator*() const { return data; }
		uint8_t* operator*() { return data; }
		uint8_t const& operator[](size_t index) const { return data[index]; }
		uint8_t& operator[](size_t index) { return data[index]; }
		uint8_t const& b() const { return data[0]; }
		uint8_t const& g() const { return data[1]; }
		uint8_t const& r() const { return data[2]; }
		uint8_t const& a() const { return data[3]; }
		uint8_t& b() { return data[0]; }
		uint8_t& g() { return data[1]; }
		uint8_t& r() { return data[2]; }
		uint8_t& a() { return data[3]; }
	};
	struct Palette : public AbstactObject {
		std::vector<Color> data;
		using AbstactObject::AbstactObject;
		auto const& operator*() const { return data; }
		auto& operator*() { return data; }
		auto const* operator->() const { return &data; }
		auto* operator->() { return &data; }
		Color const& operator[](size_t index) const { return data[index]; }
		Color& operator[](size_t index) { return data[index]; }
	};

	enum class primitive_type {
		points, lines, line_strip, line_loop, triangles, triangle_strip, triangle_fan, quads, quad_strip, polygon
	};
	primitive_type convert_to_primitive_type(Tree tree);
	uint8_t convert(primitive_type const&);
	struct Primitive : public AbstactObject {
		primitive_type type;
		size_t vertices_per_instance;
		std::vector<double> data;
		Primitive(primitive_type type, size_t vertices_per_instance) : type(type), vertices_per_instance(vertices_per_instance), AbstactObject(true) {}
		Primitive& operator=(Primitive const& other) {
			data.clear();
			type = other.type;
			vertices_per_instance = other.vertices_per_instance;
			data = other.data;
			return *this;
		}
		auto const& operator*() const { return data; }
		auto& operator*() { return data; }
		auto const* operator->() const { return &data; }
		auto* operator->() { return &data; }
	};
	class Object : public AbstactObject {
		using ColorStorage = std::vector<Color>;
		using ColorIterator = size_t;
		using DataType = std::vector<std::pair<ColorIterator, Primitive>>;

		ColorStorage colors;
		ColorIterator current_color;
		DataType data;
	public:
		using AbstactObject::AbstactObject;
		void color(Color const& color) { colors.push_back(color); current_color = colors.size() - 1; }
		Color const& color() const { return colors[current_color]; }
		void primitive(Primitive const& primitive) { data.push_back(std::make_pair(current_color, primitive)); }
		auto const& operator*() const { return data; }
		auto& operator*() { return data; }
		auto const& operator->() const { return data; }
		auto& operator->() { return data; }
		auto const& operator[](ColorIterator const& index) const { return colors[index]; }
		auto& operator[](ColorIterator const& index) { return colors[index]; }
	};

	struct ObjectMap {
		std::map<std::string, Color> colors;
		std::map<std::string, Palette> palettes;
		std::map<std::string, Primitive> primitives;
		std::map<std::string, Object> objects;

		void add(std::string name, Color color) { colors.emplace(name, color); }
		void add(std::string name, Palette palette) { palettes.emplace(name, palette); }
		void add(std::string name, Primitive primitive) { primitives.emplace(name, primitive); }
		void add(std::string name, Object object) { objects.emplace(name, object); }
	};
}