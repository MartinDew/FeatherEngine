export module feather.reflection:mesh_data;

import std.compat;
import feather.framework;
import feather.math;

export namespace feather {

using Index = uint32_t;

class FEATHER_REFLECTION_API MeshData {
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
