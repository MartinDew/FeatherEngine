#include "resource_loader.h"
#include <core/main/class_db.h>
#include <algorithm>
#include <iostream>

namespace feather {

ResourceLoader* ResourceLoader::_instance = nullptr;

void ResourceLoader::_bind_members() {}

ResourceLoader* ResourceLoader::get() {
	[[unlikely]]
	if (!_instance)
		_instance = new ResourceLoader;

	return _instance;
}

RID ResourceLoader::generate_rid() { return RID { get()->m_counter.fetch_add(1, std::memory_order_relaxed) }; }

void ResourceLoader::register_resource(std::shared_ptr<Resource> res) {
	res->_rid = generate_rid();
	get()->_cache[res->_rid] = res;
}

std::shared_ptr<Resource> ResourceLoader::load(const std::string& path) {
	auto it = get()->_path_cache.find(path);
	if (it != get()->_path_cache.end()) {
		return it->second;
	}

	size_t ext_pos = path.find_last_of('.');
	if (ext_pos == std::string::npos) {
		std::cerr << "ResourceLoader: Cannot load resource without extension: " << path << std::endl;
		return nullptr;
	}
	
	std::string extension = path.substr(ext_pos + 1);

	for (const auto& loader : get()->_format_loaders) {
		if (loader->recognize_extension(extension)) {
			std::shared_ptr<Resource> res = loader->load(path);
			if (res) {
				res->_rid = generate_rid();
				get()->_cache[res->_rid] = res;
				get()->_path_cache[path] = res;
				return res;
			}
		}
	}

	std::cerr << "ResourceLoader: No unrecognized loader for extension '" << extension << "' for resource: " << path << std::endl;
	return nullptr;
}

void ResourceLoader::add_resource_format_loader(std::shared_ptr<ResourceFormatLoader> loader) {
	get()->_format_loaders.push_back(loader);
}

void ResourceLoader::remove_resource_format_loader(std::shared_ptr<ResourceFormatLoader> loader) {
	auto& loaders = get()->_format_loaders;
	auto it = std::find(loaders.begin(), loaders.end(), loader);
	if (it != loaders.end()) {
		loaders.erase(it);
	}
}

INPLACE_REGISTER_BEGIN(ResourceStorage);
ClassDB::register_singleton_class<ResourceLoader>();
INPLACE_REGISTER_END(ResourceStorage);

} // namespace feather