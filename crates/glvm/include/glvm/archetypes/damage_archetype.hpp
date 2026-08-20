#pragma once

#include "glvm/archetype_ecs/archetype_interface.hpp"
#include "glvm/components/attack_component.hpp"
#include "glvm/components/font_component.hpp"
#include "glvm/components/health_component.hpp"
#include "glvm/globals.hpp"

namespace glvm::ecs::arch {
constexpr uint32_t DAMAGE_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(components::attack) + sizeof(components::health)
       + sizeof(components::font));

struct DamageArchetype: Archetype {
    components::attack attacks[DAMAGE_ARCH_CHUNK_SIZE];
    components::health health[DAMAGE_ARCH_CHUNK_SIZE];
    components::font fonts[DAMAGE_ARCH_CHUNK_SIZE];

    DamageArchetype() {
        components[ComponentsIndices::ATTACK_COMPONENT] = attacks;
        components[ComponentsIndices::HEALTH_COMPONENT] = health;
        components[ComponentsIndices::FONT_COMPONENT] = fonts;

        mask = (1ull << ComponentsIndices::ATTACK_COMPONENT)
            | (1ull << ComponentsIndices::HEALTH_COMPONENT)
            | (1ull << ComponentsIndices::FONT_COMPONENT);

        componentIds[0] = ComponentsIndices::ATTACK_COMPONENT;
        componentIds[1] = ComponentsIndices::HEALTH_COMPONENT;
        componentIds[2] = ComponentsIndices::FONT_COMPONENT;
        componentCount = 3;
    }
};
}; // namespace glvm::ecs::arch
