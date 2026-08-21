#pragma once

#include "glvm_log/format.hpp"
#include "glvm_log/level.hpp"
#include "glvm_log/log.hpp"
#include "glvm_log/logger.hpp"
#include "glvm_log/macros.hpp"

#include <string>

namespace glvm_app::app {
struct App;
}

namespace glvm_log {

struct LogPlugin {
    LogLevel level = LogLevel::Info;
    std::string filter = "";
    LogFormat format = LogFormat::full();

    auto build() const -> void {
        Logger::configure(level, filter, format);
        log::info(
            "Logging configured - global_level={}, filter=\"{}\"",
            level_name(Logger::instance().global_level),
            filter
        );
    }
};

} // namespace glvm_log
