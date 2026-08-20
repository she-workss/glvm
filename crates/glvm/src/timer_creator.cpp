#include "glvm/timer_creator.hpp"

#ifdef __linux__
#include "glvm/unix_api/chrono_x.hpp"
#endif
#ifdef _WIN32
#include "glvm/win_api/chrono_win.hpp"
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
