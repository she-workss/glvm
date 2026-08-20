#pragma once

#include "glvm/i_chrono.hpp"

namespace glvm::Time {
class CTimerCreator {
public:
    ~CTimerCreator() {}

    IChrono* Create();
};
} // namespace glvm::Time
