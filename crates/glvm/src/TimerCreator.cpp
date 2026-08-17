// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/TimerCreator.hpp"

#ifdef __linux__
#include "glvm/UnixApi/ChronoX.hpp"
#endif
#ifdef _WIN32
#include "glvm/WinApi/ChronoWin.hpp"
#endif

namespace GLVM::Time {
IChrono* CTimerCreator::Create() {
#ifdef __linux__
    return new CTimerX;
#endif

#ifdef _WIN32
    return new CTimerWin;
#endif
}
} // namespace GLVM::Time
