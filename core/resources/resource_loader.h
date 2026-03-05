#pragma once

#include "resource.h"
#include "rid.h"

#include <framework/reflected.h>
#include <framework/reflection_macros.h>

namespace feather {

class ResourceLoader : public Reflected {
	FCLASS(ResourceLoader, Reflected);

	static ResourceLoader* _instance;

	ResourceLoader() = default;

	std::atomic<size_t> m_counter { 1 };

	std::unordered_map<RID, std::shared_ptr<Resource>> _cache;
	// Todo : Add resource loaders
protected:
	static void _bind_members();

public:
	static ResourceLoader* get();

	static RID generate_rid();

	// Allows manually registering a resource
	// Useful for resources that are static and don't need to be loaded
	static void register_resource(std::shared_ptr<Resource> res);
};

} //namespace feather