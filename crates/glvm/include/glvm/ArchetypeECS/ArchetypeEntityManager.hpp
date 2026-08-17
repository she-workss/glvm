#ifndef ARCHETYPE_ENTITY_MANAGER
#define ARCHETYPE_ENTITY_MANAGER

#include "glvm/ArchetypeECS/ArchECS_Utils.hpp"
#include "glvm/Vector.hpp"

#include <cstdint>
#include <mutex>

namespace GLVM::ecs::arch {

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
}; // namespace GLVM::ecs::arch

#endif
