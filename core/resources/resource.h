#pragma once

#include "rid.h"
#include <core/framework/reflected.h>
#include <core/framework/reflection_macros.h>

namespace feather {

class Resource : public Reflected {
	FCLASS(Resource, Reflected);
	friend class ResourceLoader;

	RID _rid;

protected:
	static void _bind_members();

public:
	// Resources may give specific ids
	virtual RID get_rid() { return _rid; };
};

} // namespace feather