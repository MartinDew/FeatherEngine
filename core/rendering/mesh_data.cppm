module;

#include <array>
#include <vector>

export module feather.core:rendering.mesh_data;

import :framework.cow_vector;
import :math.math_defs;

export namespace feather {

using Index = uint32_t;

class MeshData {
public:
	MeshData() = default;
	MeshData(std::vector<Vertex> vertices, std::vector<Index> indices);

	const CowVector<Vertex>& get_vertices() const;
	const CowVector<Index>& get_indices() const;

	void set_vertices(const CowVector<Vertex>& vertices);
	void set_indices(const CowVector<Index>& indices);

protected:
	CowVector<Vertex> _vertices {};
	CowVector<Index> _indices {};
};

} //namespace feather
