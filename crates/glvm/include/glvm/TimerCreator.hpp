#pragma once

#include "glvm/IChrono.hpp"

namespace glvm::Time {
class CTimerCreator {
public:
    ~CTimerCreator() {}

    IChrono* Create();
};
} // namespace glvm::Time
