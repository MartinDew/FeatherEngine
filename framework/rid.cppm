export module feather.framework:rid;

import std.compat;

export namespace feather {

struct FEATHER_FRAMEWORK_API RID {
	size_t id { 0 };

	static constexpr RID invalid();
	[[nodiscard]] bool is_valid() const;
	bool operator==(const RID&) const = default;
	inline bool operator<(const RID& o) const { return id < o.id; }
};

} // namespace feather

// Hash support for use in unordered_map/set
namespace std {
template <>
struct FEATHER_FRAMEWORK_API hash<feather::RID> {
	inline size_t operator()(const feather::RID& r) const noexcept { return r.id; }
};
} //namespace std
