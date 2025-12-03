#pragma once

#include <math/math_defs.h>
#include <array>
#include <vector>

namespace feather {

struct Vertex {
	Vector3 position;
	Vector2 uv;

	Vertex() = default;
	constexpr Vertex(real_t x, real_t y, real_t z, real_t u, real_t v) : position(x, y, z), uv { u, v } {}
};

using Index = uint32_t;

class TriangleMesh {
public:
	TriangleMesh() = default;
	TriangleMesh(std::vector<Vertex> vertices, std::vector<Index> indices);

	const std::vector<Vertex>& get_vertices() const;
	const std::vector<Index>& get_indices() const;

protected:
	std::vector<Vertex> _vertices {};
	std::vector<Index> _indices {};
};

} //namespace feather