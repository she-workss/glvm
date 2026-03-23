// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "glvm/i_chrono.hpp"

namespace GLVM::Time {
class CTimerCreator {
public:
    ~CTimerCreator() {}

    IChrono* Create();
};
} // namespace GLVM::Time
