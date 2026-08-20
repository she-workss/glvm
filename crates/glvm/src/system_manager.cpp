#include "glvm/system_manager.hpp"

namespace glvm::ecs {
CSystemManager* CSystemManager::pInstance_ = nullptr;
std::mutex CSystemManager::Mutex_;

CSystemManager::CSystemManager() {}

CSystemManager::~CSystemManager() {
    delete pInstance_;
    pInstance_ = nullptr;
}

CSystemManager* CSystemManager::GetInstance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new CSystemManager();
    }
    return pInstance_;
}

void CSystemManager::ActivateSystem(ISystem* _System) {
    tSystemContainer.push_back(_System);
    ++s_iSystem_ID;
}

void CSystemManager::DeactivateSystem(DeactivatedSystems system) {
    deactivatedSystems.push_back(system);
}

void CSystemManager::ReturnSystemToActivatedState(DeactivatedSystems system) {
    for (unsigned int i = 0; i < deactivatedSystems.size(); ++i) {
        if (system == deactivatedSystems[i]) {
            deactivatedSystems.erase(deactivatedSystems.begin() + i);
            return;
        }
    }
}

void CSystemManager::Update() {
    bool removedSystemFlag = false;
    for (unsigned int i = 0; i < s_iSystem_ID; ++i) {
        for (unsigned int j = 0; j < deactivatedSystems.size(); ++j) {
            if ((unsigned int)deactivatedSystems[j] == i) {
                removedSystemFlag = true;
                continue;
            }
        }

        if (removedSystemFlag) {
            removedSystemFlag = false;
            continue;
        } else {
            tSystemContainer[i]->Update();
        }
    }
}
} // namespace glvm::ecs
