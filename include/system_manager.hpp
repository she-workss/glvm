// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "i_system.hpp"
#include "vector.hpp"

#include <mutex>

namespace GLVM::ecs {
class CSystemManager: public ISystem {
    static CSystemManager* pInstance_;
    static std::mutex Mutex_;

    CSystemManager();
    ~CSystemManager();

public:
    // Dont need to make cope because of singleton property.
    CSystemManager(CSystemManager& _system_Manager) = delete;
    // Dont need assignment operator because of singleton property.
    void operator=(const CSystemManager& _system_Manager) = delete;
    // It possibly to get only one instance of this class with this method.
    static CSystemManager* GetInstance();

    inline static unsigned int s_iSystem_ID = 0;
    core::vector<ISystem*> tSystemContainer;

    void ActivateSystem(ISystem* _System);

    void Update() override;
};
} // namespace GLVM::ecs
