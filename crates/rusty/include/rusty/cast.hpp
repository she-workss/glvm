#pragma once

#include <concepts>

namespace rusty::cast {
template<typename To, typename From>
    requires std::convertible_to<From, To>
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
