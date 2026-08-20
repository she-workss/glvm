#pragma once

#include "glvm/component_manager.hpp"

#include <vector>

namespace glvm::ecs {
class EntityManager {
    static EntityManager* pInstance_;
    static std::mutex Mutex_;

    inline static unsigned int u_iID = 0;
    std::vector<unsigned int> tRemoved_Entity_Registry_;
    std::vector<unsigned int> tActive_Entity_Registry_;

    EntityManager();

public: // TODO: Delete this.
    ~EntityManager();
    // Don't need to make cope because of singleton property.
    EntityManager(EntityManager& _entity_Manager) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const EntityManager& _entity_Manager) = delete;
    // It possibly to get only one instance of this class with this method.
    static EntityManager* GetInstance();
    [[nodiscard]] unsigned int CreateEntity();
    void RemoveEntity(
        unsigned int& _Entity_ID,
        ComponentManager* _ComponentManager
    );
    bool isEntitiesCollectionChanged = true;
};
} // namespace glvm::ecs
