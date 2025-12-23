#pragma once

#include <math/math_defs.h>
#include <array>
#include <vector>

namespace feather {

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