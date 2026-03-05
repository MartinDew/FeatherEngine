#include "resource_loader.h"

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

INPLACE_REGISTER_BEGIN(ResourceStorage);
ClassDB::register_singleton_class<ResourceLoader>();
INPLACE_REGISTER_END(ResourceStorage);

} // namespace feather