#include "glvm/archetype_ecs/arch_ecs_utils.hpp"

namespace glvm::ecs::arch {
bool matchesRequiredMask(
    const uint64_t archetypeMask,
    const uint64_t& systemMask
) {
    return (archetypeMask & systemMask) == systemMask;
}

uint64_t makeEntity(uint32_t id_, uint32_t generation_) {
    return ((uint64_t)generation_ << ENTITY_ID_BITS) | id_;
}

uint32_t getId(uint64_t entity_) {
    return entity_ & entityBitsMask;
}

uint32_t getGen(uint64_t entity_) {
    return entity_ >> ENTITY_ID_BITS;
}
}; // namespace glvm::ecs::arch
