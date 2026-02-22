#include "mesh_data.h"

namespace feather {

MeshData::MeshData(std::vector<Vertex> vertices, std::vector<Index> indices)
		: _vertices { vertices.begin(), vertices.end() }
		, _indices { indices.begin(), indices.end() } {}

const std::vector<Vertex>& MeshData::get_vertices() const { return _vertices; }

const std::vector<Index>& MeshData::get_indices() const { return _indices; }

} //namespace feather