#include "glvm/archetype_ecs/archetype_entity_manager.hpp"

#include <cstdint>

namespace glvm::ecs::arch {
ArchetypeEntityManager* ArchetypeEntityManager::pInstance_ = nullptr;
std::mutex ArchetypeEntityManager::Mutex_;

ArchetypeEntityManager::ArchetypeEntityManager() {}

ArchetypeEntityManager::~ArchetypeEntityManager() {}

ArchetypeEntityManager* ArchetypeEntityManager::getInstance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new ArchetypeEntityManager();
    }
    return pInstance_;
}

[[nodiscard]] uint64_t ArchetypeEntityManager::createEntity() {
    uint32_t newId = 0;
    // Check out wether or not free ID in removed entities registry.
    if (!freeList.empty()) {
        newId = freeList.back();
        freeList.pop_back();
    } else {
        newId = nextId++;
        generations.push_back(1);
    }

    return makeEntity(newId, generations[newId]);
}

void ArchetypeEntityManager::removeEntity(uint64_t entity_) {
    uint32_t id_ = getId(entity_);

    if (!isAlive(entity_)) {
        return;
    }

    generations[id_]++;
    freeList.push_back(id_);
}

bool ArchetypeEntityManager::isAlive(uint64_t entity_) const {
    uint32_t id_ = getId(entity_);

    if (!(id_ < generations.size())) {
        std::cout << "id < getSize protuh" << std::endl;
    }

    if (!(generations[id_] == getGen(entity_))) {
        std::cout << "gen == getGen protuh" << std::endl;
        std::cout << "generation[id_]: " << generations[id_] << std::endl;
        std::cout << "genGet( entity_ ): " << getGen(entity_) << std::endl;
        std::cout << "entity: " << entity_ << std::endl;
    }

    return id_ < generations.size() && generations[id_] == getGen(entity_);
}
}; // namespace glvm::ecs::arch
