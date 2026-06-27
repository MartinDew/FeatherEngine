#pragma once
#include "resource.h"

namespace feather {

class SharedLibrary;
class ExtensionFormatLoader;

class Extension final : public Resource {
	FCLASS(Extension, Resource);

	friend class ExtensionFormatLoader;

	std::string _extension_name;
	// std::string _entry_point;
	std::function<void()> _register_func;
	std::shared_ptr<SharedLibrary> _library_handle;

protected:
	static void _bind_members();

public:
	Extension() = default;
	~Extension() override = default;

	Extension(const std::string_view& name, const std::function<void()> register_func);

	bool is_loaded() override { return _library_handle != nullptr; }

	const std::string& get_name() const { return _extension_name; }
	// const std::string& get_entry_point() const { return _entry_point; }
	const bool register_extension() const;
	const bool unregister_extension() const;
};

} // namespace feather