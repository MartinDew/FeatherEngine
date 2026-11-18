export module feather.reflection:callable;

import std.compat;
import feather.framework;
import :variant;

export namespace feather {

namespace callable_detail {

// Converts params to TArgs and calls func with them; a void result becomes NIL.
template <class TRet, class... TArgs, class TFunc>
Variant invoke_marshaled(TFunc&& func, std::span<Variant> params) {
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

} // namespace callable_detail

class FEATHER_REFLECTION_API Callable {
	std::function<Variant(std::span<Variant>)> _internal_func;
	uint8_t _param_amount;

public:
	Callable();
	template <class TRet, class... TArgs>
		requires(VariantCompatible<std::decay_t<TRet>>) && (VariantCompatible<std::decay_t<TArgs>> && ...)
	Callable(std::function<TRet(TArgs...)> func)
			: _internal_func { [func](std::span<Variant> params) {
				return callable_detail::invoke_marshaled<TRet, TArgs...>(func, params);
			} }
			, _param_amount { sizeof...(TArgs) } {}

	template <class TRet, class... TArgs>
		requires(VariantCompatible<std::decay_t<TRet>>) && (VariantCompatible<std::decay_t<TArgs>> && ...)
	Callable(TRet (*func_ptr)(TArgs...)) : Callable(std::function<TRet(TArgs...)>(func_ptr)) {}

	Callable(Callable&&) = default;
	Callable(const Callable&) = default;
	Callable& operator=(const Callable&) = default;
	Callable& operator=(Callable&&) = default;

	Variant call(std::span<Variant> params) const;

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
