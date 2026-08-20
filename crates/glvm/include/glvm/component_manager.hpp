#pragma once

#include "glvm/components/actor_component.hpp"
#include "glvm/components/animation_move_component.hpp"
#include "glvm/components/attack_component.hpp"
#include "glvm/components/collider_component.hpp"
#include "glvm/components/controller_component.hpp"
#include "glvm/components/directional_light_component.hpp"
#include "glvm/components/enemy_component.hpp"
#include "glvm/components/font_component.hpp"
#include "glvm/components/health_component.hpp"
#include "glvm/components/material_component.hpp"
#include "glvm/components/move_component.hpp"
#include "glvm/components/point_light_component.hpp"
#include "glvm/components/projectile_component.hpp"
#include "glvm/components/rigid_body_component.hpp"
#include "glvm/components/spot_light_component.hpp"
#include "glvm/components/state_component.hpp"
#include "glvm/components/transform_component.hpp"
#include "glvm/components/vertex_component.hpp"
#include "glvm/components/view_component.hpp"

#include <cassert>
#include <compare>
#include <concepts>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

namespace glvm::ecs {
class ComponentManager {
    static ComponentManager* pInstance_;
    static std::mutex Mutex_;
    unsigned int numberOfBaseComponents;

    ComponentManager();

    template<typename componentType>
    unsigned int CreateComponentContainer() {
        static unsigned int localContainerID = 0;
        static bool existComponentContainerFlag = false;
        if (existComponentContainerFlag) {
            return localContainerID;
        }
        // Give a value of global component container ID's counter to local
        // container ID of current component type.
        localContainerID = componentsContainerID;
        existComponentContainerFlag = true;
        // Create component container of current type.
        worldComponentsContainer.push_back(
            std::make_shared<std::vector<componentType>>()
        );
        // Create ID's component container.
        std::vector<unsigned int>* sparseEntitiesMapToComponents =
            new std::vector<unsigned int>;
        worldSparseEntitiesMapToComponents.push_back(
            sparseEntitiesMapToComponents
        );
        // Create ID's component container.
        std::vector<unsigned int>* denseEntitiesMapToComponents =
            new std::vector<unsigned int>;
        worldDenseComponentsMapToEntities.push_back(
            denseEntitiesMapToComponents
        );
        componentsTypes.push_back(typeid(componentType).name());
        ++componentsContainerID;
        return localContainerID;
    }

public:
    inline static unsigned int componentsContainerID = 0;
    // Contains all local containers for different types of components.
    std::vector<std::shared_ptr<void>> worldComponentsContainer;
    // Contains all local container with IDs for different types of components.
    std::vector<std::vector<unsigned int>*> worldSparseEntitiesMapToComponents;
    std::vector<std::vector<unsigned int>*> worldDenseComponentsMapToEntities;

    std::vector<const char*> componentsTypes;
    bool isComponentsCollectionChanged = true;

    ~ComponentManager();
    // Don't need to make cope because of singleton property.
    ComponentManager(ComponentManager& componentManager) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const ComponentManager& componentManager) = delete;
    // It possibly to get only one instance of this class with this method.
    static ComponentManager* GetInstance();

    template<typename componentType>
    void CreateComponent(const unsigned int& entity) {
        // Index for world components and world ID's containers.
        unsigned int localContainerID = 0;
        componentType Component;
        localContainerID = CreateComponentContainer<componentType>();

        std::vector<unsigned int>& sparse = *static_cast<std::vector<unsigned int>*>(
            worldSparseEntitiesMapToComponents[localContainerID]
        );
        std::vector<unsigned int>& dense = *static_cast<std::vector<unsigned int>*>(
            worldDenseComponentsMapToEntities[localContainerID]
        );
        std::vector<componentType>& components =
            *std::static_pointer_cast<std::vector<componentType>>(
                worldComponentsContainer[localContainerID]
            );
        if (checkAvailability(sparse, dense, entity)) {
            return;
        }

        if (entity >= sparse.size()) {
            sparse.resize(entity + 1);
        }

        assert(dense.size() == components.size());

        sparse[entity] = dense.size();
        dense.push_back(entity);
        components.push_back(Component);
        isComponentsCollectionChanged = true;
    }

    bool checkAvailability(
        std::vector<unsigned int>& sparse,
        std::vector<unsigned int>& dense,
        unsigned int entity
    );

    // Allow to give a various components to chosen entity.
    template<typename componentType1, typename componentType2, typename... Args>
    void CreateComponent(unsigned int& entity) {
        CreateComponent<componentType2, Args...>(entity);
        CreateComponent<componentType1>(entity);
    }

    template<typename componentType, typename... Args>
    std::vector<unsigned int> collectLinkedEntities() {
        numberOfBaseComponents = 0;
        unsigned int firstComponentArrayIndex =
            CreateComponentContainer<componentType>();
        std::vector<unsigned int>& dense = *static_cast<std::vector<unsigned int>*>(
            worldDenseComponentsMapToEntities[firstComponentArrayIndex]
        );

        if (dense.size() > 0) {
            ++numberOfBaseComponents;
            numberOfBaseComponents += sizeof...(Args);
        }
        std::vector<unsigned int> returnVector;
        for (unsigned int i = 0; i < dense.size(); ++i) {
            if (multiCheckAvailability<Args...>(dense[i])) {
                returnVector.push_back(dense[i]);
            }
        }
        return returnVector;
    }

    template<typename componentType, typename... Args>
    std::vector<unsigned int> collectUniqueLinkedEntities() {
        std::vector<unsigned int> baseSubSetEntities;
        baseSubSetEntities = collectLinkedEntities<componentType, Args...>();
        unsigned int numberOfComponentArrays = 0;
        for (unsigned int j = 0; j < baseSubSetEntities.size(); ++j) {
            numberOfComponentArrays = 0;
            for (unsigned int i = 0;
                 i < worldDenseComponentsMapToEntities.size();
                 ++i) {
                std::vector<unsigned int>& sparse =
                    *static_cast<std::vector<unsigned int>*>(
                        worldSparseEntitiesMapToComponents[i]
                    );
                std::vector<unsigned int>& dense = *static_cast<std::vector<unsigned int>*>(
                    worldDenseComponentsMapToEntities[i]
                );

                if (checkAvailability(sparse, dense, baseSubSetEntities[j])) {
                    ++numberOfComponentArrays;
                }
            }
            if (numberOfComponentArrays > numberOfBaseComponents) {
                baseSubSetEntities.erase(
                    baseSubSetEntities.begin() + baseSubSetEntities[j]
                );
                --j;
            }
        }
        return baseSubSetEntities;
    }

    template<typename... Args>
    bool multiCheckAvailability(unsigned int entity) {
        return (multiCheckAvailabilityBase<Args>(entity) && ...);
    }

    template<typename componentType>
    bool multiCheckAvailabilityBase(unsigned int entity) {
        unsigned int componentArrayIndex =
            CreateComponentContainer<componentType>();
        std::vector<unsigned int>& sparse = *static_cast<std::vector<unsigned int>*>(
            worldSparseEntitiesMapToComponents[componentArrayIndex]
        );
        std::vector<unsigned int>& dense = *static_cast<std::vector<unsigned int>*>(
            worldDenseComponentsMapToEntities[componentArrayIndex]
        );
        return checkAvailability(sparse, dense, entity);
    }

    template<typename componentType>
    bool isComponentExists(const unsigned int& entity) {
        unsigned int localContainerID;
        localContainerID = CreateComponentContainer<componentType>();
        std::vector<unsigned int>& sparse = *static_cast<std::vector<unsigned int>*>(
            worldSparseEntitiesMapToComponents[localContainerID]
        );
        std::vector<unsigned int>& dense = *static_cast<std::vector<unsigned int>*>(
            worldDenseComponentsMapToEntities[localContainerID]
        );
        return checkAvailability(sparse, dense, entity);
    }

    template<typename componentType>
    componentType* GetComponent(const unsigned int& entity) {
        unsigned int localContainerID;
        localContainerID = CreateComponentContainer<componentType>();
        std::vector<unsigned int>& sparse = *static_cast<std::vector<unsigned int>*>(
            worldSparseEntitiesMapToComponents[localContainerID]
        );
        std::vector<unsigned int>& dense = *static_cast<std::vector<unsigned int>*>(
            worldDenseComponentsMapToEntities[localContainerID]
        );
        std::vector<componentType>& components =
            *std::static_pointer_cast<std::vector<componentType>>(
                worldComponentsContainer[localContainerID]
            );
        if (checkAvailability(sparse, dense, entity)) {
            unsigned int componentIndex = sparse[entity];
            return &components[componentIndex];
        } else {
            return nullptr;
        }
    }

    // Don't need to delete real component in this method. Because systems don't
    // work with component without indices for that component in ordered
    // container.
    template<typename componentType>
    void RemoveComponent(unsigned int& entity) {
        unsigned int localContainerID;
        localContainerID = CreateComponentContainer<componentType>();
        std::vector<unsigned int>& sparse = *static_cast<std::vector<unsigned int>*>(
            worldSparseEntitiesMapToComponents[localContainerID]
        );
        std::vector<unsigned int>& dense = *static_cast<std::vector<unsigned int>*>(
            worldDenseComponentsMapToEntities[localContainerID]
        );
        std::vector<componentType>& components =
            *std::static_pointer_cast<std::vector<componentType>>(
                worldComponentsContainer[localContainerID]
            );
        if (checkAvailability(sparse, dense, entity)) {
            assert(dense.size() == components.size());
            unsigned int indexInDenseOfRemovableEntity = sparse[entity];
            unsigned int indexInSparseOfSwapableEntity = dense.back();
            const componentType& componentFromLastIndex = components.back();
            dense[indexInDenseOfRemovableEntity] =
                indexInSparseOfSwapableEntity;
            dense.pop_back();
            components[indexInDenseOfRemovableEntity] = componentFromLastIndex;
            components.pop_back();
            sparse[indexInSparseOfSwapableEntity] =
                indexInDenseOfRemovableEntity;
            isComponentsCollectionChanged = true;
        }
    }

    void RemoveAllComponents(unsigned int& entity) {
        for (unsigned int i = 0; i < worldComponentsContainer.size(); ++i) {
            if (componentsTypes[i] == typeid(components::transform).name()) {
                RemoveComponent<components::transform>(entity);
            } else if (componentsTypes[i] == typeid(components::beholder).name()) {
                RemoveComponent<components::beholder>(entity);
            } else if (componentsTypes[i] == typeid(components::rigidBody).name()) {
                RemoveComponent<components::rigidBody>(entity);
            } else if (componentsTypes[i] == typeid(components::collider).name()) {
                RemoveComponent<components::collider>(entity);
            } else if (
                componentsTypes[i]
                == typeid(components::directionalLight).name()
            ) {
                RemoveComponent<components::directionalLight>(entity);
            } else if (
                componentsTypes[i] == typeid(components::pointLight).name()
            ) {
                RemoveComponent<components::pointLight>(entity);
            } else if (componentsTypes[i] == typeid(components::spotLight).name()) {
                RemoveComponent<components::spotLight>(entity);
            } else if (componentsTypes[i] == typeid(components::material).name()) {
                RemoveComponent<components::material>(entity);
            } else if (componentsTypes[i] == typeid(components::move).name()) {
                RemoveComponent<components::move>(entity);
            } else if (componentsTypes[i] == typeid(components::mesh).name()) {
                RemoveComponent<components::mesh>(entity);
            } else if (
                componentsTypes[i]
                == typeid(glvm::ecs::components::controller).name()
            ) {
                RemoveComponent<glvm::ecs::components::controller>(entity);
            } else if (
                componentsTypes[i] == typeid(components::projectile).name()
            ) {
                RemoveComponent<components::projectile>(entity);
            } else if (componentsTypes[i] == typeid(components::enemy).name()) {
                RemoveComponent<components::enemy>(entity);
            } else if (componentsTypes[i] == typeid(components::font).name()) {
                RemoveComponent<components::font>(entity);
            } else if (componentsTypes[i] == typeid(components::health).name()) {
                RemoveComponent<components::health>(entity);
            } else if (componentsTypes[i] == typeid(components::state).name()) {
                RemoveComponent<components::state>(entity);
            } else if (componentsTypes[i] == typeid(components::actor).name()) {
                RemoveComponent<components::actor>(entity);
            } else {
                continue;
            }
        }
    }

    unsigned int GetContainerID();

    template<typename componentType>
    std::vector<componentType>* GetComponentContainer() {
        return std::static_pointer_cast<std::vector<componentType>>(
                   worldComponentsContainer
                       [CreateComponentContainer<componentType>()]
        )
            .get();
    }

    template<typename componentType>
    std::vector<unsigned int>* GetEntityContainer() {
        return static_cast<std::vector<unsigned int>*>(
            worldDenseComponentsMapToEntities
                [CreateComponentContainer<componentType>()]
        );
    }
};

} // namespace glvm::ecs
