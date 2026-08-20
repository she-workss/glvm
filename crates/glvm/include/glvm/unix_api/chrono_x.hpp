#pragma once

#include "glvm/i_chrono.hpp"

#include <ctime>

namespace glvm::Time {
class CTimerX: public IChrono {
    timespec start_;
    timespec now_;
    double lFrequency_;
    double lSeconds_;
    double lNanoseconds_;

public:
    CTimerX();

    double InitFrequency();
    double Reset();
    double GetElapsed();
};
} // namespace glvm::Time
