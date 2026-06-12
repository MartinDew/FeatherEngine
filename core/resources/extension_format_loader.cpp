#include "extension_format_loader.h"
#include "extension.h"
#include <framework/shared_library.h>
#include <iostream>

namespace feather {

void ExtensionFormatLoader::_bind_members() {}

bool ExtensionFormatLoader::recognize_extension(const std::string& extension) const {
	return extension == ".dll" || extension == ".so" || extension == ".dylib";
}

std::shared_ptr<Resource> ExtensionFormatLoader::load(const Path& path) {
	auto lib = std::make_shared<SharedLibrary>();
	if (!lib->load(path.string())) {
		std::cerr << "ExtensionFormatLoader: Failed to load library: " << path << std::endl;
		return nullptr;
	}

	using LoadExtensionFn = Extension* (*)();
	auto load_fn = lib->get_typed_symbol<LoadExtensionFn>("_load_extension");
	if (!load_fn) {
		// Not an engine extension — silently ignore
		return nullptr;
	}

	Extension* ext_raw = load_fn();
	if (!ext_raw) {
		return nullptr;
	}

	std::shared_ptr<Extension> ext(ext_raw);
	ext->_library_handle = lib;
	ext->set_path(path);

	Callable entry_fn = lib->get_symbol(ext->get_entry_point());
	if (entry_fn.is_valid()) {
		entry_fn.call();
		std::println(std::cout, "ExtensionFormatLoader: Loaded extension '{}' from {}", ext->get_name(), path.string());
	} else {
		std::cerr << "ExtensionFormatLoader: Entry point '" << ext->get_entry_point()
				  << "' not found in extension: " << path << std::endl;
	}

	return ext;
}

} // namespace feather
