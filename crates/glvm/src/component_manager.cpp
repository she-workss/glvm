#include "glvm/component_manager.hpp"

namespace glvm::ecs {
ComponentManager* ComponentManager::pInstance_ = nullptr;
std::mutex ComponentManager::Mutex_;

ComponentManager::ComponentManager() = default;

ComponentManager::~ComponentManager() {
    for (int j = 0, iSize_Ordered = worldSparseEntitiesMapToComponents.size();
         j < iSize_Ordered;
         ++j) {
        delete worldSparseEntitiesMapToComponents[j];
        worldSparseEntitiesMapToComponents[j] = nullptr;
    }
    for (int j = 0, iSize_Ordered = worldDenseComponentsMapToEntities.size();
         j < iSize_Ordered;
         ++j) {
        delete worldDenseComponentsMapToEntities[j];
        worldDenseComponentsMapToEntities[j] = nullptr;
    }
}

bool ComponentManager::checkAvailability(
    std::vector<unsigned int>& sparse,
    std::vector<unsigned int>& dense,
    unsigned int entity
) {
    return entity < sparse.size() && sparse[entity] < dense.size()
        && dense[sparse[entity]] == entity;
}

unsigned int ComponentManager::GetContainerID() {
    return componentsContainerID;
}

ComponentManager* ComponentManager::GetInstance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new ComponentManager();
    }
    return pInstance_;
}
} // namespace glvm::ecs
