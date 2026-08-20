#pragma once

#include "glvm/archetype_ecs/arch_ecs_utils.hpp"

#include <cstdint>
#include <mutex>
#include <vector>

namespace glvm::ecs::arch {

struct ArchetypeEntityManager {
    inline static uint32_t nextId = 0;
    std::vector<uint32_t> generations;
    std::vector<uint32_t> freeList;

    ArchetypeEntityManager();
    static ArchetypeEntityManager* getInstance();

    uint64_t createEntity();
    void removeEntity(uint64_t entity_);
    bool isAlive(uint64_t entity_) const;

private:
    static ArchetypeEntityManager* pInstance_;
    static std::mutex Mutex_;

    ~ArchetypeEntityManager();
};
}; // namespace glvm::ecs::arch
