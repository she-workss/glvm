#pragma once

#include "rusty/prelude.hpp"

namespace glvm_log {
using namespace rusty::prelude;

enum class LogLevel : u8 {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Off = 5,
};

constexpr auto level_name(LogLevel l) -> str {
    switch (l) {
        case LogLevel::Trace:
            return "TRACE";
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO ";
        case LogLevel::Warn:
            return "WARN ";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Off:
            return "OFF  ";
    }
    return "?????";
}

constexpr auto level_from_str(str s) -> LogLevel {
    if (s == "trace") {
        return LogLevel::Trace;
    }
    if (s == "debug") {
        return LogLevel::Debug;
    }
    if (s == "info") {
        return LogLevel::Info;
    }
    if (s == "warn") {
        return LogLevel::Warn;
    }
    if (s == "error") {
        return LogLevel::Error;
    }
    if (s == "off") {
        return LogLevel::Off;
    }
    return LogLevel::Info;
}
} // namespace glvm_log
