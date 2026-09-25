#include "assets/textures/container2.h"
#include "assets/textures/container2_specular.h"
#include "assets/textures/crosshair.h"
#include "assets/textures/font_atlas.h"
#include "assets/textures/gray.h"
#include "assets/textures/human.h"
#include "assets/textures/inventory_slot.h"
#include "assets/textures/tileset.h"
#include "assets/textures/witch.h"
#include "glvm/glvm.hpp"
#include "glvm_log/prelude.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <random>

using namespace glvm;
using namespace glvm_log::prelude;

enum States : u8 { IDLE, ATTACK, ROAMING };

struct CrosshairTagComponent {};

struct LevelChunkTagComponent {};

struct PlayerTagComponent {};

struct ProjectileTagComponent {};

struct StaticMeshTagComponent {};

struct Crosshair {};

struct Enemy {
    f32 detect_radius;
};

struct InventorySlot {
    u32 item_entity = UINT_MAX;
};

struct ItemSlotType {
    u32 height;
    u32 width;
};

struct Item {
    // Array that contains entities with InventorySlotComponent.
    Vec<u32> occupied_slots;
    ItemSlotType item_slot_type {};
    bool is_actor {};
};

struct Projectile {
public:
    u32 owner {};
    bool collision_status = false;
};

struct State {
    States state;
};

struct Inventory {
public:
    Inventory() {
        for (u32 i = 0; i < row; ++i) {
            slots[i] = new u32[col];
        }

        for (u32 i = 0; i < row; ++i) {
            for (u32 j = 0; j < col; ++j) {
                slots[i][j] = -1;
            }
        }
    }

    Inventory(const Inventory& inv) {
        for (u32 i = 0; i < row; ++i) {
            this->slots[i] = new u32[col];
        }

        for (u32 i = 0; i < row; ++i) {
            for (u32 j = 0; j < col; ++j) {
                this->slots[i][j] = inv.slots[i][j];
            }
        }

        this->entity_owner = inv.entity_owner;
        this->highlighted_slots = inv.highlighted_slots;
        this->is_available_highlighted_slots =
            inv.is_available_highlighted_slots;
    }

    ~Inventory() {
        for (u32 i = 0; i < row; ++i) {
            delete[] slots[i];
        }

        delete[] slots;
    }

    u32 row = 8;
    u32 col = 8;
    // Array with entities containing InventorySlotComponents.
    u32** slots = new u32*[row];
    u32 entity_owner = UINT_MAX;
    Vec<u32> highlighted_slots;
    bool is_available_highlighted_slots = false;
    MeshHandle slot_mesh_id {};
    f32 slot_scale {};
};

// Component indices owned by the game. Engine indices occupy 0..31;
// both share the 64-bit archetype mask, so every index must stay below 64
// (see MAX_COMPONENTS in the engine).
struct GameComponentsIndices {
    enum Types : u32 {
        HealthComponent = 32,
        DamageComponent,
        AttackComponent,
        MoveComponent,
        RotationComponent,
        StateComponent,
        EnemyComponent,
        InventoryComponent,
        ItemComponent,
        ProjectileBundleComponent,
        LevelChunkTagComponent,
        PlayerTagComponent,
        CrosshairTagComponent,
        StaticMeshTagComponent,
        ProjectileTagComponent,
    };
};

struct Health {
    /// Maximum health points.
    f32 max_health;
    /// Current health points.
    f32 current_health;
    /// Whether a health bar is drawn for the entity.
    bool renderable = true;
};

struct Damage {
    f32 maximum_damage;
    f32 minimum_damage;
    f32 critical_hit_rate;
    f32 critical_modifier;
};

struct Attack {
    f32 damage;
};

struct Move {
    EventKind event = EventKind::Default;
    Vector<f32, 3> frame_movement {0.0f, 0.0f, 0.0f};
    Vector<f32, 3> gravity {0.0f, 0.0f, 0.0f};
};

struct Rotation {
    f32 yaw = 0.0f;
    f32 pitch = 0.0f;
};

struct ProjectileBundle {
    Projectile projectile;
    Damage damage {};
    Material material;
};

constexpr auto CROSSHAIR_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Material)
       + sizeof(CrosshairTagComponent));

struct CrosshairArchetype: Archetype {
    Array<Transform, CROSSHAIR_ARCH_CHUNK_SIZE> transforms;
    Array<Mesh, CROSSHAIR_ARCH_CHUNK_SIZE> meshes;
    Array<Material, CROSSHAIR_ARCH_CHUNK_SIZE> materials;
    Array<CrosshairTagComponent, CROSSHAIR_ARCH_CHUNK_SIZE>
        crosshair_tag_components {};

    CrosshairArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[ComponentsIndices::MaterialComponent] = materials.data();
        components[GameComponentsIndices::CrosshairTagComponent] =
            crosshair_tag_components.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << GameComponentsIndices::CrosshairTagComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::MaterialComponent;
        component_ids[3] = GameComponentsIndices::CrosshairTagComponent;
        component_count = 4;
    }
};

constexpr auto DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Material)
       + sizeof(DirectionalLightComponent));

struct DirectionalLightArchetype: Archetype {
    Array<Transform, DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE> transforms;
    Array<Mesh, DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE> meshes;
    Array<Material, DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE> materials;
    Array<DirectionalLightComponent, DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE>
        directional_lights;

    DirectionalLightArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[ComponentsIndices::MaterialComponent] = materials.data();
        components[ComponentsIndices::DirectionalLightComponent] =
            directional_lights.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::DirectionalLightComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::MaterialComponent;
        component_ids[3] = ComponentsIndices::DirectionalLightComponent;
        component_count = 4;
    }
};

constexpr auto ENEMY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Enemy) + sizeof(State) + sizeof(Font)
       + sizeof(Animation) + sizeof(Material) + sizeof(Mesh) + sizeof(Collider)
       + sizeof(ColliderFlags) + sizeof(Health) + sizeof(RigidBody)
       + sizeof(Attack) + sizeof(Rotation) + sizeof(Move));

struct EnemyArchetype: Archetype {
    Array<Transform, ENEMY_ARCH_CHUNK_SIZE> transforms;
    Array<Enemy, ENEMY_ARCH_CHUNK_SIZE> enemies {};
    Array<State, ENEMY_ARCH_CHUNK_SIZE> states {};
    Array<Font, ENEMY_ARCH_CHUNK_SIZE> fonts;
    Array<Animation, ENEMY_ARCH_CHUNK_SIZE> animations;
    Array<Material, ENEMY_ARCH_CHUNK_SIZE> materials;
    Array<Mesh, ENEMY_ARCH_CHUNK_SIZE> meshes;
    Array<Collider, ENEMY_ARCH_CHUNK_SIZE> colliders;
    Array<ColliderFlags, ENEMY_ARCH_CHUNK_SIZE> collider_flags {};
    Array<Health, ENEMY_ARCH_CHUNK_SIZE> health {};
    Array<RigidBody, ENEMY_ARCH_CHUNK_SIZE> rigid_bodies;
    Array<Attack, ENEMY_ARCH_CHUNK_SIZE> attacks {};
    Array<Rotation, ENEMY_ARCH_CHUNK_SIZE> rotations;
    Array<Move, ENEMY_ARCH_CHUNK_SIZE> moves;

    EnemyArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[GameComponentsIndices::EnemyComponent] = enemies.data();
        components[GameComponentsIndices::StateComponent] = states.data();
        components[ComponentsIndices::FontComponent] = fonts.data();
        components[ComponentsIndices::AnimationComponent] = animations.data();
        components[ComponentsIndices::MaterialComponent] = materials.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[ComponentsIndices::ColliderComponent] = colliders.data();
        components[ComponentsIndices::ColliderFlagsComponent] =
            collider_flags.data();
        components[GameComponentsIndices::HealthComponent] = health.data();
        components[ComponentsIndices::RigidBodyComponent] = rigid_bodies.data();
        components[GameComponentsIndices::AttackComponent] = attacks.data();
        components[GameComponentsIndices::RotationComponent] = rotations.data();
        components[GameComponentsIndices::MoveComponent] = moves.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << GameComponentsIndices::EnemyComponent)
            | (1ull << GameComponentsIndices::StateComponent)
            | (1ull << ComponentsIndices::FontComponent)
            | (1ull << ComponentsIndices::AnimationComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << GameComponentsIndices::HealthComponent)
            | (1ull << ComponentsIndices::RigidBodyComponent)
            | (1ull << GameComponentsIndices::AttackComponent)
            | (1ull << GameComponentsIndices::RotationComponent)
            | (1ull << GameComponentsIndices::MoveComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = GameComponentsIndices::EnemyComponent;
        component_ids[2] = GameComponentsIndices::StateComponent;
        component_ids[3] = ComponentsIndices::FontComponent;
        component_ids[4] = ComponentsIndices::AnimationComponent;
        component_ids[5] = ComponentsIndices::MaterialComponent;
        component_ids[6] = ComponentsIndices::MeshComponent;
        component_ids[7] = ComponentsIndices::ColliderComponent;
        component_ids[8] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[9] = GameComponentsIndices::HealthComponent;
        component_ids[10] = ComponentsIndices::RigidBodyComponent;
        component_ids[11] = GameComponentsIndices::AttackComponent;
        component_ids[12] = GameComponentsIndices::RotationComponent;
        component_ids[13] = GameComponentsIndices::MoveComponent;
        component_count = 14;
    }
};

constexpr auto INVENTORY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Inventory) + sizeof(Material));

struct InventoryArchetype: Archetype {
    Array<Transform, INVENTORY_ARCH_CHUNK_SIZE> transforms;
    Array<Mesh, INVENTORY_ARCH_CHUNK_SIZE> meshes;
    Array<Inventory, INVENTORY_ARCH_CHUNK_SIZE> inventories;
    Array<Material, INVENTORY_ARCH_CHUNK_SIZE> materials;

    InventoryArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[GameComponentsIndices::InventoryComponent] =
            inventories.data();
        components[ComponentsIndices::MaterialComponent] = materials.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << GameComponentsIndices::InventoryComponent)
            | (1ull << ComponentsIndices::MaterialComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = GameComponentsIndices::InventoryComponent;
        component_ids[3] = ComponentsIndices::MaterialComponent;
        component_count = 4;
    }
};

constexpr auto ITEM_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Collider) + sizeof(ColliderFlags)
       + sizeof(Mesh) + sizeof(RigidBody) + sizeof(Material) + sizeof(Rotation)
       + sizeof(Move) + sizeof(Item));

struct ItemArchetype: Archetype {
    Array<Transform, ITEM_ARCH_CHUNK_SIZE> transforms;
    Array<Collider, ITEM_ARCH_CHUNK_SIZE> colliders;
    Array<ColliderFlags, ITEM_ARCH_CHUNK_SIZE> collider_flags {};
    Array<Mesh, ITEM_ARCH_CHUNK_SIZE> meshes;
    Array<RigidBody, ITEM_ARCH_CHUNK_SIZE> rigid_bodies;
    Array<Material, ITEM_ARCH_CHUNK_SIZE> materials;
    Array<Rotation, ITEM_ARCH_CHUNK_SIZE> rotations;
    Array<Move, ITEM_ARCH_CHUNK_SIZE> moves;
    Array<Item, ITEM_ARCH_CHUNK_SIZE> items;

    ItemArchetype();
};

ItemArchetype::ItemArchetype() {
    components[ComponentsIndices::TransformComponent] = transforms.data();
    components[ComponentsIndices::ColliderComponent] = colliders.data();
    components[ComponentsIndices::ColliderFlagsComponent] =
        collider_flags.data();
    components[ComponentsIndices::MeshComponent] = meshes.data();
    components[ComponentsIndices::RigidBodyComponent] = rigid_bodies.data();
    components[ComponentsIndices::MaterialComponent] = materials.data();
    components[GameComponentsIndices::RotationComponent] = rotations.data();
    components[GameComponentsIndices::MoveComponent] = moves.data();
    components[GameComponentsIndices::ItemComponent] = items.data();

    mask = (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::ColliderComponent)
        | (1ull << ComponentsIndices::ColliderFlagsComponent)
        | (1ull << ComponentsIndices::MeshComponent)
        | (1ull << ComponentsIndices::RigidBodyComponent)
        | (1ull << ComponentsIndices::MaterialComponent)
        | (1ull << GameComponentsIndices::RotationComponent)
        | (1ull << GameComponentsIndices::MoveComponent)
        | (1ull << GameComponentsIndices::ItemComponent);

    component_ids[0] = ComponentsIndices::TransformComponent;
    component_ids[1] = ComponentsIndices::ColliderComponent;
    component_ids[2] = ComponentsIndices::ColliderFlagsComponent;
    component_ids[3] = ComponentsIndices::MeshComponent;
    component_ids[4] = ComponentsIndices::RigidBodyComponent;
    component_ids[5] = ComponentsIndices::MaterialComponent;
    component_ids[6] = GameComponentsIndices::RotationComponent;
    component_ids[7] = GameComponentsIndices::MoveComponent;
    component_ids[8] = GameComponentsIndices::ItemComponent;
    component_count = 9;
}

constexpr auto LEVEL_CHUNK_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Material) + sizeof(Mesh) + sizeof(Collider)
       + sizeof(ColliderFlags) + sizeof(Rotation)
       + sizeof(LevelChunkTagComponent));

struct LevelChunkArchetype: Archetype {
    Array<Transform, LEVEL_CHUNK_ARCH_CHUNK_SIZE> transforms;
    Array<Material, LEVEL_CHUNK_ARCH_CHUNK_SIZE> materials;
    Array<Mesh, LEVEL_CHUNK_ARCH_CHUNK_SIZE> meshes;
    Array<Collider, LEVEL_CHUNK_ARCH_CHUNK_SIZE> colliders;
    Array<ColliderFlags, LEVEL_CHUNK_ARCH_CHUNK_SIZE> collider_flags {};
    Array<Rotation, LEVEL_CHUNK_ARCH_CHUNK_SIZE> rotations;
    Array<LevelChunkTagComponent, LEVEL_CHUNK_ARCH_CHUNK_SIZE>
        level_chunk_tag_components {};

    LevelChunkArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::MaterialComponent] = materials.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[ComponentsIndices::ColliderComponent] = colliders.data();
        components[ComponentsIndices::ColliderFlagsComponent] =
            collider_flags.data();
        components[GameComponentsIndices::RotationComponent] = rotations.data();
        components[GameComponentsIndices::LevelChunkTagComponent] =
            level_chunk_tag_components.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << GameComponentsIndices::RotationComponent)
            | (1ull << GameComponentsIndices::LevelChunkTagComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MaterialComponent;
        component_ids[2] = ComponentsIndices::MeshComponent;
        component_ids[3] = ComponentsIndices::ColliderComponent;
        component_ids[4] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[5] = GameComponentsIndices::RotationComponent;
        component_ids[6] = GameComponentsIndices::LevelChunkTagComponent;
        component_count = 7;
    }
};

constexpr auto PLAYER_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Beholder) + sizeof(Collider)
       + sizeof(ColliderFlags) + sizeof(Mesh) + sizeof(RigidBody)
       + sizeof(Health) + sizeof(Material) + sizeof(Move) + sizeof(Attack)
       + sizeof(Animation) + sizeof(Font) + sizeof(Rotation)
       + sizeof(PlayerTagComponent));

struct PlayerArchetype: Archetype {
    Array<Transform, PLAYER_ARCH_CHUNK_SIZE> transforms;
    Array<Beholder, PLAYER_ARCH_CHUNK_SIZE> beholders;
    Array<Collider, PLAYER_ARCH_CHUNK_SIZE> colliders;
    Array<ColliderFlags, PLAYER_ARCH_CHUNK_SIZE> collider_flags {};
    Array<Mesh, PLAYER_ARCH_CHUNK_SIZE> meshes;
    Array<RigidBody, PLAYER_ARCH_CHUNK_SIZE> rigid_bodies;
    Array<Health, PLAYER_ARCH_CHUNK_SIZE> health {};
    Array<Material, PLAYER_ARCH_CHUNK_SIZE> materials;
    Array<Move, PLAYER_ARCH_CHUNK_SIZE> moves;
    Array<Attack, PLAYER_ARCH_CHUNK_SIZE> attacks {};
    Array<Animation, PLAYER_ARCH_CHUNK_SIZE> animations;
    Array<Font, PLAYER_ARCH_CHUNK_SIZE> fonts;
    Array<Rotation, PLAYER_ARCH_CHUNK_SIZE> rotations;
    Array<PlayerTagComponent, PLAYER_ARCH_CHUNK_SIZE> player_tag_components {};

    PlayerArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::ViewComponent] = beholders.data();
        components[ComponentsIndices::ColliderComponent] = colliders.data();
        components[ComponentsIndices::ColliderFlagsComponent] =
            collider_flags.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[ComponentsIndices::RigidBodyComponent] = rigid_bodies.data();
        components[GameComponentsIndices::HealthComponent] = health.data();
        components[ComponentsIndices::MaterialComponent] = materials.data();
        components[GameComponentsIndices::MoveComponent] = moves.data();
        components[GameComponentsIndices::AttackComponent] = attacks.data();
        components[ComponentsIndices::AnimationComponent] = animations.data();
        components[ComponentsIndices::FontComponent] = fonts.data();
        components[GameComponentsIndices::RotationComponent] = rotations.data();
        components[GameComponentsIndices::PlayerTagComponent] =
            player_tag_components.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::ViewComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::RigidBodyComponent)
            | (1ull << GameComponentsIndices::HealthComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << GameComponentsIndices::MoveComponent)
            | (1ull << GameComponentsIndices::AttackComponent)
            | (1ull << ComponentsIndices::AnimationComponent)
            | (1ull << ComponentsIndices::FontComponent)
            | (1ull << GameComponentsIndices::RotationComponent)
            | (1ull << GameComponentsIndices::PlayerTagComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::ViewComponent;
        component_ids[2] = ComponentsIndices::ColliderComponent;
        component_ids[3] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[4] = ComponentsIndices::MeshComponent;
        component_ids[5] = ComponentsIndices::RigidBodyComponent;
        component_ids[6] = GameComponentsIndices::HealthComponent;
        component_ids[7] = ComponentsIndices::MaterialComponent;
        component_ids[8] = GameComponentsIndices::MoveComponent;
        component_ids[9] = GameComponentsIndices::AttackComponent;
        component_ids[10] = ComponentsIndices::AnimationComponent;
        component_ids[11] = ComponentsIndices::FontComponent;
        component_ids[12] = GameComponentsIndices::RotationComponent;
        component_ids[13] = GameComponentsIndices::PlayerTagComponent;
        component_count = 14;
    }
};

constexpr auto POINT_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Material)
       + sizeof(PointLightComponent));

struct PointLightArchetype: Archetype {
    Array<Transform, POINT_LIGHT_ARCH_CHUNK_SIZE> transforms;
    Array<Mesh, POINT_LIGHT_ARCH_CHUNK_SIZE> meshes;
    Array<Material, POINT_LIGHT_ARCH_CHUNK_SIZE> materials;
    Array<PointLightComponent, POINT_LIGHT_ARCH_CHUNK_SIZE> point_lights;

    PointLightArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[ComponentsIndices::MaterialComponent] = materials.data();
        components[ComponentsIndices::PointLightComponent] =
            point_lights.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::PointLightComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::MaterialComponent;
        component_ids[3] = ComponentsIndices::PointLightComponent;
        component_count = 4;
    }
};

constexpr auto PROJECTILE_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Collider)
       + sizeof(ColliderFlags) + sizeof(Rotation) + sizeof(ProjectileBundle)
       + sizeof(Health) + sizeof(Attack) + sizeof(Font)
       + sizeof(ProjectileTagComponent));

struct ProjectileArchetype: Archetype {
    Array<Transform, PROJECTILE_ARCH_CHUNK_SIZE> transforms;
    Array<Mesh, PROJECTILE_ARCH_CHUNK_SIZE> meshes;
    Array<Collider, PROJECTILE_ARCH_CHUNK_SIZE> colliders;
    Array<ColliderFlags, PROJECTILE_ARCH_CHUNK_SIZE> collider_flags {};
    Array<Rotation, PROJECTILE_ARCH_CHUNK_SIZE> rotations;
    Array<ProjectileBundle, PROJECTILE_ARCH_CHUNK_SIZE> projectile_bundles;
    Array<Health, PROJECTILE_ARCH_CHUNK_SIZE> health {};
    Array<Attack, PROJECTILE_ARCH_CHUNK_SIZE> attacks {};
    Array<Font, PROJECTILE_ARCH_CHUNK_SIZE> fonts;
    Array<ProjectileTagComponent, PROJECTILE_ARCH_CHUNK_SIZE>
        projectile_tag_components {};

    ProjectileArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[ComponentsIndices::ColliderComponent] = colliders.data();
        components[ComponentsIndices::ColliderFlagsComponent] =
            collider_flags.data();
        components[GameComponentsIndices::RotationComponent] = rotations.data();
        components[GameComponentsIndices::ProjectileBundleComponent] =
            projectile_bundles.data();
        components[GameComponentsIndices::HealthComponent] = health.data();
        components[GameComponentsIndices::AttackComponent] = attacks.data();
        components[ComponentsIndices::FontComponent] = fonts.data();
        components[GameComponentsIndices::ProjectileTagComponent] =
            projectile_tag_components.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << GameComponentsIndices::RotationComponent)
            | (1ull << GameComponentsIndices::ProjectileBundleComponent)
            | (1ull << GameComponentsIndices::HealthComponent)
            | (1ull << GameComponentsIndices::AttackComponent)
            | (1ull << ComponentsIndices::FontComponent)
            | (1ull << GameComponentsIndices::ProjectileTagComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::ColliderComponent;
        component_ids[3] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[4] = GameComponentsIndices::RotationComponent;
        component_ids[5] = GameComponentsIndices::ProjectileBundleComponent;
        component_ids[6] = GameComponentsIndices::HealthComponent;
        component_ids[7] = GameComponentsIndices::AttackComponent;
        component_ids[8] = ComponentsIndices::FontComponent;
        component_ids[9] = GameComponentsIndices::ProjectileTagComponent;
        component_count = 10;
    }
};

constexpr auto RIGID_BODY_ARCH_CHUNK_SIZE =
    ARCHETYPE_CHUNK_SIZE / (sizeof(glvm::Transform) + sizeof(glvm::RigidBody));

struct RigidBodyArch {
    Array<Transform, RIGID_BODY_ARCH_CHUNK_SIZE> transforms;
    Array<RigidBody, RIGID_BODY_ARCH_CHUNK_SIZE> rigid_bodies;
};

constexpr auto SPOT_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Material)
       + sizeof(SpotLightComponent));

struct SpotLightArchetype: Archetype {
    Array<Transform, SPOT_LIGHT_ARCH_CHUNK_SIZE> transforms;
    Array<Mesh, SPOT_LIGHT_ARCH_CHUNK_SIZE> meshes;
    Array<Material, SPOT_LIGHT_ARCH_CHUNK_SIZE> materials;
    Array<SpotLightComponent, SPOT_LIGHT_ARCH_CHUNK_SIZE> spot_lights;

    SpotLightArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[ComponentsIndices::MaterialComponent] = materials.data();
        components[ComponentsIndices::SpotLightComponent] = spot_lights.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::SpotLightComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::MaterialComponent;
        component_ids[3] = ComponentsIndices::SpotLightComponent;
        component_count = 4;
    }
};

constexpr auto STATIC_MESH_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Collider) + sizeof(ColliderFlags)
       + sizeof(Mesh) + sizeof(Material) + sizeof(Font) + sizeof(Rotation)
       + sizeof(StaticMeshTagComponent));

struct StaticMeshArchetype: Archetype {
    Array<Transform, STATIC_MESH_ARCH_CHUNK_SIZE> transforms;
    Array<Collider, STATIC_MESH_ARCH_CHUNK_SIZE> colliders;
    Array<ColliderFlags, STATIC_MESH_ARCH_CHUNK_SIZE> collider_flags {};
    Array<Mesh, STATIC_MESH_ARCH_CHUNK_SIZE> meshes;
    Array<Material, STATIC_MESH_ARCH_CHUNK_SIZE> materials;
    Array<Font, STATIC_MESH_ARCH_CHUNK_SIZE> fonts;
    Array<Rotation, STATIC_MESH_ARCH_CHUNK_SIZE> rotations;
    Array<StaticMeshTagComponent, STATIC_MESH_ARCH_CHUNK_SIZE>
        static_mesh_tag_components {};

    StaticMeshArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::ColliderComponent] = colliders.data();
        components[ComponentsIndices::ColliderFlagsComponent] =
            collider_flags.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[ComponentsIndices::MaterialComponent] = materials.data();
        components[ComponentsIndices::FontComponent] = fonts.data();
        components[GameComponentsIndices::RotationComponent] = rotations.data();
        components[GameComponentsIndices::StaticMeshTagComponent] =
            static_mesh_tag_components.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::FontComponent)
            | (1ull << GameComponentsIndices::RotationComponent)
            | (1ull << GameComponentsIndices::StaticMeshTagComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::ColliderComponent;
        component_ids[2] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[3] = ComponentsIndices::MeshComponent;
        component_ids[4] = ComponentsIndices::MaterialComponent;
        component_ids[5] = ComponentsIndices::FontComponent;
        component_ids[6] = GameComponentsIndices::RotationComponent;
        component_ids[7] = GameComponentsIndices::StaticMeshTagComponent;
        component_count = 8;
    }
};

constexpr auto COLLIDER_ARCH_CHUNK_SIZE =
    ARCHETYPE_CHUNK_SIZE / (sizeof(Collider) + sizeof(ColliderFlags));

struct ColliderArchetype: Archetype {
    Array<Collider, COLLIDER_ARCH_CHUNK_SIZE> colliders;
    Array<ColliderFlags, COLLIDER_ARCH_CHUNK_SIZE> collider_flags {};

    ColliderArchetype() {
        components[ComponentsIndices::ColliderComponent] = colliders.data();
        components[ComponentsIndices::ColliderFlagsComponent] =
            collider_flags.data();

        mask = (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent);

        component_ids[0] = ComponentsIndices::ColliderComponent;
        component_ids[1] = ComponentsIndices::ColliderFlagsComponent;
        component_count = 2;
    }
};

constexpr auto DAMAGE_ARCH_CHUNK_SIZE =
    ARCHETYPE_CHUNK_SIZE / (sizeof(Attack) + sizeof(Health) + sizeof(Font));

struct DamageArchetype: Archetype {
    Array<Attack, DAMAGE_ARCH_CHUNK_SIZE> attacks {};
    Array<Health, DAMAGE_ARCH_CHUNK_SIZE> health {};
    Array<Font, DAMAGE_ARCH_CHUNK_SIZE> fonts;

    DamageArchetype() {
        components[GameComponentsIndices::AttackComponent] = attacks.data();
        components[GameComponentsIndices::HealthComponent] = health.data();
        components[ComponentsIndices::FontComponent] = fonts.data();

        mask = (1ull << GameComponentsIndices::AttackComponent)
            | (1ull << GameComponentsIndices::HealthComponent)
            | (1ull << ComponentsIndices::FontComponent);

        component_ids[0] = GameComponentsIndices::AttackComponent;
        component_ids[1] = GameComponentsIndices::HealthComponent;
        component_ids[2] = ComponentsIndices::FontComponent;
        component_count = 3;
    }
};

constexpr auto PHYSICS_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Collider) + sizeof(ColliderFlags)
       + sizeof(Move) + sizeof(RigidBody));

struct PhysicsArchetype: Archetype {
    Array<Transform, PHYSICS_ARCH_CHUNK_SIZE> transforms;
    Array<Collider, PHYSICS_ARCH_CHUNK_SIZE> colliders;
    Array<ColliderFlags, PHYSICS_ARCH_CHUNK_SIZE> collider_flags {};
    Array<Move, PHYSICS_ARCH_CHUNK_SIZE> moves;
    Array<RigidBody, PHYSICS_ARCH_CHUNK_SIZE> rigid_bodies;

    PhysicsArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::ColliderComponent] = colliders.data();
        components[ComponentsIndices::ColliderFlagsComponent] =
            collider_flags.data();
        components[GameComponentsIndices::MoveComponent] = moves.data();
        components[ComponentsIndices::RigidBodyComponent] = rigid_bodies.data();

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << GameComponentsIndices::MoveComponent)
            | (1ull << ComponentsIndices::RigidBodyComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::ColliderComponent;
        component_ids[2] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[3] = GameComponentsIndices::MoveComponent;
        component_ids[4] = ComponentsIndices::RigidBodyComponent;
        component_count = 5;
    }
};

struct InventorySystem: public System {
public:
    u32 crosshair_archetypes_number = 0;
    u32 inventory_archetypes_number = 0;

    struct ArchView {
        Archetype* crosshair_cached_archetype = nullptr;
        Archetype* inventory_cached_archetype = nullptr;
    } arch_view;

    struct ComponentsView {
        Transform* crosshair_transforms_view = nullptr;

        Transform* inventory_transforms_view = nullptr;
        Inventory* inventory_view = nullptr;
        Mesh* inventory_meshes_view = nullptr;
    } components_view;

    u64 crosshair_required_mask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << GameComponentsIndices::CrosshairTagComponent);

    u64 inventory_required_mask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << GameComponentsIndices::InventoryComponent)
        | (1ull << ComponentsIndices::MeshComponent);

    auto update() -> void override;
    auto determine_swappable_status_and_slots(
        Item* item_component,
        Transform* inventory_transform_component,
        Vec<u32>& potential_occupied_slots,
        Transform* crosshair_transform_component,
        Point2D<i32> intersection_slot,
        Inventory* inventory_component,
        f32 inventory_slot_scale
    ) -> i32;
    static auto fill_inventory_slots(
        Item* item_component,
        i32 item_width,
        i32 item_height,
        Inventory* inventory_component,
        i32 fill_value
    ) -> void;
    static auto determine_swappable_field(
        Item* item_component,
        i32 item_width,
        i32 item_height,
        i32 pivot_row,
        i32 pivot_column,
        Inventory* inventory_component,
        Vec<u32>& potential_occupied_slots
    ) -> i32;
    static auto calculate_basic_offset(
        i32 item_axis_size,
        f32 axis_value,
        f32 crosshair_axis_position,
        i32 axis_slot_index,
        f32 inventory_slot_scale
    ) -> i32;
    auto check_crosshair_inventory_intersection(
        Transform* crosshair_transform_component,
        Transform* inventory_transform_component,
        Inventory* inventory_component,
        f32 inventory_slot_scale,
        f32 inventory_slot_half_scale
    ) const -> bool;
    auto determine_actual_intersection_slot(
        Transform* crosshair_transform_component,
        Transform* inventory_transform_component,
        f32 inventory_slot_scale,
        f32 inventory_slot_half_scale
    ) const -> Point2D<i32>;

    bool is_inventory_opened {};
    i32* is_item_dragged {};
    bool* is_left_mouse_button_released {};
    bool is_left_mouse_button_pressed {};
    f32 mouse_offset_x = 0;
    f32 mouse_offset_y = 0;
    // Window aspect ratio, set by the game each frame.
    f32 aspect_ratio = 0.0f;
    Archetype* crosshair_cached_archetype {};
    Archetype* cached_inventory_archetype {};
};

struct ItemSystem: public System {
    u32 inventory_archetypes_number = 0;
    u32 item_archetypes_number = 0;
    u32 crosshair_archetypes_number = 0;

    struct ArchView {
        Archetype* inventory_cached_archetype = nullptr;
        Archetype* item_archetype = nullptr;
        Archetype* crosshair_archetype = nullptr;
    } arch_view;

    struct ComponentsView {
        Inventory* inventories_view = nullptr;

        Item* items_view = nullptr;
        Collider* item_colliders_view = nullptr;
        Transform* item_transforms_view = nullptr;

        Transform* crosshair_transforms = nullptr;
    } components_view;

    u64 inventory_required_mask =
        (1ull << GameComponentsIndices::InventoryComponent);

    u64 item_required_mask = (1ull << ComponentsIndices::TransformComponent)
        | (1ull << GameComponentsIndices::ItemComponent)
        | (1ull << ComponentsIndices::MeshComponent)
        | (1ull << ComponentsIndices::MaterialComponent)
        | (1ull << ComponentsIndices::ColliderComponent)
        | (1ull << ComponentsIndices::ColliderFlagsComponent);

    u64 crosshair_required_mask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << GameComponentsIndices::CrosshairTagComponent);

    auto update() -> void override;
    static auto put_item2x2(Inventory* inventory_component, u32 item_entity)
        -> bool;

    EventStack* input_stack {};
    bool is_inventory_opened {};
    i32* dragged_item_entity {};
    bool* is_left_mouse_button_released {};
    bool is_left_mouse_button_pressed {};
    f32 mouse_offset_x = 0;
    f32 mouse_offset_y = 0;
};

auto create_projectile(
    const Vector<f32, 3>& projectile_position,
    const Vector<f32, 3>& projectile_forward,
    const MeshHandle& mesh_handle,
    const Material& material,
    const Damage& damage,
    const EntityLocation& projectile_location
) -> void;

struct MovementSystem: public System {
public:
    f32 delta_frame_time {};
    f32 gravity {};
    EventStack& input_stack;
    f32 prev_delta_x = 0.0f;
    Vector<f32, 3> prev_forward;

    u32 player_archetypes_number = 0;
    u32 rigid_body_contained_archetypes_number = 0;

    struct MovementArchView {
        Archetype* player_cached_archetype = nullptr;
        Array<Archetype*, 32> rigid_body_contained_archetypes_cache {};
    } arch_view;

    struct MovementComponentsView {
        Move* player_moves = nullptr;
        Beholder* player_views = nullptr;
        ColliderFlags* player_collider_flags = nullptr;
        RigidBody* player_rigid_body = nullptr;
        Transform* player_transforms = nullptr;
        Rotation* player_rotations = nullptr;

        // Components related to archetypes contains rigid.
        Transform* transforms = nullptr;
        RigidBody* rigid_bodies = nullptr;
        Move* moves = nullptr;
        Item* items = nullptr;
    } components_view;

    u64 player_required_mask =
        (1ull << GameComponentsIndices::PlayerTagComponent);
    u64 rigid_body_required_mask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::RigidBodyComponent)
        | (1ull << GameComponentsIndices::MoveComponent);

    explicit MovementSystem(EventStack& input_stack);

    auto update() -> void override;
    static auto calculate_vector_rl(Beholder& beholder) -> Vector<f32, 3>;
    auto calculate_vector_fb(Beholder& beholder, Event& event)
        -> Vector<f32, 3>;
};

struct ProceduralLevelGeneratingSystem: public System {
public:
    u32 level_number = 0;
    bool stupid_flag = false;
    u32 previous_half_x_rand = 0;
    u32 previous_half_z_rand = 0;
    Vector<f32, 3> current_level_position = {5.0f, 0.0f, 15.0f};
    Vector<f32, 3> transition_bridge_position = {0.0f, 0.0f, 0.0f};
    u32 next_level_transition_direction = 0;
    u32 previous_iteration_transition_bridge_direction = 0;

    u32 cached_level_chunk_arch_number = 0;
    u32 cached_player_arch_number = 0;

    struct ProceduralLevelArchView {
        Archetype* cached_level_chunk_arch = nullptr;
        Archetype* cached_player_arch = nullptr;
    } arch_view;

    struct ComponentsView {
        Transform* player_transforms = nullptr;
    } components_view;

    u64 player_required_mask =
        (1ull << GameComponentsIndices::PlayerTagComponent);

    u64 required_mask = (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::MaterialComponent)
        | (1ull << ComponentsIndices::MeshComponent)
        | (1ull << ComponentsIndices::ColliderComponent)
        | (1ull << ComponentsIndices::ColliderFlagsComponent)
        | (1ull << GameComponentsIndices::LevelChunkTagComponent);

    Vec<MeshHandle> mesh_handles;
    Vec<TextureHandle> texture_handlers;

    Vec<Vec<Vertex>> level_generated_vertices;
    // Wavefront .obj indices.
    Vec<Vec<u32>> level_generated_indices;
    // Keep axis limiting values for every axis per mesh in current iteration
    // while initializing Wavefront .obj and GLTF.
    MeshAxisLimitingValues mesh_axis_limiting_values;
    // Contains maximum coordinate value in every direction for all generated
    // levels.
    MeshAxisLimitingValues coordinate_maximum_value_per_direction;

    auto update() -> void override;
    static auto set_half_extents_from_direction(
        f32& half_x,
        f32& half_z,
        const f32& transition_bridge_half_width,
        const f32& transition_bridge_half_height,
        const f32& next_level_transition_direction
    ) -> void;
    auto generate_level(
        u32 level_half_x,
        u32 level_half_y,
        u32 level_half_z,
        f32 transition_bridge_half_width,
        f32 transition_bridge_half_height
    ) -> void;
    auto generate_transition_bridge(
        u32 level_half_x,
        u32 level_half_y,
        u32 level_half_z,
        f32 transition_bridge_half_width,
        f32 transition_bridge_half_height
    ) -> void;
    auto make_cube_object_vertices(
        Vector<f32, 4> joint_indices,
        Vector<f32, 4> weights,
        f32 half_x,
        f32 half_y,
        f32 half_z,
        Vec<Vertex>& destination_vertices_container
    ) -> void;
    [[nodiscard]] auto check_collision_intersection_with_maximum_coordinates(
        Vector<f32, 3> position,
        f32 half_x,
        f32 half_y,
        f32 half_z
    ) const -> bool;
};

struct EnemySystem: public System {
public:
    u32 player_archetypes_number = 0;
    u32 enemy_archetypes_number = 0;
    u32 projectile_archetypes_number = 0;

    struct ArchView {
        Archetype* player_cached_archetype = nullptr;
        Archetype* enemy_cached_archetype = nullptr;
        Archetype* projectile_archetype = nullptr;
    } arch_view;

    struct ComponentsView {
        Transform* player_transforms = nullptr;

        Transform* enemy_transforms = nullptr;
        State* enemy_states = nullptr;
        Enemy* enemies = nullptr;
    } components_view;

    u64 player_required_mask =
        (1ull << GameComponentsIndices::PlayerTagComponent);

    u64 enemy_required_mask = (1ull << ComponentsIndices::TransformComponent)
        | (1ull << GameComponentsIndices::StateComponent)
        | (1ull << GameComponentsIndices::EnemyComponent);

    u64 projectile_required_mask =
        (1ull << GameComponentsIndices::ProjectileTagComponent);

    auto update() -> void override;
    SoundEngine* sound_engine {};
    Vec<TextureHandle> texture_handlers;
    Vec<MeshHandle> mesh_handles;
    f32 projectile_cooldown = 5.0f;
    f32 delta_frame_time {};
    // Game-owned shoot sound, set by the game (no hardcoded asset paths).
    const char* shoot_sound_path = nullptr;
    u32 shoot_sound_duration = 5;
    u32 shoot_sound_rate = 22050;
    f32 shoot_sound_volume = 0.05f;
};

template<typename T>
concept UnitOrEnemy =
    std::is_same_v<T, PlayerArchetype> || std::is_same_v<T, EnemyArchetype>;

template<typename T>
concept HasAttack = requires(T* t) {
    { t->attacks };
};

struct ProjectileSystem: public System {
public:
    f32 yaw = -90.0f;
    f32 pitch = 0.0f;
    bool first_mouse = true;
    EventStack& input_stack;
    Vec<TextureHandle> texture_handlers;
    Vec<MeshHandle> mesh_handles;
    SoundEngine* sound_engine {};
    f32 projectile_cooldown = 2.0f;
    f32 delta_frame_time {};
    bool is_inventory_opened {};
    // Game-owned shoot sound, set by the game (no hardcoded asset paths).
    const char* shoot_sound_path = nullptr;
    u32 shoot_sound_duration = 5;
    u32 shoot_sound_rate = 22050;
    f32 shoot_sound_volume = 0.05f;

    u32 player_archetypes_number = 0;
    u32 projectile_archetypes_number = 0;

    struct ArchView {
        Archetype* player_cached_archetype = nullptr;
        Archetype* projectile_archetype = nullptr;
    } arch_view;

    struct ComponentsView {
        Transform* player_transforms = nullptr;
        Beholder* player_views = nullptr;

        Transform* projectile_transforms = nullptr;
        ColliderFlags* projectile_collider_flags = nullptr;
        Collider* projectile_colliders = nullptr;
        ProjectileBundle* projectile_bundles = nullptr;
        Health* projectile_health = nullptr;
        Attack* projectile_attacks = nullptr;
    } components_view;

    u64 player_required_mask =
        (1ull << GameComponentsIndices::PlayerTagComponent);

    u64 projectile_required_mask =
        (1ull << GameComponentsIndices::ProjectileTagComponent);

    explicit ProjectileSystem(EventStack& input_stack);
    auto update() -> void override;
    template<typename T>
        requires UnitOrEnemy<T> && HasAttack<T>
    static auto mark_as_attacked(
        T* arch,
        Damage* projectile_damage,
        u32 entity_index
    ) -> void;
};

template<typename T>
    requires UnitOrEnemy<T> && HasAttack<T>
auto ProjectileSystem::mark_as_attacked(
    T* arch,
    Damage* projectile_damage,
    u32 entity_index
) -> void {
    arch->attacks[entity_index].damage = projectile_damage->maximum_damage;
}

struct DamageSystem: public System {
public:
    auto update() -> void override;

    f32 delta_time;

    u32 cached_attackable_archetypes_number = 0;
    u32 cached_font_archetypes_number = 0;

    struct ArchView {
        Array<Archetype*, 32> cached_attackable_archetypes;
        Array<Archetype*, 32> cached_font_archetypes;
    } arch_view;

    struct ComponentsView {
        Attack* attackable_attacks = nullptr;
        Health* attackable_health = nullptr;
        Font* attackable_fonts = nullptr;

        Font* fonts = nullptr;
    } components_view;

    u64 attackable_required_mask =
        (1ull << GameComponentsIndices::AttackComponent)
        | (1ull << GameComponentsIndices::HealthComponent)
        | (1ull << ComponentsIndices::FontComponent);

    u64 font_required_mask = (1ull << ComponentsIndices::FontComponent);
};

struct CollisionSystem: public System {
public:
    f32 delta_time;
    f32 gravity;
    bool is_inventory_opened;
    bool* is_item_dragged;
    bool is_left_mouse_button_pressed;
    bool* is_left_mouse_button_released;
    EventStack& input_stack;
    Array<Archetype*, 32> cached_archetypes;
    u32 cached_archetypes_number = 0;

    struct CollisionComponentsView {
        Transform* backtracking_transforms = nullptr;
        Collider* backtracking_colliders = nullptr;
        ColliderFlags* backtracking_collider_flags = nullptr;
        Mesh* backtracking_meshes = nullptr;
        Move* backtracking_move = nullptr;
        Transform* compared_transforms = nullptr;
        Mesh* compared_meshes = nullptr;
        Move* compared_move = nullptr;
    } view;

    u64 required_mask = (1ull << ComponentsIndices::ColliderComponent)
        | (1ull << ComponentsIndices::ColliderFlagsComponent)
        | (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::MeshComponent);

    CollisionSystem(EventStack& stack) : input_stack(stack) {
    }

    auto update() -> void override;
    auto upper_actor_check(
        Vector<f32, 3> backtracking_position,
        Vector<f32, 3> compared_position,
        f32 backtracking_scale,
        f32 compared_scale,
        MeshHandle backtracking_mesh_handle,
        MeshHandle compared_mesh_handle
    ) -> bool;
};

struct PhysicsSystem: public System {
public:
    f32 acceleration_of_gravity;
    f32 delta_time;
    f32& gravity;
    EventStack& input_stack;

    u32 cached_archetypes_number = 0;

    struct ArchView {
        Array<Archetype*, 32> cached_archetypes;
    } arch_view;

    struct ComponentsView {
        Transform* transforms_view = nullptr;
        Move* moves_view = nullptr;
        RigidBody* rigid_bodies_view = nullptr;
        ColliderFlags* collider_flags_view = nullptr;
        Collider* colliders_view = nullptr;
        Mesh* meshes_view = nullptr;
    } components_view;

    u64 required_mask = (1ull << ComponentsIndices::TransformComponent)
        | (1ull << GameComponentsIndices::MoveComponent)
        | (1ull << ComponentsIndices::RigidBodyComponent)
        | (1ull << ComponentsIndices::ColliderComponent)
        | (1ull << ComponentsIndices::MeshComponent);

    PhysicsSystem(f32& initial_gravity, EventStack& stack) :
        gravity(initial_gravity),
        input_stack(stack) {
    }

    // Sets the backtracking entity's transform Y to the ground entity's upper Y.

    // This update searches for entities referring to colliders and checks their
    // transform components for collisions; if a collision is detected, it
    // checks whether the backtracking entity has a gravity component to call
    // the gravity function.
    auto update() -> void override;
};

// Game UI helpers (moved out of Engine): build render data for the
// inventory grid, the dragged/world item icons and the crosshair cursor.
[[nodiscard]] auto game_update_data_ubo_ui(
    u32 current_inventory_row,
    u32 current_inventory_column,
    Inventory* inventory_component,
    Transform* slot_transform_component,
    Mesh* mesh_component,
    f32 aspect_ratio
) -> SlotData;

auto game_update_data_ubo_icons_ui(
    Transform* item_transform_component,
    Collider* item_collider_component,
    Item* item_component,
    u32 row_inventory,
    u32 column_inventory,
    Transform* inventory_transform_component,
    Mesh* item_mesh,
    i32 item_entity,
    i32 dragged_item_entity,
    f32 aspect_ratio
) -> Matrix<f32, 4>;

auto game_update_data_hud_screen_ubo(
    Transform* cursor_transform,
    f32 hud_screen_x,
    f32 hud_screen_y,
    bool is_ui_opened,
    bool is_cursor_released
) -> Matrix<f32, 4>;

// Fills game actor/UI render data, called by the engine via frame_data_hook.
// Takes the next free actor index, returns the updated one.
auto game_fill_frame_data(u32 actor_counter, i32 dragged_item_entity) -> u32;

auto main() -> i32 {
    glvm_log::info("hello", "main start");
    auto* entity_manager = EntityManager::get_instance();
    auto* component_manager = ComponentManager::get_instance();
    auto* arch_entity_manager = ArchetypeEntityManager::get_instance();
    auto* engine = Engine::get_instance();
    glvm_log::info("hello", "engine created");
    auto hyper_cube = engine->load_mesh_from_gltf(
        "../../../examples/assets/gltf/hyper_cube.gltf"
    );
    auto hyper_cube2 = engine->load_mesh_from_gltf(
        "../../../examples/assets/gltf/hyper_cube2.gltf"
    );
    auto mega_human = engine->load_mesh_from_gltf(
        "../../../examples/assets/gltf/mega_chel.gltf"
    );
    auto simple_cube = engine->load_mesh_from_gltf(
        "../../../examples/assets/gltf/simpleCube2.gltf"
    );
    auto crosshair_001_handle = engine->load_mesh_from_gltf(
        "../../../examples/assets/gltf/crosshair_001.gltf"
    );
    auto inventory_handle = engine->load_mesh_from_gltf(
        "../../../examples/assets/gltf/inventory.gltf"
    );
    auto cyborg_handle = engine->load_mesh_from_gltf(
        "../../../examples/assets/gltf/cyborg11.gltf"
    );
    auto robot0_handle =
        engine->load_mesh_from_gltf("../../../examples/assets/gltf/scene.gltf");
    auto human_texture =
        engine->load_texture_from_address(128, 96, human_dat_len, human_dat);
    auto witch_texture =
        engine->load_texture_from_address(32, 32, witch_dat_len, witch_dat);
    auto gray_texture =
        engine->load_texture_from_address(32, 32, gray_dat_len, gray_dat);
    auto container2 = engine->load_texture_from_address(
        500,
        500,
        container2_dat_len,
        container2_dat
    );
    auto container2_specular_texture = engine->load_texture_from_address(
        500,
        500,
        container2_specular_dat_len,
        container2_specular_dat
    );
    auto crosshair_texture = engine->load_texture_from_address(
        32,
        32,
        crosshair_dat_len,
        crosshair_dat
    );
    auto font_atlas_texture = engine->load_texture_from_address(
        84,
        132,
        font_atlas_dat_len,
        font_atlas_dat
    );
    auto inventory_texture = engine->load_texture_from_address(
        64,
        64,
        inventory_slot_dat_len,
        inventory_slot_dat
    );
    auto tileset_texture =
        engine
            ->load_texture_from_address(512, 512, tileset_dat_len, tileset_dat);
    glvm_log::info("hello", "meshes and textures loaded");
    // Game-owned asset paths: the engine ships no default locations.
    engine->set_model_cache_path("../../../examples/assets/cache/models/cache");
    constexpr auto* SHOOT_SOUND_PATH =
        "../../../examples/assets/sounds/pistol.wav";
    // Register game components for generic swap-remove in archetypes.
    register_component<State>(GameComponentsIndices::StateComponent);
    register_component<Enemy>(GameComponentsIndices::EnemyComponent);
    register_component<Inventory>(GameComponentsIndices::InventoryComponent);
    register_component<Item>(GameComponentsIndices::ItemComponent);
    register_component<ProjectileBundle>(
        GameComponentsIndices::ProjectileBundleComponent
    );
    register_component<CrosshairTagComponent>(
        GameComponentsIndices::CrosshairTagComponent
    );
    register_component<LevelChunkTagComponent>(
        GameComponentsIndices::LevelChunkTagComponent
    );
    register_component<PlayerTagComponent>(
        GameComponentsIndices::PlayerTagComponent
    );
    register_component<ProjectileTagComponent>(
        GameComponentsIndices::ProjectileTagComponent
    );
    register_component<StaticMeshTagComponent>(
        GameComponentsIndices::StaticMeshTagComponent
    );
    register_component<Health>(GameComponentsIndices::HealthComponent);
    register_component<Damage>(GameComponentsIndices::DamageComponent);
    register_component<Attack>(GameComponentsIndices::AttackComponent);
    register_component<Move>(GameComponentsIndices::MoveComponent);
    register_component<Rotation>(GameComponentsIndices::RotationComponent);
    // Game systems.
    auto* movement_system = new MovementSystem(global_input_stack);
    auto* projectile_system = new ProjectileSystem(global_input_stack);
    auto* enemy_system = new EnemySystem();
    auto* item_system = new ItemSystem();
    auto* procedural_level_system = new ProceduralLevelGeneratingSystem();
    auto* inventory_system = new InventorySystem();
    auto* collision_system = new CollisionSystem(global_input_stack);
    auto* damage_system = new DamageSystem();
    f32 physics_gravity = 0.0f;
    auto* physics_system =
        new PhysicsSystem(physics_gravity, global_input_stack);
    projectile_system->texture_handlers = engine->texture_handlers;
    projectile_system->mesh_handles = engine->mesh_handles;
    projectile_system->shoot_sound_path = SHOOT_SOUND_PATH;
    enemy_system->texture_handlers = engine->texture_handlers;
    enemy_system->mesh_handles = engine->mesh_handles;
    enemy_system->shoot_sound_path = SHOOT_SOUND_PATH;
    procedural_level_system->mesh_handles = engine->mesh_handles;
    procedural_level_system->texture_handlers = engine->texture_handlers;
    // If don't have any dragged item then this variable have value of -1.
    i32 dragged_item_entity = -1;
    inventory_system->is_item_dragged = &dragged_item_entity;
    item_system->dragged_item_entity = &dragged_item_entity;
    engine->add_system(procedural_level_system);
    engine->add_system(movement_system);
    engine->add_system(enemy_system);
    engine->add_system(projectile_system);
    // Pipeline order matters (was: procedural, movement, enemy, projectile,
    // spatial, collision, damage, physics, inventory, item): level chunks
    // must exist before the first spatial indexing pass, and movement writes
    // must precede collision/physics in the same frame.
    engine->add_base_systems();
    engine->add_system(collision_system);
    engine->add_system(damage_system);
    engine->add_system(physics_system);
    engine->add_system(inventory_system);
    engine->add_system(item_system);
    bool is_inventory_key_held = false;
    bool was_inventory_opened = false;
    f32 orbit_prev_x = 0.0f;
    f32 orbit_prev_y = 0.0f;
    engine->set_pre_update_hook([&]() {
        auto* renderer = engine->renderer();
        bool inventory_key_pressed =
            (global_input_stack.search_element(EventKind::InventoryToggle)
             == EventKind::InventoryToggle);
        if (inventory_key_pressed && !is_inventory_key_held) {
            renderer->is_inventory_opened = !renderer->is_inventory_opened;
            if (renderer->is_inventory_opened) {
                SystemManager::get_instance()->deactivate_system(
                    movement_system
                );
                engine->set_hud_screen(0.0f, 0.0f);
            } else {
                SystemManager::get_instance()->return_system_to_activated_state(
                    movement_system
                );
            }
        }
        is_inventory_key_held = inventory_key_pressed;
        if (was_inventory_opened && !renderer->is_inventory_opened) {
            // Cursor was free while the inventory was open; reset the mouse
            // state so the first locked sample doesn't feed a fake delta to
            // the camera.
            global_event.mouse_pointer_position.offset_x = 0;
            global_event.mouse_pointer_position.offset_y = 0;
            renderer->prev_x = 0.0f;
            renderer->prev_y = 0.0f;
            renderer->current_x = 0.0f;
            renderer->current_y = 0.0f;
            engine->set_previous_mouse_offsets(0.0f, 0.0f);
        }
        was_inventory_opened = renderer->is_inventory_opened;
        movement_system->delta_frame_time = engine->get_delta_frame_time();
        movement_system->gravity = engine->get_gravity();
        enemy_system->delta_frame_time = engine->get_delta_frame_time();
        enemy_system->sound_engine = engine->get_sound_engine();
        projectile_system->delta_frame_time = engine->get_delta_frame_time();
        projectile_system->sound_engine = engine->get_sound_engine();
        projectile_system->is_inventory_opened = renderer->is_inventory_opened;
        collision_system->delta_time = engine->get_delta_frame_time();
        damage_system->delta_time = engine->get_delta_frame_time();
        physics_system->delta_time = engine->get_delta_frame_time();
        inventory_system->is_inventory_opened = renderer->is_inventory_opened;
        inventory_system->aspect_ratio = renderer->aspect_ratio;
        inventory_system->is_left_mouse_button_released =
            &global_event.is_left_mouse_button_released;
        inventory_system->is_left_mouse_button_pressed =
            engine->left_mouse_button_pressed();
        inventory_system->mouse_offset_x = engine->get_hud_screen_x();
        inventory_system->mouse_offset_y = engine->get_hud_screen_y();
        item_system->input_stack = &global_input_stack;
        item_system->is_inventory_opened = renderer->is_inventory_opened;
        item_system->is_left_mouse_button_released =
            &global_event.is_left_mouse_button_released;
        item_system->is_left_mouse_button_pressed =
            engine->left_mouse_button_pressed();
        item_system->mouse_offset_x = engine->get_hud_screen_x();
        item_system->mouse_offset_y = engine->get_hud_screen_y();
    });
    engine->set_post_update_hook([&]() {
        auto* renderer = engine->renderer();
        renderer->level_generated_vertices =
            procedural_level_system->level_generated_vertices;
        renderer->level_generated_indices =
            procedural_level_system->level_generated_indices;
        procedural_level_system->level_generated_vertices.clear();
        procedural_level_system->level_generated_indices.clear();
        renderer->dragged_item_entity = dragged_item_entity;
        // Third-person camera: look at the player right before
        // SetViewMatrix, which applies mouse deltas on top of this aim.
        // Both forwards are set: the engine keeps the renderer's stored
        // one when the mouse is still.
        Array<Archetype*, 32> camera_cache {};
        auto camera_count = 0u;
        world.search_cache_archetypes(
            (1ull << GameComponentsIndices::PlayerTagComponent),
            camera_cache.data(),
            camera_count
        );
        if (camera_count > 0) {
            auto* camera_transforms = as<Transform*>(
                camera_cache[0]
                    ->components[ComponentsIndices::TransformComponent]
            );
            auto* camera_views = as<Beholder*>(
                camera_cache[0]->components[ComponentsIndices::ViewComponent]
            );
            // Third-person orbit: the mouse moves the camera offset around
            // the player (offsets accumulate in the engine, so diff them).
            const auto mouse_x =
                as<f32>(global_event.mouse_pointer_position.offset_x);
            const auto mouse_y =
                as<f32>(global_event.mouse_pointer_position.offset_y);
            const auto delta_x = mouse_x - orbit_prev_x;
            const auto delta_y = mouse_y - orbit_prev_y;
            orbit_prev_x = 0.0f;
            orbit_prev_y = 0.0f;
            auto& offset = camera_views[0].position;
            constexpr auto ORBIT_SENSITIVITY = 0.0025f;
            constexpr auto MAX_ORBIT_PITCH = 1.45f;
            const auto orbit_radius = vec_length(offset);
            auto orbit_yaw =
                std::atan2(offset[0], offset[2]) + delta_x * ORBIT_SENSITIVITY;
            auto orbit_pitch = std::asin(clamp(
                                    as<f32>(-1.0f),
                                    offset[1] / orbit_radius,
                                    as<f32>(1.0f)
                                ))
                - delta_y * ORBIT_SENSITIVITY;
            orbit_pitch =
                clamp(-MAX_ORBIT_PITCH, orbit_pitch, MAX_ORBIT_PITCH);
            offset[0] =
                orbit_radius * std::cos(orbit_pitch) * std::sin(orbit_yaw);
            offset[1] = orbit_radius * std::sin(orbit_pitch);
            offset[2] =
                orbit_radius * std::cos(orbit_pitch) * std::cos(orbit_yaw);
            // Neutralize engine mouse-look: it would rotate on top of the
            // aim and snap back every frame. Movement already consumed
            // these offsets earlier in the frame.
            global_event.mouse_pointer_position.offset_x = 0;
            global_event.mouse_pointer_position.offset_y = 0;
            const auto eye =
                camera_views[0].position + camera_transforms[0].position;
            camera_views[0].forward =
                normalize(camera_transforms[0].position - eye);
            renderer->forward = camera_views[0].forward;
        }
    });
    engine->set_frame_data_hook([&](u32 actor_counter) -> u32 {
        return game_fill_frame_data(actor_counter, dragged_item_entity);
    });
    {
        auto* level_chunk_arch = new LevelChunkArchetype;
        auto* player_arch = new PlayerArchetype;
        auto* enemy_arch = new EnemyArchetype;
        auto* projectile_arch = new ProjectileArchetype;
        auto* static_mesh_arch = new StaticMeshArchetype;
        auto* crosshair_arch = new CrosshairArchetype;
        auto* inventory_arch = new InventoryArchetype;
        auto* item_arch = new ItemArchetype;
        auto* directional_light_arch = new DirectionalLightArchetype;
        auto* point_light_arch = new PointLightArchetype;
        auto* spot_light_arch = new SpotLightArchetype;
        world.archetypes.push_back(level_chunk_arch);
        world.archetypes.push_back(player_arch);
        world.archetypes.push_back(enemy_arch);
        world.archetypes.push_back(projectile_arch);
        world.archetypes.push_back(static_mesh_arch);
        world.archetypes.push_back(crosshair_arch);
        world.archetypes.push_back(inventory_arch);
        world.archetypes.push_back(item_arch);
        world.archetypes.push_back(directional_light_arch);
        world.archetypes.push_back(point_light_arch);
        world.archetypes.push_back(spot_light_arch);
    }
    auto player = arch_entity_manager->create_entity();
    world.add_entity_to_archetype(player, world.archetypes[1]);
    auto player_location = world.entity_locations[get_id(player)];
    auto* player_arch = dynamic_cast<PlayerArchetype*>(player_location.arch);
    const auto player_index = player_location.index;
    player_arch->transforms[player_index] = {
        .position = {15.0f, 15.0f, 15.0f},
        .scale = 1.0f
    };
    player_arch->rotations[player_index] = {.yaw = 3.14f, .pitch = 0.0f};
    player_arch->rigid_bodies[player_index] = {.mass = 3.0f};
    player_arch->health[player_index] = {
        .max_health = 100,
        .current_health = 100
    };
    player_arch->beholders[player_index] = {
        .position = {0.0f, 2.0f, -3.0f},
        .forward = {0.0f, 0.0f, -1.0f}
    };
    player_arch->meshes[player_index] = {.handle = mega_human, .gltf = true};
    player_arch->materials[player_index] = {
        .diffuse_texture_id = gray_texture,
        .specular_texture_id = gray_texture,
        .ambient = {0.05f, 0.05f, 0.0f},
        .shininess = 128.0f * 0.078125f
    };
    std::random_device rd;
    BTreeMap<i32, i32> hist;
    std::mt19937 mersenne(rd());
    std::uniform_int_distribution<i32> dist(0, 3);
    for (auto i = 0; i < 5; ++i) {
        auto enemy = arch_entity_manager->create_entity();
        world.add_entity_to_archetype(enemy, world.archetypes[2]);
        auto enemy_location = world.entity_locations[get_id(enemy)];
        auto* enemy_arch = dynamic_cast<EnemyArchetype*>(enemy_location.arch);
        const auto enemy_index = enemy_location.index;
        auto random = dist(mersenne);
        Vector<f32, 3> random_direction = {};
        switch (random) {
            case 0:
                random_direction = Vector<f32, 3>(3.0f, 0.0f, 0.0f, 0.0f);
                break;
            case 1:
                random_direction = Vector<f32, 3>(-3.0f, 0.0f, 0.0f, 0.0f);
                break;
            case 2:
                random_direction = Vector<f32, 3>(0.0f, 0.0f, 3.0f, 0.0f);
                break;
            case 3:
                random_direction = Vector<f32, 3>(0.0f, 0.0f, -3.0f, 0.0f);
                break;
            default:
                break;
        }
        enemy_arch->transforms[enemy_index] = {
            .position =
                {Vector<f32, 3>(static_cast<f32>(i) * 5, 3.0f, -3.0f)
                 + random_direction},
            .scale = 0.02f
        };
        enemy_arch->states[enemy_index] = {.state = States::ROAMING};
        enemy_arch->rigid_bodies[enemy_index] = {.mass = 0.0f};
        enemy_arch->enemies[enemy_index] = {.detect_radius = 15.0f};
        enemy_arch->health[enemy_index] = {
            .max_health = 100,
            .current_health = 100
        };
        auto* enemy_font_component = &enemy_arch->fonts[enemy_index];
        if (i < 3) {
            enemy_font_component->font_string.push_back('1');
        } else if (i < 6) {
            enemy_font_component->font_string.push_back('1');
            enemy_font_component->font_string.push_back('0');
        } else if (i < 10) {
            enemy_font_component->font_string.push_back('1');
            enemy_font_component->font_string.push_back('0');
            enemy_font_component->font_string.push_back('E');
        } else {
            enemy_font_component->font_string.push_back('J');
            enemy_font_component->font_string.push_back('r');
        }
        enemy_font_component->lifetime = 0.0f;
        enemy_font_component->removable = false;
        enemy_arch->meshes[enemy_index] = {
            .handle = robot0_handle,
            .gltf = true
        };
        enemy_arch->materials[enemy_index] = {
            .diffuse_texture_id = gray_texture,
            .specular_texture_id = gray_texture,
            .ambient = {0.05f, 0.05f, 0.05f},
            .shininess = 32.0f * 0.078125f
        };
    }
    for (auto i = 0; i < 5; ++i) {
        auto cube = arch_entity_manager->create_entity();
        world.add_entity_to_archetype(cube, world.archetypes[4]);
        auto cube_location = world.entity_locations[get_id(cube)];
        auto* cube_arch =
            dynamic_cast<StaticMeshArchetype*>(cube_location.arch);
        const auto cube_index = cube_location.index;
        cube_arch->transforms[cube_index] = {
            .position = {7.0f, 2.0f, 10.0f + (static_cast<f32>(i) * 2.0f)},
            .scale = 1.0f
        };
        cube_arch->meshes[cube_index] = {.handle = hyper_cube2, .gltf = true};
        cube_arch->materials[cube_index] = {
            .diffuse_texture_id = tileset_texture,
            .specular_texture_id = container2_specular_texture,
            .ambient = {0.05f, 0.05f, 0.05f},
            .shininess = 128.0f * 0.078125f
        };
        cube_arch->fonts[cube_index].font_string.push_back('R');
    }
    auto crosshair = arch_entity_manager->create_entity();
    world.add_entity_to_archetype(crosshair, world.archetypes[5]);
    auto crosshair_location = world.entity_locations[get_id(crosshair)];
    auto* crosshair_arch =
        dynamic_cast<CrosshairArchetype*>(crosshair_location.arch);
    const auto crosshair_index = crosshair_location.index;
    crosshair_arch->transforms[crosshair_index] = {.scale = 0.01f};
    crosshair_arch->meshes[crosshair_index].handle = crosshair_001_handle;
    crosshair_arch->materials[crosshair_index] = {
        .diffuse_texture_id = container2,
        .specular_texture_id = container2_specular_texture,
        .ambient = {0.05f, 0.05f, 0.05f},
        .shininess = 128.0f * 0.078125f
    };
    auto inventory = arch_entity_manager->create_entity();
    world.add_entity_to_archetype(inventory, world.archetypes[6]);
    auto inventory_location = world.entity_locations[get_id(inventory)];
    auto* inventory_arch =
        dynamic_cast<InventoryArchetype*>(inventory_location.arch);
    const auto inventory_index = inventory_location.index;
    auto* inventory_component = &inventory_arch->inventories[inventory_index];
    inventory_component->entity_owner = player;
    inventory_component->slot_mesh_id = inventory_handle;
    inventory_component->slot_scale = 0.05f;
    auto* inventory_mesh = &inventory_arch->meshes[inventory_index];
    inventory_mesh->gltf = true;
    inventory_arch->transforms[inventory_index] = {
        .position = {0.0f, -0.5f, 0.0f},
        .scale = 1.0f
    };
    inventory_arch->materials[inventory_index] = {
        .diffuse_texture_id = inventory_texture,
        .specular_texture_id = inventory_texture,
        .ambient = {0.05f, 0.05f, 0.05f},
        .shininess = 128.0f * 0.078125f
    };
    for (auto i = 0; i < 5; ++i) {
        u64 item = arch_entity_manager->create_entity();
        world.add_entity_to_archetype(item, world.archetypes[7]);
        EntityLocation item_location = world.entity_locations[get_id(item)];
        auto* item_arch = dynamic_cast<ItemArchetype*>(item_location.arch);
        const auto item_index = item_location.index;
        auto row = i + 1;
        item_arch->items[item_index].item_slot_type = {
            .height = 2,
            .width = static_cast<u32>(row)
        };
        item_arch->items[item_index].is_actor = true;
        item_arch->transforms[item_index] = {
            .position = {3.0f, 5.0f, 10.0f + (static_cast<f32>(i) * 2.0f)},
            .scale = 0.05f
        };
        item_arch->rigid_bodies[item_index] = {.mass = 0.0f};
        item_arch->meshes[item_index].handle = hyper_cube;
        item_arch->materials[item_index] = {
            .diffuse_texture_id = container2,
            .specular_texture_id = container2_specular_texture,
            .ambient = {0.05f, 0.05f, 0.05f},
            .shininess = 128.0f * 0.078125f
        };
    }
    auto directional_light = arch_entity_manager->create_entity();
    world.add_entity_to_archetype(directional_light, world.archetypes[8]);
    auto directional_light_location =
        world.entity_locations[get_id(directional_light)];
    auto* directional_light_arch = dynamic_cast<DirectionalLightArchetype*>(
        directional_light_location.arch
    );
    const auto directional_light_index = directional_light_location.index;
    directional_light_arch->directional_lights[directional_light_index] = {
        .position = {0.0f, 25.0f, 15.0f},
        .direction = {1.0f, 10.0f, 0.0f},
        .ambient = {0.05f, 0.05f, 0.05f},
        .diffuse = {0.4f, 0.4f, 0.4f},
        .specular = {1.0f, 1.0f, 1.0f}
    };
    directional_light_arch->transforms[directional_light_index] = {
        .position = {0.0f, 10.0f, -15.0f},
        .scale = 0.1f
    };
    directional_light_arch->meshes[directional_light_index].handle = hyper_cube;
    directional_light_arch->materials[directional_light_index] = {
        .diffuse_texture_id = container2,
        .specular_texture_id = container2,
        .ambient = {0.05f, 0.05f, 0.0f},
        .shininess = 128.0f * 0.078125f
    };
    auto point_light = arch_entity_manager->create_entity();
    world.add_entity_to_archetype(point_light, world.archetypes[9]);
    auto point_light_location = world.entity_locations[get_id(point_light)];
    auto* point_light_arch =
        dynamic_cast<PointLightArchetype*>(point_light_location.arch);
    const auto point_light_index = point_light_location.index;
    point_light_arch->point_lights[point_light_index] = {
        .position = {3.0f, 10.0f, 15.0f},
        .ambient = {0.1f, 0.1f, 0.1f},
        .diffuse = {0.8f, 0.8f, 0.8f},
        .specular = {2.0f, 2.0f, 2.0f},
        .constant = 1.0f,
        .linear = 0.09f,
        .quadratic = 0.032f
    };
    point_light_arch->transforms[point_light_index] = {
        .position = {3.0f, 10.0f, 15.0f},
        .scale = 0.2f
    };
    point_light_arch->meshes[point_light_index].handle = hyper_cube;
    point_light_arch->materials[point_light_index] = {
        .diffuse_texture_id = container2,
        .specular_texture_id = container2,
        .ambient = {0.05f, 0.05f, 0.0f},
        .shininess = 128.0f * 0.078125f
    };
    auto spot_light = arch_entity_manager->create_entity();
    world.add_entity_to_archetype(spot_light, world.archetypes[10]);
    auto spot_light_location = world.entity_locations[get_id(spot_light)];
    auto* spot_light_arch =
        dynamic_cast<SpotLightArchetype*>(spot_light_location.arch);
    const auto spot_light_index = spot_light_location.index;
    spot_light_arch->spot_lights[spot_light_index] = {
        .position = {1.0f, 12.0f, 5.0f},
        .direction = {0.0f, -1.0f, 2.0f},
        .cut_off = 32.5f,
        .outer_cut_off = 37.5f,
        .ambient = {0.05f, 0.05f, 0.05f},
        .diffuse = {3.8f, 3.8f, 3.8f},
        .specular = {5.0f, 5.0f, 5.0f},
        .constant = 1.0f,
        .linear = 0.09f,
        .quadratic = 0.032f
    };
    spot_light_arch->transforms[spot_light_index] = {
        .position = {1.0f, 12.0f, 5.0f},
        .scale = 0.2f
    };
    spot_light_arch->meshes[spot_light_index].handle = simple_cube;
    spot_light_arch->materials[spot_light_index] = {
        .diffuse_texture_id = gray_texture,
        .specular_texture_id = gray_texture
    };
    glvm_log::info("hello", "scene built, entering game loop");
    engine->game_loop();
    glvm_log::info("hello", "game loop exited");
    engine->game_kill();
    delete procedural_level_system;
    delete movement_system;
    delete enemy_system;
    delete projectile_system;
    delete collision_system;
    delete damage_system;
    delete physics_system;
    delete inventory_system;
    delete item_system;
    delete entity_manager;
    delete component_manager;
    delete engine;
    glvm_log::info("hello", "teardown done, exiting");
}

auto create_projectile(
    const Vector<f32, 3>& projectile_position,
    const Vector<f32, 3>& projectile_forward,
    const MeshHandle& mesh_handle,
    const Material& material,
    const Damage& damage,
    const EntityLocation& projectile_location
) -> void {
    auto* projectile_arch = as<ProjectileArchetype*>(projectile_location.arch);
    const auto projectile_index = projectile_location.index;
    auto* projectile_mesh = &projectile_arch->meshes[projectile_index];
    projectile_mesh->handle = mesh_handle;
    auto* projectile_bundle =
        &projectile_arch->projectile_bundles[projectile_index];
    projectile_bundle->material = material;
    auto* projectile_transform = &projectile_arch->transforms[projectile_index];
    auto* projectile_health = &projectile_arch->health[projectile_index];
    projectile_health->max_health = 100;
    projectile_health->current_health = 100;
    // Flying projectiles need no health bars.
    projectile_health->renderable = false;
    projectile_arch->colliders[projectile_index].colliders.clear();
    projectile_transform->scale = 0.1f;
    projectile_transform->position = projectile_position;
    projectile_transform->forward = projectile_forward;
    projectile_transform->position += projectile_transform->forward;
    projectile_bundle->damage = damage;
}

auto ProceduralLevelGeneratingSystem::update() -> void {
    using namespace glvm;
    Engine* glvm = Engine::get_instance();
    // New arch ECS.
    ArchetypeEntityManager* arch_entity_manager =
        ArchetypeEntityManager::get_instance();
    world.search_cache_archetypes(
        player_required_mask,
        &arch_view.cached_player_arch,
        cached_player_arch_number
    );
    components_view.player_transforms =
        as<Transform*>(arch_view.cached_player_arch
                           ->components[ComponentsIndices::TransformComponent]);
    while (level_number < 5) {
        Vec<Vertex> next_level;
        Vec<u32> indices;
        Vec<Vertex> transition_bridge_vertices;
        Vec<u32> transition_bridge_indices;
        if (level_number < 5) {
            std::random_device rd;
            std::mt19937 mersenne(rd());
            std::uniform_int_distribution<i32> dist_current_level_y(1, 1);
            u32 level_half_y = dist_current_level_y(mersenne);
            std::uniform_int_distribution<i32> dist_current_level_x_z(32, 32);
            u32 level_half_x = dist_current_level_x_z(mersenne);
            u32 level_half_z = dist_current_level_x_z(mersenne);
            // Need to move on half.
            constexpr auto TRANSITION_BRIDGE_HALF_WIDTH = 0.5f;
            constexpr auto TRANSITION_BRIDGE_HALF_HEIGHT = 1.0f;
            // On first iteration we dont need to define where locate current
            // level depends on previousTransitionBridge.
            if (level_number != 0) {
                generate_level(
                    level_half_x,
                    level_half_y,
                    level_half_z,
                    TRANSITION_BRIDGE_HALF_WIDTH,
                    TRANSITION_BRIDGE_HALF_HEIGHT
                );
            } else {
                // Set to first level maximum values.
                coordinate_maximum_value_per_direction.lowest_x =
                    current_level_position[0] - level_half_x;
                coordinate_maximum_value_per_direction.highest_x =
                    current_level_position[0] + level_half_x;
                coordinate_maximum_value_per_direction.lowest_y =
                    current_level_position[1] - level_half_y;
                coordinate_maximum_value_per_direction.highest_y =
                    current_level_position[1] + level_half_y;
                coordinate_maximum_value_per_direction.lowest_z =
                    current_level_position[2] - level_half_z;
                coordinate_maximum_value_per_direction.highest_z =
                    current_level_position[2] + level_half_z;
            }
            generate_transition_bridge(
                level_half_x,
                level_half_y,
                level_half_z,
                TRANSITION_BRIDGE_HALF_WIDTH,
                TRANSITION_BRIDGE_HALF_HEIGHT
            );
            for (const auto i : BOX_INDICES_FOR_INDEX_BUFFER) {
                indices.push_back(i);
            }
            mesh_axis_limiting_values.set_to_default_values();
            make_cube_object_vertices(
                {-1, -1, -1, -1},
                {1, 1, 1, 1},
                level_half_x,
                level_half_y,
                level_half_z,
                next_level
            );
            set_mesh_bounds(mesh_axis_limiting_values);
            MeshHandle game_level_mesh_handle = glvm->load_mesh();
            u64 game_level_chunk_entity = arch_entity_manager->create_entity();
            cached_level_chunk_arch_number = 0;
            // Search and cache one time for LevelChunkArch.
            world.search_cache_archetypes(
                required_mask,
                &arch_view.cached_level_chunk_arch,
                cached_level_chunk_arch_number
            );
            world.add_entity_to_archetype(
                game_level_chunk_entity,
                arch_view.cached_level_chunk_arch
            );
            EntityLocation game_level_chunk_location =
                world.entity_locations[get_id(game_level_chunk_entity)];
            LevelChunkArchetype* level_chunk_arch =
                as<LevelChunkArchetype*>(game_level_chunk_location.arch);
            const auto game_level_chunk_index = game_level_chunk_location.index;
            TextureHandle game_level_texture = texture_handlers[2];
            if (level_number == 0) {
                // Set up current level position to player position.
                components_view.player_transforms->position = Vector<f32, 3>(
                    current_level_position[0],
                    components_view.player_transforms->position[1],
                    current_level_position[2]
                );
            }
            level_chunk_arch->transforms[game_level_chunk_index] = {
                .position = current_level_position,
                .scale = 1.0f
            };
            level_chunk_arch->materials[game_level_chunk_index] = {
                .diffuse_texture_id = game_level_texture,
                .specular_texture_id = game_level_texture,
                .ambient = {0.05f, 0.05f, 0.0f},
                .shininess = 128.0f * 0.078125f
            };
            level_chunk_arch->meshes[game_level_chunk_index].handle =
                game_level_mesh_handle;

            for (const auto i : BOX_INDICES_FOR_INDEX_BUFFER) {
                transition_bridge_indices.push_back(i);
            }
            mesh_axis_limiting_values.set_to_default_values();
            f32 half_x = 0.0f;
            f32 half_y = level_half_y;
            f32 half_z = 0.0f;
            set_half_extents_from_direction(
                half_x,
                half_z,
                TRANSITION_BRIDGE_HALF_WIDTH,
                TRANSITION_BRIDGE_HALF_HEIGHT,
                next_level_transition_direction
            );
            make_cube_object_vertices(
                {-1, -1, -1, -1},
                {1, 1, 1, 1},
                half_x,
                half_y,
                half_z,
                transition_bridge_vertices
            );
            set_mesh_bounds(mesh_axis_limiting_values);
            MeshHandle transition_bridge_mesh_handle = glvm->load_mesh();
            u64 transition_bridge_entity = arch_entity_manager->create_entity();
            world.add_entity_to_archetype(
                transition_bridge_entity,
                arch_view.cached_level_chunk_arch
            );
            EntityLocation transition_bridge_location =
                world.entity_locations[get_id(transition_bridge_entity)];
            LevelChunkArchetype* transition_bridge_arch =
                as<LevelChunkArchetype*>(transition_bridge_location.arch);
            const auto transition_bridge_index =
                transition_bridge_location.index;
            TextureHandle transition_bridge_texture = texture_handlers[3];
            transition_bridge_arch->transforms[transition_bridge_index] = {
                .position = transition_bridge_position,
                .scale = 1.0f
            };
            transition_bridge_arch->materials[transition_bridge_index] = {
                .diffuse_texture_id = transition_bridge_texture,
                .specular_texture_id = transition_bridge_texture,
                .ambient = {0.05f, 0.05f, 0.0f},
                .shininess = 128.0f * 0.078125f
            };
            transition_bridge_arch->meshes[transition_bridge_index].handle =
                transition_bridge_mesh_handle;
            ++level_number;
        }
        level_generated_vertices.push_back(next_level);
        level_generated_indices.push_back(indices);
        level_generated_vertices.push_back(transition_bridge_vertices);
        level_generated_indices.push_back(transition_bridge_indices);
    }
}

auto ProceduralLevelGeneratingSystem::set_half_extents_from_direction(
    f32& half_x,
    f32& half_z,
    const f32& transition_bridge_half_width,
    const f32& transition_bridge_half_height,
    const f32& next_level_transition_direction
) -> void {
    if (next_level_transition_direction == 1
        || next_level_transition_direction == 3) {
        half_x = transition_bridge_half_width;
        half_z = transition_bridge_half_height;
    } else if (
        next_level_transition_direction == 2
        || next_level_transition_direction == 4
    ) {
        half_x = transition_bridge_half_height;
        half_z = transition_bridge_half_width;
    }
}

auto ProceduralLevelGeneratingSystem::generate_level(
    const u32 level_half_x,
    const u32 level_half_y,
    const u32 level_half_z,
    const f32 transition_bridge_half_width,
    const f32 transition_bridge_half_height
) -> void {
    std::random_device rd;
    std::mt19937 mersenne(rd());
    u32 previous_transition_bridge_anchor_point = 0;
    bool valid_level = false;
    while (!valid_level) {
        switch (previous_iteration_transition_bridge_direction) {
            case 1: {
                std::uniform_int_distribution<i32>
                    dist_previous_transition_bridge_anchor_point(
                        0,
                        (level_half_x * 2) - 1
                    );
                previous_transition_bridge_anchor_point =
                    dist_previous_transition_bridge_anchor_point(mersenne);
                current_level_position[0] = transition_bridge_position[0]
                    - level_half_x + transition_bridge_half_width
                    + previous_transition_bridge_anchor_point;
                current_level_position[2] = transition_bridge_position[2]
                    + level_half_z + transition_bridge_half_height;
            } break;
            case 2: {
                std::uniform_int_distribution<i32>
                    dist_previous_transition_bridge_anchor_point(
                        0,
                        (level_half_z * 2) - 1
                    );
                previous_transition_bridge_anchor_point =
                    dist_previous_transition_bridge_anchor_point(mersenne);
                current_level_position[2] = transition_bridge_position[2]
                    - level_half_z + transition_bridge_half_width
                    + previous_transition_bridge_anchor_point;
                current_level_position[0] = transition_bridge_position[0]
                    + level_half_x + transition_bridge_half_height;
            } break;
            case 3: {
                std::uniform_int_distribution<i32>
                    dist_previous_transition_bridge_anchor_point(
                        0,
                        (level_half_x * 2) - 1
                    );
                previous_transition_bridge_anchor_point =
                    dist_previous_transition_bridge_anchor_point(mersenne);
                current_level_position[0] = transition_bridge_position[0]
                    - level_half_x + transition_bridge_half_width
                    + previous_transition_bridge_anchor_point;
                current_level_position[2] = transition_bridge_position[2]
                    - level_half_z - transition_bridge_half_height;
            } break;
            case 4: {
                std::uniform_int_distribution<i32>
                    dist_previous_transition_bridge_anchor_point(
                        0,
                        (level_half_z * 2) - 1
                    );
                previous_transition_bridge_anchor_point =
                    dist_previous_transition_bridge_anchor_point(mersenne);
                current_level_position[2] = transition_bridge_position[2]
                    - level_half_z + transition_bridge_half_width
                    + previous_transition_bridge_anchor_point;
                current_level_position[0] = transition_bridge_position[0]
                    - level_half_x - transition_bridge_half_height;
            } break;
        }
        if (check_collision_intersection_with_maximum_coordinates(
                current_level_position,
                level_half_x,
                level_half_y,
                level_half_z
            )) {
            previous_iteration_transition_bridge_direction =
                ((4 + previous_iteration_transition_bridge_direction) % 4) + 1;
        } else {
            coordinate_maximum_value_per_direction
                .compare_per_direction_and_set_to_maximum_value_by_module(
                    current_level_position,
                    as<f32>(level_half_x),
                    as<f32>(level_half_y),
                    as<f32>(level_half_z)
                );
            valid_level = true;
        }
    }
    current_level_position[1] = 0.0f;
}

auto ProceduralLevelGeneratingSystem::generate_transition_bridge(
    const u32 level_half_x,
    const u32 level_half_y,
    const u32 level_half_z,
    const f32 transition_bridge_half_width,
    const f32 transition_bridge_half_height
) -> void {
    std::random_device rd;
    std::mt19937 mersenne(rd());
    // 1 - north, 2 - east, 3 - south, 4 - west.
    std::uniform_int_distribution<i32> dist_next_level_transition_direction(
        1,
        4
    );
    // Randomly choose the direction where the next level will appear.
    next_level_transition_direction =
        dist_next_level_transition_direction(mersenne);
    u32 transition_bridge_anchor_point = 0;
    f32 transition_bridge_offset_x = 0.0f;
    f32 transition_bridge_offset_z = 0.0f;
    bool valid_transition_bridge = false;
    while (!valid_transition_bridge) {
        // Choose up (1) or down (3) insert point direction.
        if (next_level_transition_direction == 1
            || next_level_transition_direction == 3) {
            // In what point we connect next transition bridge to current level.
            std::uniform_int_distribution<i32>
                dist_transition_bridge_anchor_point(0, (level_half_x * 2) - 1);
            transition_bridge_anchor_point =
                dist_transition_bridge_anchor_point(mersenne);
            // Sum the leftmost position with the random value of the point
            // where the transition bridge will be inserted.
            transition_bridge_offset_x = -as<f32>(level_half_x)
                + as<f32>(transition_bridge_anchor_point);
            if (next_level_transition_direction == 1) {
                // Move to the bottom level edge.
                transition_bridge_offset_z = level_half_z;
                transition_bridge_position = {
                    current_level_position[0] + transition_bridge_offset_x
                        + transition_bridge_half_width,
                    as<f32>(level_half_y),
                    current_level_position[2] + transition_bridge_offset_z
                        + transition_bridge_half_height
                };
            } else {
                // Move to the upper level edge.
                transition_bridge_offset_z = -as<f32>(level_half_z);
                transition_bridge_position = {
                    current_level_position[0] + transition_bridge_offset_x
                        + transition_bridge_half_width,
                    as<f32>(level_half_y),
                    current_level_position[2] + transition_bridge_offset_z
                        - transition_bridge_half_height
                };
            }
            // Choose left (2) or right (4) insert point direction.
        } else if (
            next_level_transition_direction == 2
            || next_level_transition_direction == 4
        ) {
            // In what point we connect next transition bridge to current level.
            std::uniform_int_distribution<i32>
                dist_transition_bridge_anchor_point(0, (level_half_z * 2) - 1);
            transition_bridge_anchor_point =
                dist_transition_bridge_anchor_point(mersenne);
            // Sum the foremost position with the random value of the point
            // where the transition bridge will be inserted.
            transition_bridge_offset_z = -as<f32>(level_half_z)
                + as<f32>(transition_bridge_anchor_point);
            if (next_level_transition_direction == 2) {
                // Move to the right level edge.
                transition_bridge_offset_x = level_half_x;
                transition_bridge_position = {
                    current_level_position[0] + transition_bridge_offset_x
                        + transition_bridge_half_height,
                    as<f32>(level_half_y),
                    current_level_position[2] + transition_bridge_offset_z
                        + transition_bridge_half_width
                };
            } else {
                // Move to the left level edge.
                transition_bridge_offset_x = -as<f32>(level_half_x);
                transition_bridge_position = {
                    current_level_position[0] + transition_bridge_offset_x
                        - transition_bridge_half_height,
                    as<f32>(level_half_y),
                    current_level_position[2] + transition_bridge_offset_z
                        + transition_bridge_half_width
                };
            }
        }
        f32 width = 0;
        f32 height = 0;
        // Choose transition_bridge_half_width as X and
        // transition_bridge_half_height as Z.
        set_half_extents_from_direction(
            width,
            height,
            transition_bridge_half_width,
            transition_bridge_half_height,
            next_level_transition_direction
        );
        if (check_collision_intersection_with_maximum_coordinates(
                transition_bridge_position,
                width,
                level_half_y,
                height
            )) {
            // Need to choose another direction if we got collided with level.
            next_level_transition_direction =
                ((4 + next_level_transition_direction) % 4) + 1;
        } else {
            // Setting up bounds for all levels.
            coordinate_maximum_value_per_direction
                .compare_per_direction_and_set_to_maximum_value_by_module(
                    transition_bridge_position,
                    as<f32>(width),
                    as<f32>(level_half_y),
                    as<f32>(height)
                );
            valid_transition_bridge = true;
        }
    }
    transition_bridge_position[1] = current_level_position[1];
    previous_iteration_transition_bridge_direction =
        next_level_transition_direction;
}

auto ProceduralLevelGeneratingSystem::make_cube_object_vertices(
    Vector<f32, 4> joint_indices,
    Vector<f32, 4> weights,
    f32 half_x,
    f32 half_y,
    f32 half_z,
    Vec<Vertex>& destination_vertices_container
) -> void {
    u32 cube_vertices = 8;
    for (u32 i = 0; i < cube_vertices; ++i) {
        Position vertex {};
        switch (i) {
            case 0:
                vertex[0] = half_x;
                vertex[1] = half_y;
                vertex[2] = half_z;
                break;
            case 1:
                vertex[0] = -as<f32>(half_x);
                vertex[1] = half_y;
                vertex[2] = half_z;
                break;
            case 2:
                vertex[0] = -as<f32>(half_x);
                vertex[1] = -as<f32>(half_y);
                vertex[2] = half_z;
                break;
            case 3:
                vertex[0] = half_x;
                vertex[1] = -as<f32>(half_y);
                vertex[2] = half_z;
                break;
            case 4:
                vertex[0] = half_x;
                vertex[1] = half_y;
                vertex[2] = -as<f32>(half_z);
                break;
            case 5:
                vertex[0] = -as<f32>(half_x);
                vertex[1] = half_y;
                vertex[2] = -as<f32>(half_z);
                break;
            case 6:
                vertex[0] = -as<f32>(half_x);
                vertex[1] = -as<f32>(half_y);
                vertex[2] = -as<f32>(half_z);
                break;
            case 7:
                vertex[0] = half_x;
                vertex[1] = -as<f32>(half_y);
                vertex[2] = -as<f32>(half_z);
                break;
        }
        mesh_axis_limiting_values
            .compare_per_direction_and_set_to_maximum_value_by_module(vertex);
        Position normal {};
        normal[0] = 0;
        normal[1] = 1;
        normal[2] = 0;
        Position texture {};
        texture[0] = 0;
        texture[1] = 1;
        destination_vertices_container.push_back(
            {.pos = {vertex[0], vertex[1], vertex[2]},
             .color = {normal[0], normal[1], normal[2]},
             .tex_coord = {texture[0], texture[1]},
             .joint_indices =
                 {joint_indices[0],
                  joint_indices[1],
                  joint_indices[2],
                  joint_indices[3]},
             .weights = {weights[0], weights[1], weights[2], weights[3]}}
        );
    }
}

auto ProceduralLevelGeneratingSystem::
    check_collision_intersection_with_maximum_coordinates(
        Vector<f32, 3> position,
        f32 half_x,
        f32 half_y,
        f32 half_z
    ) const -> bool {
    return position[0] + half_x
        > coordinate_maximum_value_per_direction.lowest_x
        && position[0] - half_x
        < coordinate_maximum_value_per_direction.highest_x
        && position[1] + half_y
        > coordinate_maximum_value_per_direction.lowest_y
        && position[1] - half_y
        < coordinate_maximum_value_per_direction.highest_y
        && position[2] + half_z
        > coordinate_maximum_value_per_direction.lowest_z
        && position[2] - half_z
        < coordinate_maximum_value_per_direction.highest_z;
}

auto EnemySystem::update() -> void {
    player_archetypes_number = 0;
    world.search_cache_archetypes(
        player_required_mask,
        &arch_view.player_cached_archetype,
        player_archetypes_number
    );
    components_view.player_transforms =
        as<Transform*>(arch_view.player_cached_archetype
                           ->components[ComponentsIndices::TransformComponent]);
    enemy_archetypes_number = 0;
    world.search_cache_archetypes(
        enemy_required_mask,
        &arch_view.enemy_cached_archetype,
        enemy_archetypes_number
    );
    components_view.enemy_transforms =
        as<Transform*>(arch_view.enemy_cached_archetype
                           ->components[ComponentsIndices::TransformComponent]);
    components_view.enemy_states =
        as<State*>(arch_view.enemy_cached_archetype
                       ->components[GameComponentsIndices::StateComponent]);
    components_view.enemies =
        as<Enemy*>(arch_view.enemy_cached_archetype
                       ->components[GameComponentsIndices::EnemyComponent]);
    projectile_archetypes_number = 0;
    world.search_cache_archetypes(
        projectile_required_mask,
        &arch_view.projectile_archetype,
        projectile_archetypes_number
    );
    for (u32 j = 0; j < arch_view.player_cached_archetype->entity_count; ++j) {
        Transform* player_transform_component =
            &components_view.player_transforms[j];
        for (u32 i = 0; i < arch_view.enemy_cached_archetype->entity_count;
             ++i) {
            auto* enemy_transform_component =
                &components_view.enemy_transforms[i];
            auto* state_enemy_component = &components_view.enemy_states[i];
            auto* enemy_component = &components_view.enemies[i];
            Vector<f32, 3> distance = player_transform_component->position
                - enemy_transform_component->position;
            f32 camera_speed = 5.5f * delta_frame_time;
            if (projectile_cooldown > 0) {
                projectile_cooldown -= camera_speed;
            }
            if (distance.length() > enemy_component->detect_radius
                && state_enemy_component->state == States::ATTACK) {
                f32 delta_length =
                    distance.length() - enemy_component->detect_radius;
                Vector<f32, 3> enemy_move =
                    distance * (delta_length / distance.length());
                enemy_transform_component->position += enemy_move;
            }
            if (distance.length() <= enemy_component->detect_radius) {
                if (projectile_cooldown <= 0) {
                    MeshHandle mesh_handle {};
                    const auto sphere_mesh_handle_index = 2;
                    if (mesh_handles.size() > 2) {
                        mesh_handle = mesh_handles[sphere_mesh_handle_index];
                    }
                    TextureHandle texture_handle {};
                    const auto gray_texture_handle = 2;
                    if (texture_handlers.size() > 2) {
                        texture_handle = texture_handlers[gray_texture_handle];
                    }
                    const Material material = {
                        .diffuse_texture_id = texture_handle,
                        .specular_texture_id = texture_handle,
                        .ambient = {0.05f, 0.05f, 0.05f},
                        .shininess = 128.0f * 0.078125f
                    };
                    const Damage damage = {
                        .maximum_damage = 40,
                        .minimum_damage = 20,
                        .critical_hit_rate = 0,
                        .critical_modifier = 0
                    };
                    ArchetypeEntityManager* arch_entity_manager =
                        ArchetypeEntityManager::get_instance();
                    u64 projectile_entity =
                        arch_entity_manager->create_entity();
                    world.add_entity_to_archetype(
                        projectile_entity,
                        arch_view.projectile_archetype
                    );
                    EntityLocation projectile_location =
                        world.entity_locations[get_id(projectile_entity)];

                    create_projectile(
                        enemy_transform_component->position,
                        player_transform_component->position
                            - enemy_transform_component->position,
                        mesh_handle,
                        material,
                        damage,
                        projectile_location
                    );
                    if (shoot_sound_path != nullptr) {
                        sound_engine->create_sound_sample(
                            shoot_sound_path,
                            shoot_sound_duration,
                            shoot_sound_rate,
                            shoot_sound_volume
                        );
                    }
                    projectile_cooldown = 15.0f;
                }
                state_enemy_component->state = States::ATTACK;
            }
        }
    }
}

auto InventorySystem::update() -> void {
    if (is_inventory_opened) {
        crosshair_archetypes_number = 0;
        world.search_cache_archetypes(
            crosshair_required_mask,
            &arch_view.crosshair_cached_archetype,
            crosshair_archetypes_number
        );
        components_view.crosshair_transforms_view = as<Transform*>(
            arch_view.crosshair_cached_archetype
                ->components[ComponentsIndices::TransformComponent]
        );
        inventory_archetypes_number = 0;
        world.search_cache_archetypes(
            inventory_required_mask,
            &arch_view.inventory_cached_archetype,
            inventory_archetypes_number
        );
        components_view.inventory_transforms_view = as<Transform*>(
            arch_view.inventory_cached_archetype
                ->components[ComponentsIndices::TransformComponent]
        );
        components_view.inventory_view = as<Inventory*>(
            arch_view.inventory_cached_archetype
                ->components[GameComponentsIndices::InventoryComponent]
        );
        components_view.inventory_meshes_view =
            as<Mesh*>(arch_view.inventory_cached_archetype
                          ->components[ComponentsIndices::MeshComponent]);
        if (components_view.crosshair_transforms_view
            && components_view.inventory_transforms_view
            && components_view.inventory_view
            && components_view.inventory_meshes_view) {
            Transform* crosshair_transform_component =
                &components_view.crosshair_transforms_view[0];
            Transform* inventory_transform_component =
                &components_view.inventory_transforms_view[0];
            Inventory* inventory_component = &components_view.inventory_view[0];
            Mesh* inventory_mesh_component =
                &components_view.inventory_meshes_view[0];
            const auto inventory_slot_scale = inventory_mesh_component->gltf
                ? inventory_component->slot_scale * 2.0f
                : inventory_component->slot_scale;
            const auto inventory_slot_half_scale =
                inventory_mesh_component->gltf
                ? inventory_component->slot_scale
                : inventory_component->slot_scale * 0.5f;
            // Take an item from inventory.
            if (*is_item_dragged < 0 && is_left_mouse_button_pressed
                && *is_left_mouse_button_released) {
                if (check_crosshair_inventory_intersection(
                        crosshair_transform_component,
                        inventory_transform_component,
                        inventory_component,
                        inventory_slot_scale,
                        inventory_slot_half_scale
                    )) {
                    Point2D<i32> intersection_slot =
                        determine_actual_intersection_slot(
                            crosshair_transform_component,
                            inventory_transform_component,
                            inventory_slot_scale,
                            inventory_slot_half_scale
                        );
                    const auto row = intersection_slot.y;
                    const auto column = intersection_slot.x;
                    const auto entity = inventory_component->slots[row][column];
                    // Check slot is not empty and hold an item.
                    if (entity != UINT_MAX && entity >= 0) {
                        EntityLocation item_location =
                            world.entity_locations[get_id(entity)];
                        auto* item_arch =
                            as<ItemArchetype*>(item_location.arch);
                        const auto item_index = item_location.index;
                        Item* item_component = &item_arch->items[item_index];
                        if (item_component != nullptr) {
                            for (unsigned int occupied_slot :
                                 item_component->occupied_slots) {
                                u32 row_index =
                                    occupied_slot / inventory_component->row;
                                u32 col_index =
                                    occupied_slot % inventory_component->col;
                                // Need to free all slots that hold an item.
                                inventory_component
                                    ->slots[row_index][col_index] = UINT_MAX;
                            }
                        }
                        // Set currently dragged item entity.
                        *is_item_dragged = entity;
                    }
                }
                *is_left_mouse_button_released = false;
            }

            if (*is_item_dragged >= 0) {
                if (check_crosshair_inventory_intersection(
                        crosshair_transform_component,
                        inventory_transform_component,
                        inventory_component,
                        inventory_slot_scale,
                        inventory_slot_half_scale
                    )) {
                    Point2D<i32> intersection_slot =
                        determine_actual_intersection_slot(
                            crosshair_transform_component,
                            inventory_transform_component,
                            inventory_slot_scale,
                            inventory_slot_half_scale
                        );

                    EntityLocation item_location =
                        world.entity_locations[get_id(*is_item_dragged)];
                    auto* item_arch = as<ItemArchetype*>(item_location.arch);
                    const auto item_index = item_location.index;
                    Item* item_component = &item_arch->items[item_index];

                    Vec<u32> potential_occupied_slots;
                    i32 is_swappable = 0;
                    is_swappable = determine_swappable_status_and_slots(
                        item_component,
                        inventory_transform_component,
                        potential_occupied_slots,
                        crosshair_transform_component,
                        intersection_slot,
                        inventory_component,
                        inventory_slot_scale
                    );

                    inventory_component->highlighted_slots =
                        potential_occupied_slots;
                    inventory_component->is_available_highlighted_slots =
                        is_swappable == -1 || is_swappable >= 0;
                } else {
                    inventory_component->highlighted_slots.clear();
                }
            }

            // Item drop to inventory, swapped or we just can't place.
            if (*is_item_dragged >= 0 && is_left_mouse_button_pressed
                && *is_left_mouse_button_released) {
                i32 is_swappable = 0;
                if (check_crosshair_inventory_intersection(
                        crosshair_transform_component,
                        inventory_transform_component,
                        inventory_component,
                        inventory_slot_scale,
                        inventory_slot_half_scale
                    )) {
                    Point2D<i32> intersection_slot =
                        determine_actual_intersection_slot(
                            crosshair_transform_component,
                            inventory_transform_component,
                            inventory_slot_scale,
                            inventory_slot_half_scale
                        );

                    EntityLocation item_location =
                        world.entity_locations[get_id(*is_item_dragged)];
                    auto* item_arch = as<ItemArchetype*>(item_location.arch);
                    const auto item_index = item_location.index;
                    Item* item_component = &item_arch->items[item_index];

                    Vec<u32> potential_occupied_slots;
                    is_swappable = determine_swappable_status_and_slots(
                        item_component,
                        inventory_transform_component,
                        potential_occupied_slots,
                        crosshair_transform_component,
                        intersection_slot,
                        inventory_component,
                        inventory_slot_scale
                    );

                    const auto item_width =
                        item_component->item_slot_type.width;
                    const auto item_height =
                        item_component->item_slot_type.height;

                    // Default value. Just drop item to all empty slots.
                    if (is_swappable == -1) {
                        item_component->occupied_slots =
                            potential_occupied_slots;
                        fill_inventory_slots(
                            item_component,
                            item_width,
                            item_height,
                            inventory_component,
                            *is_item_dragged
                        );
                        // Swap one item that we dragging to another one in
                        // inventory.
                    } else if (is_swappable > 0) {
                        EntityLocation item_location =
                            world.entity_locations[get_id(is_swappable)];
                        auto* item_arch =
                            as<ItemArchetype*>(item_location.arch);
                        const auto item_index = item_location.index;
                        Item* swapped_item_component =
                            &item_arch->items[item_index];

                        item_component->occupied_slots =
                            potential_occupied_slots;
                        fill_inventory_slots(
                            swapped_item_component,
                            swapped_item_component->item_slot_type.width,
                            swapped_item_component->item_slot_type.height,
                            inventory_component,
                            UINT_MAX
                        );

                        swapped_item_component->occupied_slots.clear();
                        fill_inventory_slots(
                            item_component,
                            item_width,
                            item_height,
                            inventory_component,
                            *is_item_dragged
                        );
                    }

                    if (is_swappable == -1) {
                        *is_left_mouse_button_released = false;
                        *is_item_dragged = -1;
                        // Already have 2 or more items in potential inventory
                        // slots.
                    } else if (is_swappable == -2) {
                        *is_left_mouse_button_released = false;
                    } else {
                        *is_left_mouse_button_released = false;
                        *is_item_dragged = is_swappable;
                    }
                    // Dropping the item to the ground.
                } else {
                    EntityLocation item_location =
                        world.entity_locations[get_id(*is_item_dragged)];
                    auto* item_arch = as<ItemArchetype*>(item_location.arch);
                    const auto item_index = item_location.index;
                    item_arch->rigid_bodies[item_index] = {.mass = 2.0f};
                    Transform* item_transform =
                        &item_arch->transforms[item_index];
                    Item* item = &item_arch->items[item_index];
                    item->is_actor = true;
                    // TODO: Remove this workaround.
                    const auto player = 0;
                    EntityLocation player_location =
                        world.entity_locations[get_id(player)];
                    if (player_location.arch != nullptr) {
                        auto* player_arch =
                            as<PlayerArchetype*>(player_location.arch);
                        const auto player_index = player_location.index;
                        Transform* player_transform =
                            &player_arch->transforms[player_index];
                        item_transform->position = player_transform->position;
                        Vector<f32, 3> normalized_forward =
                            normalize(player_transform->forward);
                        item_transform->position[0] +=
                            normalized_forward[0] * 2.5f;
                        item_transform->position[1] +=
                            normalized_forward[1] * 2.5f;
                        item_transform->position[2] +=
                            normalized_forward[2] * 2.5f;
                        item_transform->scale = 0.05f;
                    }

                    *is_item_dragged = -1;
                    *is_left_mouse_button_released = false;
                }
            }
        }
    }
}

auto InventorySystem::determine_swappable_status_and_slots(
    Item* item_component,
    Transform* inventory_transform_component,
    Vec<u32>& potential_occupied_slots,
    Transform* crosshair_transform_component,
    Point2D<i32> intersection_slot,
    Inventory* inventory_component,
    const f32 inventory_slot_scale
) -> i32 {
    if (item_component != nullptr) {
        const auto item_width = item_component->item_slot_type.width;
        const auto item_height = item_component->item_slot_type.height;

        const auto row = intersection_slot.y;
        const auto column = intersection_slot.x;

        // Find left-upper pivot slot inventory.
        i32 row_basic_offset = 0;
        i32 column_basic_offset = 0;

        // Set as pivot point slot in left upper corner.
        // Need to calculate offset for row and column
        // to change it from center. And need to It is
        // necessary to take into account the offset
        // relative to the center for additional correction.
        column_basic_offset = calculate_basic_offset(
            item_width,
            inventory_transform_component->position[0],
            crosshair_transform_component->position[0],
            column,
            inventory_slot_scale
        );
        row_basic_offset = calculate_basic_offset(
            item_height,
            inventory_transform_component->position[1],
            crosshair_transform_component->position[1],
            row,
            inventory_slot_scale * aspect_ratio
        );

        i32 pivot_row = row - row_basic_offset;
        i32 pivot_column = column - column_basic_offset;

        pivot_row = clamp<i32>(
            0,
            pivot_row,
            as<i32>(inventory_component->row) - item_height
        );
        pivot_column = clamp<i32>(
            0,
            pivot_column,
            as<i32>(inventory_component->col) - item_width
        );

        return determine_swappable_field(
            item_component,
            item_width,
            item_height,
            pivot_row,
            pivot_column,
            inventory_component,
            potential_occupied_slots
        );
    } // Return -3 as an error code, which means item_component is nullptr.
    return -3;
}

auto InventorySystem::fill_inventory_slots(
    Item* item_component,
    const i32 item_width,
    const i32 item_height,
    Inventory* inventory_component,
    const i32 fill_value
) -> void {
    for (i32 i = 0; i < item_height; ++i) {
        for (i32 j = 0; j < item_width; ++j) {
            const auto slots_row =
                item_component->occupied_slots[(i * item_width) + j]
                / inventory_component->col;
            const auto slots_column =
                item_component->occupied_slots[(i * item_width) + j]
                % inventory_component->col;

            inventory_component->slots[slots_row][slots_column] = fill_value;
        }
    }
}

auto InventorySystem::determine_swappable_field(
    Item* item_component,
    const i32 item_width,
    const i32 item_height,
    i32 pivot_row,
    i32 pivot_column,
    Inventory* inventory_component,
    Vec<u32>& potential_occupied_slots
) -> i32 {
    item_component->occupied_slots.clear();
    // -1: default value. -2: found two entities in potential slots. Any other
    // value: swappable.
    i32 is_swappable = -1;
    for (i32 i = 0; i < item_height; ++i) {
        for (i32 j = 0; j < item_width; ++j) {
            const auto final_row = pivot_row + i;
            const auto final_column = pivot_column + j;
            if (is_swappable == -1
                && inventory_component->slots[final_row][final_column]
                    != UINT_MAX) {
                is_swappable =
                    inventory_component->slots[final_row][final_column];
            } else if (
                is_swappable > 0
                && inventory_component->slots[final_row][final_column]
                    != UINT_MAX
                && as<i32>(inventory_component->slots[final_row][final_column])
                    != is_swappable
            ) {
                is_swappable = -2;
            }

            potential_occupied_slots.push_back(
                (final_row * inventory_component->col) + final_column
            );
        }
    }

    return is_swappable;
}

auto InventorySystem::calculate_basic_offset(
    const i32 item_axis_size,
    const f32 axis_value,
    const f32 crosshair_axis_position,
    const i32 axis_slot_index,
    const f32 inventory_slot_scale
) -> i32 {
    if (item_axis_size % 2 == 0) {
        const auto slot_center_x =
            axis_value + (as<f32>(axis_slot_index) * inventory_slot_scale);
        if (slot_center_x > crosshair_axis_position) {
            return item_axis_size / 2;
        }
        return (item_axis_size / 2) - 1;
    }
    return item_axis_size / 2;
}

auto InventorySystem::check_crosshair_inventory_intersection(
    Transform* crosshair_transform_component,
    Transform* inventory_transform_component,
    Inventory* inventory_component,
    const f32 inventory_slot_scale,
    const f32 inventory_slot_half_scale
) const -> bool {
    return crosshair_transform_component->position[0]
        > inventory_transform_component->position[0] - inventory_slot_half_scale
        && crosshair_transform_component->position[0]
        < inventory_transform_component->position[0] - inventory_slot_half_scale
            + (inventory_slot_scale * inventory_component->col)
        && crosshair_transform_component->position[1]
        > inventory_transform_component->position[1]
            - (inventory_slot_half_scale * aspect_ratio)
        && crosshair_transform_component->position[1]
        < inventory_transform_component->position[1]
            - (inventory_slot_half_scale * aspect_ratio)
            + (inventory_slot_scale * inventory_component->row * aspect_ratio);
}

auto InventorySystem::determine_actual_intersection_slot(
    Transform* crosshair_transform_component,
    Transform* inventory_transform_component,
    const f32 inventory_slot_scale,
    const f32 inventory_slot_half_scale
) const -> Point2D<i32> {
    f32 x_delta = crosshair_transform_component->position[0]
        - inventory_transform_component->position[0]
        + inventory_slot_half_scale;
    f32 y_delta = crosshair_transform_component->position[1]
        - inventory_transform_component->position[1]
        + (inventory_slot_half_scale * aspect_ratio);

    return Point2D<i32> {
        .x = as<i32>((x_delta / inventory_slot_scale)),
        .y = as<i32>((y_delta / (inventory_slot_scale * aspect_ratio)))
    };
}

// This method tries to find suitable slots for the given specific item type.
// It returns true if it finds them and false otherwise.
auto ItemSystem::put_item2x2(Inventory* inventory_component, u32 item_entity)
    -> bool {
    bool is_slot_found = false;
    u32 row = inventory_component->row;
    u32 col = inventory_component->col;
    EntityLocation item_location = world.entity_locations[get_id(item_entity)];
    auto* item_arch = as<ItemArchetype*>(item_location.arch);
    const auto item_index = item_location.index;
    Item* item_component = &item_arch->items[item_index];

    u32 item_width = item_component->item_slot_type.width;
    u32 item_height = item_component->item_slot_type.height;
    for (u32 i = 0; i < row - item_height + 1; ++i) {
        for (u32 j = 0; j < col - item_width + 1; ++j) {
            Vec<u32> maybe_available_slots;
            Vec<u32> indices_of_maybe_available_slots;
            for (u32 m = i; m < i + item_height; ++m) {
                for (u32 n = j; n < j + item_width; ++n) {
                    maybe_available_slots.push_back(
                        inventory_component->slots[m][n]
                    );
                    indices_of_maybe_available_slots.push_back((m * col) + n);
                }
            }

            u32 is_all_slots_available = 0;
            for (const auto maybe_available_slot : maybe_available_slots) {
                if (maybe_available_slot == UINT_MAX) {
                    ++is_all_slots_available;
                } else {
                    --is_all_slots_available;
                }
            }
            if (maybe_available_slots.size() == is_all_slots_available) {
                for (u32 w = 0; w < maybe_available_slots.size(); ++w) {
                    u32 row_index = indices_of_maybe_available_slots[w] / row;
                    u32 col_index = indices_of_maybe_available_slots[w] % col;
                    inventory_component->slots[row_index][col_index] =
                        item_entity;
                    item_component->occupied_slots.push_back(
                        indices_of_maybe_available_slots[w]
                    );
                }

                is_slot_found = true;
                return is_slot_found;
            }
        }
    }

    return is_slot_found;
}

auto ItemSystem::update() -> void {
    if (!is_inventory_opened) {
        inventory_archetypes_number = 0;
        world.search_cache_archetypes(
            inventory_required_mask,
            &arch_view.inventory_cached_archetype,
            inventory_archetypes_number
        );
        components_view.inventories_view = as<Inventory*>(
            arch_view.inventory_cached_archetype
                ->components[GameComponentsIndices::InventoryComponent]
        );

        item_archetypes_number = 0;
        world.search_cache_archetypes(
            item_required_mask,
            &arch_view.item_archetype,
            item_archetypes_number
        );
        components_view.items_view =
            as<Item*>(arch_view.item_archetype
                          ->components[GameComponentsIndices::ItemComponent]);
        components_view.item_colliders_view = as<Collider*>(
            arch_view.item_archetype
                ->components[ComponentsIndices::ColliderComponent]
        );

        for (u32 m = 0; m < arch_view.inventory_cached_archetype->entity_count;
             ++m) {
            Inventory* inventory_component =
                &components_view.inventories_view[m];

            for (u32 i = 0; i < arch_view.item_archetype->entity_count; ++i) {
                u32 item_entity = arch_view.item_archetype->entities[i];
                Collider* item_collider_component =
                    &components_view.item_colliders_view[i];

                for (const auto collider : item_collider_component->colliders) {
                    if (collider == inventory_component->entity_owner
                        && components_view.items_view[i].is_actor) {
                        if (put_item2x2(inventory_component, item_entity)) {
                            components_view.items_view[i].is_actor = false;
                        }
                    }
                }
            }
        }
    }

    if (is_inventory_opened) {
        crosshair_archetypes_number = 0;
        world.search_cache_archetypes(
            crosshair_required_mask,
            &arch_view.crosshair_archetype,
            crosshair_archetypes_number
        );
        components_view.crosshair_transforms = as<Transform*>(
            arch_view.crosshair_archetype
                ->components[ComponentsIndices::TransformComponent]
        );

        item_archetypes_number = 0;
        world.search_cache_archetypes(
            item_required_mask,
            &arch_view.item_archetype,
            item_archetypes_number
        );
        components_view.item_transforms_view = as<Transform*>(
            arch_view.item_archetype
                ->components[ComponentsIndices::TransformComponent]
        );

        Transform* crosshair_transform_component =
            &components_view.crosshair_transforms[0];
        for (u32 i = 0; i < arch_view.item_archetype->entity_count; ++i) {
            u32 entity_item_containing = arch_view.item_archetype->entities[i];
            Transform* item_transform_component =
                &components_view.item_transforms_view[i];
            if (*dragged_item_entity >= 0
                && *dragged_item_entity == as<i32>(entity_item_containing)) {
                // Set crosshair position to dragged items.
                item_transform_component->position =
                    crosshair_transform_component->position;
            }
        }
    }
}

MovementSystem::MovementSystem(EventStack& input_stack) :
    input_stack(input_stack) {
}

auto MovementSystem::update() -> void {
    world.search_cache_archetypes(
        player_required_mask,
        &arch_view.player_cached_archetype,
        player_archetypes_number
    );
    components_view.player_moves =
        as<Move*>(arch_view.player_cached_archetype
                      ->components[GameComponentsIndices::MoveComponent]);
    components_view.player_views =
        as<Beholder*>(arch_view.player_cached_archetype
                          ->components[ComponentsIndices::ViewComponent]);
    components_view.player_collider_flags = as<ColliderFlags*>(
        arch_view.player_cached_archetype
            ->components[ComponentsIndices::ColliderFlagsComponent]
    );
    components_view.player_rigid_body =
        as<RigidBody*>(arch_view.player_cached_archetype
                           ->components[ComponentsIndices::RigidBodyComponent]);
    components_view.player_transforms =
        as<Transform*>(arch_view.player_cached_archetype
                           ->components[ComponentsIndices::TransformComponent]);
    components_view.player_rotations = as<Rotation*>(
        arch_view.player_cached_archetype
            ->components[GameComponentsIndices::RotationComponent]
    );

    const auto camera_speed = 3.0f * delta_frame_time;
    for (u32 i = 0; i < arch_view.player_cached_archetype->entity_count; ++i) {
        const auto entity = arch_view.player_cached_archetype->entities[i];
        EntityLocation& entity_location =
            world.entity_locations[get_id(entity)];
        Beholder* player_view = &components_view.player_views[i];
        Move* player_move = &components_view.player_moves[i];
        ColliderFlags* player_collider_flags =
            &components_view.player_collider_flags[i];
        RigidBody* player_rigid_body = &components_view.player_rigid_body[i];
        Rotation* player_rotation = &components_view.player_rotations[i];
        for (i32 n = 0; n < 6; ++n) {
            Vector<f32, 3> right;
            Vector<f32, 3> forward;
            switch (input_stack[n]) {
                case EventKind::MoveLeft:
                    right = calculate_vector_rl(*player_view);
                    player_move->frame_movement -= right * camera_speed;
                    entity_location.is_dirty = true;
                    break;
                case EventKind::MoveRight:
                    right = calculate_vector_rl(*player_view);
                    player_move->frame_movement += right * camera_speed;
                    entity_location.is_dirty = true;
                    break;
                case EventKind::MoveBackward:
                    forward = calculate_vector_fb(*player_view, global_event);
                    player_move->frame_movement -= forward * camera_speed;
                    entity_location.is_dirty = true;
                    break;
                case EventKind::MoveForward:
                    forward = calculate_vector_fb(*player_view, global_event);
                    player_move->frame_movement += forward * camera_speed;
                    entity_location.is_dirty = true;
                    break;
                case EventKind::Jump: {
                    entity_location.is_dirty = true;
                    u8 is_ground_collision_mask =
                        (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                    if ((player_collider_flags->flags
                         & is_ground_collision_mask)
                        != 0) {
                        player_rigid_body->jump_accumulator = 1.0f;
                    }
                } break;
                default:
                    break;
            }
        }
        const bool is_moving =
            std::abs(player_move->frame_movement[0]) > 0.0f
            || std::abs(player_move->frame_movement[2]) > 0.0f;
        const Vector<f32, 3> facing = is_moving
            ? Vector<f32, 3>(
                  player_move->frame_movement[0],
                  0.0f,
                  player_move->frame_movement[2]
              )
            : Vector<f32, 3>(
                  player_view->forward[0],
                  0.0f,
                  player_view->forward[2]
              );
        if (vec_length(facing) > 0.001f) {
            const auto normalized_facing = normalize(facing);
            player_rotation->yaw =
                std::atan2(normalized_facing[0], normalized_facing[2]);
        }
    }

    rigid_body_contained_archetypes_number = 0;
    world.search_cache_archetypes(
        rigid_body_required_mask,
        arch_view.rigid_body_contained_archetypes_cache.data(),
        rigid_body_contained_archetypes_number
    );

    for (u32 i0 = 0; i0 < rigid_body_contained_archetypes_number; ++i0) {
        Archetype* current_arch =
            arch_view.rigid_body_contained_archetypes_cache[i0];
        components_view.transforms = as<Transform*>(
            current_arch->components[ComponentsIndices::TransformComponent]
        );
        components_view.rigid_bodies = as<RigidBody*>(
            current_arch->components[ComponentsIndices::RigidBodyComponent]
        );
        components_view.moves = as<Move*>(
            current_arch->components[GameComponentsIndices::MoveComponent]
        );
        components_view.items = as<Item*>(
            current_arch->components[GameComponentsIndices::ItemComponent]
        );

        for (u32 i1 = 0; i1 < current_arch->entity_count; ++i1) {
            const auto entity = current_arch->entities[i1];
            EntityLocation& entity_location =
                world.entity_locations[get_id(entity)];
            entity_location.is_dirty = true;

            if ((components_view.items != nullptr)
                && !components_view.items[i1].is_actor) {
                continue;
            }

            Transform* transform_component = &components_view.transforms[i1];
            RigidBody* rigid_body_component = &components_view.rigid_bodies[i1];
            Move* move_component = &components_view.moves[i1];
            transform_component->gravity_accumulator += delta_frame_time;
            f32 gravity = 9.8f * transform_component->gravity_accumulator
                * rigid_body_component->mass * 0.0005f;
            gravity = std::min(gravity, as<f32>(0.2f));

            move_component->gravity[1] -= gravity;
        }
    }
}

auto MovementSystem::calculate_vector_rl(Beholder& beholder) -> Vector<f32, 3> {
    Vector<f32, 3> normalized_vector =
        normalize(cross(beholder.forward, Vector<f32, 3> {0.0f, -1.0f, 0.0f}));
    return normalized_vector;
}

auto MovementSystem::calculate_vector_fb(Beholder& beholder, Event& /*event*/)
    -> Vector<f32, 3> {
    const Vector<f32, 3> forward(
        beholder.forward[0],
        0.0f,
        beholder.forward[2]
    );
    if (vec_length(forward) < 0.001f) {
        return Vector<f32, 3>(0.0f, 0.0f, -1.0f);
    }
    return normalize(forward);
}

ProjectileSystem::ProjectileSystem(EventStack& input_stack) :
    input_stack(input_stack) {
}

auto ProjectileSystem::update() -> void {
    f32 camera_speed = 5.5f * delta_frame_time;

    player_archetypes_number = 0;
    world.search_cache_archetypes(
        player_required_mask,
        &arch_view.player_cached_archetype,
        player_archetypes_number
    );
    components_view.player_transforms =
        as<Transform*>(arch_view.player_cached_archetype
                           ->components[ComponentsIndices::TransformComponent]);
    components_view.player_views =
        as<Beholder*>(arch_view.player_cached_archetype
                          ->components[ComponentsIndices::ViewComponent]);

    projectile_archetypes_number = 0;
    world.search_cache_archetypes(
        projectile_required_mask,
        &arch_view.projectile_archetype,
        projectile_archetypes_number
    );

    if (projectile_cooldown > 0) {
        projectile_cooldown -= camera_speed;
    }

    // Iterate on every player and create projectile if "LMB pressed" event
    // found.
    for (u32 i = 0; i < arch_view.player_cached_archetype->entity_count; ++i) {
        Beholder* player_view = &components_view.player_views[i];
        Transform* player_transform = &components_view.player_transforms[i];
        const auto max_event_number = 6;
        for (u32 n = 0; n < max_event_number; ++n) {
            if (!is_inventory_opened
                && input_stack.search_element(EventKind::MouseLeftButton)
                    == EventKind::MouseLeftButton) {
                if (projectile_cooldown <= 0) {
                    MeshHandle mesh_handle {};
                    const auto sphere_mesh_handle_index = 2;
                    if (mesh_handles.size() > 2) {
                        mesh_handle = mesh_handles[sphere_mesh_handle_index];
                    }

                    TextureHandle texture_handle {};
                    const auto gray_texture_handle = 2;
                    if (texture_handlers.size() > 2) {
                        texture_handle = texture_handlers[gray_texture_handle];
                    }

                    const Material material = {
                        .diffuse_texture_id = texture_handle,
                        .specular_texture_id = texture_handle,
                        .ambient = {0.05f, 0.05f, 0.05f},
                        .shininess = 128.0f * 0.078125f
                    };

                    const Damage damage = {
                        .maximum_damage = 40,
                        .minimum_damage = 20,
                        .critical_hit_rate = 0,
                        .critical_modifier = 0
                    };

                    ArchetypeEntityManager* arch_entity_manager =
                        ArchetypeEntityManager::get_instance();
                    u64 projectile_entity =
                        arch_entity_manager->create_entity();
                    world.add_entity_to_archetype(
                        projectile_entity,
                        arch_view.projectile_archetype
                    );
                    EntityLocation projectile_location =
                        world.entity_locations[get_id(projectile_entity)];

                    create_projectile(
                        player_transform->position,
                        player_view->forward,
                        mesh_handle,
                        material,
                        damage,
                        projectile_location
                    );

                    if (shoot_sound_path != nullptr) {
                        sound_engine->create_sound_sample(
                            shoot_sound_path,
                            shoot_sound_duration,
                            shoot_sound_rate,
                            shoot_sound_volume
                        );
                    }
                    projectile_cooldown = 2.0f;
                }
            }
        }
    }

    projectile_archetypes_number = 0;
    world.search_cache_archetypes(
        projectile_required_mask,
        &arch_view.projectile_archetype,
        projectile_archetypes_number
    );

    components_view.projectile_transforms =
        as<Transform*>(arch_view.projectile_archetype
                           ->components[ComponentsIndices::TransformComponent]);
    components_view.projectile_collider_flags = as<ColliderFlags*>(
        arch_view.projectile_archetype
            ->components[ComponentsIndices::ColliderFlagsComponent]
    );
    components_view.projectile_colliders =
        as<Collider*>(arch_view.projectile_archetype
                          ->components[ComponentsIndices::ColliderComponent]);
    components_view.projectile_bundles = as<ProjectileBundle*>(
        arch_view.projectile_archetype
            ->components[GameComponentsIndices::ProjectileBundleComponent]
    );
    components_view.projectile_health =
        as<Health*>(arch_view.projectile_archetype
                        ->components[GameComponentsIndices::HealthComponent]);
    components_view.projectile_attacks =
        as<Attack*>(arch_view.projectile_archetype
                        ->components[GameComponentsIndices::AttackComponent]);

    // Update position of every projectile.
    for (u32 x = 0; x < arch_view.projectile_archetype->entity_count; ++x) {
        Transform* projectile_transform =
            &components_view.projectile_transforms[x];
        projectile_transform->position +=
            normalize(projectile_transform->forward) * camera_speed * 2.5f;
    }
    // Iterate every projectile, check for collisions with another entities and
    // update damage info if collided entity has attack component.
    for (u32 i = 0; i < arch_view.projectile_archetype->entity_count; ++i) {
        ColliderFlags* projectile_collider_flags =
            &components_view.projectile_collider_flags[i];
        Health* projectile_health = &components_view.projectile_health[i];
        Attack* projectile_attack = &components_view.projectile_attacks[i];
        const auto wall_collision_bit = 1;
        const auto ground_collision_bit = (1 << 1);
        if (((projectile_collider_flags->flags & wall_collision_bit) != 0)
            || ((projectile_collider_flags->flags & ground_collision_bit)
                != 0)) {
            Damage* projectile_damage =
                &components_view.projectile_bundles[i].damage;
            Collider* projectile_collider =
                &components_view.projectile_colliders[i];
            for (const auto collided_entity : projectile_collider->colliders) {
                EntityLocation collided_entity_location =
                    world.entity_locations[get_id(collided_entity)];
                u64 required_mask =
                    (1ull << GameComponentsIndices::HealthComponent)
                    | (1ull << GameComponentsIndices::AttackComponent);

                if ((collided_entity_location.arch != nullptr)
                    && (collided_entity_location.arch->mask & required_mask)
                        == required_mask) {
                    auto* attacks = as<Attack*>(
                        collided_entity_location.arch
                            ->components[GameComponentsIndices::AttackComponent]
                    );
                    attacks[collided_entity_location.index].damage =
                        projectile_damage->maximum_damage;
                    projectile_health->current_health = 0;
                }
            }
        }
    }
}

auto CollisionSystem::update() -> void {
    // Common spatial grid data.
    const SpatialGrid& spatial_grid = world.spatial_grid;
    assert(
        spatial_grid.width > 0 && spatial_grid.height > 0
        && spatial_grid.depth > 0
    );
    const auto chunk_size = spatial_grid.grid[0][0][0].SIZE;

    const auto chunk_half_width = spatial_grid.width * chunk_size * 0.5f;
    const auto chunk_half_height = spatial_grid.height * chunk_size * 0.5f;
    const auto chunk_half_depth = spatial_grid.depth * chunk_size * 0.5f;

    cached_archetypes_number = 0;
    world.search_cache_archetypes(
        required_mask,
        cached_archetypes.data(),
        cached_archetypes_number
    );

    const auto camera_speed = 5.5f * delta_time;
    // Outer loop over every archetype.
    for (u32 x = 0; x < cached_archetypes_number; ++x) {
        Archetype* arch = cached_archetypes[x];
        view.backtracking_transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        view.backtracking_colliders = as<Collider*>(
            arch->components[ComponentsIndices::ColliderComponent]
        );
        view.backtracking_collider_flags = as<ColliderFlags*>(
            arch->components[ComponentsIndices::ColliderFlagsComponent]
        );
        view.backtracking_meshes =
            as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);

        for (u32 i = 0; i < arch->entity_count; ++i) {
            // Iterate over every entity in the current outer archetype.
            u32 backtracking_entity_id = arch->entities[i];

            u8 ground_collision_turn_off_mask =
                (1u << 0) | (0u << 1) | (1u << 2) | (1u << 3);
            if (view.backtracking_collider_flags && view.backtracking_colliders
                && view.backtracking_meshes && view.backtracking_transforms) {
                view.backtracking_collider_flags[i].flags =
                    view.backtracking_collider_flags[i].flags
                    & ground_collision_turn_off_mask;
                view.backtracking_colliders[i].colliders.clear();
                Mesh backtracking_entity_mesh = view.backtracking_meshes[i];
                MeshHandle backtracking_entity_mesh_handle =
                    backtracking_entity_mesh.handle;
                Transform* backtracking_transform_component =
                    &view.backtracking_transforms[i];
                Vector<f32, 3> backtracking_transform =
                    backtracking_transform_component->position;
                f32 backtracking_scale =
                    backtracking_transform_component->scale;

                u64 move_required_mask =
                    (1ull << GameComponentsIndices::MoveComponent);
                // Check if the outer (current) archetype has a move component.
                if (matches_required_mask(arch->mask, move_required_mask)) {
                    view.backtracking_move = as<Move*>(
                        arch->components[GameComponentsIndices::MoveComponent]
                    );
                    backtracking_transform +=
                        normalize(view.backtracking_move[i].frame_movement)
                        * camera_speed;
                    backtracking_transform += view.backtracking_move[i].gravity;
                }

                // Collect entities from grid chunks.
                MeshAxisMaxAbsoluteValues entity_chunk_bounds =
                    all_mesh_max_absolute_values[backtracking_entity_mesh_handle
                                                     .id];
                Vec<Vector<f32, 3>> entity_box_corner_bound_points =
                    compute_box_corner_bound_points(
                        entity_chunk_bounds,
                        backtracking_transform_component->position,
                        backtracking_transform_component->scale
                    );

                // Destination array for collected entities.
                Vec<u32> collected_entities;
                // Only the left-bottom-back corner point and the
                // right-upper-front corner point are needed to obtain all box
                // bounds.
                const Vector<f32, 3> min_entity_position =
                    entity_box_corner_bound_points[0];
                const Vector<f32, 3> max_entity_position =
                    entity_box_corner_bound_points[1];

                i32 index_min_x = as<i32>(
                    (min_entity_position[0] + chunk_half_width) / chunk_size
                );
                i32 index_min_y = as<i32>(
                    (min_entity_position[1] + chunk_half_height) / chunk_size
                );
                i32 index_min_z = as<i32>(
                    (min_entity_position[2] + chunk_half_depth) / chunk_size
                );

                i32 index_max_x = as<i32>(
                    (max_entity_position[0] + chunk_half_width) / chunk_size
                );
                i32 index_max_y = as<i32>(
                    (max_entity_position[1] + chunk_half_height) / chunk_size
                );
                i32 index_max_z = as<i32>(
                    (max_entity_position[2] + chunk_half_depth) / chunk_size
                );

                // Entity can legitimately leave the fixed-size world grid -
                // clamp to nearest edge cell instead of crashing.
                index_min_x =
                    std::clamp(index_min_x, 0, as<i32>(spatial_grid.width) - 1);
                index_min_y =
                    std::clamp(index_min_y, 0, as<i32>(spatial_grid.height) - 1);
                index_min_z =
                    std::clamp(index_min_z, 0, as<i32>(spatial_grid.depth) - 1);
                index_max_x =
                    std::clamp(index_max_x, 0, as<i32>(spatial_grid.width) - 1);
                index_max_y =
                    std::clamp(index_max_y, 0, as<i32>(spatial_grid.height) - 1);
                index_max_z =
                    std::clamp(index_max_z, 0, as<i32>(spatial_grid.depth) - 1);

                for (auto i2 = index_min_z; i2 <= index_max_z; ++i2) {
                    for (auto i3 = index_min_y; i3 <= index_max_y; ++i3) {
                        for (auto i4 = index_min_x; i4 <= index_max_x; ++i4) {
                            const Vec<u32>& chunk_entities =
                                spatial_grid.grid[i2][i3][i4].entities;
                            for (const auto entity : chunk_entities) {
                                if (!is_exist(collected_entities, entity)) {
                                    collected_entities.push_back(entity);
                                }
                            }
                        }
                    }
                }

                // Inner loop over every archetype.
                // Iterate over every entity in the current inner archetype.
                for (auto compared_entity_id : collected_entities) {
                    // Check for the same entity ID and iteration.
                    if (backtracking_entity_id == compared_entity_id) {
                        continue;
                    }

                    EntityLocation compared_entity_location =
                        world.entity_locations[get_id(compared_entity_id)];
                    const auto compared_entity_index =
                        compared_entity_location.index;

                    MeshHandle compared_entity_mesh_handle;
                    if (compared_entity_location.arch == nullptr) {
                        // Entity was removed from the world but a stale
                        // reference survived in the grid, skip it.
                        continue;
                    }
                    if (matches_required_mask(
                            compared_entity_location.arch->mask,
                            required_mask
                        )) {
                        Archetype* arch = compared_entity_location.arch;
                        view.compared_transforms = &(as<Transform*>(
                            arch->components[ComponentsIndices::TransformComponent]
                        ))[compared_entity_index];
                        view.compared_meshes = &(
                            (
                                Mesh*
                            )arch->components[ComponentsIndices::MeshComponent]
                        )[compared_entity_index];
                        compared_entity_mesh_handle =
                            view.compared_meshes->handle;

                        u64 move_required_mask =
                            (1ull << GameComponentsIndices::MoveComponent);
                        if (matches_required_mask(
                                compared_entity_location.arch->mask,
                                move_required_mask
                            )) {
                            view.compared_move = &(as<Move*>(
                                arch->components
                                    [GameComponentsIndices::MoveComponent]
                            ))[compared_entity_index];
                        }
                    }

                    Transform* compared_transform_component =
                        view.compared_transforms;
                    Move* compared_move_component = view.compared_move;

                    Vector<f32, 3> compared_transform =
                        Vector<f32, 3>(0.0f, 0.0f, 0.0f);
                    f32 compared_scale = 0.0f;
                    compared_transform = compared_transform_component->position;
                    compared_scale = compared_transform_component->scale;

                    Vector<f32, 3> gravity_test {};
                    if (compared_move_component != nullptr) {
                        compared_transform +=
                            normalize(compared_move_component->frame_movement)
                            * camera_speed;
                        compared_transform += compared_move_component->gravity;
                        gravity_test = compared_move_component->gravity;
                    }

                    bool box_collider_flag = false;
                    bool upper_actor_check_flag = false;

                    MeshAxisMaxAbsoluteValues
                        backtracking_mesh_axis_max_absolute_values =
                            all_mesh_max_absolute_values
                                [backtracking_entity_mesh_handle.id];
                    MeshAxisMaxAbsoluteValues
                        compared_mesh_axis_max_absolute_values = {};
                    if (compared_entity_mesh_handle.id
                        < all_mesh_max_absolute_values.size()) {
                        compared_mesh_axis_max_absolute_values =
                            all_mesh_max_absolute_values
                                [compared_entity_mesh_handle.id];
                    }

                    box_collider_flag = box_collider(
                        backtracking_transform,
                        compared_transform,
                        backtracking_scale,
                        compared_scale,
                        backtracking_mesh_axis_max_absolute_values,
                        compared_mesh_axis_max_absolute_values
                    );

                    if (box_collider_flag) {
                        upper_actor_check_flag = upper_actor_check(
                            backtracking_transform,
                            compared_transform,
                            backtracking_scale,
                            compared_scale,
                            backtracking_entity_mesh_handle,
                            compared_entity_mesh_handle
                        );
                    }

                    if (upper_actor_check_flag && box_collider_flag) {
                        u8 ground_collision_turn_on_mask =
                            (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                        view.backtracking_collider_flags[i].flags =
                            view.backtracking_collider_flags[i].flags
                            | ground_collision_turn_on_mask;
                        view.backtracking_colliders[i].colliders.push_back(
                            compared_entity_id
                        );

                        continue;
                    }

                    if (box_collider_flag) {
                        u8 wall_collision_turn_on_mask =
                            (1u << 0) | (0u << 1) | (0u << 2) | (0u << 3);
                        view.backtracking_collider_flags[i].flags =
                            view.backtracking_collider_flags[i].flags
                            | wall_collision_turn_on_mask;
                        view.backtracking_colliders[i].colliders.push_back(
                            compared_entity_id
                        );

                        continue;
                    }
                }
            }
        }
    }
    cached_archetypes_number = 0;
}

auto CollisionSystem::upper_actor_check(
    Vector<f32, 3> backtracking_position,
    Vector<f32, 3> compared_position,
    f32 backtracking_scale,
    f32 compared_scale,
    MeshHandle backtracking_mesh_handle,
    MeshHandle compared_mesh_handle
) -> bool {
    MeshAxisMaxAbsoluteValues backtracking_mesh_axis_max_absolute_values =
        all_mesh_max_absolute_values[backtracking_mesh_handle.id];

    MeshAxisMaxAbsoluteValues compared_mesh_axis_max_absolute_values =
        all_mesh_max_absolute_values[compared_mesh_handle.id];

    constexpr auto EPSILON = 0.15f;
    return backtracking_position[1]
        + backtracking_mesh_axis_max_absolute_values.origin_offset_y
            * backtracking_scale
        - backtracking_mesh_axis_max_absolute_values.absolute_y
            * backtracking_scale
        + EPSILON
        > compared_position[1]
        + compared_mesh_axis_max_absolute_values.origin_offset_y
            * compared_scale
        + compared_mesh_axis_max_absolute_values.absolute_y * compared_scale;
}

auto DamageSystem::update() -> void {
    cached_attackable_archetypes_number = 0;
    world.search_cache_archetypes(
        attackable_required_mask,
        arch_view.cached_attackable_archetypes.data(),
        cached_attackable_archetypes_number
    );

    for (u32 x = 0; x < cached_attackable_archetypes_number; ++x) {
        Archetype* arch = arch_view.cached_attackable_archetypes[x];
        components_view.attackable_attacks = as<Attack*>(
            arch->components[GameComponentsIndices::AttackComponent]
        );
        components_view.attackable_health = as<Health*>(
            arch->components[GameComponentsIndices::HealthComponent]
        );
        components_view.attackable_fonts =
            as<Font*>(arch->components[ComponentsIndices::FontComponent]);

        u32 i = 0;
        while (i < arch->entity_count) {
            u64 entity = arch->entities[i];
            if (&components_view.attackable_health[i] != nullptr
                && &components_view.attackable_attacks[i] != nullptr) {
                Health& health_component = components_view.attackable_health[i];
                Attack& attack_component =
                    components_view.attackable_attacks[i];

                health_component.current_health -= attack_component.damage;
                attack_component.damage = 0;
                if (health_component.current_health <= 0) {
                    ArchetypeEntityManager* arch_entity_manager =
                        ArchetypeEntityManager::get_instance();
                    arch_entity_manager->remove_entity(entity);
                    world.remove_entity(entity);
                    // Removal swap-moves the last entity into index i;
                    // reprocess it instead of touching the stale index.
                    continue;
                }

                Font& font_component = components_view.attackable_fonts[i];
                font_component.font_string.clear();
                font_component.font_string.push_back('4');
                font_component.font_string.push_back('0');
                font_component.lifetime = 0;
                font_component.removable = true;
            }
            ++i;
        }
    }

    cached_font_archetypes_number = 0;
    world.search_cache_archetypes(
        font_required_mask,
        arch_view.cached_font_archetypes.data(),
        cached_font_archetypes_number
    );

    for (u32 x = 0; x < cached_font_archetypes_number; ++x) {
        Archetype* arch = arch_view.cached_font_archetypes[x];
        components_view.fonts =
            as<Font*>(arch->components[ComponentsIndices::FontComponent]);

        for (u32 i = 0; i < arch->entity_count; ++i) {
            if (components_view.fonts) {
                Font& font_component = components_view.fonts[i];
                if (font_component.removable) {
                    font_component.lifetime += delta_time;
                }
                if (font_component.lifetime >= 1.5f) {
                }
            }
        }
    }
}

auto PhysicsSystem::update() -> void {
    cached_archetypes_number = 0;
    world.search_cache_archetypes(
        required_mask,
        arch_view.cached_archetypes.data(),
        cached_archetypes_number
    );

    for (u32 x = 0; x < cached_archetypes_number; ++x) {
        Archetype* arch = arch_view.cached_archetypes[x];

        components_view.transforms_view = as<Transform*>(
            arch_view.cached_archetypes[x]
                ->components[ComponentsIndices::TransformComponent]
        );
        components_view.moves_view =
            as<Move*>(arch_view.cached_archetypes[x]
                          ->components[GameComponentsIndices::MoveComponent]);
        components_view.rigid_bodies_view = as<RigidBody*>(
            arch_view.cached_archetypes[x]
                ->components[ComponentsIndices::RigidBodyComponent]
        );
        components_view.collider_flags_view = as<ColliderFlags*>(
            arch_view.cached_archetypes[x]
                ->components[ComponentsIndices::ColliderFlagsComponent]
        );
        components_view.colliders_view = as<Collider*>(
            arch_view.cached_archetypes[x]
                ->components[ComponentsIndices::ColliderComponent]
        );
        components_view.meshes_view =
            as<Mesh*>(arch_view.cached_archetypes[x]
                          ->components[ComponentsIndices::MeshComponent]);

        f32 frame_step = 5.5f * delta_time;
        for (u32 i = 0; i < arch->entity_count; ++i) {
            if (components_view.transforms_view
                && components_view.collider_flags_view
                && components_view.moves_view
                && components_view.rigid_bodies_view) {
                Transform& transform_component =
                    components_view.transforms_view[i];
                Move& move = components_view.moves_view[i];
                ColliderFlags& collider_flags =
                    components_view.collider_flags_view[i];
                u8 is_ground_collision_mask =
                    (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                if (collider_flags.flags & is_ground_collision_mask) {
                    move.gravity = 0;
                    transform_component.gravity_accumulator = 0.0f;
                }
                u8 is_wall_collision_mask =
                    (1u << 0) | (0u << 1) | (0u << 2) | (0u << 3);
                if (collider_flags.flags & is_wall_collision_mask) {
                    // Wall-slide: zero only the frameMovement axis blocked by
                    // a collider, keep the tangential component so the player
                    // slides along the wall instead of sticking to it.
                    Collider* colliders = components_view.colliders_view;
                    Mesh* meshes = components_view.meshes_view;
                    if (colliders && meshes
                        && colliders[i].colliders.size() > 0) {
                        const MeshAxisMaxAbsoluteValues player_bounds =
                            all_mesh_max_absolute_values[meshes[i].handle.id];
                        const Vector<f32, 3> player_position =
                            transform_component.position;
                        for (u32 c = 0; c < colliders[i].colliders.size();
                             ++c) {
                            const auto collided_entity =
                                colliders[i].colliders[c];
                            EntityLocation& collided_location =
                                world.entity_locations[get_id(collided_entity)];
                            Archetype* collided_arch = collided_location.arch;
                            if (collided_arch == nullptr) {
                                // Entity was removed this frame (e.g. by
                                // DamageSystem) after collision detection.
                                continue;
                            }
                            const auto collided_index = collided_location.index;
                            Transform* collided_transform = as<Transform*>(
                                collided_arch->components
                                    [ComponentsIndices::TransformComponent]
                            );
                            Mesh* collided_mesh = as<Mesh*>(
                                collided_arch
                                    ->components[ComponentsIndices::MeshComponent]
                            );
                            if (!collided_transform || !collided_mesh) {
                                continue;
                            }
                            collided_transform += collided_index;
                            collided_mesh += collided_index;
                            const MeshAxisMaxAbsoluteValues collided_bounds =
                                all_mesh_max_absolute_values[collided_mesh
                                                                 ->handle.id];
                            const Vector<f32, 3> collided_position =
                                collided_transform->position;
                            // Ground (player standing above) is handled by
                            // gravity, only resolve wall-like colliders.
                            if (is_above(
                                    player_position,
                                    player_bounds,
                                    transform_component.scale,
                                    collided_position,
                                    collided_bounds,
                                    collided_transform->scale
                                )) {
                                continue;
                            }
                            for (i32 axis = 0; axis < 3; ++axis) {
                                Vector<f32, 3> candidate = player_position;
                                candidate[axis] += move.frame_movement[axis];
                                bool hit = aabb_overlap(
                                    candidate,
                                    player_bounds,
                                    transform_component.scale,
                                    collided_position,
                                    collided_bounds,
                                    collided_transform->scale
                                );
                                if (hit) {
                                    move.frame_movement[axis] = 0.0f;
                                }
                            }
                        }
                    } else {
                        move.frame_movement = 0;
                    }
                    u8 wall_collision_turn_off_mask =
                        (0u << 0) | (1u << 1) | (1u << 2) | (1u << 3);
                    collider_flags.flags &= wall_collision_turn_off_mask;
                }
                transform_component.position += move.frame_movement;
                transform_component.position += move.gravity;
                move.gravity = 0.0f;
                move.frame_movement = 0.0f;
                RigidBody& rigid_body = components_view.rigid_bodies_view[i];
                if (rigid_body.jump_accumulator > 0.0f) {
                    rigid_body.jump_accumulator -= frame_step;
                    Vector<f32, 3> jump =
                        Vector<f32, 3> {0.0f, 5.0f, 0.0f} * frame_step;
                    transform_component.position += jump;
                }
            }
        }
    }
}

[[nodiscard]] auto game_update_data_ubo_ui(
    const u32 current_inventory_row,
    const u32 current_inventory_column,
    Inventory* inventory_component,
    Transform* slot_transform_component,
    Mesh* mesh_component,
    f32 aspect_ratio
) -> SlotData {
    SlotData hud_ubo {};
    Matrix<f32, 4> model(1.0f);
    const auto full_slot_scale = mesh_component->gltf
        ? inventory_component->slot_scale * 2.0f
        : inventory_component->slot_scale;
    const auto x = slot_transform_component->position[0]
        + (current_inventory_column * full_slot_scale);
    const auto y_scale_multiplier = aspect_ratio * full_slot_scale;
    const auto y = slot_transform_component->position[1]
        + (current_inventory_row * y_scale_multiplier);
    const auto inventory_slot_scale = inventory_component->slot_scale;
    model[0][0] = inventory_slot_scale;
    model[1][1] = inventory_slot_scale;
    model[2][2] = inventory_slot_scale;
    model[3][0] = x;
    model[3][1] = y;
    model[3][2] = 0.1f;

    hud_ubo.model = model;

    bool highlighted_slot = false;
    for (u32 i = 0; i < inventory_component->highlighted_slots.size(); ++i) {
        if (inventory_component->highlighted_slots[i]
            == (current_inventory_row * inventory_component->col)
                + current_inventory_column) {
            highlighted_slot = true;
            break;
        }
    }

    if (inventory_component->highlighted_slots.size() > 0) {
        if (highlighted_slot) {
            if (inventory_component->is_available_highlighted_slots) {
                hud_ubo.color = {0.0f, 0.3f, 0.0f};
            } else {
                hud_ubo.color = {0.3f, 0.0f, 0.0f};
            }
        }
    } else {
        hud_ubo.color = {0.0f, 0.0f, 0.0f};
    }

    return hud_ubo;
}

auto game_update_data_ubo_icons_ui(
    Transform* item_transform_component,
    Collider* /*item_collider_component*/,
    Item* item_component,
    const u32 row_inventory,
    const u32 column_inventory,
    Transform* inventory_transform_component,
    Mesh* item_mesh,
    i32 item_entity,
    i32 dragged_item_entity,
    f32 aspect_ratio
) -> Matrix<f32, 4> {
    f32 x_result_offset = 0.0f;
    f32 y_result_offset = 0.0f;
    if (item_component->occupied_slots.size() == 0) {
    } else {
        const auto inventory_slot_entity_0 = item_component->occupied_slots[0];
        const auto inventory_slot_entity_3 =
            item_component->occupied_slots.back();
        const auto row_index_first_slot =
            inventory_slot_entity_0 / row_inventory;
        const auto col_index_first_slot =
            inventory_slot_entity_0 % column_inventory;
        const auto row_index_second_slot =
            inventory_slot_entity_3 / row_inventory;
        const auto col_index_second_slot =
            inventory_slot_entity_3 % column_inventory;

        const auto item_scale = item_transform_component->scale;
        const auto full_slot_scale =
            item_mesh->gltf ? item_scale * 2.0f : item_scale;
        // Either division by 2.0f using multiply on 0.5f.
        constexpr auto CENTRE_MULTIPLIER = 0.5f;
        x_result_offset = inventory_transform_component->position[0]
            + (((col_index_first_slot * full_slot_scale)
                + (col_index_second_slot * full_slot_scale))
               * CENTRE_MULTIPLIER);
        y_result_offset = inventory_transform_component->position[1]
            + (((row_index_first_slot * full_slot_scale)
                + (row_index_second_slot * full_slot_scale))
               * CENTRE_MULTIPLIER * aspect_ratio);
    }
    f32 item_scale = item_transform_component->scale;

    if (dragged_item_entity != item_entity) {
        item_transform_component->position =
            Vector<f32, 3>(x_result_offset, y_result_offset, 0.1f);
    } else {
        item_scale *= 1.1f;
        item_transform_component->position[2] = 0.0f;
    }
    Matrix<f32, 4> model(1.0f);
    model[0][0] = item_scale * item_component->item_slot_type.width;
    model[1][1] = item_scale * item_component->item_slot_type.height;
    model[2][2] = 0.0f;
    model[3][0] = item_transform_component->position[0];
    model[3][1] = item_transform_component->position[1];
    model[3][2] = item_transform_component->position[2];

    return model;
}

auto game_update_data_hud_screen_ubo(
    Transform* cursor_transform,
    f32 hud_screen_x,
    f32 hud_screen_y,
    bool is_ui_opened,
    bool is_cursor_released
) -> Matrix<f32, 4> {
    Matrix<f32, 4> model;
    Vector<f32, 3> default_position = Vector<f32, 3>(0.0f, 0.0f, 0.0f);

    auto hud_x = hud_screen_x;
#ifndef VK_USE_PLATFORM_WAYLAND_KHR
    hud_x = -hud_x;
#endif
    cursor_transform->position[0] = hud_x;
    cursor_transform->position[1] = -hud_screen_y;

    if (!is_ui_opened && !is_cursor_released) {
        model[3][0] = default_position[0];
        model[3][1] = default_position[1];
        model[3][2] = default_position[2];
        model[0][0] = cursor_transform->scale;
        model[1][1] = cursor_transform->scale;
        model[2][2] = cursor_transform->scale;
        model[3][3] = 1.0f;
    } else {
        default_position[0] = hud_x;
        default_position[1] = -hud_screen_y;

        model[3][0] = default_position[0];
        model[3][1] = default_position[1];
        model[3][2] = default_position[2];
        model[0][0] = cursor_transform->scale;
        model[1][1] = cursor_transform->scale;
        model[2][2] = cursor_transform->scale;
        model[3][3] = 1.0f;
    }
    return model;
}

auto game_fill_frame_data(u32 actor_counter, i32 dragged_item_entity) -> u32 {
    Engine* engine = Engine::get_instance();
    Renderer* renderer = engine->renderer();

    renderer->health_bars.clear();
    Array<Archetype*, 32> cached_health_bars_archetypes;
    u32 health_bars_archetypes_number = 0;
    u64 health_bars_required_mask =
        (1ull << GameComponentsIndices::HealthComponent)
        | (1ull << ComponentsIndices::MeshComponent)
        | (1ull << ComponentsIndices::TransformComponent);
    world.search_cache_archetypes(
        health_bars_required_mask,
        cached_health_bars_archetypes.data(),
        health_bars_archetypes_number
    );
    u32 health_bar_counter = 0;
    for (u32 x = 0; x < health_bars_archetypes_number; ++x) {
        Archetype* arch = cached_health_bars_archetypes[x];
        auto* health_bar_transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        auto* health_bar_meshes =
            as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);
        auto* health_bars = as<Health*>(
            arch->components[GameComponentsIndices::HealthComponent]
        );

        // The bar reuses mesh geometry: camera entities (e.g. the player
        // avatar) use their own mesh, everything else uses mesh 0. This
        // keeps bars small regardless of how big an entity mesh is.
        u32 ui_vertex_id = 0;
        constexpr u64 view_mask = (1ull << ComponentsIndices::ViewComponent);
        if ((arch->mask & view_mask) == view_mask) {
            ui_vertex_id = health_bar_meshes[0].handle.id;
        }

        for (u32 i = 0; i < arch->entity_count; ++i) {
            Health* health_component = &health_bars[i];
            if (!health_component->renderable) {
                continue;
            }
            renderer->health_bars.push_back({});
            Transform* transform_component = &health_bar_transforms[i];
            renderer->health_bars[health_bar_counter].mesh_id = ui_vertex_id;
            renderer->health_bars[health_bar_counter].position =
                transform_component->position;
            renderer->health_bars[health_bar_counter].max_health =
                health_component->max_health;
            renderer->health_bars[health_bar_counter].current_health =
                health_component->current_health;
            ++health_bar_counter;
        }
    }

    if (renderer->is_inventory_opened) {
        renderer->inventories.clear();
        u32 inventory_counter = 0;
        Array<Archetype*, 32> cached_inventory_archetypes;
        u32 inventory_archetypes_number = 0;
        u64 inventory_required_mask =
            (1ull << ComponentsIndices::TransformComponent)
            | (1ull << GameComponentsIndices::InventoryComponent)
            | (1ull << ComponentsIndices::MeshComponent);
        world.search_cache_archetypes(
            inventory_required_mask,
            cached_inventory_archetypes.data(),
            inventory_archetypes_number
        );

        for (u32 x = 0; x < inventory_archetypes_number; ++x) {
            Archetype* arch = cached_inventory_archetypes[x];
            auto* inventory_transforms = as<Transform*>(
                arch->components[ComponentsIndices::TransformComponent]
            );
            auto* inventory_data = as<Inventory*>(
                arch->components[GameComponentsIndices::InventoryComponent]
            );
            auto* inventory_materials = as<Material*>(
                arch->components[ComponentsIndices::MaterialComponent]
            );
            Mesh* inventory_meshes =
                as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);

            if ((inventory_transforms != nullptr)
                && (inventory_materials != nullptr)
                && (inventory_data != nullptr)
                && (inventory_meshes != nullptr)) {
                for (u32 i = 0; i < arch->entity_count; ++i) {
                    renderer->inventories.push_back({});
                    Inventory* inventory_component = &inventory_data[i];
                    u32 inventory_texture_id =
                        inventory_materials[i].diffuse_texture_id.id;
                    u32 mesh_id = inventory_component->slot_mesh_id.id;
                    renderer->inventories[inventory_counter]
                        .inventory_texture_id = inventory_texture_id;
                    renderer->inventories[inventory_counter].mesh_id = mesh_id;
                    renderer->inventories[inventory_counter].row =
                        inventory_component->row;
                    renderer->inventories[inventory_counter].col =
                        inventory_component->col;
                    renderer->inventories[inventory_counter].slot_data.clear();
                    for (u32 j = 0; j < inventory_component->row; ++j) {
                        for (u32 m = 0; m < inventory_component->col; ++m) {
                            Transform* slot_transform_component =
                                &inventory_transforms[i];
                            renderer->inventories[inventory_counter]
                                .slot_data.push_back({});
                            renderer->inventories[inventory_counter]
                                .slot_data[(j * inventory_component->col) + m] =
                                game_update_data_ubo_ui(
                                    j,
                                    m,
                                    inventory_component,
                                    slot_transform_component,
                                    &inventory_meshes[i],
                                    renderer->aspect_ratio
                                );
                        }
                    }
                    ++inventory_counter;
                }

                for (u32 i = 0; i < arch->entity_count; ++i) {
                    Inventory* inventory_component = &inventory_data[i];
                    Transform* inventory_transform_component =
                        &inventory_transforms[i];

                    renderer->items.clear();
                    u32 item_counter = 0;
                    Array<Archetype*, 32> cached_item_archetypes;
                    u32 item_archetypes_number = 0;
                    u64 item_required_mask =
                        (1ull << ComponentsIndices::TransformComponent)
                        | (1ull << GameComponentsIndices::ItemComponent)
                        | (1ull << ComponentsIndices::MeshComponent)
                        | (1ull << ComponentsIndices::MaterialComponent)
                        | (1ull << ComponentsIndices::ColliderComponent)
                        | (1ull << ComponentsIndices::ColliderFlagsComponent);
                    world.search_cache_archetypes(
                        item_required_mask,
                        cached_item_archetypes.data(),
                        item_archetypes_number
                    );

                    for (u32 c = 0; c < item_archetypes_number; ++c) {
                        Archetype* arch = cached_item_archetypes[c];
                        auto* item_transforms = as<Transform*>(
                            arch->components[ComponentsIndices::TransformComponent]
                        );
                        Item* items = as<Item*>(
                            arch->components[GameComponentsIndices::ItemComponent]
                        );
                        auto* item_materials = as<Material*>(
                            arch->components[ComponentsIndices::MaterialComponent]
                        );
                        Mesh* item_meshes = as<Mesh*>(
                            arch->components[ComponentsIndices::MeshComponent]
                        );
                        auto* item_colliders = as<Collider*>(
                            arch->components[ComponentsIndices::ColliderComponent]
                        );

                        if ((item_transforms != nullptr)
                            && (item_materials != nullptr)
                            && (item_meshes != nullptr)
                            && (item_colliders != nullptr)
                            && (items != nullptr)) {
                            for (u32 a = 0; a < arch->entity_count; ++a) {
                                Item* item_component = &items[a];
                                if (!item_component->is_actor) {
                                    renderer->items.push_back({});
                                    u32 mesh_id = item_meshes[a].handle.id;
                                    u32 diffuse_texture_id =
                                        item_materials[a].diffuse_texture_id.id;
                                    renderer->items[item_counter].mesh_id =
                                        mesh_id;
                                    renderer->items[item_counter]
                                        .diffuse_texture_id =
                                        diffuse_texture_id;
                                    Transform* item_transform_component =
                                        &item_transforms[a];
                                    Collider* item_collider_component =
                                        &item_colliders[a];

                                    u32 item_entity = arch->entities[a];
                                    renderer->items[item_counter].model =
                                        game_update_data_ubo_icons_ui(
                                            item_transform_component,
                                            item_collider_component,
                                            item_component,
                                            inventory_component->row,
                                            inventory_component->col,
                                            inventory_transform_component,
                                            &item_meshes[a],
                                            item_entity,
                                            dragged_item_entity,
                                            renderer->aspect_ratio
                                        );
                                    ++item_counter;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    renderer->crosshairs.clear();
    Array<Archetype*, 32> cached_crosshair_actors_archetypes;
    u32 crosshair_actors_archetypes_number = 0;
    u64 crosshair_required_mask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << GameComponentsIndices::CrosshairTagComponent)
        | (1ull << ComponentsIndices::MeshComponent);
    world.search_cache_archetypes(
        crosshair_required_mask,
        cached_crosshair_actors_archetypes.data(),
        crosshair_actors_archetypes_number
    );

    for (u32 x = 0; x < crosshair_actors_archetypes_number; ++x) {
        Archetype* arch = cached_crosshair_actors_archetypes[x];
        auto* crosshair_transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        Mesh* crosshair_meshes =
            as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);

        for (u32 i = 0; i < arch->entity_count; ++i) {
            renderer->crosshairs.push_back({});
            Transform* cursor_transform = &crosshair_transforms[i];
            u32 mesh_id = crosshair_meshes[i].handle.id;
            renderer->crosshairs[i].mesh_id = mesh_id;
            renderer->crosshairs[i].model = game_update_data_hud_screen_ubo(
                cursor_transform,
                engine->get_hud_screen_x(),
                engine->get_hud_screen_y(),
                renderer->is_inventory_opened,
                renderer->is_cursor_released
            );
        }
    }

    u32 game_actors_counter = actor_counter;

    Array<Archetype*, 32> cached_animation_actors_archetypes;
    u32 animation_actors_archetypes_number = 0;
    u64 animated_actors_required_mask =
        (1ull << ComponentsIndices::MaterialComponent)
        | (1ull << ComponentsIndices::AnimationComponent)
        | (1ull << GameComponentsIndices::RotationComponent)
        | (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::MeshComponent);
    world.search_cache_archetypes(
        animated_actors_required_mask,
        cached_animation_actors_archetypes.data(),
        animation_actors_archetypes_number
    );
    for (u32 x = 0; x < animation_actors_archetypes_number; ++x) {
        Archetype* arch = cached_animation_actors_archetypes[x];
        auto* actor_transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        Mesh* actor_meshes =
            as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);
        auto* actor_materials = as<Material*>(
            arch->components[ComponentsIndices::MaterialComponent]
        );
        auto* actor_rotations = as<Rotation*>(
            arch->components[GameComponentsIndices::RotationComponent]
        );
        auto* actor_animations = as<Animation*>(
            arch->components[ComponentsIndices::AnimationComponent]
        );

        for (u32 n = 0; n < arch->entity_count; ++n) {
            renderer->actors.push_back({});
            Transform* transform_component = &actor_transforms[n];
            Material* material_component = &actor_materials[n];
            Animation* animation_component = &actor_animations[n];
            Rotation* rotation_component = &actor_rotations[n];
            if ((actor_transforms != nullptr) && (actor_materials != nullptr)
                && (actor_animations != nullptr)
                && (actor_rotations != nullptr)) {
                u32 mesh_id = actor_meshes[n].handle.id;
                renderer->actors[game_actors_counter].model_matrix =
                    engine->compute_model_matrix(
                        transform_component,
                        rotation_component->yaw
                    );
                renderer->actors[game_actors_counter].joint_matrices =
                    engine->update_animation_frames(
                        animation_component,
                        mesh_id
                    );
                renderer->actors[game_actors_counter].mesh_id = mesh_id;
                renderer->actors[game_actors_counter].diffuse_texture_index =
                    material_component->diffuse_texture_id.id;
                renderer->actors[game_actors_counter].specular_texture_index =
                    material_component->specular_texture_id.id;
                renderer->actors[game_actors_counter].ambient =
                    material_component->ambient;
                renderer->actors[game_actors_counter].shininess =
                    material_component->shininess;
                ++game_actors_counter;
            }
        }
    }

    Array<Archetype*, 32> cached_level_chunk_actors_archetypes;
    u32 level_chunk_actors_archetypes_number = 0;
    u64 level_chunk_required_mask =
        (1ull << ComponentsIndices::MaterialComponent)
        | (1ull << GameComponentsIndices::LevelChunkTagComponent)
        | (1ull << ComponentsIndices::TransformComponent)
        | (1ull << GameComponentsIndices::RotationComponent)
        | (1ull << ComponentsIndices::MeshComponent);
    world.search_cache_archetypes(
        level_chunk_required_mask,
        cached_level_chunk_actors_archetypes.data(),
        level_chunk_actors_archetypes_number
    );

    for (u32 x = 0; x < level_chunk_actors_archetypes_number; ++x) {
        Archetype* arch = cached_level_chunk_actors_archetypes[x];
        auto* level_chunk_transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        Mesh* level_chunk_meshes =
            as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);
        auto* level_chunk_materials = as<Material*>(
            arch->components[ComponentsIndices::MaterialComponent]
        );
        auto* level_chunk_rotations = as<Rotation*>(
            arch->components[GameComponentsIndices::RotationComponent]
        );
        auto* level_chunks = as<LevelChunkTagComponent*>(
            arch->components[GameComponentsIndices::LevelChunkTagComponent]
        );

        Vec<Matrix<f32, 4>> joint_matrices;
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (u32 i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<f32, 4> unit_matrix(1.0f);
            joint_matrices[i] = unit_matrix;
        }

        for (u32 n = 0; n < arch->entity_count; ++n) {
            renderer->actors.push_back({});
            Transform* transform_component = &level_chunk_transforms[n];
            Material* material_component = &level_chunk_materials[n];
            Rotation* rotation_component = &level_chunk_rotations[n];
            if ((level_chunk_transforms != nullptr)
                && (level_chunk_materials != nullptr)
                && (level_chunks != nullptr)
                && (level_chunk_rotations != nullptr)
                && (level_chunk_meshes != nullptr)) {
                u32 mesh_id = level_chunk_meshes[n].handle.id;
                renderer->actors[game_actors_counter].model_matrix =
                    engine->compute_model_matrix(
                        transform_component,
                        rotation_component->yaw
                    );
                renderer->actors[game_actors_counter].joint_matrices =
                    joint_matrices;
                renderer->actors[game_actors_counter].mesh_id = mesh_id;
                renderer->actors[game_actors_counter].diffuse_texture_index =
                    material_component->diffuse_texture_id.id;
                renderer->actors[game_actors_counter].specular_texture_index =
                    material_component->specular_texture_id.id;
                renderer->actors[game_actors_counter].ambient =
                    material_component->ambient;
                renderer->actors[game_actors_counter].shininess =
                    material_component->shininess;
                ++game_actors_counter;
            }
        }
    }

    Array<Archetype*, 32> cached_static_actors_archetypes;
    u32 static_actors_archetypes_number = 0;
    u64 static_actors_required_mask =
        (1ull << ComponentsIndices::MaterialComponent)
        | (1ull << GameComponentsIndices::StaticMeshTagComponent)
        | (1ull << ComponentsIndices::TransformComponent)
        | (1ull << GameComponentsIndices::RotationComponent)
        | (1ull << ComponentsIndices::MeshComponent);
    world.search_cache_archetypes(
        static_actors_required_mask,
        cached_static_actors_archetypes.data(),
        static_actors_archetypes_number
    );

    for (u32 x = 0; x < static_actors_archetypes_number; ++x) {
        Archetype* arch = cached_static_actors_archetypes[x];
        auto* static_actor_transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        Mesh* static_actor_meshes =
            as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);
        auto* static_actor_materials = as<Material*>(
            arch->components[ComponentsIndices::MaterialComponent]
        );
        auto* static_actor_rotations = as<Rotation*>(
            arch->components[GameComponentsIndices::RotationComponent]
        );

        Vec<Matrix<f32, 4>> joint_matrices;
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (u32 i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<f32, 4> unit_matrix(1.0f);
            joint_matrices[i] = unit_matrix;
        }

        for (u32 n = 0; n < arch->entity_count; ++n) {
            renderer->actors.push_back({});
            Transform* transform_component = &static_actor_transforms[n];
            Material* material_component = &static_actor_materials[n];
            Rotation* rotation_component = &static_actor_rotations[n];
            if ((static_actor_transforms != nullptr)
                && (static_actor_materials != nullptr)
                && (static_actor_rotations != nullptr)
                && (static_actor_meshes != nullptr)) {
                u32 mesh_id = static_actor_meshes[n].handle.id;
                renderer->actors[game_actors_counter].model_matrix =
                    engine->compute_model_matrix(
                        transform_component,
                        rotation_component->yaw
                    );
                renderer->actors[game_actors_counter].joint_matrices =
                    joint_matrices;
                renderer->actors[game_actors_counter].mesh_id = mesh_id;
                renderer->actors[game_actors_counter].diffuse_texture_index =
                    material_component->diffuse_texture_id.id;
                renderer->actors[game_actors_counter].specular_texture_index =
                    material_component->specular_texture_id.id;
                renderer->actors[game_actors_counter].ambient =
                    material_component->ambient;
                renderer->actors[game_actors_counter].shininess =
                    material_component->shininess;
                ++game_actors_counter;
            }
        }
    }

    Array<Archetype*, 32> cached_projectile_actors_archetypes;
    u32 projectile_actors_archetypes_number = 0;
    u64 projectile_required_mask =
        (1ull << GameComponentsIndices::ProjectileBundleComponent)
        | (1ull << ComponentsIndices::TransformComponent)
        | (1ull << GameComponentsIndices::RotationComponent)
        | (1ull << ComponentsIndices::MeshComponent);
    world.search_cache_archetypes(
        projectile_required_mask,
        cached_projectile_actors_archetypes.data(),
        projectile_actors_archetypes_number
    );

    for (u32 x = 0; x < projectile_actors_archetypes_number; ++x) {
        Archetype* arch = cached_projectile_actors_archetypes[x];
        auto* actor_transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        Mesh* actor_meshes =
            as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);
        auto* actor_projectile_bundles = as<ProjectileBundle*>(
            arch->components[GameComponentsIndices::ProjectileBundleComponent]
        );
        auto* actor_rotations = as<Rotation*>(
            arch->components[GameComponentsIndices::RotationComponent]
        );

        Vec<Matrix<f32, 4>> joint_matrices;
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (u32 i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<f32, 4> unit_matrix(1.0f);
            joint_matrices[i] = unit_matrix;
        }

        for (u32 n = 0; n < arch->entity_count; ++n) {
            renderer->actors.push_back({});
            Transform* transform_component = &actor_transforms[n];
            Material* material_component =
                &actor_projectile_bundles[n].material;
            Rotation* rotation_component = &actor_rotations[n];
            if ((actor_transforms != nullptr)
                && (actor_projectile_bundles != nullptr)
                && (actor_rotations != nullptr) && (actor_meshes != nullptr)) {
                u32 mesh_id = actor_meshes[n].handle.id;
                renderer->actors[game_actors_counter].model_matrix =
                    engine->compute_model_matrix(
                        transform_component,
                        rotation_component->yaw
                    );
                renderer->actors[game_actors_counter].joint_matrices =
                    joint_matrices;
                renderer->actors[game_actors_counter].mesh_id = mesh_id;
                renderer->actors[game_actors_counter].diffuse_texture_index =
                    material_component->diffuse_texture_id.id;
                renderer->actors[game_actors_counter].specular_texture_index =
                    material_component->specular_texture_id.id;
                renderer->actors[game_actors_counter].ambient =
                    material_component->ambient;
                renderer->actors[game_actors_counter].shininess =
                    material_component->shininess;
                ++game_actors_counter;
            }
        }
    }

    // Item actors render in the game world.

    Array<Archetype*, 32> cached_item_actors_archetypes;
    u32 item_actors_archetypes_number = 0;
    u64 rotation_item_required_mask =
        (1ull << GameComponentsIndices::ItemComponent)
        | (1ull << ComponentsIndices::MeshComponent)
        | (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::ColliderComponent)
        | (1ull << ComponentsIndices::ColliderFlagsComponent)
        | (1ull << GameComponentsIndices::RotationComponent)
        | (1ull << ComponentsIndices::MaterialComponent);
    world.search_cache_archetypes(
        rotation_item_required_mask,
        cached_item_actors_archetypes.data(),
        item_actors_archetypes_number
    );

    for (u32 x = 0; x < item_actors_archetypes_number; ++x) {
        Archetype* arch = cached_item_actors_archetypes[x];
        auto* item_transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        Mesh* item_meshes =
            as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);
        auto* item_materials = as<Material*>(
            arch->components[ComponentsIndices::MaterialComponent]
        );
        auto* item_rotations = as<Rotation*>(
            arch->components[GameComponentsIndices::RotationComponent]
        );
        Item* items =
            as<Item*>(arch->components[GameComponentsIndices::ItemComponent]);

        Vec<Matrix<f32, 4>> joint_matrices;
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (u32 i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<f32, 4> unit_matrix(1.0f);
            joint_matrices[i] = unit_matrix;
        }

        for (u32 n = 0; n < arch->entity_count; ++n) {
            if (items[n].is_actor) {
                renderer->actors.push_back({});
                Transform* transform_component = &item_transforms[n];
                Material* material_component = &item_materials[n];
                Rotation* rotation_component = &item_rotations[n];
                if ((item_transforms != nullptr) && (item_materials != nullptr)
                    && (item_rotations != nullptr)
                    && (item_meshes != nullptr)) {
                    u32 mesh_id = item_meshes[n].handle.id;
                    renderer->actors[game_actors_counter].model_matrix =
                        engine->compute_model_matrix(
                            transform_component,
                            rotation_component->yaw
                        );
                    renderer->actors[game_actors_counter].joint_matrices =
                        joint_matrices;
                    renderer->actors[game_actors_counter].mesh_id = mesh_id;
                    renderer->actors[game_actors_counter].diffuse_texture_index =
                        material_component->diffuse_texture_id.id;
                    renderer->actors[game_actors_counter]
                        .specular_texture_index =
                        material_component->specular_texture_id.id;
                    renderer->actors[game_actors_counter].ambient =
                        material_component->ambient;
                    renderer->actors[game_actors_counter].shininess =
                        material_component->shininess;
                    ++game_actors_counter;
                }
            }
        }
    }

    return game_actors_counter;
}
