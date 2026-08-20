#pragma once

#include "glvm/ArchetypeECS/ArchECS_Utils.hpp"
#include "glvm/Vector.hpp"

#include <cstdint>
#include <mutex>

namespace glvm::ecs::arch {

struct ArchetypeEntityManager {
    inline static id nextId = 0;
    core::vector<generation> generations;
    core::vector<id> freeList;

    ArchetypeEntityManager();
    static ArchetypeEntityManager* getInstance();

    entity createEntity();
    void removeEntity(entity entity_);
    bool isAlive(entity entity_) const;

private:
    static ArchetypeEntityManager* pInstance_;
    static std::mutex Mutex_;

    ~ArchetypeEntityManager();
};
}; // namespace glvm::ecs::arch
