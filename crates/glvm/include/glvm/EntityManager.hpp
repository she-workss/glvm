#pragma once

#include "glvm/ComponentManager.hpp"
#include "glvm/Vector.hpp"

typedef unsigned int Entity_ID;

namespace glvm::ecs {
class EntityManager {
    static EntityManager* pInstance_;
    static std::mutex Mutex_;

    inline static Entity_ID u_iID = 0;
    core::vector<Entity_ID> tRemoved_Entity_Registry_;
    core::vector<Entity_ID> tActive_Entity_Registry_;

    EntityManager();

public: // TODO: Delete this.
    ~EntityManager();
    // Don't need to make cope because of singleton property.
    EntityManager(EntityManager& _entity_Manager) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const EntityManager& _entity_Manager) = delete;
    // It possibly to get only one instance of this class with this method.
    static EntityManager* GetInstance();
    [[nodiscard]] Entity_ID CreateEntity();
    void RemoveEntity(
        Entity_ID& _Entity_ID,
        ComponentManager* _ComponentManager
    );
    bool isEntitiesCollectionChanged = true;
};
} // namespace glvm::ecs
