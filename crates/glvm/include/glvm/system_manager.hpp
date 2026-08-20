#pragma once

#include "glvm/i_system.hpp"

#include <mutex>
#include <vector>

namespace glvm::ecs {
enum DeactivatedSystems { DEACTIVATED_MOVEMENT_SYSTEM };

class CSystemManager: public ISystem {
    static CSystemManager* pInstance_;
    static std::mutex Mutex_;
    std::vector<DeactivatedSystems> deactivatedSystems;

    CSystemManager();

public:
    ~CSystemManager();
    // Don't need to make cope because of singleton property.
    CSystemManager(CSystemManager& _system_Manager) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const CSystemManager& _system_Manager) = delete;
    // It possibly to get only one instance of this class whith this method.
    static CSystemManager* GetInstance();

    inline static unsigned int s_iSystem_ID = 0;
    std::vector<ISystem*> tSystemContainer;

    void ActivateSystem(ISystem* _System);
    void DeactivateSystem(DeactivatedSystems system);
    void ReturnSystemToActivatedState(DeactivatedSystems system);

    void Update() override;
};
} // namespace glvm::ecs
