#pragma once

#include <cstdio>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif
#include <windows.h>
#undef near
#undef far
#undef TRANSPARENT
#include <io.h>
#else
#include <unistd.h>
#endif
#include "rusty/prelude.hpp"

#include <atomic>
#include <string_view>

namespace glvm_log::color {
using namespace rusty::prelude;

inline constexpr str RESET = "\x1b[0m";
inline constexpr str GRAY = "\x1b[90m";
inline constexpr str CYAN = "\x1b[96m";
inline constexpr str GREEN = "\x1b[92m";
inline constexpr str YELLOW = "\x1b[93m";
inline constexpr str RED = "\x1b[91m";
inline constexpr str MAGENTA = "\x1b[95m";

#ifdef _WIN32
inline auto enable_windows_vt() -> void {
    static AtomicBool DONE {false};
    if (DONE.load(std::memory_order_relaxed)) {
        return;
    }
    HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(h, &mode) != 0) {
            SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
    DONE.store(true, std::memory_order_relaxed);
}
#endif

[[nodiscard]] inline auto is_tty() -> bool {
    static const bool TTY = []() -> bool {
#ifdef _WIN32
        return _isatty(_fileno(stderr)) != 0;
#else
        return isatty(STDERR_FILENO) != 0;
#endif
    }();
    return TTY;
}

inline auto init() -> void {
#ifdef _WIN32
    if (is_tty()) {
        enable_windows_vt();
    }
#endif
}

[[nodiscard]] inline auto for_stderr(str ansi_code) -> str {
    return is_tty() ? ansi_code : str {};
}
} // namespace glvm_log::color
