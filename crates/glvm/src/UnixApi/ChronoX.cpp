#include "glvm/UnixApi/ChronoX.hpp"

#include <ctime>

namespace glvm::Time {
CTimerX::CTimerX() {
    InitFrequency();
    Reset();
}

double CTimerX::InitFrequency() {
    return lFrequency_ = 1e+9;
}

double CTimerX::Reset() {
    clock_gettime(CLOCK_MONOTONIC, &start_);
    return (start_.tv_sec + start_.tv_nsec);
}

double CTimerX::GetElapsed() {
    clock_gettime(CLOCK_MONOTONIC, &now_);
    lSeconds_ = now_.tv_sec - start_.tv_sec;
    lNanoseconds_ = now_.tv_nsec - start_.tv_nsec;
    return lSeconds_ + lNanoseconds_ / lFrequency_;
}
} // namespace glvm::Time
