export module feather.core:math.rtm_interop;

import rtm;
import std;

// Feather's math types declare the same fields as RTM's storage structs instead of deriving
// from them: a derived type could not carry [[get]]/[[set]] on inherited members.
export namespace feather::math {

template <class TReal, std::size_t TCount>
struct RtmStorage;

template <> struct RtmStorage<float, 2> {
	using Type = rtm::float2f;
};
template <> struct RtmStorage<float, 3> {
	using Type = rtm::float3f;
};
template <> struct RtmStorage<float, 4> {
	using Type = rtm::float4f;
};
template <> struct RtmStorage<double, 2> {
	using Type = rtm::float2d;
};
template <> struct RtmStorage<double, 3> {
	using Type = rtm::float3d;
};
template <> struct RtmStorage<double, 4> {
	using Type = rtm::float4d;
};

template <class T, class TReal, std::size_t TCount>
inline constexpr bool mirrors_rtm_storage = sizeof(T) == sizeof(typename RtmStorage<TReal, TCount>::Type) &&
		alignof(T) == alignof(typename RtmStorage<TReal, TCount>::Type) && std::is_standard_layout_v<T>;

} //namespace feather::math
