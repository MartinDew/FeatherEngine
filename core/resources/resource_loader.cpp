#include "resource_loader.h"

#include <core/main/class_db.h>
#include <main/project_settings.h>
#include <algorithm>
#include <iostream>

namespace feather {

std::unique_ptr<ResourceLoader> ResourceLoader::_instance = nullptr;

ResourceLoader::ResourceLoader() {
	fassert(!_instance, "Resource loader already initialized");
	_instance.reset(this);

	auto children = ClassDB::get_children_names(ResourceFormatLoader::get_class_static());
	for (const auto& child : children) {
		std::shared_ptr<ResourceFormatLoader> loader = ClassDB::create_object<ResourceFormatLoader>(child);
		add_resource_format_loader(loader);

		std::println(std::cout, "Registered resource format loader: {}", child);
	}
};

void ResourceLoader::_bind_members() {
}

RID ResourceLoader::generate_rid() {
	return RID { get()->m_counter.fetch_add(1, std::memory_order_relaxed) };
}

void ResourceLoader::register_resource(std::shared_ptr<Resource> res) {
	res->_rid = generate_rid();
	get()->_cache[res->_rid] = res;
}

std::shared_ptr<Resource> ResourceLoader::load(const Path& path) {
	auto it = get()->_path_cache.find(path.string());
	if (it != get()->_path_cache.end()) {
		return it->second;
	}

	size_t ext_pos = path.string().find_last_of('.');
	if (ext_pos == std::string::npos) {
		std::cerr << "ResourceLoader: Cannot load resource without extension: " << path << std::endl;
		return nullptr;
	}

	std::string extension = path.string().substr(ext_pos);

	for (const auto& loader : get()->_format_loaders) {
		if (loader->recognize_extension(extension)) {
			std::shared_ptr<Resource> res = loader->load(ProjectSettings::get()->localize_path(path));
			if (res) {
				res->_rid = generate_rid();
				get()->_cache[res->_rid] = res;
				get()->_path_cache[path.string()] = res;
				return res;
			}
		}
	}

	std::cerr << "ResourceLoader: No unrecognized loader for extension '" << extension << "' for resource: " << path
			  << std::endl;
	return nullptr;
}

void ResourceLoader::add_resource_format_loader(std::shared_ptr<ResourceFormatLoader> loader) {
	_format_loaders.push_back(loader);
}

void ResourceLoader::remove_resource_format_loader(std::shared_ptr<ResourceFormatLoader> loader) {
	auto& loaders = _format_loaders;
	auto it = std::find(loaders.begin(), loaders.end(), loader);
	if (it != loaders.end()) {
		loaders.erase(it);
	}
}

} // namespace feather