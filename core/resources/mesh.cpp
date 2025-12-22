#include "mesh.h"

#include "framework/container_utils.h"
#include "framework/high_level_array.h"
#include <core/main/class_db.h>
#include <framework/reflection_macros.h>
#include <set>
#include <vector>

namespace feather {

void Mesh::_bind_members() {}

void RawMesh::_bind_members() {
	ClassDB::bind_method(&RawMesh::add_indices, "add_indices");
	ClassDB::bind_method(&RawMesh::add_vertices, "add_vertices");
	ClassDB::bind_method(&RawMesh::get_indices, "get_indices");
	ClassDB::bind_method(&RawMesh::get_vertices, "get_vertices");

	static_assert(is_contiguous_container<CowVector<int>>);
}

INPLACE_REGISTER_BEGIN(Mesh);
ClassDB::register_abstract_class<Mesh>();
ClassDB::register_class<RawMesh>();
INPLACE_REGISTER_END(Mesh);

} // namespace feather