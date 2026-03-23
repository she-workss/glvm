// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/timer_creator.hpp"

#ifdef __linux__
#include "glvm/unix_api/chrono_x.hpp"
#endif

#ifdef _WIN32
#include "glvm/win_api/chrono_win.hpp"
#endif

namespace GLVM::Time {
auto CTimerCreator::Create() -> IChrono* {
#ifdef __linux__
    return new CTimerX;
#endif

#ifdef _WIN32
    return new CTimerWin;
#endif
}
} // namespace GLVM::Time
