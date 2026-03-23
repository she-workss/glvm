#pragma once

#include "io.hpp"
#include "types.hpp"

#include <format>
#include <source_location>
#include <utility>
#include <vector>

namespace rusty::macros {
using namespace io;
using namespace types;

template<typename... Args>
[[noreturn]] auto panic(std::format_string<Args...> fmt, Args&&... args)
    -> void {
    rseprint("panic: ");
    rseprintln(fmt, std::forward<Args>(args)...);
    std::abort();
}

[[noreturn]] inline auto panic() -> void {
    rseprintln("panic: explicit panic");
    std::abort();
}

[[noreturn]] inline auto todo(
    str msg = "",
    std::source_location loc = std::source_location::current()
) -> void {
    if (msg.empty()) {
        rseprintln("not yet implemented at {}:{}", loc.file_name(), loc.line());
    } else {
        rseprintln(
            "not yet implemented at {}:{}: {}",
            loc.file_name(),
            loc.line(),
            msg
        );
    }
    std::abort();
}

[[noreturn]] inline auto unimplemented(
    str msg = "",
    std::source_location loc = std::source_location::current()
) -> void {
    if (msg.empty()) {
        rseprintln("not implemented at {}:{}", loc.file_name(), loc.line());
    } else {
        rseprintln(
            "not implemented at {}:{}: {}",
            loc.file_name(),
            loc.line(),
            msg
        );
    }
    std::abort();
}

[[noreturn]] inline auto unreachable_safe(
    std::source_location loc = std::source_location::current()
) -> void {
#ifdef NDEBUG
    std::unreachable();
#else
    rseprintln("entered unreachable code at {}:{}", loc.file_name(), loc.line());
    std::abort();
#endif
}

template<typename... Args>
constexpr auto vec(Args&&... args) {
    return Vec {std::forward<Args>(args)...};
}

namespace detail {
template<typename T>
auto dbg_impl(T val, const char* expr, std::source_location loc) -> T {
    if constexpr (std::formattable<T, char>) {
        rseprintln("[{}:{}] {} = {}", loc.file_name(), loc.line(), expr, val);
    } else {
        rseprintln("[{}:{}] {} = <?>", loc.file_name(), loc.line(), expr);
    }
    return val;
}
} // namespace detail

} // namespace rusty::macros

#define dbg_rs(expr)                                                           \
    ::rusty::macros::detail::dbg_impl(                                         \
        (expr),                                                                \
        #expr,                                                                 \
        std::source_location::current()                                        \
    )

template<typename L, typename R>
constexpr auto assert_eq(
    const L& lhs,
    const R& rhs,
    std::source_location loc = std::source_location::current()
) -> void {
    if (lhs != rhs) {
        if constexpr (std::formattable<L, char> && std::formattable<R, char>) {
            rusty::io::rseprintln(
                "assertion failed at {}:{}: lhs == rhs\n"
                "  left:  {}\n  right: {}",
                loc.file_name(),
                loc.line(),
                lhs,
                rhs
            );
        } else {
            rusty::io::rseprintln(
                "assertion failed at {}:{}: lhs == rhs",
                loc.file_name(),
                loc.line()
            );
        }
        std::abort();
    }
}

template<typename L, typename R>
constexpr auto assert_ne(
    const L& lhs,
    const R& rhs,
    std::source_location loc = std::source_location::current()
) -> void {
    if (lhs == rhs) {
        if constexpr (std::formattable<L, char>) {
            rusty::io::rseprintln(
                "assertion failed at {}:{}: lhs != rhs\n"
                "  both equal: {}",
                loc.file_name(),
                loc.line(),
                lhs
            );
        } else {
            rusty::io::rseprintln(
                "assertion failed at {}:{}: lhs != rhs",
                loc.file_name(),
                loc.line()
            );
        }
        std::abort();
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, readability-identifier-naming)
#define TRY(expr)                                                              \
    ({                                                                         \
        auto&& _res = (expr);                                                  \
        if (!_res) {                                                           \
            return ::rusty::result::Err(_res.error());                         \
        }                                                                      \
        std::move(*_res);                                                      \
    })
