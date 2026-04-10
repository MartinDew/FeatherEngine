#pragma once

namespace feather {

template <typename T>
concept is_singleton_v =
		requires {
			// Check for static get() method returning T* or T&
			{ T::get() } -> std::convertible_to<T*>;
		} &&
		// Ensure it cannot be copied (Singleton 101)
		!std::is_copy_constructible_v<T> && !std::is_copy_assignable_v<T>;

} // namespace feather

#define FDECLARE_SINGLETON(_type)                                                                                      \
	static std::unique_ptr<_type> _instance;                                                                           \
	_type(_type&) = delete;                                                                                            \
	_type& operator=(const _type&) = delete;                                                                           \
                                                                                                                       \
public:                                                                                                                \
	static _type* get() {                                                                                              \
		if (!_instance)                                                                                                \
			fassert(false, std::format("Singleton {} not instantiated but an attempt to get was made", #_type));       \
		return _instance.get();                                                                                        \
	}                                                                                                                  \
                                                                                                                       \
private: