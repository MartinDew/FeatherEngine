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

static constexpr float DX = 1;
static constexpr float DY = 1;
static constexpr float DZ = 1;

static constexpr Vector3 Point[8] = { Vector3(-DX / 2, DY / 2, -DZ / 2), Vector3(DX / 2, DY / 2, -DZ / 2),
	Vector3(DX / 2, -DY / 2, -DZ / 2), Vector3(-DX / 2, -DY / 2, -DZ / 2), Vector3(-DX / 2, DY / 2, DZ / 2),
	Vector3(-DX / 2, -DY / 2, DZ / 2), Vector3(DX / 2, -DY / 2, DZ / 2), Vector3(DX / 2, DY / 2, DZ / 2) };

// Normals
static constexpr Vector3 N0(0.0f, 0.0f, -1.0f); // front
static constexpr Vector3 N1(0.0f, 0.0f, 1.0f); // back
static constexpr Vector3 N2(0.0f, -1.0f, 0.0f); // down
static constexpr Vector3 N3(0.0f, 1.0f, 0.0f); // up
static constexpr Vector3 N4(-1.0f, 0.0f, 0.0f); // left
static constexpr Vector3 N5(1.0f, 0.0f, 0.0f); // right

// clang-format off

static constexpr std::array<Vertex, 24> cube_vertices {
	Vertex { Point[0], N0 },
	Vertex { Point[1], N0 },
	Vertex { Point[2], N0 },
	Vertex { Point[3], N0 },
	Vertex { Point[4], N1 },
	Vertex { Point[5], N1 },
	Vertex { Point[6], N1 },
	Vertex { Point[7], N1 },
	Vertex { Point[3], N2 },
	Vertex { Point[2], N2 },
	Vertex { Point[6], N2 },
	Vertex { Point[5], N2 },
	Vertex { Point[0], N3 },
	Vertex { Point[4], N3 },
	Vertex { Point[7], N3 },
	Vertex { Point[1], N3 },
	Vertex { Point[0], N4 },
	Vertex { Point[3], N4 },
	Vertex { Point[5], N4 },
	Vertex { Point[4], N4 },
	Vertex { Point[1], N5 },
	Vertex { Point[7], N5 },
	Vertex { Point[6], N5 },
	Vertex { Point[2], N5 } };

// clang-format on

constexpr std::array<uint32_t, 36> cube_indices {
	// Front face
	0, 1, 2, // front
	0, 2, 3, // front

	5, 6, 7, // back
	5, 7, 4, // back

	8, 9, 10, // down
	8, 10, 11, // down

	13, 14, 15, // up
	13, 15, 12, // up

	19, 16, 17, // left
	19, 17, 18, // left
	20, 21, 22, // right
	20, 22, 23 // right
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