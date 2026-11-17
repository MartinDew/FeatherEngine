module;

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "resource.gen.h"
#endif

export module feather.resources:resource;

import std;
import feather.framework;
import feather.reflection;

export namespace feather {

class FEATHER_RESOURCES_API Resource : public Reflected {
	FCLASS();
	friend class ResourceLoader;

	RID _rid;

	[[name(path), get(public, ref), set(public, ref)]]
	Path _cached_path;

	[[get(public), set(protected)]]
	bool _should_be_loaded;

protected:
	Resource() = default;

public:
	inline const Path& get_path() const { return _cached_path; }
	inline void set_path(const Path& path) { _cached_path = path; }

	// Resources may give specific ids
	[[method]]
	inline virtual RID get_rid() {
		return _rid;
	};

	inline virtual bool is_loaded() { return false; };
};

} // namespace feather
