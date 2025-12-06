#include "triangle_mesh.h"

namespace feather {

TriangleMesh::TriangleMesh(std::vector<Vertex> vertices, std::vector<Index> indices)
		: _vertices { vertices.begin(), vertices.end() }
		, _indices { indices.begin(), indices.end() } {}

const std::vector<Vertex>& TriangleMesh::get_vertices() const { return _vertices; }

const std::vector<Index>& TriangleMesh::get_indices() const { return _indices; }

} //namespace feather