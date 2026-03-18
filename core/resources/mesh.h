#pragma once

#include "framework/variant_array.h"
#include "math/math_defs.h"
#include "resource.h"
#include <core/framework/reflection_macros.h>
#include <core/rendering/mesh_data.h>

#include <memory>

namespace feather {

class Mesh : public Resource {
	FCLASS(Mesh, Resource);

protected:
	std::shared_ptr<MeshData> _triangle_mesh;

	Mesh() = default;
	explicit Mesh(const std::shared_ptr<MeshData>& triangle_mesh);

	static void _bind_members();

public:
	const std::shared_ptr<MeshData>& get_triangle_mesh() { return _triangle_mesh; };
};

// Mesh using raw vertices and indices data
class ComplexMesh : public Mesh {
	FCLASS(ComplexMesh, Mesh);

protected:
	static void _bind_members();

public:
	ComplexMesh() = default;

	void add_vertices(const VariantArray vertices);
	void add_indices(const VariantArray indices);

	VariantArray get_vertices() const;
	VariantArray get_indices() const;
};

class BoxMesh : public Mesh {
	FCLASS(ComplexMesh, Mesh);

public:
	BoxMesh();
};

} // namespace feather