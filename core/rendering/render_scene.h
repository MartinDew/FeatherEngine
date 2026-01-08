#pragma once;

#include "math/projection.h"
#include "math/transform.h"
#include "resources/material.h"
#include "triangle_mesh.h"

namespace feather {

// The render capture is a capture of the current world used by the rendering pipeline to process an image
class RenderCapture {
public:
	struct EntityRender {
		Transform transform;
		TriangleMesh& triangle_mesh;
		Material material;
	};

	const Transform& get_camera_position();
	void set_camera_position(const Transform& transform);
	Projection get_camera_projection();
	void set_camera_projection(/*tbd*/);
};

} //namespace feather
