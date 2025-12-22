#pragma once

#include "framework/high_level_array.h"
#include "resource.h"
#include <core/framework/reflection_macros.h>
#include <core/rendering/triangle_mesh.h>

#include <memory>

namespace feather {

class Mesh : public Resource {
	FCLASS(Mesh, Resource);

protected:
	std::shared_ptr<TriangleMesh> _triangle_mesh;

	Mesh() = default;

	static void _bind_members();
};

// Mesh using raw vertices and indices data
class RawMesh : public Mesh {
	FCLASS(RawMesh, Mesh);

protected:
	static void _bind_members();

public:
	RawMesh() = default;

	void add_vertices(const std::vector<Vertex>& vertices);
	void add_indices(const std::vector<Index>& indices);

	const CowVector<Vertex>& get_vertices() const;
	const CowVector<Index>& get_indices() const;
};

} // namespace feather