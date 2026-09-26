#pragma once

#include "option.hpp"
#include "types.hpp"

#include <concepts>
#include <functional>
#include <type_traits>

namespace rusty::concepts {
template<typename T>
concept Formattable = std::formattable<T, char>;

template<typename T>
concept Clone = std::copy_constructible<T>;

template<typename T>
concept DefaultConstructible = std::default_initializable<T>;

template<typename From, typename To>
concept Into = std::convertible_to<From, To>;

template<typename T, typename U>
concept From = std::constructible_from<T, U>;

template<typename T>
concept PartialEq = std::equality_comparable<T>;

template<typename T>
concept Eq = PartialEq<T>;

template<typename T>
concept PartialOrd = std::totally_ordered<T>;

template<typename T>
concept Ord = PartialOrd<T>;

template<typename T>
concept Hash = requires(T val) {
    { std::hash<T> {}(val) } -> std::same_as<rusty::types::usize>;
};

template<typename T>
concept Iterator = requires(T it) {
    typename T::value_type;
    {
        it.next()
    } -> std::same_as<rusty::option::Option<typename T::value_type>>;
};

template<typename F, typename Ret, typename... Args>
concept Fn = std::regular_invocable<F, Args...>
    && std::same_as<std::invoke_result_t<F, Args...>, Ret>;

template<typename F, typename Ret, typename... Args>
concept FnMut = std::invocable<F, Args...>
    && std::same_as<std::invoke_result_t<F, Args...>, Ret>;

template<typename T>
concept Send = true;
} // namespace rusty::concepts
