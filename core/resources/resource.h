#pragma once
#include <core/framework/reflected.h>
#include <core/framework/reflection_macros.h>

namespace feather {

class Resource : public Reflected {
	FCLASS(Resource, Reflected);

protected:
	static void _bind_members() {}

public:
};

} // namespace feather