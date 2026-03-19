#pragma once

#include "resource_format_loader.h"

namespace feather {

class MeshFormatLoader : public ResourceFormatLoader {
	FCLASS(MeshFormatLoader, ResourceFormatLoader);

protected:
	static void _bind_members();
};

} //namespace feather
