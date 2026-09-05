#pragma once

#include "glvm_log/logger.hpp"

#include <format>

namespace glvm_log {

template<typename... Args>
inline auto emit(
    LogLevel lvl,
    str cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    if (Logger::instance().should_log(cat, lvl)) {
        Logger::instance()
            .log(lvl, cat, std::format(fmt, std::forward<Args>(args)...));
    }
}

template<typename... Args>
inline auto trace(
    str cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    emit(LogLevel::Trace, cat, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto debug(
    str cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    emit(LogLevel::Debug, cat, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto info(
    str cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    emit(LogLevel::Info, cat, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto warn(
    str cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    emit(LogLevel::Warn, cat, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto error(
    str cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    emit(LogLevel::Error, cat, fmt, std::forward<Args>(args)...);
}

} // namespace glvm_log
