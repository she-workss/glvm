#pragma once
#include "glvm_log/macros.hpp"

namespace glvm_log::log {

template<typename... Args>
inline auto trace(std::format_string<Args...> fmt, Args&&... args) -> void {
    glvm_log::trace("glvm_log", fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto debug(std::format_string<Args...> fmt, Args&&... args) -> void {
    glvm_log::debug("glvm_log", fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto info(std::format_string<Args...> fmt, Args&&... args) -> void {
    glvm_log::info("glvm_log", fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto warn(std::format_string<Args...> fmt, Args&&... args) -> void {
    glvm_log::warn("glvm_log", fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline auto error(std::format_string<Args...> fmt, Args&&... args) -> void {
    glvm_log::error("glvm_log", fmt, std::forward<Args>(args)...);
}

} // namespace glvm_log::log
