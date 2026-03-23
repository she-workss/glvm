#pragma once

#include <utility>
#include <variant>

namespace rusty::match {
template<typename... Fs>
struct Overloaded: Fs... {
    using Fs::operator()...;
};

template<typename... Fs>
Overloaded(Fs...) -> Overloaded<Fs...>;

template<typename Variant, typename... Fs>
constexpr auto match(Variant&& var, Fs&&... fs) -> decltype(auto) {
    return std::visit(
        Overloaded {std::forward<Fs>(fs)...},
        std::forward<Variant>(var)
    );
}
} // namespace rusty::match
