#pragma once

#include <concepts>

namespace rusty::cast {
// libstdc++'s is_convertible_v is false for extended floating-point
// conversions (e.g. _Float64 -> _Float32), so arithmetic casts are allowed
// unconditionally, like Rust's `as`.
template<typename To, typename From>
    requires std::convertible_to<From, To>
    || (std::is_arithmetic_v<From> && std::is_arithmetic_v<To>)
constexpr auto as(From val) noexcept -> To {
    return static_cast<To>(val);
}

template<typename To>
struct Cast {};

template<typename From, typename To>
    requires std::convertible_to<From, To>
constexpr auto operator|(From val, Cast<To> /*unused*/) noexcept -> To {
    return static_cast<To>(val);
}

template<typename To>
inline constexpr Cast<To> cast {};
} // namespace rusty::cast
