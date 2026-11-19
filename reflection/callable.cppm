export module feather.reflection:callable;

import std.compat;
import feather.framework;
import :variant;

export namespace feather {

class FEATHER_REFLECTION_API Callable {
	// `self` is the instance a member function is called on; free functions ignore it.
	std::function<Variant(void* self, std::span<Variant>)> _internal_func;
	uint8_t _param_amount;
	bool _is_member = false;

	template <class TRet, class... TArgs, class TFunc>
	static Variant _marshal(TFunc&& func, std::span<Variant> params) {
		[[maybe_unused]] size_t i = 0;
		std::tuple<std::decay_t<TArgs>...> converted_args = { params[i++].as<std::decay_t<TArgs>>().value()... };
		if constexpr (std::is_void_v<TRet>) {
			std::apply(func, converted_args);
			return Variant();
		}
		else {
			TRet result = std::apply(func, converted_args);
			return Variant(std::move(result));
		}
	}

public:
	Callable();
	template <class TRet, class... TArgs>
		requires(VariantCompatible<std::decay_t<TRet>>) && (VariantCompatible<std::decay_t<TArgs>> && ...)
	Callable(std::function<TRet(TArgs...)> func)
			: _internal_func { [func](void*, std::span<Variant> params) { return _marshal<TRet, TArgs...>(func, params); } }
			, _param_amount { sizeof...(TArgs) } {}

	template <class TRet, class... TArgs>
		requires(VariantCompatible<std::decay_t<TRet>>) && (VariantCompatible<std::decay_t<TArgs>> && ...)
	Callable(TRet (*func_ptr)(TArgs...)) : Callable(std::function<TRet(TArgs...)>(func_ptr)) {}

	template <class T, class TRet, class... TArgs>
		requires(VariantCompatible<std::decay_t<TRet>>) && (VariantCompatible<std::decay_t<TArgs>> && ...)
	Callable(TRet (T::*method)(TArgs...))
			: _internal_func { [method](void* self, std::span<Variant> params) {
				return _marshal<TRet, TArgs...>(
						[&](TArgs... args) -> TRet { return (static_cast<T*>(self)->*method)(args...); }, params
				);
			} }
			, _param_amount { sizeof...(TArgs) }
			, _is_member { true } {}

	template <class T, class TRet, class... TArgs>
		requires(VariantCompatible<std::decay_t<TRet>>) && (VariantCompatible<std::decay_t<TArgs>> && ...)
	Callable(TRet (T::*method)(TArgs...) const)
			: _internal_func { [method](void* self, std::span<Variant> params) {
				return _marshal<TRet, TArgs...>(
						[&](TArgs... args) -> TRet { return (static_cast<const T*>(self)->*method)(args...); }, params
				);
			} }
			, _param_amount { sizeof...(TArgs) }
			, _is_member { true } {}

	Callable(Callable&&) = default;
	Callable(const Callable&) = default;
	Callable& operator=(const Callable&) = default;
	Callable& operator=(Callable&&) = default;

	// A member function needs `self`, the instance adjusted to the class that declares it.
	Variant call_on(void* self, std::span<Variant> params) const;
	Variant call(std::span<Variant> params) const { return call_on(nullptr, params); }

	Variant call(auto&&... args) const {
		if constexpr (sizeof...(args) == 0) {
			return call(std::span<Variant>());
		}
		else {
			Variant params[] = { args... };
			return call(std::span<Variant>(params, sizeof...(args)));
		}
	}

	bool is_valid() const;
};

} // namespace feather
