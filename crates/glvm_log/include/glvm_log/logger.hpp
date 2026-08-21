#pragma once

#include "glvm_log/color.hpp"
#include "glvm_log/format.hpp"
#include "glvm_log/level.hpp"
#include "rusty/prelude.hpp"

#include <chrono>
#include <format>
#include <print>

namespace glvm_log {
using namespace rusty::prelude;

[[nodiscard]] inline auto level_color(LogLevel l) -> str {
    switch (l) {
        case LogLevel::Trace:
            return color::GRAY;
        case LogLevel::Debug:
            return color::CYAN;
        case LogLevel::Info:
            return color::GREEN;
        case LogLevel::Warn:
            return color::YELLOW;
        case LogLevel::Error:
            return color::RED;
        case LogLevel::Off:
            return {};
    }
    return {};
}

struct Logger {
    LogLevel global_level = LogLevel::Info;
    HashMap<String, LogLevel> filters;
    LogFormat fmt;
    mutable Mutex mtx;

    [[nodiscard]] auto effective_level(str category) const -> LogLevel {
        auto it = filters.find(String(category));
        return it != filters.end() ? it->second : global_level;
    }

    [[nodiscard]] auto should_log(str category, LogLevel lvl) const -> bool {
        return lvl >= effective_level(category);
    }

    auto parse_filter(str filter) -> void {
        filters.clear();
        if (filter.empty()) {
            return;
        }
        auto start = usize {0};
        while (start <= filter.size()) {
            auto end = filter.find(',', start);
            if (end == str::npos) {
                end = filter.size();
            }
            auto token = filter.substr(start, end - start);
            if (!token.empty()) {
                auto eq = token.find('=');
                if (eq != str::npos) {
                    auto cat = String(token.substr(0, eq));
                    auto lvl_str = token.substr(eq + 1);
                    filters[cat] = level_from_str(lvl_str);
                } else {
                    global_level = level_from_str(token);
                }
            }
            start = end + 1;
        }
    }

    auto log(LogLevel lvl, str cat, str msg) const -> void {
        auto lock = MutexGuard(mtx);
        auto use_color = fmt.with_ansi && color::is_tty();
        auto line = String();
        if (fmt.timestamp == LogTimestamp::SystemTime) {
            auto now = std::chrono::floor<std::chrono::microseconds>(
                std::chrono::system_clock::now()
            );
            if (use_color) {
                line += std::format(
                    "{}{:%Y-%m-%dT%H:%M:%S}Z{}  ",
                    color::GRAY,
                    now,
                    color::RESET
                );
            } else {
                line += std::format("{:%Y-%m-%dT%H:%M:%S}Z  ", now);
            }
        } else if (fmt.timestamp == LogTimestamp::Uptime) {
            auto elapsed =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::steady_clock::now() - start_time
                );
            auto secs = elapsed.count() / 1'000'000LL;
            auto us_part = elapsed.count() % 1'000'000LL;
            if (use_color) {
                line += std::format(
                    "{}{}.{:06}s{}  ",
                    color::GRAY,
                    secs,
                    us_part,
                    color::RESET
                );
            } else {
                line += std::format("{}.{:06}s  ", secs, us_part);
            }
        }
        if (fmt.with_level) {
            if (use_color) {
                line += std::format(
                    "{}{}{} ",
                    color::for_stderr(level_color(lvl)),
                    level_name(lvl),
                    color::RESET
                );
            } else {
                line += std::format("{} ", level_name(lvl));
            }
        }
        if (fmt.with_target) {
            if (use_color) {
                line += std::format("{}{}{}: ", color::GRAY, cat, color::RESET);
            } else {
                line += std::format("{}: ", cat);
            }
        }
        line += msg;
        std::println(stderr, "{}", line);
    }

    static auto instance() -> Logger& {
        static Logger INST;
        return INST;
    }

    static auto configure(LogLevel lvl, str filter, LogFormat format = {})
        -> void {
        auto& inst = instance();
        auto lock = MutexGuard(inst.mtx);
        inst.global_level = lvl;
        inst.fmt = format;
        inst.parse_filter(filter);
    }

private:
    std::chrono::steady_clock::time_point start_time =
        std::chrono::steady_clock::now();
};
} // namespace glvm_log
