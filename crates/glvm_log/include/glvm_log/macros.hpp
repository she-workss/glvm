#pragma once

#include "glvm_log/logger.hpp"

#include <format>

namespace glvm_log {

template<typename... Args>
inline auto emit(
    LogLevel lvl,
    std::string_view cat,
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
    std::string_view cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    emit(LogLevel::Trace, cat, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto debug(
    std::string_view cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    emit(LogLevel::Debug, cat, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto info(
    std::string_view cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    emit(LogLevel::Info, cat, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto warn(
    std::string_view cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    emit(LogLevel::Warn, cat, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto error(
    std::string_view cat,
    std::format_string<Args...> fmt,
    Args&&... args
) -> void {
    emit(LogLevel::Error, cat, fmt, std::forward<Args>(args)...);
}

} // namespace glvm_log
