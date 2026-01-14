#pragma once

#include "framework/high_level_array.h"
#include "math/math_defs.h"
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
	explicit Mesh(const std::shared_ptr<TriangleMesh>& triangle_mesh);

	static void _bind_members();

public:
	const std::shared_ptr<TriangleMesh>& get_triangle_mesh() { return _triangle_mesh; };
};

// Mesh using raw vertices and indices data
class RawMesh : public Mesh {
	FCLASS(RawMesh, Mesh);

protected:
	static void _bind_members();

public:
	RawMesh() = default;

	void add_vertices(const HighLevelArray vertices);
	void add_indices(const HighLevelArray indices);

	HighLevelArray get_vertices() const;
	HighLevelArray get_indices() const;
};

class BoxMesh : public Mesh {
	FCLASS(RawMesh, Mesh);

public:
	BoxMesh();
};

} // namespace feather