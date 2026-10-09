module;

#include <assimp/Importer.hpp>
#include <framework/export_defs.h>

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "mesh_format_loader.gen.h"
#endif

export module feather.core:resources.mesh_format_loader;

import :resources.resource_format_loader;
import :main.class_db;

export namespace feather {

// Todo : mesh format loader needs to be reworked because we don't necessarily load only mesh.
// It's possible to load a full scene. The problem is identifying the underlying meshes afterwards from the VFP.
class FEATHER_API MeshFormatLoader : public ResourceFormatLoader {
	FCLASS();

	std::unique_ptr<Assimp::Importer> _importer;

protected:
	std::shared_ptr<Resource> instantiate(const Path& path) override;
	void load(std::shared_ptr<Resource> resource, const Path& path) override;

public:
	MeshFormatLoader();
	~MeshFormatLoader() override;

	[[method]]
	bool recognize_extension(const std::string& extension) const override;
};

} //namespace feather
