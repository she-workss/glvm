#pragma once

#include "io.hpp"
#include "types.hpp"

#include <concepts>
#include <functional>
#include <optional>
#include <source_location>
#include <type_traits>
#include <utility>

namespace rusty::option {
using namespace io;
using namespace types;

template<typename T>
struct IsOptionTrait: std::false_type {};

template<typename T>
concept IsOption = IsOptionTrait<T>::value;

template<typename T>
struct [[nodiscard]] Option {
private:
    std::optional<T> inner;

public:
    Option() noexcept : inner(std::nullopt) {}

    explicit Option(std::nullopt_t /*unused*/) noexcept : inner(std::nullopt) {}

    explicit Option(T val) noexcept(std::is_nothrow_move_constructible_v<T>) :
        inner(std::move(val)) {}

    [[nodiscard]] auto is_some() const noexcept -> bool {
        return this->inner.has_value();
    }

    [[nodiscard]] auto is_none() const noexcept -> bool {
        return !this->inner.has_value();
    }

    [[nodiscard]] auto unwrap(
        std::source_location loc = std::source_location::current()
    ) && -> T {
        if (!this->inner) {
            rseprintln(
                "panic at {}:{}: called unwrap() on None",
                loc.file_name(),
                loc.line()
            );
            std::abort();
        }
        return std::move(*this->inner);
    }

    [[nodiscard]] auto unwrap(
        std::source_location loc = std::source_location::current()
    ) & -> T& {
        if (!this->inner) {
            rseprintln(
                "panic at {}:{}: called unwrap() on None",
                loc.file_name(),
                loc.line()
            );
            std::abort();
        }
        return *this->inner;
    }

    [[nodiscard]] auto expect(
        str msg,
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

    [[nodiscard]] auto unwrap_or_default() && -> T
        requires std::default_initializable<T>
    {
        return std::move(this->inner).value_or(T {});
    }

    template<std::invocable<T> F>
    [[nodiscard]] auto map(F&& f) && -> Option<std::invoke_result_t<F, T>> {
        if (!this->inner) {
            return std::nullopt;
        }
        return std::invoke(std::forward<F>(f), std::move(*this->inner));
    }

    template<std::invocable<T> F>
        requires IsOption<std::invoke_result_t<F, T>>
    [[nodiscard]] auto and_then(F&& f) && -> std::invoke_result_t<F, T> {
        if (!this->inner) {
            return std::nullopt;
        }
        return std::invoke(std::forward<F>(f), std::move(*this->inner));
    }

    template<std::invocable F>
        requires std::same_as<std::invoke_result_t<F>, Option<T>>
    [[nodiscard]] auto or_else(F&& f) && -> Option<T> {
        if (this->inner) {
            return std::move(*this);
        }
        return std::invoke(std::forward<F>(f));
    }

    [[nodiscard]] auto filter(std::predicate<T> auto&& pred) && -> Option<T> {
        if (this->inner && std::invoke(pred, *this->inner)) {
            return std::move(*this);
        }
        return std::nullopt;
    }

    [[nodiscard]] auto take() noexcept -> Option<T> {
        return Option<T>(std::exchange(this->inner, std::nullopt));
    }

    auto replace(T val) noexcept(std::is_nothrow_move_constructible_v<T>)
        -> Option<T> {
        return Option<T>(std::exchange(this->inner, std::move(val)));
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

    auto operator==(std::nullopt_t /*unused*/) const noexcept -> bool {
        return is_none();
    }

    auto operator==(const Option& other) const noexcept -> bool
        requires std::equality_comparable<T>
    {
        return this->inner == other.inner;
    }
};

template<typename T>
struct IsOptionTrait<Option<T>>: std::true_type {};

template<typename T>
constexpr auto Some(T&& val) -> Option<std::decay_t<T>> {
    return std::forward<T>(val);
}
} // namespace rusty::option
