#include "glvm/ArchetypeECS/ArchECS_Utils.hpp"

namespace glvm::ecs::arch {
bool matchesRequiredMask(
    const componentMask archetypeMask,
    const componentMask& systemMask
) {
    return (archetypeMask & systemMask) == systemMask;
}

entity makeEntity(id id_, generation generation_) {
    return ((uint64_t)generation_ << ENTITY_ID_BITS) | id_;
}

id getId(entity entity_) {
    return entity_ & entityBitsMask;
}

generation getGen(entity entity_) {
    return entity_ >> ENTITY_ID_BITS;
}
}; // namespace glvm::ecs::arch
