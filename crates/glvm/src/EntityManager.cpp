#include "glvm/EntityManager.hpp"

#include "glvm/ComponentManager.hpp"
#include "glvm/Vector.hpp"

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

[[nodiscard]] Entity_ID EntityManager::CreateEntity() {
    Entity_ID _Entity_ID;
    // Check out wether or not free ID in removed entities registry.
    if (tRemoved_Entity_Registry_.GetSize() > k_iNull) {
        _Entity_ID = tRemoved_Entity_Registry_.GetFirstItem();
        tActive_Entity_Registry_.Push(tRemoved_Entity_Registry_.GetFirstItem());
        tRemoved_Entity_Registry_.RemoveFirstItem();
    } else {
        tActive_Entity_Registry_.Push(u_iID);
        _Entity_ID = u_iID;
        ++u_iID;
    }
    isEntitiesCollectionChanged = true;
    return _Entity_ID;
}

// Don't need to delete real component in this method. Because systems dont work
// with component without indices for that component in ordered container.
void EntityManager::RemoveEntity(
    Entity_ID& _Entity_ID,
    ComponentManager* _ComponentManager
) {
    _ComponentManager->RemoveAllComponents(_Entity_ID);
    tActive_Entity_Registry_[_Entity_ID] = k_iUint_Max;
    tRemoved_Entity_Registry_.Push(_Entity_ID);
    isEntitiesCollectionChanged = true;
}
} // namespace glvm::ecs
