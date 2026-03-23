#pragma once

#include <format>
#include <print>

namespace rusty::io {
template<typename... Args>
[[gnu::always_inline]] inline auto rsprint(
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    std::print(stdout, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
[[gnu::always_inline]] inline auto rsprintln(
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    std::println(stdout, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
[[gnu::always_inline]] inline auto rseprint(
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    std::print(stderr, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
[[gnu::always_inline]] inline auto rseprintln(
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    std::println(stderr, fmt, std::forward<Args>(args)...);
}
} // namespace rusty::io
