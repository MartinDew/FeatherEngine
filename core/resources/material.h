#pragma once

#include "resource.h"

namespace feather {

class Material : public Resource {
	FCLASS(Material, Resource);

protected:
	static void _bind_members();
};

} //namespace feather
