module;

#include <framework/static_string.hpp>

module feather.core;

namespace feather {

Scene::Scene(StaticString name) : _scene_id(name) {
}

bool Scene::operator==(const Scene& other) const {
	return _scene_id == other._scene_id;
}

} //namespace feather
