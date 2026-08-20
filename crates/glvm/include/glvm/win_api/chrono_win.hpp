#pragma once

#include "glvm/i_chrono.hpp"

#include <windows.h>

namespace glvm::Time {
class CTimerWin: public IChrono {
    __int64 i64Freq_;
    __int64 i64Start_;
    __int64 i64Now_;

public:
    CTimerWin();

    double InitFrequency();
    double Reset();
    double GetElapsed();
};
} // namespace glvm::Time
