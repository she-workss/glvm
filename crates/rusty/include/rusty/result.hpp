#pragma once

#include "io.hpp"
#include "option.hpp"

#include <concepts>
#include <expected>
#include <functional>
#include <source_location>
#include <type_traits>
#include <utility>

namespace rusty::result {
using namespace io;

template<typename T>
struct OkProxy {
    T val;
};

template<typename T, typename E>
struct [[nodiscard]] Result {
private:
    std::expected<T, E> inner;

public:
    explicit Result(OkProxy<T> proxy) : inner(std::move(proxy.val)) {
    }

    explicit Result(T val) : inner(std::move(val)) {
    }

    explicit Result(std::unexpected<E> err) : inner(std::move(err)) {
    }

    template<typename E2>
        requires(!std::same_as<E2, E>) && std::convertible_to<E2, E>
    explicit Result(std::unexpected<E2> err) :
        inner(std::unexpected<E>(static_cast<E>(std::move(err.error())))) {
    }

    [[nodiscard]] auto is_ok() const noexcept -> bool {
        return this->inner.has_value();
    }

    [[nodiscard]] auto is_err() const noexcept -> bool {
        return !this->inner.has_value();
    }

    [[nodiscard]] auto unwrap(
        std::source_location loc = std::source_location::current()
    ) && -> T {
        if (!this->inner) {
            if constexpr (std::formattable<E, char>) {
                rseprintln(
                    "panic at {}:{}: called unwrap() on Err({})",
                    loc.file_name(),
                    loc.line(),
                    this->inner.error()
                );
            } else {
                rseprintln(
                    "panic at {}:{}: called unwrap() on Err",
                    loc.file_name(),
                    loc.line()
                );
            }
            std::abort();
        }
        return std::move(*inner);
    }

    [[nodiscard]] auto unwrap_err(
        std::source_location loc = std::source_location::current()
    ) && -> E {
        if (this->inner) {
            rseprintln(
                "panic at {}:{}: called unwrap_err() on Ok",
                loc.file_name(),
                loc.line()
            );
            std::abort();
        }
        return std::move(this->inner.error());
    }

    [[nodiscard]] auto expect(
        std::string_view msg,
        std::source_location loc = std::source_location::current()
    ) && -> T {
        if (!this->inner) {
            rseprintln("panic at {}:{}: {}", loc.file_name(), loc.line(), msg);
            std::abort();
        }
        return std::move(*this->inner);
    }

    [[nodiscard]] auto unwrap_or(T default_val) && -> T {
        return std::move(this->inner).value_or(std::move(default_val));
    }

    template<std::invocable F>
        requires std::same_as<std::invoke_result_t<F>, T>
    [[nodiscard]] auto unwrap_or_else(F&& f) && -> T {
        if (this->inner) {
            return std::move(*this->inner);
        }
        return std::invoke(std::forward<F>(f));
    }

    template<std::invocable<T> F>
    [[nodiscard]] auto map(F&& f) && -> Result<std::invoke_result_t<F, T>, E> {
        if (!this->inner) {
            return std::unexpected(std::move(this->inner.error()));
        }
        return OkProxy<std::invoke_result_t<F, T>> {
            std::invoke(std::forward<F>(f), std::move(*this->inner))
        };
    }

    template<std::invocable<E> F>
    [[nodiscard]] auto map_err(
        F&& f
    ) && -> Result<T, std::invoke_result_t<F, E>> {
        if (this->inner) {
            return OkProxy<T> {std::move(*this->inner)};
        }
        return std::unexpected(
            std::invoke(std::forward<F>(f), std::move(this->inner.error()))
        );
    }

    template<std::invocable<T> F>
    [[nodiscard]] auto and_then(F&& f) && -> std::invoke_result_t<F, T> {
        if (!this->inner) {
            return std::unexpected(std::move(this->inner.error()));
        }
        return std::invoke(std::forward<F>(f), std::move(*this->inner));
    }

    template<std::invocable<E> F>
    [[nodiscard]] auto or_else(F&& f) && -> std::invoke_result_t<F, E> {
        if (this->inner) {
            return OkProxy<T> {std::move(*this->inner)};
        }
        return std::invoke(std::forward<F>(f), std::move(this->inner.error()));
    }

    [[nodiscard]] auto ok() && -> rusty::option::Option<T> {
        if (this->inner) {
            return std::move(*this->inner);
        }
        return std::nullopt;
    }

    [[nodiscard]] auto err() && -> rusty::option::Option<E> {
        if (!this->inner) {
            return std::move(this->inner.error());
        }
        return std::nullopt;
    }

    [[nodiscard]] auto has_value() const noexcept -> bool {
        return this->inner.has_value();
    }

    [[nodiscard]] auto value() & -> T& {
        return this->inner.value();
    }

    [[nodiscard]] auto value() const& -> const T& {
        return this->inner.value();
    }

    [[nodiscard]] auto value() && -> T&& {
        return std::move(this->inner).value();
    }

    [[nodiscard]] auto error() & -> E& {
        return this->inner.error();
    }

    [[nodiscard]] auto error() const& -> const E& {
        return this->inner.error();
    }

    [[nodiscard]] auto error() && -> E&& {
        return std::move(this->inner).error();
    }

    explicit operator bool() const noexcept {
        return this->inner.has_value();
    }

    auto operator*() & -> T& {
        return *this->inner;
    }

    auto operator*() const& -> const T& {
        return *this->inner;
    }

    auto operator*() && -> T&& {
        return std::move(*this->inner);
    }

    auto operator->() -> T* {
        return this->inner.operator->();
    }

    auto operator->() const -> const T* {
        return this->inner.operator->();
    }
};

template<typename T>
constexpr auto Ok(T&& val) -> OkProxy<std::decay_t<T>> {
    return OkProxy<std::decay_t<T>> {std::forward<T>(val)};
}

template<typename E>
constexpr auto Err(E&& err) {
    return std::unexpected(std::forward<E>(err));
}

} // namespace rusty::result
