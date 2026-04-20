#pragma once

#include "rid.h"
#include <core/framework/reflected.h>
#include <core/framework/reflection_macros.h>

namespace feather {

class Resource : public Reflected {
	FABSTRACT(Resource, Reflected);
	friend class ResourceLoader;

	RID _rid;

	Path _cached_path;

protected:
	static void _bind_members();

public:
	// Resources may give specific ids
	virtual RID get_rid() { return _rid; };

	virtual bool is_loaded() { return false; };

	Path get_path() { return _cached_path; };
	void set_path(Path path) { _cached_path = path; };
};

} // namespace feather