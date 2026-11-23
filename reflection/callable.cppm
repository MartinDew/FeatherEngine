export module feather.reflection:callable;

import std.compat;
import feather.framework;
import :variant;

export namespace feather {

class FEATHER_REFLECTION_API Callable {
	std::function<Variant(std::span<Variant>)> _internal_func;
	uint8_t _param_amount;

public:
	Callable();
	template <class TRet, class... TArgs>
		requires VariantBindable<TRet, TArgs...>
	Callable(std::function<TRet(TArgs...)> func)
			: _internal_func { [func](std::span<Variant> params) {
				return invoke_with_variants<TRet, TArgs...>(func, params);
			} }
			, _param_amount { sizeof...(TArgs) } {}

	// Already marshalled: `func` reads its `param_amount` Variants itself.
	inline Callable(std::function<Variant(std::span<Variant>)> func, uint8_t param_amount)
			: _internal_func { std::move(func) }
			, _param_amount { param_amount } {}

	template <class TRet, class... TArgs>
		requires VariantBindable<TRet, TArgs...>
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
