#pragma once

#include <concepts>

namespace rusty::cast {
// Like Rust's `as`: anything `static_cast` can do, including arithmetic,
// pointer-to-pointer and explicit conversions.
template<typename To, typename From>
    requires requires(From v) { static_cast<To>(v); }
constexpr auto as(From val) noexcept -> To {
    return static_cast<To>(val);
}

template<typename To>
struct Cast {};

template<typename From, typename To>
    requires requires(From v) { static_cast<To>(v); }
constexpr auto operator|(From val, Cast<To> /*unused*/) noexcept -> To {
    return static_cast<To>(val);
}

template<typename To>
inline constexpr Cast<To> cast {};
} // namespace rusty::cast
