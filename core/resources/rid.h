#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace feather {

struct RID {
	size_t id { 0 };

	static constexpr RID invalid();
	;
	bool is_valid() const;
	bool operator==(const RID&) const = default;
	bool operator<(const RID& o) const { return id < o.id; }
};

} // namespace feather

// Hash support for use in unordered_map/set
namespace std {
template <>
struct hash<feather::RID> {
	size_t operator()(const feather::RID& r) const noexcept { return r.id; }
};
} //namespace std