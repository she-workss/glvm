#include "glvm/entity_manager.hpp"

#include "glvm/component_manager.hpp"
#include "glvm/constants.hpp"

#include <vector>

namespace glvm::ecs {
EntityManager* EntityManager::pInstance_ = nullptr;
std::mutex EntityManager::Mutex_;

EntityManager::EntityManager() {}

EntityManager::~EntityManager() {}

EntityManager* EntityManager::GetInstance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new EntityManager();
    }
    return pInstance_;
}

[[nodiscard]] unsigned int EntityManager::CreateEntity() {
    unsigned int _Entity_ID;
    // Check out wether or not free ID in removed entities registry.
    if (tRemoved_Entity_Registry_.size() > k_iNull) {
        _Entity_ID = tRemoved_Entity_Registry_.front();
        tActive_Entity_Registry_.push_back(tRemoved_Entity_Registry_.front());
        tRemoved_Entity_Registry_.erase(tRemoved_Entity_Registry_.begin());
    } else {
        tActive_Entity_Registry_.push_back(u_iID);
        _Entity_ID = u_iID;
        ++u_iID;
    }
    isEntitiesCollectionChanged = true;
    return _Entity_ID;
}

// Don't need to delete real component in this method. Because systems dont work
// with component without indices for that component in ordered container.
void EntityManager::RemoveEntity(
    unsigned int& _Entity_ID,
    ComponentManager* _ComponentManager
) {
    _ComponentManager->RemoveAllComponents(_Entity_ID);
    tActive_Entity_Registry_[_Entity_ID] = k_iUint_Max;
    tRemoved_Entity_Registry_.push_back(_Entity_ID);
    isEntitiesCollectionChanged = true;
}
} // namespace glvm::ecs
