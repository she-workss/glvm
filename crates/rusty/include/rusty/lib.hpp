#pragma once

#include "types.hpp"

#include <memory>
#include <utility>

namespace rusty {
using namespace types;

template<typename T, typename... Args>
auto make_box(Args&&... args) -> Box<T> {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
auto make_rc(Args&&... args) -> Rc<T> {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T>
struct PhantomData {
    constexpr auto operator==(const PhantomData&) const -> bool = default;
};
} // namespace rusty
