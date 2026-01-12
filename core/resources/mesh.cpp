#include "mesh.h"

#include "framework/container_utils.h"
#include "framework/high_level_array.h"
#include "math/math_defs.h"
#include "rendering/triangle_mesh.h"
#include <core/main/class_db.h>
#include <framework/reflection_macros.h>
#include <set>
#include <vector>

namespace feather {

Mesh::Mesh(const std::shared_ptr<TriangleMesh>& triangle_mesh) : _triangle_mesh(triangle_mesh) {}

void Mesh::_bind_members() {}

void RawMesh::_bind_members() {
	// ClassDB::bind_method(&RawMesh::add_indices, "add_indices");
	// ClassDB::bind_method(&RawMesh::add_vertices, "add_vertices");
	// ClassDB::bind_method(&RawMesh::get_indices, "get_indices");
	// ClassDB::bind_method(&RawMesh::get_vertices, "get_vertices");

	// static_assert(is_contiguous_container<CowVector<int>>);
}

void RawMesh::add_vertices(const HighLevelArray vertices) {}
void RawMesh::add_indices(const HighLevelArray indices) {}

HighLevelArray RawMesh::get_vertices() const {
	return { _triangle_mesh->get_vertices().begin(), _triangle_mesh->get_vertices().end() };
}

HighLevelArray RawMesh::get_indices() const {
	return { _triangle_mesh->get_indices().begin(), _triangle_mesh->get_indices().end() };
}

//// Box Mesh ////

constexpr std::array<Vertex, 8> cube_vertices {
	// Front face (normals pointing outward from cube corners)
	Vertex { -0.5f, -0.5f, 0.5f, -0.577f, -0.577f, 0.577f }, // 0: bottom-left
	Vertex { 0.5f, -0.5f, 0.5f, 0.577f, -0.577f, 0.577f }, // 1: bottom-right
	Vertex { 0.5f, 0.5f, 0.5f, 0.577f, 0.577f, 0.577f }, // 2: top-right
	Vertex { -0.5f, 0.5f, 0.5f, -0.577f, 0.577f, 0.577f }, // 3: top-left
	// Back face
	Vertex { -0.5f, -0.5f, -0.5f, -0.577f, -0.577f, -0.577f }, // 4: bottom-left
	Vertex { 0.5f, -0.5f, -0.5f, 0.577f, -0.577f, -0.577f }, // 5: bottom-right
	Vertex { 0.5f, 0.5f, -0.5f, 0.577f, 0.577f, -0.577f }, // 6: top-right
	Vertex { -0.5f, 0.5f, -0.5f, -0.577f, 0.577f, -0.577f }, // 7: top-left}
};

constexpr std::array<uint32_t, 36> cube_indices { // Front face
	{ 0, 1, 2, 2, 3, 0,
			// Back face
			4, 6, 5, 6, 4, 7,
			// Left face
			4, 0, 3, 3, 7, 4,
			// Right face
			1, 5, 6, 6, 2, 1,
			// Top face
			3, 2, 6, 6, 7, 3,
			// Bottom face
			4, 5, 1, 1, 0, 4 }
};

static std::shared_ptr<TriangleMesh> box_mesh =
		std::make_shared<TriangleMesh>(std::vector<Vertex> { cube_vertices.begin(), cube_vertices.end() },
				std::vector<uint32_t> { cube_indices.begin(), cube_indices.end() });

BoxMesh::BoxMesh() : Super(box_mesh) {}

INPLACE_REGISTER_BEGIN(Mesh);
ClassDB::register_abstract_class<Mesh>();
ClassDB::register_class<RawMesh>();
ClassDB::register_class<BoxMesh>();
INPLACE_REGISTER_END(Mesh);

} // namespace feather