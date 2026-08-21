#pragma once

#include "rusty/prelude.hpp"

namespace glvm_log {
using namespace rusty::prelude;

enum class LogTimestamp : u8 {
    None,
    SystemTime,
    Uptime,
};

struct LogFormat {
    LogTimestamp timestamp = LogTimestamp::SystemTime;
    bool with_ansi = true;
    bool with_level = true;
    bool with_target = true;

    [[nodiscard]] static auto full() -> LogFormat {
        return {};
    }

    [[nodiscard]] static auto compact() -> LogFormat {
        return {.timestamp = LogTimestamp::None};
    }

    [[nodiscard]] auto without_time() const -> LogFormat {
        auto copy = *this;
        copy.timestamp = LogTimestamp::None;
        return copy;
    }

    [[nodiscard]] auto with_timestamp(LogTimestamp ts) const -> LogFormat {
        auto copy = *this;
        copy.timestamp = ts;
        return copy;
    }

    [[nodiscard]] auto with_ansi_colors(bool enabled) const -> LogFormat {
        auto copy = *this;
        copy.with_ansi = enabled;
        return copy;
    }
};
} // namespace glvm_log
