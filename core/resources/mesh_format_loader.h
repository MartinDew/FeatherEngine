#pragma once

#include "resource_format_loader.h"

namespace Assimp {
class Importer;
};

namespace feather {

class MeshFormatLoader : public ResourceFormatLoader {
	FCLASS(MeshFormatLoader, ResourceFormatLoader);

	std::unique_ptr<Assimp::Importer> _importer;

protected:
	static void _bind_members();

	std::shared_ptr<Resource> load(const Path& path) override;

public:
	MeshFormatLoader();
	~MeshFormatLoader() override;

	bool recognize_extension(const std::string& extension) const override;
};

} //namespace feather
