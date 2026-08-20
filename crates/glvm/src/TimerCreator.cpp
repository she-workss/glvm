#include "glvm/TimerCreator.hpp"

#ifdef __linux__
#include "glvm/UnixApi/ChronoX.hpp"
#endif
#ifdef _WIN32
#include "glvm/WinApi/ChronoWin.hpp"
#endif

namespace glvm::Time {
IChrono* CTimerCreator::Create() {
#ifdef __linux__
    return new CTimerX;
#endif

#ifdef _WIN32
    return new CTimerWin;
#endif
}
} // namespace glvm::Time
