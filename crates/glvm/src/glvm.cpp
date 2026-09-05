#include "glvm/glvm.hpp"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "rusty/prelude.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <mutex>
#include <ostream>
#include <pthread.h>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>
#ifdef _WIN32
#include "imgui_impl_win32.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // NOLINT(readability-identifier-naming)
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI // NOLINT(readability-identifier-naming)
#endif
// clang-format off
#include <windows.h>
#include <mmsystem.h>
// clang-format on
#endif // _WIN32
#ifdef __linux__
#include <wayland-client-core.h>
#endif // __linux__

using namespace rusty::prelude;

#ifdef _WIN32
constexpr auto VK_W = 0x57;
constexpr auto VK_S = 0x53;
constexpr auto VK_A = 0x41;
constexpr auto VK_D = 0x44;
constexpr auto VK_I = 0x49;
#endif

namespace glvm {
auto matches_required_mask(
    const uint64_t archetype_mask,
    const uint64_t& system_mask
) -> bool {
    return (archetype_mask & system_mask) == system_mask;
}

auto make_entity(uint32_t id, uint32_t generation) -> uint64_t {
    return ((uint64_t)generation << ENTITY_ID_BITS) | id;
}

auto get_id(uint64_t entity) -> uint32_t {
    return entity & ENTITY_BITS_MASK;
}

auto get_gen(uint64_t entity) -> uint32_t {
    return entity >> ENTITY_ID_BITS;
}
}; // namespace glvm

namespace glvm {
World WORLD = {};

World::World() {
    assert(
        spatial_grid.width > 0 && spatial_grid.height > 0
        && spatial_grid.depth > 0
    );

    const float chunk_size = spatial_grid.grid[0][0][0].SIZE;
    const float half_world_width = spatial_grid.width * chunk_size * 0.5f;
    const float half_world_height = spatial_grid.height * chunk_size * 0.5f;
    const float half_world_depth = spatial_grid.depth * chunk_size * 0.5f;
    const float half_chunk_size = chunk_size * 0.5f;
    const Vector<float, 3> pivot = Vector<float, 3>(
        -half_world_width + half_chunk_size,
        -half_world_height + half_chunk_size,
        -half_world_depth + half_chunk_size
    );
    for (uint32_t i0 = 0; i0 < spatial_grid.depth; ++i0) {
        for (uint32_t i1 = 0; i1 < spatial_grid.height; ++i1) {
            for (uint32_t i2 = 0; i2 < spatial_grid.width; ++i2) {
                spatial_grid.grid[i0][i1][i2].position = Vector<float, 3>(
                                                             i2 * chunk_size,
                                                             i1 * chunk_size,
                                                             i0 * chunk_size
                                                         )
                    + pivot;
            }
        }
    }
}

World::~World() {
    for (unsigned int i = 0; i < archetypes.size(); ++i) {
        delete archetypes[i];
        archetypes[i] = nullptr;
    }
}

void World::add_entity_to_archetype(uint64_t entity, Archetype* arch) {
    uint32_t id = get_id(entity);

    if (id >= entity_locations.size()) {
        entity_locations.resize(id + 1);
    }

    EntityLocation& location = entity_locations[id];

    if (location.arch != nullptr) {
        assert(false && "unsigned int already assigned to archetype");
    }

    uint32_t index = arch->add_entity(entity);

    location.arch = arch;
    location.index = index;
}

void World::remove_entity(uint64_t entity) {
    uint32_t id = get_id(entity);
    EntityLocation& location = entity_locations[id];
    // Remove entity from spatial grid cells it occupies, otherwise stale
    // references crash collision/physics on later frames.
    if (location.grid_cell_counter > 0) {
        for (uint8_t i = 0; i < location.grid_cell_counter; ++i) {
            uint32_t z = location.grid_cell_indicies[i][0];
            uint32_t y = location.grid_cell_indicies[i][1];
            uint32_t x = location.grid_cell_indicies[i][2];
            std::vector<uint32_t>& chunk_entities =
                spatial_grid.grid[z][y][x].entities;
            for (uint32_t k = 0; k < chunk_entities.size(); ++k) {
                if (chunk_entities[k] == entity) {
                    chunk_entities.erase(chunk_entities.begin() + k);
                    break;
                }
            }
        }
        location.grid_cell_counter = 0;
    }

    Archetype* arch = location.arch;
    uint32_t index = location.index;

    uint64_t moved = arch->remove_entity(index);

    if (moved != entity) {
        uint32_t moved_id = get_id(moved);
        entity_locations[moved_id].index = index;
        entity_locations[moved_id].arch = arch;
    }
    location.arch = nullptr;
}

void World::search_cache_archetypes(
    uint64_t required_mask,
    Archetype* cached_archetypes[],
    uint32_t& cached_archetypes_number
) {
    for (uint32_t i = 0; i < WORLD.archetypes.size(); ++i) {
        Archetype* arch = WORLD.archetypes[i];

        if ((arch->mask & required_mask) == required_mask) {
            cached_archetypes[cached_archetypes_number] = arch;
            ++cached_archetypes_number;
        }
    }
}
}; // namespace glvm

namespace glvm {
ArchetypeEntityManager* ArchetypeEntityManager::p_instance = nullptr;
std::mutex ArchetypeEntityManager::mutex;

ArchetypeEntityManager::ArchetypeEntityManager() {
}

ArchetypeEntityManager::~ArchetypeEntityManager() {
}

ArchetypeEntityManager* ArchetypeEntityManager::get_instance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (p_instance == nullptr) {
        p_instance = new ArchetypeEntityManager();
    }
    return p_instance;
}

[[nodiscard]] uint64_t ArchetypeEntityManager::create_entity() {
    uint32_t new_id = 0;
    // Check out wether or not free ID in removed entities registry.
    if (!free_list.empty()) {
        new_id = free_list.back();
        free_list.pop_back();
    } else {
        new_id = next_id++;
        generations.push_back(1);
    }

    return make_entity(new_id, generations[new_id]);
}

void ArchetypeEntityManager::remove_entity(uint64_t entity) {
    uint32_t id = get_id(entity);

    if (!is_alive(entity)) {
        return;
    }

    generations[id]++;
    free_list.push_back(id);
}

bool ArchetypeEntityManager::is_alive(uint64_t entity) const {
    uint32_t id = get_id(entity);
    return id < generations.size() && generations[id] == get_gen(entity);
}
}; // namespace glvm

namespace glvm {
uint32_t Archetype::add_entity(uint64_t entity) {
    uint32_t index = entity_count++;
    assert(index < CAPACITY);
    entities[index] = entity;

    return index;
}

// Swap-remove.
uint64_t Archetype::remove_entity(uint32_t index) {
    uint32_t last = entity_count - 1;

    for (uint32_t i = 0; i < component_count; ++i) {
        const uint32_t component_id = component_ids[i];

        switch (component_id) {
            case ComponentsIndices::TransformComponent:
                static_cast<Transform*>(components[component_id])[index] =
                    static_cast<Transform*>(components[component_id])[last];
                break;
            case ComponentsIndices::RigidBodyComponent:
                static_cast<RigidBody*>(components[component_id])[index] =
                    static_cast<RigidBody*>(components[component_id])[last];
                break;
            case ComponentsIndices::MeshComponent:
                static_cast<Mesh*>(components[component_id])[index] =
                    static_cast<Mesh*>(components[component_id])[last];
                break;
            case ComponentsIndices::FontComponent:
                static_cast<Font*>(components[component_id])[index] =
                    static_cast<Font*>(components[component_id])[last];
                break;
            case ComponentsIndices::ColliderComponent:
                static_cast<Collider*>(components[component_id])[index] =
                    static_cast<Collider*>(components[component_id])[last];
                break;
            case ComponentsIndices::ColliderFlagsComponent:
                static_cast<ColliderFlags*>(components[component_id])[index] =
                    static_cast<ColliderFlags*>(components[component_id])[last];
                break;
            case ComponentsIndices::MaterialComponent:
                static_cast<Material*>(components[component_id])[index] =
                    static_cast<Material*>(components[component_id])[last];
                break;
            case ComponentsIndices::ViewComponent:
                static_cast<Beholder*>(components[component_id])[index] =
                    static_cast<Beholder*>(components[component_id])[last];
                break;
            case ComponentsIndices::HealthComponent:
                static_cast<Health*>(components[component_id])[index] =
                    static_cast<Health*>(components[component_id])[last];
                break;
            case ComponentsIndices::AnimationComponent:
                static_cast<Animation*>(components[component_id])[index] =
                    static_cast<Animation*>(components[component_id])[last];
                break;
            case ComponentsIndices::StateComponent:
                static_cast<State*>(components[component_id])[index] =
                    static_cast<State*>(components[component_id])[last];
                break;
            case ComponentsIndices::EnemyComponent:
                static_cast<Enemy*>(components[component_id])[index] =
                    static_cast<Enemy*>(components[component_id])[last];
                break;
            case ComponentsIndices::DamageComponent:
                static_cast<Damage*>(components[component_id])[index] =
                    static_cast<Damage*>(components[component_id])[last];
                break;
            case ComponentsIndices::AttackComponent:
                static_cast<Attack*>(components[component_id])[index] =
                    static_cast<Attack*>(components[component_id])[last];
                break;
            case ComponentsIndices::InventoryComponent:
                static_cast<Inventory*>(components[component_id])[index] =
                    static_cast<Inventory*>(components[component_id])[last];
                break;
            case ComponentsIndices::DirectionalLightComponent:
                static_cast<DirectionalLightComponent*>(
                    components[component_id]
                )[index] =
                    static_cast<DirectionalLightComponent*>(
                        components[component_id]
                    )[last];
                break;
            case ComponentsIndices::SpotLightComponent:
                static_cast<SpotLightComponent*>(
                    components[component_id]
                )[index] =
                    static_cast<SpotLightComponent*>(
                        components[component_id]
                    )[last];
                break;
            case ComponentsIndices::PointLightComponent:
                static_cast<PointLightComponent*>(
                    components[component_id]
                )[index] =
                    static_cast<PointLightComponent*>(
                        components[component_id]
                    )[last];
                break;
            case ComponentsIndices::ItemComponent:
                static_cast<Item*>(components[component_id])[index] =
                    static_cast<Item*>(components[component_id])[last];
                break;
            case ComponentsIndices::MoveComponent:
                static_cast<Move*>(components[component_id])[index] =
                    static_cast<Move*>(components[component_id])[last];
                break;
            case ComponentsIndices::ProjectileBundleComponent:
                static_cast<ProjectileBundle*>(components[component_id])[index] =
                    static_cast<ProjectileBundle*>(
                        components[component_id]
                    )[last];
                break;
            case ComponentsIndices::LevelChunkTagComponent:
                static_cast<LevelChunkTagComponent*>(
                    components[component_id]
                )[index] =
                    static_cast<LevelChunkTagComponent*>(
                        components[component_id]
                    )[last];
                break;
            case ComponentsIndices::ProjectileTagComponent:
                static_cast<ProjectileTagComponent*>(
                    components[component_id]
                )[index] =
                    static_cast<ProjectileTagComponent*>(
                        components[component_id]
                    )[last];
                break;
            case ComponentsIndices::PlayerTagComponent:
                static_cast<PlayerTagComponent*>(
                    components[component_id]
                )[index] =
                    static_cast<PlayerTagComponent*>(
                        components[component_id]
                    )[last];
                break;
        }
    }

    uint64_t moved = entities[last];
    entities[index] = moved;
    --entity_count;

    return moved;
}
}; // namespace glvm

namespace glvm {
bool box_collider(
    const Vector<float, 3> backtracking_pos,
    const Vector<float, 3> compared_pos,
    const float backtracking_scale,
    const float compared_scale,
    const MeshAxisMaxAbsoluteValues& backtracking_mesh_axis_max_absolute_values,
    const MeshAxisMaxAbsoluteValues& compared_mesh_axis_max_absolute_values
) {
    return backtracking_pos[0]
            + backtracking_mesh_axis_max_absolute_values.origin_offset_x
                * backtracking_scale
            + (backtracking_mesh_axis_max_absolute_values.absolute_x
               * backtracking_scale)
        > compared_pos[0]
            + compared_mesh_axis_max_absolute_values.origin_offset_x
                * compared_scale
            - (compared_mesh_axis_max_absolute_values.absolute_x
               * compared_scale)
        && backtracking_pos[0]
            + backtracking_mesh_axis_max_absolute_values.origin_offset_x
                * backtracking_scale
            - (backtracking_mesh_axis_max_absolute_values.absolute_x
               * backtracking_scale)
        < compared_pos[0]
            + compared_mesh_axis_max_absolute_values.origin_offset_x
                * compared_scale
            + (compared_mesh_axis_max_absolute_values.absolute_x
               * compared_scale)
        && backtracking_pos[1]
            + backtracking_mesh_axis_max_absolute_values.origin_offset_y
                * backtracking_scale
            + (backtracking_mesh_axis_max_absolute_values.absolute_y
               * backtracking_scale)
        > compared_pos[1]
            + compared_mesh_axis_max_absolute_values.origin_offset_y
                * compared_scale
            - (compared_mesh_axis_max_absolute_values.absolute_y
               * compared_scale)
        && backtracking_pos[1]
            + backtracking_mesh_axis_max_absolute_values.origin_offset_y
                * backtracking_scale
            - (backtracking_mesh_axis_max_absolute_values.absolute_y
               * backtracking_scale)
        < compared_pos[1]
            + compared_mesh_axis_max_absolute_values.origin_offset_y
                * compared_scale
            + (compared_mesh_axis_max_absolute_values.absolute_y
               * compared_scale)
        && backtracking_pos[2]
            + backtracking_mesh_axis_max_absolute_values.origin_offset_z
                * backtracking_scale
            + (backtracking_mesh_axis_max_absolute_values.absolute_z
               * backtracking_scale)
        > compared_pos[2]
            + compared_mesh_axis_max_absolute_values.origin_offset_z
                * compared_scale
            - (compared_mesh_axis_max_absolute_values.absolute_z
               * compared_scale)
        && backtracking_pos[2]
            + backtracking_mesh_axis_max_absolute_values.origin_offset_z
                * backtracking_scale
            - (backtracking_mesh_axis_max_absolute_values.absolute_z
               * backtracking_scale)
        < compared_pos[2]
            + compared_mesh_axis_max_absolute_values.origin_offset_z
                * compared_scale
            + (compared_mesh_axis_max_absolute_values.absolute_z
               * compared_scale);
}

std::vector<Vector<float, 3>> compute_box_corner_bound_points(
    const MeshAxisMaxAbsoluteValues entity_chunk_bounds,
    Vector<float, 3> entity_position,
    const float scale
) {
    const float half_widht = entity_chunk_bounds.absolute_x * scale;
    const float half_height = entity_chunk_bounds.absolute_y * scale;
    const float half_depth = entity_chunk_bounds.absolute_z * scale;
    const Vector<float, 3> center_offset = {
        entity_chunk_bounds.origin_offset_x * scale,
        entity_chunk_bounds.origin_offset_y * scale,
        entity_chunk_bounds.origin_offset_z * scale
    };
    std::vector<Vector<float, 3>> result;
    // Left bottom back.
    result.push_back(
        entity_position + center_offset
        + Vector<float, 3>(-half_widht, -half_height, -half_depth)
    );
    // Right upper front.
    result.push_back(
        entity_position + center_offset
        + Vector<float, 3>(half_widht, half_height, half_depth)
    );
    return result;
}

void set_mesh_bounds(MeshAxisLimitingValues mesh_axis_limiting_values) {
    ALL_MESH_MAX_ABSOLUTE_VALUES.push_back({});

    ALL_MESH_MAX_ABSOLUTE_VALUES[ALL_MESH_MAX_ABSOLUTE_VALUES.size() - 1]
        .absolute_x = (mesh_axis_limiting_values.highest_x
                       - mesh_axis_limiting_values.lowest_x)
        / 2.0f;
    ALL_MESH_MAX_ABSOLUTE_VALUES[ALL_MESH_MAX_ABSOLUTE_VALUES.size() - 1]
        .absolute_y = (mesh_axis_limiting_values.highest_y
                       - mesh_axis_limiting_values.lowest_y)
        / 2.0f;
    ALL_MESH_MAX_ABSOLUTE_VALUES[ALL_MESH_MAX_ABSOLUTE_VALUES.size() - 1]
        .absolute_z = (mesh_axis_limiting_values.highest_z
                       - mesh_axis_limiting_values.lowest_z)
        / 2.0f;

    ALL_MESH_MAX_ABSOLUTE_VALUES[ALL_MESH_MAX_ABSOLUTE_VALUES.size() - 1]
        .origin_offset_x = (mesh_axis_limiting_values.highest_x
                            + mesh_axis_limiting_values.lowest_x)
        / 2.0f;
    ALL_MESH_MAX_ABSOLUTE_VALUES[ALL_MESH_MAX_ABSOLUTE_VALUES.size() - 1]
        .origin_offset_y = (mesh_axis_limiting_values.highest_y
                            + mesh_axis_limiting_values.lowest_y)
        / 2.0f;
    ALL_MESH_MAX_ABSOLUTE_VALUES[ALL_MESH_MAX_ABSOLUTE_VALUES.size() - 1]
        .origin_offset_z = (mesh_axis_limiting_values.highest_z
                            + mesh_axis_limiting_values.lowest_z)
        / 2.0f;
}

void create_projectile(
    const Vector<float, 3>& projectile_position,
    const Vector<float, 3>& projectile_forward,
    const MeshHandle& mesh_handle,
    const Material& material,
    const Damage& damage,
    const EntityLocation& projectile_location
) {
    ProjectileArchetype* projectile_arch =
        static_cast<ProjectileArchetype*>(projectile_location.arch);
    const uint32_t projectile_index = projectile_location.index;

    Mesh* projectile_mesh = &projectile_arch->meshes[projectile_index];
    projectile_mesh->handle = mesh_handle;

    ProjectileBundle* projectile_bundle =
        &projectile_arch->projectile_bundles[projectile_index];
    projectile_bundle->material = material;

    Transform* r_transform_projectile =
        &projectile_arch->transforms[projectile_index];
    Health* projectile_health = &projectile_arch->heath[projectile_index];
    projectile_health->max_health = 100;
    projectile_health->current_health = 100;

    projectile_arch->colliders[projectile_index].colliders.clear();

    r_transform_projectile->scale = 0.1f;
    r_transform_projectile->position = projectile_position;
    r_transform_projectile->forward = projectile_forward;
    r_transform_projectile->position += r_transform_projectile->forward;

    projectile_bundle->damage = damage;
}
}; // namespace glvm

namespace glvm {
ComponentManager* ComponentManager::p_instance = nullptr;
std::mutex ComponentManager::mutex;

ComponentManager::ComponentManager() = default;

ComponentManager::~ComponentManager() {
    for (int j = 0,
             i_size_ordered = world_sparse_entities_map_to_components.size();
         j < i_size_ordered;
         ++j) {
        delete world_sparse_entities_map_to_components[j];
        world_sparse_entities_map_to_components[j] = nullptr;
    }
    for (int j = 0,
             i_size_ordered = world_dense_components_map_to_entities.size();
         j < i_size_ordered;
         ++j) {
        delete world_dense_components_map_to_entities[j];
        world_dense_components_map_to_entities[j] = nullptr;
    }
}

bool ComponentManager::check_availability(
    std::vector<unsigned int>& sparse,
    std::vector<unsigned int>& dense,
    unsigned int entity
) {
    return entity < sparse.size() && sparse[entity] < dense.size()
        && dense[sparse[entity]] == entity;
}

unsigned int ComponentManager::get_container_id() {
    return components_container_id;
}

ComponentManager* ComponentManager::get_instance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (p_instance == nullptr) {
        p_instance = new ComponentManager();
    }
    return p_instance;
}
} // namespace glvm

glvm::CStack INPUT_STACK {};

int X_POINTER;
int Y_POINTER;

#ifdef __linux__
#endif

glvm::CEvent G_E_EVENT;
// Contains all maximum absolute axis values.
std::vector<glvm::MeshAxisMaxAbsoluteValues> ALL_MESH_MAX_ABSOLUTE_VALUES;

namespace glvm {
Engine* Engine::p_instance = nullptr;
std::mutex Engine::mutex;

void playback_sound(
    ISoundEngine* sound_engine,
    std::atomic<bool>& running_sound
) {
    running_sound = true;
    while (running_sound) {
        sound_engine->sound_stream();
    }
}

Engine::Engine() {
    chrono = CTimerCreator().create();
    sound_engine = CSoundEngineFactory().create_sound_engine();

    spatial_grid_system = new SpatialGridSystem();
    collision_system = new CCollisionSystem(INPUT_STACK);
    movement_system = new CMovementSystem(INPUT_STACK);
    physics_system = new CPhysicsSystem(gravity, INPUT_STACK);
    projectile_system = new CProjectileSystem(INPUT_STACK);
    damage_system = new DamageSystem();
    enemy_sytem = new EnemySystem();
    item_system = new ItemSystem();
    procudural_level_generating_system = new ProceduralLevelGeneratingSystem();
    inventory_system = new InventorySystem();

    delta_frame_time = 0.0;
    G_E_EVENT.set_event(EDefault);

    CSystemManager* p_system_manager = CSystemManager::get_instance();

    // Call of ActivateSystem function must be in this order.
    p_system_manager->activate_system(procudural_level_generating_system);
    p_system_manager->activate_system(movement_system);
    p_system_manager->activate_system(enemy_sytem);
    p_system_manager->activate_system(projectile_system);
    p_system_manager->activate_system(spatial_grid_system);
    p_system_manager->activate_system(collision_system);
    p_system_manager->activate_system(damage_system);
    p_system_manager->activate_system(physics_system);
    p_system_manager->activate_system(inventory_system);
    p_system_manager->activate_system(item_system);

    sound_thread = std::thread(
        playback_sound,
        std::ref(sound_engine),
        std::ref(running_sound)
    );
    sound_engine->open_device("default");
}

Engine::~Engine() {
}

Engine* Engine::get_instance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (p_instance == nullptr) {
        p_instance = new Engine();
    }
    return p_instance;
}

void Engine::game_loop() {
    render_vulkan();
}

void Engine::event_queue_flush() {
}

void Engine::render_vulkan() {
    CSystemManager* p_system_manager = CSystemManager::get_instance();
    bool b_game_loop_active = true;

    projectile_system->texture_handlers = texture_handlers;
    projectile_system->mesh_handlers = mesh_handlers;

    enemy_sytem->texture_handlers = texture_handlers;
    enemy_sytem->mesh_handlers = mesh_handlers;

    procudural_level_generating_system->mesh_handlers = mesh_handlers;
    procudural_level_generating_system->texture_handlers = texture_handlers;

    inventory_system->is_item_draged = &dragged_item_entity;
    item_system->dragged_item_entity = &dragged_item_entity;

    vulkan_renderer = new CVulkanRenderer();
    vulkan_renderer->initialize_texture_data = texture_vector;
    vulkan_renderer->paths_array = paths_array;
    vulkan_renderer->paths_gltf = paths_gltf;
    glvm::MeshManager* mesh_manager = glvm::MeshManager::get_instance();
    vulkan_renderer->set_mesh_data(
        mesh_manager->paths_array,
        mesh_manager->paths_gltf
    );

    directional_light_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        directional_light_required_mask,
        cached_directional_ligth_archetypes,
        directional_light_archetypes_number
    );
    vulkan_renderer->directional_light_number =
        directional_light_archetypes_number;

    spot_light_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        spot_light_required_mask,
        cached_spot_ligth_archetypes,
        spot_light_archetypes_number
    );
    vulkan_renderer->spot_light_number = spot_light_archetypes_number;

    point_light_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        point_light_required_mask,
        cached_point_ligth_archetypes,
        point_light_archetypes_number
    );
    vulkan_renderer->point_light_number = point_light_archetypes_number;

    animation_actors_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        animated_actors_required_mask,
        cached_animation_actors_archetypes,
        animation_actors_archetypes_number
    );

    static_actors_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        static_actors_required_mask,
        cached_static_actors_archetypes,
        static_actors_archetypes_number
    );

    load_wavefront_obj();
    initialize_gltf();
    initialize_font_data();
    vulkan_renderer->run();
    vulkan_renderer->window->input_stack = &INPUT_STACK;

#ifdef _WIN32
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#endif

    while (b_game_loop_active) {
        delta_frame_time = chrono->get_elapsed();
        chrono->reset();
        gravity += delta_frame_time;

        vulkan_renderer->window->clear_display();

        vulkan_renderer->window->handle_event(G_E_EVENT);
        if ((INPUT_STACK.search_element(EEvents::EGameLoopKill))
            == EEvents::EGameLoopKill) {
            b_game_loop_active = false;
        }

        if ((INPUT_STACK.search_element(EEvents::ECursorReleased))
            == EEvents::ECursorReleased) {
            vulkan_renderer->is_cursor_released =
                !vulkan_renderer->is_cursor_released;
            INPUT_STACK.remove(EEvents::ECursorReleased);
            INPUT_STACK.remove(EEvents::ECursorReleased);
            // The click-to-relock must not trigger on the button that was
            // already held while Esc was pressed.
            INPUT_STACK.remove(EEvents::EMouseLeftButton);
        }
        if (vulkan_renderer->is_cursor_released
            && (INPUT_STACK.search_element(EEvents::EMouseLeftButton))
                == EEvents::EMouseLeftButton
            && !vulkan_renderer->imgui_overlay->wants_mouse()) {
            vulkan_renderer->is_cursor_released = false;
        }

        if ((INPUT_STACK.search_element(EEvents::EMouseLeftButton))
            == EEvents::EMouseLeftButton) {
            is_left_mouse_button_pressed = true;
        } else {
            is_left_mouse_button_pressed = false;
        }

        bool inventory_key_pressed =
            (INPUT_STACK.search_element(EEvents::EInventory)
             == EEvents::EInventory);
        if (inventory_key_pressed && !is_inventory_key_held) {
            vulkan_renderer->is_inventory_opened =
                !vulkan_renderer->is_inventory_opened;
            if (vulkan_renderer->is_inventory_opened) {
                p_system_manager->deactivate_system(
                    DeactivatedSystems::DeactivatedMovementSystem
                );
                hud_screen_x = 0.0f;
                hud_screen_y = 0.0f;
            } else {
                p_system_manager->return_system_to_activated_state(
                    DeactivatedSystems::DeactivatedMovementSystem
                );
            }
        }
        is_inventory_key_held = inventory_key_pressed;
        G_E_EVENT.set_last_event(INPUT_STACK);

#ifndef VK_USE_PLATFORM_WAYLAND_KHR
        const bool cursor_should_be_hidden =
            !vulkan_renderer->is_inventory_opened
            && vulkan_renderer->window->is_focused
            && !vulkan_renderer->is_cursor_released
            && !vulkan_renderer->imgui_overlay->wants_mouse();
        if (cursor_should_be_hidden && !is_cursor_hidden) {
            ShowCursor(FALSE);
            is_cursor_hidden = true;
        } else if (!cursor_should_be_hidden && is_cursor_hidden) {
            ShowCursor(TRUE);
            is_cursor_hidden = false;
        }
        if (cursor_should_be_hidden) {
            vulkan_renderer->window->cursor_lock(
                G_E_EVENT.mouse_pointer_position.position_x,
                G_E_EVENT.mouse_pointer_position.position_y,
                &G_E_EVENT.mouse_pointer_position.offset_x,
                &G_E_EVENT.mouse_pointer_position.offset_y
            );
        }

        if (was_inventory_opened && !vulkan_renderer->is_inventory_opened) {
            // Cursor was free while the inventory was open; reset the mouse
            // state so the first locked sample doesn't feed a fake delta to the
            // camera. WindowWinVulkan::cursor_lock also discards the >250px
            // teleport on its own.
            G_E_EVENT.mouse_pointer_position.offset_x = 0;
            G_E_EVENT.mouse_pointer_position.offset_y = 0;
            vulkan_renderer->prev_x = 0.0f;
            vulkan_renderer->prev_y = 0.0f;
            vulkan_renderer->current_x = 0.0f;
            vulkan_renderer->current_y = 0.0f;
            movement_system->prev_x = 0.0f;
            previous_mouse_offset_x = 0.0f;
            previous_mouse_offset_y = 0.0f;
        }
        was_inventory_opened = vulkan_renderer->is_inventory_opened;
#else
        if (vulkan_renderer->window->is_focused) {
            vulkan_renderer->window->cursor_lock(
                G_E_EVENT.mouse_pointer_position.position_x,
                G_E_EVENT.mouse_pointer_position.position_y,
                &G_E_EVENT.mouse_pointer_position.offset_x,
                &G_E_EVENT.mouse_pointer_position.offset_y
            );
        }
#endif

        compute_hud_screeen_coordinates();
        damage_system->delta_time = delta_frame_time;
        movement_system->delta_frame_time = delta_frame_time;
        movement_system->gravity = gravity;
        collision_system->f_delta_time = delta_frame_time;
        collision_system->gravity = gravity;
        collision_system->is_inventory_opened =
            vulkan_renderer->is_inventory_opened;
        collision_system->is_left_mouse_button_pressed =
            is_left_mouse_button_pressed;
        collision_system->is_left_mouse_button_released =
            &G_E_EVENT.is_left_mouse_button_released;
        enemy_sytem->delta_frame_time = delta_frame_time;
        enemy_sytem->sound_engine = sound_engine;
        projectile_system->delta_frame_time = delta_frame_time;
        projectile_system->sound_engine = sound_engine;
        projectile_system->is_inventory_opened =
            vulkan_renderer->is_inventory_opened;
        physics_system->f_delta_time = delta_frame_time;
        physics_system->f_acceleration_of_gravity += (delta_frame_time / 20);
        physics_system->gravity = gravity;
        inventory_system->is_inventory_opened =
            vulkan_renderer->is_inventory_opened;
        inventory_system->aspect_rate = vulkan_renderer->aspect_rate;
        inventory_system->is_left_mouse_button_released =
            &G_E_EVENT.is_left_mouse_button_released;
        inventory_system->is_left_mouse_button_pressed =
            is_left_mouse_button_pressed;
        inventory_system->mouse_offset_x = hud_screen_x;
        inventory_system->mouse_offset_y = hud_screen_y;
        item_system->input_stack = &INPUT_STACK;
        item_system->is_inventory_opened = vulkan_renderer->is_inventory_opened;
        item_system->is_left_mouse_button_released =
            &G_E_EVENT.is_left_mouse_button_released;
        item_system->is_left_mouse_button_pressed =
            is_left_mouse_button_pressed;
        item_system->mouse_offset_x = hud_screen_x;
        item_system->mouse_offset_y = hud_screen_y;
        enlarge_frame_accumulator(delta_frame_time);
        p_system_manager->update();
        vulkan_renderer->level_generated_vertices =
            procudural_level_generating_system->level_generated_vertices;
        vulkan_renderer->level_generated_indices =
            procudural_level_generating_system->level_generated_indices;
        procudural_level_generating_system->level_generated_vertices.clear();
        procudural_level_generating_system->level_generated_indices.clear();
        vulkan_renderer->dragged_item_entity = dragged_item_entity;
        vulkan_renderer->hud_screen_x = hud_screen_x;
        vulkan_renderer->hud_screen_y = hud_screen_y;
        vulkan_renderer->initialize_game_level_vertices();
        set_frame_data();
        if (!vulkan_renderer->is_inventory_opened) {
            set_view_matrix();
            set_projection_matrix();
        }
        vulkan_renderer->draw();
        vulkan_renderer->window->swap_buffers();
    }
    delete vulkan_renderer;
}

void Engine::enlarge_frame_accumulator(float value) {
    animation_archetypes_number = 0;
    for (uint32_t m = 0; m < WORLD.archetypes.size(); ++m) {
        Archetype* arch = WORLD.archetypes[m];
        uint64_t required_mask = (1ul << ComponentsIndices::MeshComponent)
            | (1ul << ComponentsIndices::AnimationComponent);

        if (matches_required_mask(arch->mask, required_mask)) {
            cached_animation_archetypes[animation_archetypes_number] = arch;
            ++animation_archetypes_number;
        }
    }

    for (uint32_t n = 0; n < animation_archetypes_number; ++n) {
        Archetype* arch = cached_animation_archetypes[n];
        Animation* animation_view = nullptr;
        Mesh* mesh_view = nullptr;
        if (arch != nullptr) {
            switch (arch->mask) {
                case ENEMY_COMPONENT_MASK:
                    animation_view =
                        static_cast<EnemyArchetype*>(arch)->animations;
                    mesh_view = static_cast<EnemyArchetype*>(arch)->meshes;
                    break;
                case PLAYER_COMPONENT_MASK:
                    animation_view =
                        static_cast<PlayerArchetype*>(arch)->animations;
                    mesh_view = static_cast<PlayerArchetype*>(arch)->meshes;
                    break;
            }

            for (unsigned int i = 0;
                 i < cached_animation_archetypes[n]->entity_count;
                 ++i) {
                if (&mesh_view[i] != nullptr && &animation_view[i] != nullptr) {
                    unsigned int mesh_id = mesh_view[i].handle.id;
                    if (vulkan_renderer->joint_matrices_per_mesh.size() > 0
                        && vulkan_renderer->joint_matrices_per_mesh[mesh_id]
                                .size()
                            > 0) {
                        animation_view[i].frame_accumulator += value;
                    }
                }
            }
        }
    }
}

void Engine::set_view_matrix() {
    player_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        player_required_mask,
        cached_player_archetypes,
        player_archetypes_number
    );

    for (uint32_t n = 0; n < player_archetypes_number; ++n) {
        Archetype* arch = cached_player_archetypes[n];
        Beholder* views =
            (Beholder*)arch->components[ComponentsIndices::ViewComponent];
        Transform* transfroms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];

        for (uint32_t x = 0; x < arch->entity_count; ++x) {
            Beholder* camera_component = &views[x];
            Transform* player_transform = &transfroms[x];

            Matrix<float, 4> view_matrix(1.0f);
            const float k_sensitivity = 0.1f;
            f_yaw = G_E_EVENT.mouse_pointer_position.offset_x;
            f_pitch = G_E_EVENT.mouse_pointer_position.offset_y;
            f_yaw *= k_sensitivity;
            f_pitch *= k_sensitivity;

            G_E_EVENT.mouse_pointer_position.pitch = f_pitch;
            G_E_EVENT.mouse_pointer_position.yaw = f_yaw;

            vulkan_renderer->current_x =
                (float)G_E_EVENT.mouse_pointer_position.offset_x;
            vulkan_renderer->current_y =
                (float)G_E_EVENT.mouse_pointer_position.offset_y;
            float delta_x = 0.0f;
            float delta_y = 0.0f;
            if (!vulkan_renderer->is_inventory_opened) {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
                delta_x = vulkan_renderer->current_x;
                delta_y = vulkan_renderer->current_y;
#else
                delta_x = vulkan_renderer->current_x - vulkan_renderer->prev_x;
                delta_y = vulkan_renderer->current_y - vulkan_renderer->prev_y;
                delta_y *= -1.0f;
#endif
            }

            const Vector<float, 3> right_vec = cross(
                camera_component->forward,
                Vector<float, 3>(0.0f, -1.0f, 0.0)
            );
            const Vector<float, 3> new_up_vec =
                cross(right_vec, camera_component->forward);
            // 1. The mouse direction determines the "intended direction of
            // rotation" for the object.
            // 2. The camera is "looking forward."
            // 3. To make the object "rotate as if the mouse is pushing it," you
            // need to rotate it around an axis that is perpendicular to both
            // the view direction and the mouse movement.
            const Vector<float, 3> rotate_axis = normalize(cross(
                camera_component->forward,
                right_vec * delta_x + new_up_vec * delta_y
            ));

            if (vec_length(rotate_axis) >= 0.001f) {
                // A vector in the screen's tangent plane: it indicates the
                // direction in which the mouse moved, but expressed in world
                // (or 3D) space.
                float rotation_angle =
                    sqrt(delta_y * delta_y + delta_x * delta_x);
                constexpr float ANGLE_SCALE = 0.05f;
                rotation_angle = radians(rotation_angle * ANGLE_SCALE);
                // Quaternions need devision by 2.
                constexpr float QUAT_ANGLE_CORRECTION = 0.5f;
                const float sin_rotation_angle =
                    sinf(rotation_angle * QUAT_ANGLE_CORRECTION);
                Point applied_rotation_point =
                    exp(rotation_angle,
                        Rline {
                            .rx = -rotate_axis.m_vector[0],
                            .ry = -rotate_axis.m_vector[1],
                            .rz = -rotate_axis.m_vector[2]
                        })
                    >> Point {
                        .x = camera_component->forward[0],
                        .y = camera_component->forward[1],
                        .z = camera_component->forward[2],
                        .w = 1.0f
                    };
                vulkan_renderer->forward[0] = applied_rotation_point.x;
                vulkan_renderer->forward[1] = applied_rotation_point.y;
                vulkan_renderer->forward[2] = applied_rotation_point.z;
            }
            camera_component->forward = normalize(vulkan_renderer->forward);
            // Pitch limit by ANGLE, not pixels: independent of screen
            // resolution and mouse sensitivity. Keeps the camera off the
            // vertical pole, where the view basis Cross(forward, up)
            // degenerates and the world starts rolling.
            constexpr float MAX_PITCH_SIN = 0.9999996f; // sin(89.95°).
            if (camera_component->forward[1] > MAX_PITCH_SIN) {
                camera_component->forward[1] = MAX_PITCH_SIN;
            } else if (camera_component->forward[1] < -MAX_PITCH_SIN) {
                camera_component->forward[1] = -MAX_PITCH_SIN;
            }
            camera_component->forward = normalize(camera_component->forward);
            player_transform->forward = camera_component->forward;
            Matrix<float, 4> view = look_at_main(
                camera_component->position + player_transform->position,
                camera_component->position + player_transform->position
                    + camera_component->forward,
                Vector<float, 3>(0.0f, -1.0f, 0.0)
            );
            for (unsigned int i = 0; i < 4; ++i) {
                for (unsigned int j = 0; j < 4; ++j) {
                    view_matrix[i][j] = view[i][j];
                }
            }

            vulkan_renderer->view_matrix = view_matrix;

            vulkan_renderer->prev_y =
                (float)G_E_EVENT.mouse_pointer_position.offset_y;
            vulkan_renderer->prev_x =
                (float)G_E_EVENT.mouse_pointer_position.offset_x;
        }
    }
}

void Engine::set_projection_matrix() {
    Matrix<float, 4> t_projection_matrix =
        perspective(radians(90.0f), vulkan_renderer->aspect_rate, 0.1f, 100.0f);
    vulkan_renderer->projection_matrix = t_projection_matrix;
    vulkan_renderer->projection_matrix[1][1] *= 1.0f;
}

[[nodiscard]] std::vector<Matrix<float, 4>> Engine::update_animation_frames(
    Animation* animation_component,
    unsigned int mesh_id
) {
    if (vulkan_renderer->joint_matrices_per_mesh.size() > 0
        && vulkan_renderer->joint_matrices_per_mesh[mesh_id].size() > 0
        && animation_component->frame_accumulator
            >= vulkan_renderer->frames[mesh_id][animation_component
                                                    ->current_animation_frame]
                * 1.0f) {
        ++animation_component->current_animation_frame;
        if (vulkan_renderer->joint_matrices_per_mesh[mesh_id].size() > 0
            && animation_component->current_animation_frame
                == vulkan_renderer->frames[mesh_id].size()) {
            animation_component->current_animation_frame = 0;
            animation_component->frame_accumulator = 0.0f;
        }
    }

    unsigned int join_matrices_data_size {};
    if (vulkan_renderer->joint_matrices_per_mesh.size() > 0) {
        join_matrices_data_size =
            vulkan_renderer->joint_matrices_per_mesh[mesh_id].size();
    }

    std::vector<Matrix<float, 4>> joint_matrices;
    if (join_matrices_data_size == 0) {
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<float, 4> unit_matrix(1.0f);
            joint_matrices[i] = unit_matrix;
        }

    } else {
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < join_matrices_data_size; ++i) {
            if (mesh_id >= vulkan_renderer->joint_matrices_per_mesh.size()) {
                throw("sdfsdf");
            } else if (
                i >= vulkan_renderer->joint_matrices_per_mesh[mesh_id].size()
            ) {
                throw("sdfsdf");
            } else if (
                animation_component->current_animation_frame
                >= vulkan_renderer->joint_matrices_per_mesh[mesh_id][i].size()
            ) {
                throw("sdfsdf");
            }

            joint_matrices[i] =
                vulkan_renderer->joint_matrices_per_mesh
                    [mesh_id][i][animation_component->current_animation_frame];
        }

        for (uint32_t j = join_matrices_data_size; j < MAX_JOINTS_NUMBER; ++j) {
            Matrix<float, 4> unit_matrix(1.0f);
            joint_matrices[j] = unit_matrix;
        }
    }

    return joint_matrices;
}

Matrix<float, 4> Engine::update_directional_light_space_matrix_shadow_map_ubo(
    DirectionalLightComponent* light
) {
    float near_plane_flat_shadow_map = 5.5f;
    float far_plane_flat_shadow_map = 100.0f;
    Matrix<float, 4> directional_projection_matrix_light = ortho(
        -50.0f,
        50.0f,
        -50.0f,
        50.0f,
        near_plane_flat_shadow_map,
        far_plane_flat_shadow_map
    );

    Vector<float, 3> position_vector_light = light->position;
    Vector<float, 3> direction_vector_light = light->direction;

    Matrix<float, 4> view_matrix_light = look_at_main(
        position_vector_light,
        direction_vector_light,
        {0.0f, -1.0f, 0.0f}
    );
    return view_matrix_light * directional_projection_matrix_light;
}

Matrix<float, 4> Engine::update_spot_light_space_matrix_shadow_map_ubo(
    SpotLightComponent* light
) {
    float near_plane_flat_shadow_map = 0.5f;
    float far_plane_flat_shadow_map = 100.0f;
    Matrix<float, 4> spot_projection_matrix_light = perspective(
        radians(90.0f),
        (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE,
        near_plane_flat_shadow_map,
        far_plane_flat_shadow_map
    );

    Vector<float, 3> position_vector_light = light->position;
    Vector<float, 3> direction_vector_light = light->direction;
    Matrix<float, 4> view_matrix_light = look_at_main(
        position_vector_light,
        direction_vector_light,
        {0.0f, -1.0f, 0.0f}
    );
    return view_matrix_light * spot_projection_matrix_light;
}

Matrix<float, 4> Engine::update_point_light_space_matrix_shadow_map_ubo(
    PointLightComponent* light,
    uint32_t layer
) {
    Vector<float, 3> position_vector_light = light->position;
    Vector<float, 3> directional_vector_light =
        Vector<float, 3>(0.0f, 0.0f, 0.0f);
    Vector<float, 3> up_vector = {0.0, 0.0, 0.0};

    switch (layer) {
        case 0:
            // Positive X.
            directional_vector_light =
                position_vector_light + Vector<float, 3>(1.0f, 0.0f, 0.0f);
            up_vector = Vector<float, 3>(0.0f, -1.0f, 0.0f);
            break;
        case 1:
            // Negative X.
            directional_vector_light =
                position_vector_light + Vector<float, 3>(-1.0f, 0.0f, 0.0f);
            up_vector = Vector<float, 3>(0.0f, -1.0f, 0.0f);
            break;
        case 2:
            // Positive Y.
            directional_vector_light =
                position_vector_light + Vector<float, 3>(0.0f, 1.0f, 0.0f);
            up_vector = Vector<float, 3>(0.0f, 0.0f, 1.0f);
            break;
        case 3:
            // Negative Y.
            directional_vector_light =
                position_vector_light + Vector<float, 3>(0.0f, -1.0f, 0.0f);
            up_vector = Vector<float, 3>(0.0f, 0.0f, -1.0f);
            break;
        case 4:
            // Positive Z.
            directional_vector_light =
                position_vector_light + Vector<float, 3>(0.0f, 0.0f, 1.0f);
            up_vector = Vector<float, 3>(0.0f, -1.0f, 0.0f);
            break;
            // Negative Z.
        case 5:
            directional_vector_light =
                position_vector_light + Vector<float, 3>(0.0f, 0.0f, -1.0f);
            up_vector = Vector<float, 3>(0.0f, -1.0f, 0.0f);
            break;
        default:
            break;
    }

    Matrix<float, 4> projection_matrix_cube_shadow_map = perspective(
        radians(90.0f),
        (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE,
        0.3f,
        100.0f
    );

    Matrix<float, 4> view_matrix_light =
        look_at_main(position_vector_light, directional_vector_light, up_vector);

    return view_matrix_light * projection_matrix_cube_shadow_map;
}

[[nodiscard]] SlotData Engine::update_data_ubo_ui(
    const unsigned int current_inventory_row,
    const unsigned int current_inventory_column,
    Inventory* inventory_component,
    Transform* slot_transfrom_component,
    Mesh* mesh_component
) {
    SlotData hud_ubo {};
    Matrix<float, 4> model(1.0);
    const float full_slot_scale = mesh_component->gltf
        ? inventory_component->slot_scale * 2.0f
        : inventory_component->slot_scale;
    const float x = slot_transfrom_component->position[0]
        + current_inventory_column * full_slot_scale;
    const float y_scale_multilayer =
        vulkan_renderer->aspect_rate * full_slot_scale;
    const float y = slot_transfrom_component->position[1]
        + current_inventory_row * y_scale_multilayer;
    const float inventory_slot_scale = inventory_component->slot_scale;
    model[0][0] = inventory_slot_scale;
    model[1][1] = inventory_slot_scale;
    model[2][2] = inventory_slot_scale;
    model[3][0] = x;
    model[3][1] = y;
    model[3][2] = 0.1f;

    hud_ubo.model = model;

    bool high_lighted_slot = false;
    for (unsigned int i = 0; i < inventory_component->highlighted_slots.size();
         ++i) {
        if (inventory_component->highlighted_slots[i]
            == current_inventory_row * inventory_component->col
                + current_inventory_column) {
            high_lighted_slot = true;
            break;
        } else {
            continue;
        }
    }

    if (inventory_component->highlighted_slots.size() > 0) {
        if (high_lighted_slot) {
            if (inventory_component->is_available_highlighted_slots) {
                hud_ubo.color = {0.0, 0.3, 0.0};
            } else {
                hud_ubo.color = {0.3, 0.0, 0.0};
            }
        }
    } else {
        hud_ubo.color = {0.0, 0.0, 0.0};
    }

    return hud_ubo;
}

Matrix<float, 4> Engine::update_data_ubo_icons_ui(
    Transform* item_transfrom_component,
    Collider* item_collider_component,
    Item* item_component,
    const unsigned int row_inventory,
    const unsigned int column_inventory,
    Transform* inventory_transform_component,
    Mesh* item_mesh,
    int item_entity
) {
    float x_result_offset = 0.0f;
    float y_result_offset = 0.0f;
    if (item_component->occupied_slots.size() == 0) {
    } else {
        const unsigned int inventory_slot_entity_0 =
            item_component->occupied_slots[0];
        const unsigned int inventory_slot_entity_3 =
            item_component->occupied_slots.back();
        const unsigned int row_index_first_slot =
            inventory_slot_entity_0 / row_inventory;
        const unsigned int col_index_first_slot =
            inventory_slot_entity_0 % column_inventory;
        const unsigned int row_index_second_slot =
            inventory_slot_entity_3 / row_inventory;
        const unsigned int col_index_second_slot =
            inventory_slot_entity_3 % column_inventory;

        const float item_scale = item_transfrom_component->scale;
        const float full_slot_scale =
            item_mesh->gltf ? item_scale * 2.0f : item_scale;
        // Eather division by 2.0f using multiply on 0.5f.
        constexpr float CENTRE_MULTIPLAYER = 0.5f;
        x_result_offset = inventory_transform_component->position[0]
            + (col_index_first_slot * full_slot_scale
               + col_index_second_slot * full_slot_scale)
                * CENTRE_MULTIPLAYER;
        y_result_offset = inventory_transform_component->position[1]
            + (row_index_first_slot * full_slot_scale
               + row_index_second_slot * full_slot_scale)
                * CENTRE_MULTIPLAYER * vulkan_renderer->aspect_rate;
    }
    float item_scale = item_transfrom_component->scale;

    if (dragged_item_entity != item_entity) {
        item_transfrom_component->position =
            Vector<float, 3>(x_result_offset, y_result_offset, 0.1f);
    } else {
        item_scale *= 1.1f;
        item_transfrom_component->position[2] = 0.0f;
    }
    Matrix<float, 4> model(1.0);
    model[0][0] = item_scale * item_component->item_slot_type.width;
    model[1][1] = item_scale * item_component->item_slot_type.height;
    model[2][2] = 0.0f;
    model[3][0] = item_transfrom_component->position[0];
    model[3][1] = item_transfrom_component->position[1];
    model[3][2] = item_transfrom_component->position[2];

    return model;
}

Matrix<float, 4> Engine::update_data_hud_screen_ubo(
    Transform* cursor_transform
) {
    Matrix<float, 4> model;
    Vector<float, 3> default_position = Vector<float, 3>(0.0, 0.0, 0.0);

    float hud_screen_x = hud_screen_x;
#ifndef VK_USE_PLATFORM_WAYLAND_KHR
    hud_screen_x = -hud_screen_x;
#endif

    cursor_transform->position[0] = hud_screen_x;
    cursor_transform->position[1] = -hud_screen_y;

    if (!vulkan_renderer->is_inventory_opened
        && !vulkan_renderer->is_cursor_released) {
        model[3][0] = default_position[0];
        model[3][1] = default_position[1];
        model[3][2] = default_position[2];
        model[0][0] = cursor_transform->scale;
        model[1][1] = cursor_transform->scale;
        model[2][2] = cursor_transform->scale;
        model[3][3] = 1.0f;
    } else {
        default_position[0] = hud_screen_x;
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

void Engine::set_frame_data() {
    vulkan_renderer->directional_lights.clear();
    directional_light_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        directional_light_required_mask,
        cached_directional_ligth_archetypes,
        directional_light_archetypes_number
    );

    uint32_t directional_light_counter = 0;
    for (uint32_t x = 0; x < directional_light_archetypes_number; ++x) {
        Archetype* arch = cached_directional_ligth_archetypes[x];
        DirectionalLightComponent* directional_lights =
            (DirectionalLightComponent*)
                arch->components[ComponentsIndices::DirectionalLightComponent];

        for (uint32_t x1 = 0; x1 < arch->entity_count; ++x1) {
            if (directional_lights) {
                vulkan_renderer->directional_lights.push_back({});
                DirectionalLightComponent* light = &directional_lights[x1];
                vulkan_renderer->directional_lights[directional_light_counter]
                    .directional_light_space_matrix =
                    update_directional_light_space_matrix_shadow_map_ubo(light);
                vulkan_renderer->directional_lights[directional_light_counter]
                    .position = Vector<float, 4>(
                    light->position[0],
                    light->position[1],
                    light->position[2],
                    0.0
                );
                vulkan_renderer->directional_lights[directional_light_counter]
                    .direction = Vector<float, 4>(
                    light->direction[0],
                    light->direction[1],
                    light->direction[2],
                    0.0
                );
                vulkan_renderer->directional_lights[directional_light_counter]
                    .ambient = Vector<float, 4>(
                    light->ambient[0],
                    light->ambient[1],
                    light->ambient[2],
                    0.0
                );
                vulkan_renderer->directional_lights[directional_light_counter]
                    .diffuse = Vector<float, 4>(
                    light->diffuse[0],
                    light->diffuse[1],
                    light->diffuse[2],
                    0.0
                );
                vulkan_renderer->directional_lights[directional_light_counter]
                    .specular = Vector<float, 4>(
                    light->specular[0],
                    light->specular[1],
                    light->specular[2],
                    0.0
                );
                ++directional_light_counter;
            }
        }
    }

    vulkan_renderer->spot_lights.clear();
    spot_light_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        spot_light_required_mask,
        cached_spot_ligth_archetypes,
        spot_light_archetypes_number
    );

    uint32_t spot_light_counter = 0;
    for (uint32_t x = 0; x < spot_light_archetypes_number; ++x) {
        Archetype* arch = cached_spot_ligth_archetypes[x];
        SpotLightComponent* spot_lights =
            (SpotLightComponent*)
                arch->components[ComponentsIndices::SpotLightComponent];

        for (uint32_t x1 = 0; x1 < arch->entity_count; ++x1) {
            if (spot_lights) {
                vulkan_renderer->spot_lights.push_back({});
                SpotLightComponent* light = &spot_lights[x1];
                vulkan_renderer->spot_lights[spot_light_counter]
                    .spot_ligth_space_matrix =
                    update_spot_light_space_matrix_shadow_map_ubo(light);
                vulkan_renderer->spot_lights[spot_light_counter].position =
                    light->position;
                vulkan_renderer->spot_lights[spot_light_counter].direction =
                    light->direction;
                vulkan_renderer->spot_lights[spot_light_counter].cut_off =
                    light->cut_off;
                vulkan_renderer->spot_lights[spot_light_counter].outer_cut_off =
                    light->outer_cut_off;
                vulkan_renderer->spot_lights[spot_light_counter].ambient =
                    light->ambient;
                vulkan_renderer->spot_lights[spot_light_counter].diffuse =
                    light->diffuse;
                vulkan_renderer->spot_lights[spot_light_counter].specular =
                    light->specular;
                vulkan_renderer->spot_lights[spot_light_counter].constant =
                    light->constant;
                vulkan_renderer->spot_lights[spot_light_counter].linear =
                    light->linear;
                vulkan_renderer->spot_lights[spot_light_counter].quadratic =
                    light->quadratic;
                ++spot_light_counter;
            }
        }
    }

    vulkan_renderer->point_lights.clear();
    point_light_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        point_light_required_mask,
        cached_point_ligth_archetypes,
        point_light_archetypes_number
    );

    uint32_t point_light_counter = 0;
    for (uint32_t x = 0; x < point_light_archetypes_number; ++x) {
        Archetype* arch = cached_point_ligth_archetypes[x];
        PointLightComponent* point_lights =
            (PointLightComponent*)
                arch->components[ComponentsIndices::PointLightComponent];

        for (uint32_t x1 = 0; x1 < arch->entity_count; ++x1) {
            if (point_lights) {
                vulkan_renderer->point_lights.push_back({});
                PointLightComponent* light = &point_lights[x1];
                uint32_t max_cube_map_layers = 6;
                // 6 is a number of cube map layers.
                for (uint32_t cube_map_layer_counter = 0;
                     cube_map_layer_counter < max_cube_map_layers;
                     ++cube_map_layer_counter) {
                    vulkan_renderer->point_lights[point_light_counter]
                        .point_light_space_matrix[cube_map_layer_counter] =
                        update_point_light_space_matrix_shadow_map_ubo(
                            light,
                            cube_map_layer_counter
                        );
                }
                vulkan_renderer->point_lights[point_light_counter].position =
                    light->position;
                vulkan_renderer->point_lights[point_light_counter].ambient =
                    light->ambient;
                vulkan_renderer->point_lights[point_light_counter].diffuse =
                    light->diffuse;
                vulkan_renderer->point_lights[point_light_counter].specular =
                    light->specular;
                vulkan_renderer->point_lights[point_light_counter].constant =
                    light->constant;
                vulkan_renderer->point_lights[point_light_counter].linear =
                    light->linear;
                vulkan_renderer->point_lights[point_light_counter].quadratic =
                    light->quadratic;
                ++point_light_counter;
            }
        }
    }

    vulkan_renderer->health_bars.clear();
    health_bars_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        health_bars_required_mask,
        cached_health_bars_archetypes,
        health_bars_archetypes_number
    );

    uint32_t health_bar_counter = 0;
    for (uint32_t x = 0; x < health_bars_archetypes_number; ++x) {
        Archetype* arch = cached_health_bars_archetypes[x];
        Transform* health_bar_transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];
        Mesh* health_bar_meshes =
            (Mesh*)arch->components[ComponentsIndices::MeshComponent];
        Health* health_bars =
            (Health*)arch->components[ComponentsIndices::HealthComponent];

        unsigned int ui_vertex_id = 0;
        if (matches_required_mask(arch->mask, PLAYER_COMPONENT_MASK)) {
            ui_vertex_id = health_bar_meshes[0].handle.id;
        }

        for (unsigned int i = 0; i < arch->entity_count; ++i) {
            vulkan_renderer->health_bars.push_back({});
            Transform* transform_component = &health_bar_transforms[i];
            Health* health_component = &health_bars[i];
            vulkan_renderer->health_bars[health_bar_counter].mesh_id =
                ui_vertex_id;
            vulkan_renderer->health_bars[health_bar_counter].position =
                transform_component->position;
            vulkan_renderer->health_bars[health_bar_counter].max_health =
                health_component->max_health;
            vulkan_renderer->health_bars[health_bar_counter].current_health =
                health_component->current_health;
            ++health_bar_counter;
        }
    }

    vulkan_renderer->fonts.clear();
    fonts_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        font_required_mask,
        cached_fonts_archetypes,
        fonts_archetypes_number
    );

    uint32_t font_counter = 0;
    for (uint32_t x = 0; x < fonts_archetypes_number; ++x) {
        Archetype* arch = cached_fonts_archetypes[x];
        Transform* font_transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];
        Font* fonts = (Font*)arch->components[ComponentsIndices::FontComponent];

        for (unsigned int i = 0; i < arch->entity_count; ++i) {
            vulkan_renderer->fonts.push_back({});
            Font* font_component = &fonts[i];
            Transform* transform_component = &font_transforms[i];
            vulkan_renderer->fonts[font_counter].position =
                transform_component->position;
            vulkan_renderer->fonts[font_counter].font_string =
                font_component->font_string;
            vulkan_renderer->fonts[font_counter].life_time =
                font_component->life_time;
            ++font_counter;
        }
    }

    if (vulkan_renderer->is_inventory_opened) {
        vulkan_renderer->inventories.clear();
        uint32_t inventory_counter = 0;
        inventory_archetypes_number = 0;
        WORLD.search_cache_archetypes(
            inventory_required_mask,
            cached_inventory_archetypes,
            inventory_archetypes_number
        );

        for (uint32_t x = 0; x < inventory_archetypes_number; ++x) {
            Archetype* arch = cached_inventory_archetypes[x];
            Transform* inventory_transforms =
                (Transform*)
                    arch->components[ComponentsIndices::TransformComponent];
            Inventory* inventory_data =
                (Inventory*)
                    arch->components[ComponentsIndices::InventoryComponent];
            Material* inventory_materials =
                (Material*)
                    arch->components[ComponentsIndices::MaterialComponent];
            Mesh* inventory_meshes =
                (Mesh*)arch->components[ComponentsIndices::MeshComponent];

            if (inventory_transforms && inventory_materials && inventory_data
                && inventory_meshes) {
                for (unsigned int i = 0; i < arch->entity_count; ++i) {
                    vulkan_renderer->inventories.push_back({});
                    Inventory* inventory_component = &inventory_data[i];
                    unsigned int inventory_texture_id =
                        inventory_materials[i].diffuse_texture_id.id;
                    unsigned int mesh_id = inventory_component->slot_mesh_id.id;
                    vulkan_renderer->inventories[inventory_counter]
                        .inventory_texture_id = inventory_texture_id;
                    vulkan_renderer->inventories[inventory_counter].mesh_id =
                        mesh_id;
                    vulkan_renderer->inventories[inventory_counter].row =
                        inventory_component->row;
                    vulkan_renderer->inventories[inventory_counter].col =
                        inventory_component->col;
                    vulkan_renderer->inventories[inventory_counter]
                        .slot_data.clear();
                    for (unsigned int j = 0; j < inventory_component->row;
                         ++j) {
                        for (unsigned int m = 0; m < inventory_component->col;
                             ++m) {
                            Transform* slot_transform_component =
                                &inventory_transforms[i];
                            vulkan_renderer->inventories[inventory_counter]
                                .slot_data.push_back({});
                            vulkan_renderer->inventories[inventory_counter]
                                .slot_data[j * inventory_component->col + m] =
                                update_data_ubo_ui(
                                    j,
                                    m,
                                    inventory_component,
                                    slot_transform_component,
                                    &inventory_meshes[i]
                                );
                        }
                    }
                    ++inventory_counter;
                }

                for (unsigned int i = 0; i < arch->entity_count; ++i) {
                    Inventory* inventory_component = &inventory_data[i];
                    Transform* inventory_transform_component =
                        &inventory_transforms[i];

                    vulkan_renderer->items.clear();
                    uint32_t item_counter = 0;
                    item_archetypes_number = 0;
                    WORLD.search_cache_archetypes(
                        item_required_mask,
                        cached_item_archetypes,
                        item_archetypes_number
                    );

                    for (uint32_t c = 0; c < item_archetypes_number; ++c) {
                        Archetype* arch = cached_item_archetypes[c];
                        Transform* item_transforms =
                            (Transform*)arch->components
                                [ComponentsIndices::TransformComponent];
                        Item* items =
                            (Item*)arch
                                ->components[ComponentsIndices::ItemComponent];
                        Material* item_materials =
                            (Material*)arch->components
                                [ComponentsIndices::MaterialComponent];
                        Mesh* item_meshes =
                            (Mesh*)arch
                                ->components[ComponentsIndices::MeshComponent];
                        Collider* item_colliders =
                            (Collider*)arch->components
                                [ComponentsIndices::ColliderComponent];

                        if (item_transforms && item_materials && item_meshes
                            && item_colliders && items) {
                            for (unsigned int a = 0; a < arch->entity_count;
                                 ++a) {
                                Item* item_component = &items[a];
                                if (!item_component->is_actor) {
                                    vulkan_renderer->items.push_back({});
                                    unsigned int mesh_id =
                                        item_meshes[a].handle.id;
                                    unsigned int diffuse_texture_id =
                                        item_materials[a].diffuse_texture_id.id;
                                    vulkan_renderer->items[item_counter]
                                        .mesh_id = mesh_id;
                                    vulkan_renderer->items[item_counter]
                                        .diffuse_texture_id =
                                        diffuse_texture_id;
                                    Transform* item_transform_component =
                                        &item_transforms[a];
                                    Collider* item_collider_component =
                                        &item_colliders[a];

                                    uint32_t item_entity = arch->entities[a];
                                    vulkan_renderer->items[item_counter].model =
                                        update_data_ubo_icons_ui(
                                            item_transform_component,
                                            item_collider_component,
                                            item_component,
                                            inventory_component->row,
                                            inventory_component->col,
                                            inventory_transform_component,
                                            &item_meshes[a],
                                            item_entity
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

    vulkan_renderer->crosshairs.clear();
    crosshair_actors_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        crosshair_required_mask,
        cached_crosshair_actors_archetypes,
        crosshair_actors_archetypes_number
    );

    for (uint32_t x = 0; x < crosshair_actors_archetypes_number; ++x) {
        Archetype* arch = cached_crosshair_actors_archetypes[x];
        Transform* crosshair_transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];
        Mesh* crosshair_meshes =
            (Mesh*)arch->components[ComponentsIndices::MeshComponent];

        for (unsigned int i = 0; i < arch->entity_count; ++i) {
            vulkan_renderer->crosshairs.push_back({});
            Transform* cursor_transform = &crosshair_transforms[i];
            unsigned int mesh_id = crosshair_meshes[i].handle.id;
            vulkan_renderer->crosshairs[i].mesh_id = mesh_id;
            vulkan_renderer->crosshairs[i].model =
                update_data_hud_screen_ubo(cursor_transform);
        }
    }

    vulkan_renderer->actors.clear();
    level_chunk_actors_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        level_chunk_required_mask,
        cached_level_chunk_actors_archetypes,
        level_chunk_actors_archetypes_number
    );

    uint32_t level_chunk_actors_counter = 0;
    for (uint32_t x = 0; x < level_chunk_actors_archetypes_number; ++x) {
        Archetype* arch = cached_level_chunk_actors_archetypes[x];
        Transform* level_chunk_transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];
        Mesh* level_chunk_meshes =
            (Mesh*)arch->components[ComponentsIndices::MeshComponent];
        Material* level_chunk_materials =
            (Material*)arch->components[ComponentsIndices::MaterialComponent];
        Rotation* level_chunk_rotations =
            (Rotation*)arch->components[ComponentsIndices::RotationComponent];
        LevelChunkTagComponent* level_chunks =
            (LevelChunkTagComponent*)
                arch->components[ComponentsIndices::LevelChunkTagComponent];

        std::vector<Matrix<float, 4>> joint_matrices;
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<float, 4> unit_matrix(1.0f);
            joint_matrices[i] = unit_matrix;
        }

        for (uint32_t n = 0; n < arch->entity_count; ++n) {
            vulkan_renderer->actors.push_back({});
            Transform* transform_component = &level_chunk_transforms[n];
            Material* material_component = &level_chunk_materials[n];
            Rotation* rotation_component = &level_chunk_rotations[n];
            if (level_chunk_transforms && level_chunk_materials && level_chunks
                && level_chunk_rotations && level_chunk_meshes) {
                unsigned int mesh_id = level_chunk_meshes[n].handle.id;
                vulkan_renderer->actors[level_chunk_actors_counter]
                    .model_matrix = compute_model_matrix(
                    transform_component,
                    rotation_component
                );
                vulkan_renderer->actors[level_chunk_actors_counter]
                    .joint_matrices = joint_matrices;
                vulkan_renderer->actors[level_chunk_actors_counter].mesh_id =
                    mesh_id;
                vulkan_renderer->actors[level_chunk_actors_counter]
                    .diffuse_texture_index =
                    material_component->diffuse_texture_id.id;
                vulkan_renderer->actors[level_chunk_actors_counter]
                    .specular_texture_index =
                    material_component->specular_texture_id.id;
                vulkan_renderer->actors[level_chunk_actors_counter].ambient =
                    material_component->ambient;
                vulkan_renderer->actors[level_chunk_actors_counter].shininess =
                    material_component->shininess;
                ++level_chunk_actors_counter;
            }
        }
    }

    animation_actors_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        animation_required_mask,
        cached_animation_archetypes,
        animation_actors_archetypes_number
    );

    uint32_t animation_actors_counter = level_chunk_actors_counter;
    for (uint32_t x = 0; x < animation_actors_archetypes_number; ++x) {
        Archetype* arch = cached_animation_actors_archetypes[x];
        Transform* actor_transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];
        Mesh* actor_meshes =
            (Mesh*)arch->components[ComponentsIndices::MeshComponent];
        Material* actor_materials =
            (Material*)arch->components[ComponentsIndices::MaterialComponent];
        Rotation* actor_rotations =
            (Rotation*)arch->components[ComponentsIndices::RotationComponent];
        Animation* actor_animations =
            (Animation*)arch->components[ComponentsIndices::AnimationComponent];

        for (uint32_t n = 0; n < arch->entity_count; ++n) {
            vulkan_renderer->actors.push_back({});
            Transform* transform_component = &actor_transforms[n];
            Material* material_component = &actor_materials[n];
            Animation* animation_component = &actor_animations[n];
            Rotation* rotation_component = &actor_rotations[n];
            if (actor_transforms && actor_materials && actor_animations
                && actor_rotations) {
                unsigned int mesh_id = actor_meshes[n].handle.id;
                vulkan_renderer->actors[animation_actors_counter].model_matrix =
                    compute_model_matrix(
                        transform_component,
                        rotation_component
                    );
                vulkan_renderer->actors[animation_actors_counter]
                    .joint_matrices =
                    update_animation_frames(animation_component, mesh_id);
                vulkan_renderer->actors[animation_actors_counter].mesh_id =
                    mesh_id;
                vulkan_renderer->actors[animation_actors_counter]
                    .diffuse_texture_index =
                    material_component->diffuse_texture_id.id;
                vulkan_renderer->actors[animation_actors_counter]
                    .specular_texture_index =
                    material_component->specular_texture_id.id;
                vulkan_renderer->actors[animation_actors_counter].ambient =
                    material_component->ambient;
                vulkan_renderer->actors[animation_actors_counter].shininess =
                    material_component->shininess;
                ++animation_actors_counter;
            }
        }
    }

    static_actors_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        static_actors_required_mask,
        cached_static_actors_archetypes,
        static_actors_archetypes_number
    );

    uint32_t static_actors_counter = animation_actors_counter;
    for (uint32_t x = 0; x < static_actors_archetypes_number; ++x) {
        Archetype* arch = cached_static_actors_archetypes[x];
        Transform* static_actor_transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];
        Mesh* static_actor_meshes =
            (Mesh*)arch->components[ComponentsIndices::MeshComponent];
        Material* static_actor_materials =
            (Material*)arch->components[ComponentsIndices::MaterialComponent];
        Rotation* static_actor_rotations =
            (Rotation*)arch->components[ComponentsIndices::RotationComponent];

        std::vector<Matrix<float, 4>> joint_matrices;
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<float, 4> unit_matrix(1.0f);
            joint_matrices[i] = unit_matrix;
        }

        for (uint32_t n = 0; n < arch->entity_count; ++n) {
            vulkan_renderer->actors.push_back({});
            Transform* transform_component = &static_actor_transforms[n];
            Material* material_component = &static_actor_materials[n];
            Rotation* rotation_component = &static_actor_rotations[n];
            if (static_actor_transforms && static_actor_materials
                && static_actor_rotations && static_actor_meshes) {
                unsigned int mesh_id = static_actor_meshes[n].handle.id;
                vulkan_renderer->actors[static_actors_counter].model_matrix =
                    compute_model_matrix(
                        transform_component,
                        rotation_component
                    );
                vulkan_renderer->actors[static_actors_counter].joint_matrices =
                    joint_matrices;
                vulkan_renderer->actors[static_actors_counter].mesh_id =
                    mesh_id;
                vulkan_renderer->actors[static_actors_counter]
                    .diffuse_texture_index =
                    material_component->diffuse_texture_id.id;
                vulkan_renderer->actors[static_actors_counter]
                    .specular_texture_index =
                    material_component->specular_texture_id.id;
                vulkan_renderer->actors[static_actors_counter].ambient =
                    material_component->ambient;
                vulkan_renderer->actors[static_actors_counter].shininess =
                    material_component->shininess;
                ++static_actors_counter;
            }
        }
    }

    projectile_actors_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        projectile_required_mask,
        cached_projectile_actors_archetypes,
        projectile_actors_archetypes_number
    );

    uint32_t projectile_actors_counter = static_actors_counter;
    for (uint32_t x = 0; x < projectile_actors_archetypes_number; ++x) {
        Archetype* arch = cached_projectile_actors_archetypes[x];
        Transform* actor_transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];
        Mesh* actor_meshes =
            (Mesh*)arch->components[ComponentsIndices::MeshComponent];
        ProjectileBundle* actor_projectile_bundles =
            (ProjectileBundle*)
                arch->components[ComponentsIndices::ProjectileBundleComponent];
        Rotation* actor_rotations =
            (Rotation*)arch->components[ComponentsIndices::RotationComponent];

        std::vector<Matrix<float, 4>> joint_matrices;
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<float, 4> unit_matrix(1.0f);
            joint_matrices[i] = unit_matrix;
        }

        for (uint32_t n = 0; n < arch->entity_count; ++n) {
            vulkan_renderer->actors.push_back({});
            Transform* transform_component = &actor_transforms[n];
            Material* material_component =
                &actor_projectile_bundles[n].material;
            Rotation* rotation_component = &actor_rotations[n];
            if (actor_transforms && actor_projectile_bundles && actor_rotations
                && actor_meshes) {
                unsigned int mesh_id = actor_meshes[n].handle.id;
                vulkan_renderer->actors[projectile_actors_counter].model_matrix =
                    compute_model_matrix(
                        transform_component,
                        rotation_component
                    );
                vulkan_renderer->actors[projectile_actors_counter]
                    .joint_matrices = joint_matrices;
                vulkan_renderer->actors[projectile_actors_counter].mesh_id =
                    mesh_id;
                vulkan_renderer->actors[projectile_actors_counter]
                    .diffuse_texture_index =
                    material_component->diffuse_texture_id.id;
                vulkan_renderer->actors[projectile_actors_counter]
                    .specular_texture_index =
                    material_component->specular_texture_id.id;
                vulkan_renderer->actors[projectile_actors_counter].ambient =
                    material_component->ambient;
                vulkan_renderer->actors[projectile_actors_counter].shininess =
                    material_component->shininess;
                ++projectile_actors_counter;
            }
        }
    }

    // Item actors renders in the game world.

    item_actors_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        rotation_item_required_mask,
        cached_item_actors_archetypes,
        item_actors_archetypes_number
    );

    uint32_t item_actors_counter = projectile_actors_counter;
    for (uint32_t x = 0; x < item_actors_archetypes_number; ++x) {
        Archetype* arch = cached_item_actors_archetypes[x];
        Transform* item_transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];
        Mesh* item_meshes =
            (Mesh*)arch->components[ComponentsIndices::MeshComponent];
        Material* item_materials =
            (Material*)arch->components[ComponentsIndices::MaterialComponent];
        Rotation* item_rotations =
            (Rotation*)arch->components[ComponentsIndices::RotationComponent];
        Item* items = (Item*)arch->components[ComponentsIndices::ItemComponent];

        std::vector<Matrix<float, 4>> joint_matrices;
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            Matrix<float, 4> unit_matrix(1.0f);
            joint_matrices[i] = unit_matrix;
        }

        for (uint32_t n = 0; n < arch->entity_count; ++n) {
            if (items[n].is_actor) {
                vulkan_renderer->actors.push_back({});
                Transform* transform_component = &item_transforms[n];
                Material* material_component = &item_materials[n];
                Rotation* rotation_component = &item_rotations[n];
                if (item_transforms && item_materials && item_rotations
                    && item_meshes) {
                    unsigned int mesh_id = item_meshes[n].handle.id;
                    vulkan_renderer->actors[item_actors_counter].model_matrix =
                        compute_model_matrix(
                            transform_component,
                            rotation_component
                        );
                    vulkan_renderer->actors[item_actors_counter].joint_matrices =
                        joint_matrices;
                    vulkan_renderer->actors[item_actors_counter].mesh_id =
                        mesh_id;
                    vulkan_renderer->actors[item_actors_counter]
                        .diffuse_texture_index =
                        material_component->diffuse_texture_id.id;
                    vulkan_renderer->actors[item_actors_counter]
                        .specular_texture_index =
                        material_component->specular_texture_id.id;
                    vulkan_renderer->actors[item_actors_counter].ambient =
                        material_component->ambient;
                    vulkan_renderer->actors[item_actors_counter].shininess =
                        material_component->shininess;
                    ++item_actors_counter;
                }
            }
        }
    }

    vulkan_renderer->players.clear();
    player_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        player_required_mask,
        cached_player_archetypes,
        player_archetypes_number
    );

    uint32_t player_entity_count = 0;
    for (uint32_t x = 0; x < player_archetypes_number; ++x) {
        Archetype* arch = cached_player_archetypes[x];
        Transform* player_transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];

        for (unsigned int n = 0; n < arch->entity_count; ++n) {
            vulkan_renderer->players.push_back({});
            Transform* player_transform_component = &player_transforms[n];
            if (&player_transforms[n] != nullptr) {
                vulkan_renderer->players[player_entity_count].position =
                    player_transform_component->position;
                vulkan_renderer->players[player_entity_count].forward =
                    player_transform_component->forward;
            }
        }
        ++player_entity_count;
    }
}

void Engine::load_wavefront_obj() {
    for (unsigned int m = 0; m < paths_array.size(); ++m) {
        CWaveFrontObjParser parser;
        CWaveFrontObjParser* wavefront_obj_parser = &parser;

        wavefront_obj_parser->read_file(paths_array[m]);
        wavefront_obj_parser->parse_file();

        vulkan_renderer->a_indices.emplace_back();
        vulkan_renderer->a_vertices.emplace_back();
        vulkan_renderer->highest_gltf_y.emplace_back();
        vulkan_renderer->highest_gltf_y[m] = -999.999f;

        vulkan_renderer->frames.push_back({});
        vulkan_renderer->joint_matrices_per_mesh.push_back({});

        unsigned int vertex_index = 0;
        unsigned int texture_index = 0;
        unsigned int normal_index = 0;
        unsigned int face_vertices_size =
            wavefront_obj_parser->get_faces().size();
        vulkan_renderer->mesh_axis_limiting_values.set_to_default_values();

        for (unsigned int i = 0; i < face_vertices_size; ++i) {
            for (int j = 0; j < 3; ++j) {
                vertex_index = wavefront_obj_parser->get_faces()[i][0][j] - 1;
                vulkan_renderer->a_indices[m].push_back(i * 3 + j);
                SVertex vertex = wavefront_obj_parser
                                     ->get_coordinate_vertices()[vertex_index];
                texture_index = wavefront_obj_parser->get_faces()[i][1][j] - 1;
                SVertex texture =
                    wavefront_obj_parser->get_texture_vertices()[texture_index];
                normal_index = wavefront_obj_parser->get_faces()[i][2][j] - 1;
                SVertex normal =
                    wavefront_obj_parser->get_normals()[normal_index];

                Vector<float, 4> joint_indices;
                Vector<float, 4> weights;

                if (vertex[1] > vulkan_renderer->highest_gltf_y[m]) {
                    vulkan_renderer->highest_gltf_y[m] = vertex[1];
                }

                if (vertex[0]
                    < vulkan_renderer->mesh_axis_limiting_values.lowest_x) {
                    vulkan_renderer->mesh_axis_limiting_values.lowest_x =
                        vertex[0];
                } else if (
                    vertex[0]
                    > vulkan_renderer->mesh_axis_limiting_values.highest_x
                ) {
                    vulkan_renderer->mesh_axis_limiting_values.highest_x =
                        vertex[0];
                }

                if (vertex[1]
                    < vulkan_renderer->mesh_axis_limiting_values.lowest_y) {
                    vulkan_renderer->mesh_axis_limiting_values.lowest_y =
                        vertex[1];
                } else if (
                    vertex[1]
                    > vulkan_renderer->mesh_axis_limiting_values.highest_y
                ) {
                    vulkan_renderer->mesh_axis_limiting_values.highest_y =
                        vertex[1];
                }

                if (vertex[2]
                    < vulkan_renderer->mesh_axis_limiting_values.lowest_z) {
                    vulkan_renderer->mesh_axis_limiting_values.lowest_z =
                        vertex[2];
                } else if (
                    vertex[2]
                    > vulkan_renderer->mesh_axis_limiting_values.highest_z
                ) {
                    vulkan_renderer->mesh_axis_limiting_values.highest_z =
                        vertex[2];
                }

                joint_indices[0] = -1;
                joint_indices[1] = -1;
                joint_indices[2] = -1;
                joint_indices[3] = -1;

                weights[0] = 1.0f;
                weights[1] = 1.0f;
                weights[2] = 1.0f;
                weights[2] = 1.0f;

                vulkan_renderer->a_vertices[m].push_back(
                    {{vertex[0], vertex[1], vertex[2]},
                     {normal[0], normal[1], normal[2]},
                     {texture[0], texture[1]},
                     {joint_indices[0], joint_indices[1], joint_indices[2]},
                     {weights[0], weights[1], weights[2]}}
                );
            }
        }
        set_mesh_bounds(vulkan_renderer->mesh_axis_limiting_values);
        ++wavefront_obj_counter;
    }
}

void Engine::calculate_mesh_bounds(const Vector<float, 4>& animated_vertex) {
    if (animated_vertex[0]
        < vulkan_renderer->mesh_axis_limiting_values.lowest_x) {
        vulkan_renderer->mesh_axis_limiting_values.lowest_x =
            animated_vertex[0];
    } else if (
        animated_vertex[0]
        > vulkan_renderer->mesh_axis_limiting_values.highest_x
    ) {
        vulkan_renderer->mesh_axis_limiting_values.highest_x =
            animated_vertex[0];
    }

    if (animated_vertex[1]
        < vulkan_renderer->mesh_axis_limiting_values.lowest_y) {
        vulkan_renderer->mesh_axis_limiting_values.lowest_y =
            animated_vertex[1];
    } else if (
        animated_vertex[1]
        > vulkan_renderer->mesh_axis_limiting_values.highest_y
    ) {
        vulkan_renderer->mesh_axis_limiting_values.highest_y =
            animated_vertex[1];
    }

    if (animated_vertex[2]
        < vulkan_renderer->mesh_axis_limiting_values.lowest_z) {
        vulkan_renderer->mesh_axis_limiting_values.lowest_z =
            animated_vertex[2];
    } else if (
        animated_vertex[2]
        > vulkan_renderer->mesh_axis_limiting_values.highest_z
    ) {
        vulkan_renderer->mesh_axis_limiting_values.highest_z =
            animated_vertex[2];
    }
}

bool Engine::is_model_cache_exists(const std::string& model_file_path) {
    std::ofstream models_cache(
        "../../../examples/assets/cache/models/cache",
        std::ios::app
    );
    if (!models_cache.is_open()) {
        std::cerr << "Error opening the models cache file" << std::endl;
        throw std::runtime_error("Failed to load mesh cache");
    }
    std::ifstream file("../../../examples/assets/cache/models/cache");
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(model_file_path) != std::string::npos) {
            std::istringstream iss(line);

            std::string keyword;
            float highest_x, lowest_x, highest_y, lowest_y, highest_z, lowest_z;

            iss >> keyword >> highest_x >> lowest_x >> highest_y >> lowest_y
                >> highest_z >> lowest_z;
            vulkan_renderer->mesh_axis_limiting_values.highest_x = highest_x;
            vulkan_renderer->mesh_axis_limiting_values.lowest_x = lowest_x;
            vulkan_renderer->mesh_axis_limiting_values.highest_y = highest_y;
            vulkan_renderer->mesh_axis_limiting_values.lowest_y = lowest_y;
            vulkan_renderer->mesh_axis_limiting_values.highest_z = highest_z;
            vulkan_renderer->mesh_axis_limiting_values.lowest_z = lowest_z;

            is_already_cached = true;

            models_cache.close();
            return true;
        }
    }
    models_cache.close();
    return false;
}

void Engine::write_models_cache(const std::string& model_file_path) {
    std::ofstream models_cache(
        "../../../examples/assets/cache/models/cache",
        std::ios::app
    );
    if (!models_cache.is_open()) {
        std::cerr << "Error opening the models cache file" << std::endl;
        throw std::runtime_error("Failed to load mesh cache");
    }
    std::size_t pos = model_file_path.find(' ');
    std::string first_part = (pos == std::string::npos)
        ? model_file_path
        : model_file_path.substr(0, pos);

    models_cache << model_file_path;
    models_cache << " " << vulkan_renderer->mesh_axis_limiting_values.highest_x
                 << " " << vulkan_renderer->mesh_axis_limiting_values.lowest_x
                 << " " << vulkan_renderer->mesh_axis_limiting_values.highest_y
                 << " " << vulkan_renderer->mesh_axis_limiting_values.lowest_y
                 << " " << vulkan_renderer->mesh_axis_limiting_values.highest_z
                 << " " << vulkan_renderer->mesh_axis_limiting_values.lowest_z
                 << std::endl;

    models_cache.close();
}

void Engine::initialize_gltf() {
    std::vector<bool> animation_flags;
    for (unsigned int m = 0; m < paths_gltf.size(); ++m) {
        CJsonParser json_parser;
        vulkan_renderer->a_vertexes_temp.emplace_back();
        vulkan_renderer->a_indices.emplace_back();
        vulkan_renderer->frames.push_back({});
        vulkan_renderer->joint_matrices_per_mesh.push_back({});
        animation_flags.push_back({});
        vulkan_renderer->highest_gltf_y.emplace_back();
        uint32_t next_index_gltf = wavefront_obj_counter + m;
        bool animation_flag = false;
        json_parser.load_gltf(
            paths_gltf[m],
            vulkan_renderer->a_vertexes_temp[m],
            vulkan_renderer->a_indices[next_index_gltf],
            vulkan_renderer->joint_matrices_per_mesh[next_index_gltf],
            vulkan_renderer->frames[next_index_gltf],
            animation_flag,
            vulkan_renderer->highest_gltf_y[next_index_gltf]
        );
        animation_flags[m] = animation_flag;
    }

    for (unsigned int m = 0; m < paths_gltf.size(); ++m) {
        vulkan_renderer->a_vertices.emplace_back();
        vulkan_renderer->mesh_axis_limiting_values.set_to_default_values();

        is_already_cached = false;
        is_model_cache_exists(paths_gltf[m]);

        int step_offset = 0;
        if (animation_flags[m]) {
            step_offset = 8;
        } else {
            step_offset = 16;
        }

        for (unsigned int n = 0; n < vulkan_renderer->a_vertexes_temp[m].size();
             n += step_offset) {
            SVertex vertex;
            vertex[0] = vulkan_renderer->a_vertexes_temp[m][n];
            vertex[1] = vulkan_renderer->a_vertexes_temp[m][n + 1];
            vertex[2] = vulkan_renderer->a_vertexes_temp[m][n + 2];
            SVertex normal;
            normal[0] = vulkan_renderer->a_vertexes_temp[m][n + 3];
            normal[1] = vulkan_renderer->a_vertexes_temp[m][n + 4];
            normal[2] = vulkan_renderer->a_vertexes_temp[m][n + 5];
            SVertex texture;
            texture[0] = vulkan_renderer->a_vertexes_temp[m][n + 6];
            texture[1] = vulkan_renderer->a_vertexes_temp[m][n + 7];

            Vector<float, 4> join_indices;
            Vector<float, 4> weights;
            if (animation_flags[m]) {
                join_indices[0] = -1;
                join_indices[1] = -1;
                join_indices[2] = -1;
                join_indices[3] = -1;

                weights[0] = 1;
                weights[1] = 1;
                weights[2] = 1;
                weights[3] = 1;

            } else {
                join_indices[0] = vulkan_renderer->a_vertexes_temp[m][n + 8];
                join_indices[1] = vulkan_renderer->a_vertexes_temp[m][n + 9];
                join_indices[2] = vulkan_renderer->a_vertexes_temp[m][n + 10];
                join_indices[3] = vulkan_renderer->a_vertexes_temp[m][n + 11];

                weights[0] = vulkan_renderer->a_vertexes_temp[m][n + 12];
                weights[1] = vulkan_renderer->a_vertexes_temp[m][n + 13];
                weights[2] = vulkan_renderer->a_vertexes_temp[m][n + 14];
                weights[3] = vulkan_renderer->a_vertexes_temp[m][n + 15];
            }

            uint32_t next_index_gltf = wavefront_obj_counter + m;
            vulkan_renderer->a_vertices[next_index_gltf].push_back(
                {{vertex[0], vertex[1], vertex[2]},
                 {normal[0], normal[1], normal[2]},
                 {texture[0], texture[1]},
                 {join_indices[0],
                  join_indices[1],
                  join_indices[2],
                  join_indices[3]},
                 {weights[0], weights[1], weights[2], weights[3]}}
            );

            if (is_already_cached) {
                continue;
            }

            Vector<float, 4> animated_vertex =
                Vector<float, 4>(vertex[0], vertex[1], vertex[2], 1.0);
            if (!animation_flags[m]
                && vulkan_renderer->joint_matrices_per_mesh[next_index_gltf]
                        .size()
                    > 0) {
                for (unsigned int frame = 0;
                     frame < vulkan_renderer
                                 ->joint_matrices_per_mesh[next_index_gltf][0]
                                 .size();
                     ++frame) {
                    Matrix<float, 4> skin_matrix =
                        (vulkan_renderer->joint_matrices_per_mesh
                             [next_index_gltf][int(join_indices[0])][frame]
                         * weights[0])
                        + (vulkan_renderer->joint_matrices_per_mesh
                               [next_index_gltf][int(join_indices[1])][frame]
                           * weights[1])
                        + (vulkan_renderer->joint_matrices_per_mesh
                               [next_index_gltf][int(join_indices[2])][frame]
                           * weights[2])
                        + (vulkan_renderer->joint_matrices_per_mesh
                               [next_index_gltf][int(join_indices[3])][frame]
                           * weights[3]);

                    animated_vertex =
                        Vector<float, 4>(vertex[0], vertex[1], vertex[2], 1.0)
                        * skin_matrix;
                    calculate_mesh_bounds(animated_vertex);
                }
            } else {
                calculate_mesh_bounds(animated_vertex);
            }
        }

        if (!is_already_cached) {
            write_models_cache(paths_gltf[m]);
        }
        set_mesh_bounds(vulkan_renderer->mesh_axis_limiting_values);
    }
}

void Engine::initialize_font_data() {
    constexpr float FONT_STEP = 1.0 / 12;
    constexpr unsigned int GLYPH_ROW = 7;
    constexpr unsigned int GLYPH_COLUMN = 12;

    vulkan_renderer->font_vertex_buffer_container.resize(128);
    vulkan_renderer->font_vertex_buffer_memory_container.resize(128);

    vulkan_renderer->font_index_buffer_container.resize(128);
    vulkan_renderer->font_index_buffer_memory_contaner.resize(128);

    for (unsigned int i = 0; i < GLYPH_ROW; ++i) {
        for (unsigned int j = 0; j < GLYPH_COLUMN; ++j) {
            std::vector<Vertex> symbol_g_vertices;
            symbol_g_vertices.push_back(
                {{-0.5f, 0.5f, 0.0f},
                 {0.0f, 1.0f, 0.0f},
                 {FONT_STEP * j, FONT_STEP * i + FONT_STEP},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.push_back(
                {{0.5f, 0.5f, 0.0f},
                 {1.0f, 1.0f, 0.0f},
                 {FONT_STEP * j + FONT_STEP, FONT_STEP * i + FONT_STEP},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.push_back(
                {{-0.5f, -0.5f, 0.0f},
                 {0.0f, 0.0f, 0.0f},
                 {FONT_STEP * j, FONT_STEP * i},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.push_back(
                {{0.5f, -0.5f, 0.0f},
                 {1.0f, 0.0f, 0.0f},
                 {FONT_STEP * j + FONT_STEP, FONT_STEP * i},
                 {0.0f, 0.0f, 0.0f, 0.0f},
                 {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            unsigned int current_buffer_index = i * GLYPH_COLUMN + j;

            bool exit_flag = false;
            const unsigned int next_buffer_index =
                static_cast<const unsigned int>(
                    vulkan_renderer->glyphs[current_buffer_index]
                );
            // TODO: Fix gabage algorithm.
            for (unsigned int n = 0;
                 n < vulkan_renderer->font_indices_container.size();
                 ++n) {
                if (next_buffer_index
                    == vulkan_renderer->font_indices_container[n]) {
                    exit_flag = true;
                }
            }

            if (exit_flag) {
                continue;
            }

            vulkan_renderer->symbol_g_vertices_container.push_back(
                symbol_g_vertices
            );
            vulkan_renderer->font_indices_container.push_back(next_buffer_index);
        }
    }
}

Matrix<float, 4> Engine::compute_model_matrix(
    Transform* transform,
    Rotation* rotation
) {
    Matrix<float, 4> rotation_matrix(1.0f);
    Matrix<float, 4> scaling_matrix(1.0f);
    Matrix<float, 4> translation_matrix(1.0f);

    scaling_matrix[0][0] = transform->scale;
    scaling_matrix[1][1] = transform->scale;
    scaling_matrix[2][2] = transform->scale;

    translation_matrix[3][0] = transform->position[0];
    translation_matrix[3][1] = transform->position[1];
    translation_matrix[3][2] = transform->position[2];
    translation_matrix[3][3] = 1.0f;

    float sin_pitch = std::sin(radians(-rotation->pitch / 2));
    float cos_pitch = std::cos(radians(-rotation->pitch / 2));
    float sin_yaw = std::sin(radians((rotation->yaw) / 2));
    float cos_yaw = std::cos(radians((rotation->yaw) / 2));

    Quaternion pitch_quat;
    Quaternion yaw_quat;
    pitch_quat.w = cos_pitch;
    pitch_quat.x = sin_pitch;
    pitch_quat.y = 0.0f;
    pitch_quat.z = 0.0f;

    yaw_quat.w = cos_yaw;
    yaw_quat.x = 0.0f;
    yaw_quat.y = sin_yaw;
    yaw_quat.z = 0.0f;
    return scaling_matrix * translation_matrix;
}

void Engine::compute_hud_screeen_coordinates() {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    hud_screen_y -= G_E_EVENT.mouse_pointer_position.offset_y
        / (float)vulkan_renderer->window->height;
    hud_screen_x += G_E_EVENT.mouse_pointer_position.offset_x
        / (float)vulkan_renderer->window->width;
#else
    if (vulkan_renderer->is_inventory_opened
        || vulkan_renderer->is_cursor_released) {
        // Cursor is free while the inventory is open or the cursor is released:
        // track its real position instead of the locked-mouse offsets.
        hud_screen_x = 1.0f
            - G_E_EVENT.mouse_pointer_position.position_x
                / ((float)vulkan_renderer->window->width / 2.0f);
        hud_screen_y =
            -(G_E_EVENT.mouse_pointer_position.position_y
                  / ((float)vulkan_renderer->window->height / 2.0f)
              - 1.0f);
    } else {
        hud_screen_y -= (previous_mouse_offset_y
                         - G_E_EVENT.mouse_pointer_position.offset_y)
            / (float)vulkan_renderer->window->height;
        hud_screen_x += (previous_mouse_offset_x
                         - G_E_EVENT.mouse_pointer_position.offset_x)
            / (float)vulkan_renderer->window->width;
    }
    previous_mouse_offset_x = G_E_EVENT.mouse_pointer_position.offset_x;
    previous_mouse_offset_y = G_E_EVENT.mouse_pointer_position.offset_y;
#endif
    if (hud_screen_x > 1.0f) {
        hud_screen_x = 1.0f;
    } else if (hud_screen_x < -1.0f) {
        hud_screen_x = -1.0f;
    }

    if (hud_screen_y > 1.0f) {
        hud_screen_y = 1.0f;
    } else if (hud_screen_y < -1.0f) {
        hud_screen_y = -1.0f;
    }
}

TextureHandle Engine::load_texture_from_file(const char* path_to_texture) {
    uint32_t texture_id = texture_vector.size();
    TextureHandle texture_handle;
    texture_handle.id = texture_id;
    texture_vector.push_back({.path_to_image = path_to_texture});
    texture_handlers.push_back(texture_handle);

    return texture_handle;
}

auto Engine::load_texture_from_address(
    unsigned int i_width,
    unsigned int i_height,
    unsigned int dat_length,
    unsigned char* u_i_data
) -> TextureHandle {
    uint32_t texture_id = texture_vector.size();
    TextureHandle texture_handle;
    texture_handle.id = texture_id;
    texture_vector.push_back(
        {.i_width = i_width,
         .i_height = i_height,
         .dat_length = dat_length,
         .u_i_data = u_i_data}
    );
    texture_handlers.push_back(texture_handle);

    return texture_handle;
}

MeshHandle Engine::load_mesh_from_obj(const char* mesh_path) {
    MeshHandle mesh_handle;
    mesh_handle.id = mesh_id;
    paths_array.push_back(mesh_path);
    mesh_handlers.push_back(mesh_handle);
    ++mesh_id;

    return mesh_handle;
}

MeshHandle Engine::load_mesh_from_gltf(const char* path_to_mesh) {
    MeshHandle mesh_handle;
    mesh_handle.id = mesh_id;
    paths_gltf.push_back(path_to_mesh);
    mesh_handlers.push_back(mesh_handle);
    ++mesh_id;

    return mesh_handle;
}

MeshHandle Engine::load_mesh() {
    MeshHandle mesh_handle;
    mesh_handle.id = mesh_id;
    mesh_handlers.push_back(mesh_handle);
    ++mesh_id;

    return mesh_handle;
}

void Engine::game_kill() {
    running_sound = false;
    sound_engine->close_device();

    if (sound_thread.joinable()) {
        sound_thread.join();
    }

    delete sound_engine;
    sound_engine = nullptr;

    delete chrono;
    chrono = nullptr;
    delete collision_system;
    collision_system = nullptr;
    delete movement_system;
    movement_system = nullptr;
    delete physics_system;
    physics_system = nullptr;
    delete projectile_system;
    projectile_system = nullptr;
    delete damage_system;
    damage_system = nullptr;
    delete enemy_sytem;
    enemy_sytem = nullptr;
    delete item_system;
    item_system = nullptr;
}
} // namespace glvm

namespace glvm {
EntityManager* EntityManager::instance = nullptr;
std::mutex EntityManager::mutex;

EntityManager::EntityManager() {
}

EntityManager::~EntityManager() {
}

EntityManager* EntityManager::get_instance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new EntityManager();
    }
    return instance;
}

[[nodiscard]] unsigned int EntityManager::create_entity() {
    unsigned int new_id;
    // Check out wether or not free ID in removed entities registry.
    if (removed_entity_registry.size() > K_I_NULL) {
        new_id = removed_entity_registry.front();
        active_entity_registry.push_back(removed_entity_registry.front());
        removed_entity_registry.erase(removed_entity_registry.begin());
    } else {
        active_entity_registry.push_back(id);
        new_id = id;
        ++id;
    }
    is_entities_collection_changed = true;
    return new_id;
}

// Don't need to delete real component in this method. Because systems dont work
// with component without indices for that component in ordered container.
void EntityManager::remove_entity(
    unsigned int& entity_id,
    ComponentManager* component_manager
) {
    component_manager->remove_all_components(entity_id);
    active_entity_registry[entity_id] = K_I_UINT_MAX;
    removed_entity_registry.push_back(entity_id);
    is_entities_collection_changed = true;
}
} // namespace glvm

namespace glvm {
CEvent::CEvent() {
}

EEvents& CEvent::get_event() {
    return e_event;
}

void CEvent::set_event(EEvents new_event) {
    e_event = new_event;
}

void CEvent::set_next_event(EEvents new_event) {
    next_event = new_event;
}

EEvents CEvent::get_next_event() {
    return next_event;
}

void CEvent::set_last_event(CStack stack) {
    switch (stack.pop()) {
        case glvm::EMoveRight:
            set_event(glvm::EEvents::EMoveRight);
            break;
        case glvm::EMoveLeft:
            set_event(glvm::EEvents::EMoveLeft);
            break;
        case glvm::EMoveBackward:
            set_event(glvm::EEvents::EMoveBackward);
            break;
        case glvm::EMoveForward:
            set_event(glvm::EEvents::EMoveForward);
            break;
        case glvm::EMouseLeftButton:
            set_event(glvm::EEvents::EMouseLeftButton);
            break;
        default:
            break;
    }
}
} // namespace glvm

namespace glvm {
std::vector<VkDescriptorSet> DESCRIPTOR_SETS_CHUNKS;
std::vector<VkRenderPass> RENDER_PASSES;
std::vector<Descriptor> GPU_DESCRIPTORS;
} // namespace glvm

namespace glvm {
void descriptor_set_builder() {
    // Counts ds bindings indexes inside ds.
    static unsigned int DS_GLOBAL_BINDINGS_COUNTER = 0;
    // Counts host data ds.
    static unsigned int DS_HOST_NUMBER = 0;
    // Counts offsets data descriptors.
    static unsigned int GLOBAL_DESCRIPTORS_OFFSET = 0;

    for (unsigned int ds_counter = 0;
         ds_counter < DescriptorSetDataLink::DescriptorChunksNumber;
         ++ds_counter) {
        // Offset for indexing inside descriptorSetsChunks.
        DESCRIPTOR_SETS_CONFIG[ds_counter].descriptor_set_offset =
            DS_HOST_NUMBER;
        DS_HOST_NUMBER +=
            DESCRIPTOR_SETS_CONFIG[ds_counter].host_descriptor_number;

        for (unsigned int ds_local_bindings_counter = 0;
             ds_local_bindings_counter
             < DESCRIPTOR_SETS_CONFIG[ds_counter]
                   .actual_linked_descriptor_bindings_number;
             ++ds_local_bindings_counter) {
            const uint32_t ds_sum_bindings_counter =
                DS_GLOBAL_BINDINGS_COUNTER + ds_local_bindings_counter;
            // Global offset for descriptors inside ds binding.
            DESCRIPTOR_BINDINGS_CONFIG[ds_sum_bindings_counter]
                .global_descriptor_offset = GLOBAL_DESCRIPTORS_OFFSET;

            // Index for ds bindings inside ds.
            DESCRIPTOR_SETS_CONFIG[ds_counter]
                .descriptors_bindings_i_ds[ds_local_bindings_counter] =
                ds_sum_bindings_counter;
            if (DESCRIPTOR_BINDINGS_CONFIG[ds_sum_bindings_counter].vk_type
                == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                for (unsigned int descriptor_counter = 0; descriptor_counter
                     < DESCRIPTOR_BINDINGS_CONFIG[ds_sum_bindings_counter]
                           .shader_descriptors_number;
                     ++descriptor_counter) {
                    GPU_DESCRIPTORS.push_back({});
                    GPU_DESCRIPTORS[GPU_DESCRIPTORS.size() - 1].gpu_buffer =
                        new GPUBuffer;
                    ++GLOBAL_DESCRIPTORS_OFFSET;
                }
            } else if (
                DESCRIPTOR_BINDINGS_CONFIG[ds_sum_bindings_counter].vk_type
                == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            ) {
                for (unsigned int descriptor_counter = 0; descriptor_counter
                     < DESCRIPTOR_BINDINGS_CONFIG[ds_sum_bindings_counter]
                           .shader_descriptors_number;
                     ++descriptor_counter) {
                    GPU_DESCRIPTORS.push_back({});
                    GPU_DESCRIPTORS[GPU_DESCRIPTORS.size() - 1].gpu_image =
                        new GpuImage;
                    ++GLOBAL_DESCRIPTORS_OFFSET;
                }
            }
        }
        DS_GLOBAL_BINDINGS_COUNTER +=
            DESCRIPTOR_SETS_CONFIG[ds_counter]
                .actual_linked_descriptor_bindings_number;
    }
    DESCRIPTOR_SETS_CHUNKS.resize(DS_HOST_NUMBER);
}

void pipeline_builder() {
    static unsigned int DESCRIPTOR_SETS_LAYOUT_ID_COUNTER = 0;
    for (unsigned int pipeline_counter = 0;
         pipeline_counter < SpecificPipeline::PipelinesNumber;
         ++pipeline_counter) {
        for (unsigned int linked_ds_layout_counter = 0; linked_ds_layout_counter
             < PIPELINE_CONFIGS[pipeline_counter]
                   .actual_linked_descriptor_sets_number;
             ++linked_ds_layout_counter) {
            PIPELINE_CONFIGS[pipeline_counter]
                .linked_descriptor_set_i_ds[linked_ds_layout_counter] =
                DESCRIPTOR_SETS_LAYOUT_ID_COUNTER + linked_ds_layout_counter;
        }
        DESCRIPTOR_SETS_LAYOUT_ID_COUNTER +=
            PIPELINE_CONFIGS[pipeline_counter]
                .actual_linked_descriptor_sets_number;
    }
}

void render_passes_builder() {
    RENDER_PASSES.resize(SpecificPipeline::PipelinesNumber);
}
}; // namespace glvm

namespace glvm {
VkResult create_debug_utils_messenger_ext(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator,
    VkDebugUtilsMessengerEXT* p_debug_messenger
) {
    static auto FUNC = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (FUNC != nullptr) {
        return FUNC(instance, p_create_info, p_allocator, p_debug_messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void create_begin_debug_utils_label_ext(
    VkInstance instance,
    VkCommandBuffer command_buffer,
    const VkDebugUtilsLabelEXT* label_info
) {
#ifndef NDEBUG
    static auto FUNC = (PFN_vkCmdBeginDebugUtilsLabelEXT)
        vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
    FUNC(command_buffer, label_info);
#endif
}

void create_end_debug_utils_label_ext(
    VkInstance instance,
    VkCommandBuffer command_buffer
) {
#ifndef NDEBUG
    static auto FUNC = (PFN_vkCmdEndDebugUtilsLabelEXT)
        vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
    FUNC(command_buffer);
#endif
}

void destroy_debug_utils_messenger_ext(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* p_allocator
) {
    static auto FUNC = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (FUNC != nullptr) {
        FUNC(instance, debug_messenger, p_allocator);
    }
}

VkResult set_debug_object_name(
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* object_name_info
) {
    static auto FUNC = (PFN_vkSetDebugUtilsObjectNameEXT)
        vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
    if (FUNC != nullptr) {
        return FUNC(device, object_name_info);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void set_image_debug_object_name(
    VkDevice device,
    GpuImage image,
    std::string image_name
) {
    VkDebugUtilsObjectNameInfoEXT image_object_info {};
    image_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string image_name1 = std::string(VK_DEBUG_IMAGE_SET_RED) + " \x1b[31m"
        + image_name + " pipeline #\x1b[0m " + std::to_string(0);
    const char* str_image_name = image_name1.c_str();
    image_object_info.pObjectName = str_image_name;
    image_object_info.objectType = VK_OBJECT_TYPE_IMAGE;
    image_object_info.objectHandle = (uint64_t)image.image;
    set_debug_object_name(device, &image_object_info);
}

void set_pipeline_debug_object_name(
    VkDevice device,
    VkPipeline pipeline,
    std::string pipeline_name
) {
    VkDebugUtilsObjectNameInfoEXT main_pipeline_object_info {};
    main_pipeline_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string main_pipe_line_image_name = std::string(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31m" + pipeline_name + " pipeline #\x1b[0m "
        + std::to_string(0);
    const char* main_pipe_line_str_image_name =
        main_pipe_line_image_name.c_str();
    main_pipeline_object_info.pObjectName = main_pipe_line_str_image_name;
    main_pipeline_object_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    main_pipeline_object_info.objectHandle = (uint64_t)pipeline;
    set_debug_object_name(device, &main_pipeline_object_info);
}

void set_descriptor_set_object_name(
    VkDevice device,
    VkDescriptorSet descriptor_set,
    std::string descriptor_set_name,
    unsigned int index
) {
    VkDebugUtilsObjectNameInfoEXT descriptor_set_object_info {};
    descriptor_set_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string name = std::string(VK_DEBUG_DESCRIPTOR_SET_RED) + " \x1b[31m"
        + descriptor_set_name + " descriptor set #\x1b[0m "
        + std::to_string(index);
    const char* str_name = name.c_str();
    descriptor_set_object_info.pObjectName = str_name;
    descriptor_set_object_info.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    descriptor_set_object_info.objectHandle = (uint64_t)descriptor_set;
    set_debug_object_name(device, &descriptor_set_object_info);
}

void set_debug_object_names(
    VkDevice device,
    const std::vector<VkBuffer>& vertex_buffer_container,
    const std::vector<VkBuffer>& index_buffer_container,
    const std::vector<Descriptor>& gpu_descriptors,
    const std::vector<unsigned int>& font_indices_container,
    const std::vector<VkBuffer>& font_vertex_buffer_container,
    const std::vector<VkBuffer>& font_index_buffer_container
) {
    set_pipeline_debug_object_name(
        device,
        PIPELINE_CONFIGS[SpecificPipeline::FontPipeline].pipeline,
        "fontPipeline"
    );
    set_pipeline_debug_object_name(
        device,
        PIPELINE_CONFIGS[SpecificPipeline::UiPipeline].pipeline,
        "uiPipeline"
    );
    set_pipeline_debug_object_name(
        device,
        PIPELINE_CONFIGS[SpecificPipeline::UiIconsPipeline].pipeline,
        "uiIconsPipeline"
    );

    VkDebugUtilsObjectNameInfoEXT main_pipeline_object_info {};
    main_pipeline_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string main_pipe_line_image_name = std::string(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mMain pipeline #\x1b[0m " + std::to_string(0);
    const char* main_pipe_line_str_image_name =
        main_pipe_line_image_name.c_str();
    main_pipeline_object_info.pObjectName = main_pipe_line_str_image_name;
    main_pipeline_object_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    main_pipeline_object_info.objectHandle =
        (uint64_t)PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
            .pipeline;
    set_debug_object_name(device, &main_pipeline_object_info);

    VkDebugUtilsObjectNameInfoEXT main_pipeline_layout_object_info {};
    main_pipeline_layout_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string main_pipeline_layout_image_name =
        std::string(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mMain pipeline layout #\x1b[0m " + std::to_string(0);
    const char* main_pipeline_layout_str_image_name =
        main_pipeline_layout_image_name.c_str();
    main_pipeline_layout_object_info.pObjectName =
        main_pipeline_layout_str_image_name;
    main_pipeline_layout_object_info.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    main_pipeline_layout_object_info.objectHandle =
        (uint64_t)PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
            .pipeline_layout;
    set_debug_object_name(device, &main_pipeline_object_info);

    VkDebugUtilsObjectNameInfoEXT directional_light_pipeline_object_info {};
    directional_light_pipeline_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string directional_light_pipe_line_image_name =
        std::string(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mDirectional light pipeline #\x1b[0m " + std::to_string(0);
    const char* directional_light_pipe_line_str_image_name =
        directional_light_pipe_line_image_name.c_str();
    directional_light_pipeline_object_info.pObjectName =
        directional_light_pipe_line_str_image_name;
    directional_light_pipeline_object_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    directional_light_pipeline_object_info.objectHandle =
        (uint64_t)PIPELINE_CONFIGS[SpecificPipeline::DirectionalLightPipeline]
            .pipeline;
    set_debug_object_name(device, &directional_light_pipeline_object_info);

    VkDebugUtilsObjectNameInfoEXT
        directional_light_pipeline_layout_object_info {};
    directional_light_pipeline_layout_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string directional_light_pipeline_layout_image_name =
        std::string(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mDirectional light pipeline layout #\x1b[0m "
        + std::to_string(0);
    const char* directional_light_pipeline_layout_str_image_name =
        directional_light_pipeline_layout_image_name.c_str();
    directional_light_pipeline_layout_object_info.pObjectName =
        directional_light_pipeline_layout_str_image_name;
    directional_light_pipeline_layout_object_info.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    directional_light_pipeline_layout_object_info.objectHandle =
        (uint64_t)PIPELINE_CONFIGS[SpecificPipeline::DirectionalLightPipeline]
            .pipeline_layout;
    set_debug_object_name(
        device,
        &directional_light_pipeline_layout_object_info
    );

    VkDebugUtilsObjectNameInfoEXT spot_light_pipeline_object_info {};
    spot_light_pipeline_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string spot_light_pipe_line_image_name =
        std::string(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mSpot light pipeline #\x1b[0m " + std::to_string(0);
    const char* spot_light_pipe_line_str_image_name =
        spot_light_pipe_line_image_name.c_str();
    spot_light_pipeline_object_info.pObjectName =
        spot_light_pipe_line_str_image_name;
    spot_light_pipeline_object_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    spot_light_pipeline_object_info.objectHandle =
        (uint64_t)PIPELINE_CONFIGS[SpecificPipeline::SpotLightPipeline].pipeline;
    set_debug_object_name(device, &spot_light_pipeline_object_info);

    VkDebugUtilsObjectNameInfoEXT spot_light_pipeline_layout_object_info {};
    spot_light_pipeline_layout_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string spot_light_pipeline_layout_image_name =
        std::string(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mSpot light pipeline layout #\x1b[0m " + std::to_string(0);
    const char* spot_light_pipeline_layout_str_image_name =
        spot_light_pipeline_layout_image_name.c_str();
    spot_light_pipeline_layout_object_info.pObjectName =
        spot_light_pipeline_layout_str_image_name;
    spot_light_pipeline_layout_object_info.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    spot_light_pipeline_layout_object_info.objectHandle =
        (uint64_t)PIPELINE_CONFIGS[SpecificPipeline::SpotLightPipeline]
            .pipeline_layout;
    set_debug_object_name(device, &spot_light_pipeline_layout_object_info);

    VkDebugUtilsObjectNameInfoEXT point_light_pipeline_object_info {};
    point_light_pipeline_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string point_light_pipe_line_image_name =
        std::string(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mPoint light pipeline #\x1b[0m " + std::to_string(0);
    const char* point_light_pipe_line_str_image_name =
        point_light_pipe_line_image_name.c_str();
    point_light_pipeline_object_info.pObjectName =
        point_light_pipe_line_str_image_name;
    point_light_pipeline_object_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    point_light_pipeline_object_info.objectHandle =
        (uint64_t)PIPELINE_CONFIGS[SpecificPipeline::PointLightPipeline]
            .pipeline;
    set_debug_object_name(device, &point_light_pipeline_object_info);

    VkDebugUtilsObjectNameInfoEXT point_light_pipeline_layout_object_info {};
    point_light_pipeline_layout_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string point_light_pipeline_layout_image_name =
        std::string(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mPoint light pipeline layout #\x1b[0m " + std::to_string(0);
    const char* point_light_pipeline_layout_str_image_name =
        point_light_pipeline_layout_image_name.c_str();
    point_light_pipeline_layout_object_info.pObjectName =
        point_light_pipeline_layout_str_image_name;
    point_light_pipeline_layout_object_info.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    point_light_pipeline_layout_object_info.objectHandle =
        (uint64_t)PIPELINE_CONFIGS[SpecificPipeline::PointLightPipeline]
            .pipeline_layout;
    set_debug_object_name(device, &point_light_pipeline_layout_object_info);

    VkDebugUtilsObjectNameInfoEXT hud_uniform_buffer_object_info {};
    hud_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string hud_image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Hud uniform buffer # " + std::to_string(0);
    const char* hud_str_image_name = hud_image_name.c_str();
    hud_uniform_buffer_object_info.pObjectName = hud_str_image_name;
    hud_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int hud_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::HUD]
            .descriptors_bindings_i_ds[0];
    hud_uniform_buffer_object_info.objectHandle =
        (uint64_t)gpu_descriptors
            [DESCRIPTOR_BINDINGS_CONFIG[hud_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->buffer;
    set_debug_object_name(device, &hud_uniform_buffer_object_info);

    VkDebugUtilsObjectNameInfoEXT font_uniform_buffer_object_info {};
    font_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string font_image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Font uniform buffer # " + std::to_string(0);
    const char* font_str_image_name = font_image_name.c_str();
    font_uniform_buffer_object_info.pObjectName = font_str_image_name;
    font_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int font_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::FontRenderUbo]
            .descriptors_bindings_i_ds[0];
    font_uniform_buffer_object_info.objectHandle =
        (uint64_t)gpu_descriptors
            [DESCRIPTOR_BINDINGS_CONFIG[font_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->buffer;
    set_debug_object_name(device, &font_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT ui_uniform_buffer_object_info {};
    ui_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string ui_image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " UI uniform buffer # " + std::to_string(0);
    const char* ui_str_image_name = ui_image_name.c_str();
    ui_uniform_buffer_object_info.pObjectName = ui_str_image_name;
    ui_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int ui_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::UI]
            .descriptors_bindings_i_ds[0];
    ui_uniform_buffer_object_info.objectHandle =
        (uint64_t)gpu_descriptors
            [DESCRIPTOR_BINDINGS_CONFIG[ui_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->buffer;
    set_debug_object_name(device, &ui_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT ui_icons_uniform_buffer_object_info {};
    ui_icons_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string ui_icons_image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " UI icons uniform buffer # " + std::to_string(0);
    const char* ui_icons_str_image_name = ui_icons_image_name.c_str();
    ui_icons_uniform_buffer_object_info.pObjectName = ui_icons_str_image_name;
    ui_icons_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int ui_icons_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::UiIcons]
            .descriptors_bindings_i_ds[0];
    ui_icons_uniform_buffer_object_info.objectHandle =
        (uint64_t)gpu_descriptors
            [DESCRIPTOR_BINDINGS_CONFIG[ui_icons_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->buffer;
    set_debug_object_name(device, &ui_icons_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT
        directional_light_uniform_buffer_object_info {};
    directional_light_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string directional_light_image_name =
        std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Shadow map directional light model matrix uniform buffer # "
        + std::to_string(0);
    const char* directional_light_str_image_name =
        directional_light_image_name.c_str();
    directional_light_uniform_buffer_object_info.pObjectName =
        directional_light_str_image_name;
    directional_light_uniform_buffer_object_info.objectType =
        VK_OBJECT_TYPE_BUFFER;
    unsigned int shadow_map_directional_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapDirectionalLight]
            .descriptors_bindings_i_ds[0];
    directional_light_uniform_buffer_object_info.objectHandle =
        (uint64_t)gpu_descriptors
            [DESCRIPTOR_BINDINGS_CONFIG
                 [shadow_map_directional_light_descriptor_binding_index]
                     .global_descriptor_offset]
                .gpu_buffer->buffer;
    set_debug_object_name(device, &directional_light_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT point_light_uniform_buffer_object_info {};
    point_light_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string point_light_image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Shadow map point light model matrix uniform buffer # "
        + std::to_string(0);
    const char* point_light_str_image_name = point_light_image_name.c_str();
    point_light_uniform_buffer_object_info.pObjectName =
        point_light_str_image_name;
    point_light_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int shadow_map_point_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapPointLight]
            .descriptors_bindings_i_ds[0];
    point_light_uniform_buffer_object_info.objectHandle =
        (uint64_t)
            gpu_descriptors[DESCRIPTOR_BINDINGS_CONFIG
                                [shadow_map_point_light_descriptor_binding_index]
                                    .global_descriptor_offset]
                .gpu_buffer->buffer;
    set_debug_object_name(device, &point_light_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT spot_light_uniform_buffer_object_info {};
    spot_light_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string spot_light_image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Shadow map spot light model matrix uniform buffer # "
        + std::to_string(0);
    const char* spot_light_str_image_name = spot_light_image_name.c_str();
    spot_light_uniform_buffer_object_info.pObjectName =
        spot_light_str_image_name;
    spot_light_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int shadow_map_spot_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapSpotLight]
            .descriptors_bindings_i_ds[0];
    spot_light_uniform_buffer_object_info.objectHandle =
        (uint64_t)
            gpu_descriptors[DESCRIPTOR_BINDINGS_CONFIG
                                [shadow_map_spot_light_descriptor_binding_index]
                                    .global_descriptor_offset]
                .gpu_buffer->buffer;
    set_debug_object_name(device, &spot_light_uniform_buffer_object_info);
    for (unsigned long i = 0; i < vertex_buffer_container.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniform_buffer_object_info {};
        uniform_buffer_object_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
            + " Vertex uniform buffer # " + std::to_string(i);
        const char* str_image_name = image_name.c_str();
        uniform_buffer_object_info.pObjectName = str_image_name;
        uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
        uniform_buffer_object_info.objectHandle =
            (uint64_t)vertex_buffer_container[i];
        set_debug_object_name(device, &uniform_buffer_object_info);
    }
    for (unsigned long i = 0; i < index_buffer_container.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniform_buffer_object_info {};
        uniform_buffer_object_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
            + " Index uniform buffer # " + std::to_string(i);
        const char* str_image_name = image_name.c_str();
        uniform_buffer_object_info.pObjectName = str_image_name;
        uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
        uniform_buffer_object_info.objectHandle =
            (uint64_t)index_buffer_container[i];
        set_debug_object_name(device, &uniform_buffer_object_info);
    }
    for (unsigned long i = 0; i < font_indices_container.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniform_buffer_object_info {};
        uniform_buffer_object_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
            + " Font vertex uniform buffer # "
            + std::to_string(font_indices_container[i]);
        const char* str_image_name = image_name.c_str();
        uniform_buffer_object_info.pObjectName = str_image_name;
        uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
        uniform_buffer_object_info.objectHandle =
            (uint64_t)font_vertex_buffer_container[font_indices_container[i]];
        set_debug_object_name(device, &uniform_buffer_object_info);
    }
    for (unsigned long i = 0; i < font_indices_container.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniform_buffer_object_info {};
        uniform_buffer_object_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        std::string image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
            + " Font index uniform buffer # "
            + std::to_string(font_indices_container[i]);
        const char* str_image_name = image_name.c_str();
        uniform_buffer_object_info.pObjectName = str_image_name;
        uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
        uniform_buffer_object_info.objectHandle =
            (uint64_t)font_index_buffer_container[font_indices_container[i]];
        set_debug_object_name(device, &uniform_buffer_object_info);
    }
    VkDebugUtilsObjectNameInfoEXT uniform_buffer_object_info {};
    uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Model matrix uniform buffer # " + std::to_string(0);
    const char* str_image_name = image_name.c_str();
    uniform_buffer_object_info.pObjectName = str_image_name;
    uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    uniform_buffer_object_info.objectHandle =
        (uint64_t)gpu_descriptors[DescriptorSetDataLink::MainRenderMatrixUbo]
            .gpu_buffer->buffer;
    set_debug_object_name(device, &uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT light_data_uniform_buffer_object_info {};
    light_data_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    std::string light_data_image_name = std::string(VK_DEBUG_IMAGE_SET_RED)
        + " Light data uniform buffer # " + std::to_string(0);
    const char* light_data_str_image_name = light_data_image_name.c_str();
    light_data_uniform_buffer_object_info.pObjectName =
        light_data_str_image_name;
    light_data_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    unsigned int light_data_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_i_ds[0];
    light_data_uniform_buffer_object_info.objectHandle =
        (uint64_t)gpu_descriptors
            [DESCRIPTOR_BINDINGS_CONFIG[light_data_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->buffer;
    set_debug_object_name(device, &light_data_uniform_buffer_object_info);
}
}; // namespace glvm

namespace glvm {
namespace {
// 16k verts, shared line + quad buffer.
constexpr uint32_t K_MAX_DEBUG_VERTICES = 1 << 14;

void push_line(
    std::vector<DebugVertex>& out,
    const Vector<float, 3>& a,
    const Vector<float, 3>& b,
    const Vector<float, 3>& color
) {
    out.push_back({a[0], a[1], a[2], color[0], color[1], color[2]});
    out.push_back({b[0], b[1], b[2], color[0], color[1], color[2]});
}

void push_box(
    std::vector<DebugVertex>& out,
    const Vector<float, 3> corners[8],
    const Vector<float, 3>& color
) {
    static const unsigned int EDGES[12][2] = {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 0},
        {4, 5},
        {5, 6},
        {6, 7},
        {7, 4},
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}
    };
    for (const auto& edge : EDGES) {
        push_line(out, corners[edge[0]], corners[edge[1]], color);
    }
}

Vector<float, 4> to_vec4(const Vector<float, 3>& v, float w) {
    return Vector<float, 4>(v[0], v[1], v[2], w);
}

Vector<float, 3> from_vec4(const Vector<float, 4>& v) {
    return Vector<float, 3>(v[0], v[1], v[2]);
}
} // namespace

ImGuiOverlay::ImGuiOverlay(CVulkanRenderer& renderer) : renderer(renderer) {
}

bool ImGuiOverlay::wants_mouse() const {
    if (!initialized) {
        return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

void ImGuiOverlay::init() {
    if (initialized) {
        return;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    create_render_pass();
    create_vertex_buffer();
    create_line_pipeline();
#ifdef _WIN32
    ImGui_ImplWin32_Init(renderer.window->get_modern_window_hwnd());
#endif
    ImGui_ImplVulkan_InitInfo init_info {};
    init_info.ApiVersion = VK_API_VERSION_1_0;
    init_info.Instance = renderer.instance;
    init_info.PhysicalDevice = renderer.physical_device;
    init_info.Device = renderer.device;
    init_info.QueueFamily =
        renderer.find_queue_families(renderer.physical_device)
            .graphics_family.value();
    init_info.Queue = renderer.graphics_queue;
    init_info.DescriptorPoolSize = 512;
    init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount =
        static_cast<uint32_t>(renderer.swap_chain_images.size());
    init_info.PipelineInfoMain.RenderPass = render_pass;
    init_info.PipelineInfoMain.Subpass = 0;
    if (!ImGui_ImplVulkan_Init(&init_info)) {
        throw std::runtime_error("failed to init ImGui vulkan backend!");
    }
    initialized = true;
    create_swap_chain_resources();
}

void ImGuiOverlay::shutdown() {
    if (!initialized) {
        return;
    }
    vkDeviceWaitIdle(renderer.device);
    destroy_swap_chain_resources();
    vkDestroyBuffer(renderer.device, vertex_buffer, nullptr);
    vkFreeMemory(renderer.device, vertex_buffer_memory, nullptr);
    vertex_buffer = VK_NULL_HANDLE;
    vertex_buffer_mapped = nullptr;
    vkDestroyPipeline(renderer.device, line_pipeline, nullptr);
    vkDestroyPipelineLayout(renderer.device, line_layout, nullptr);
    vkDestroyRenderPass(renderer.device, render_pass, nullptr);
    render_pass = VK_NULL_HANDLE;
    ImGui_ImplVulkan_Shutdown();
#ifdef _WIN32
    ImGui_ImplWin32_Shutdown();
#endif
    ImGui::DestroyContext();
    initialized = false;
}

void ImGuiOverlay::create_swap_chain_resources() {
    if (!initialized) {
        return;
    }
    framebuffers.resize(renderer.swap_chain_image_views.size());
    for (size_t i = 0; i < framebuffers.size(); ++i) {
        VkImageView attachment = renderer.swap_chain_image_views[i];
        VkFramebufferCreateInfo info {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = render_pass;
        info.attachmentCount = 1;
        info.pAttachments = &attachment;
        info.width = renderer.swap_chain_extent.width;
        info.height = renderer.swap_chain_extent.height;
        info.layers = 1;
        if (vkCreateFramebuffer(renderer.device, &info, nullptr, &framebuffers[i])
            != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create imgui overlay framebuffer!"
            );
        }
    }
}

void ImGuiOverlay::destroy_swap_chain_resources() {
    for (VkFramebuffer& framebuffer : framebuffers) {
        vkDestroyFramebuffer(renderer.device, framebuffer, nullptr);
    }
    framebuffers.clear();
}

void ImGuiOverlay::new_frame() {
    if (!initialized) {
        return;
    }
#ifdef _WIN32
    ImGui_ImplWin32_NewFrame();
#else
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize =
        ImVec2((float)renderer.window->width, (float)renderer.window->height);
    static auto last_frame_time = std::chrono::steady_clock::now();
    const auto now_time = std::chrono::steady_clock::now();
    io.DeltaTime =
        std::chrono::duration<float>(now_time - last_frame_time).count();
    last_frame_time = now_time;
    // No OS cursor plumbing yet: let ImGui draw its own cursor.
    io.MouseDrawCursor = true;
    io.AddMousePosEvent(
        G_E_EVENT.mouse_pointer_position.offset_x,
        G_E_EVENT.mouse_pointer_position.offset_y
    );
    io.AddMouseButtonEvent(
        ImGuiMouseButton_Left,
        !G_E_EVENT.is_left_mouse_button_released
    );
#endif
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();

    if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
        show_panel = !show_panel;
    }
    build_panel();
    build_debug_vertices();
    ImGui::Render();
}

void ImGuiOverlay::record_command_buffer(
    VkCommandBuffer command_buffer,
    uint32_t image_index
) {
    if (!initialized) {
        return;
    }
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = renderer.swap_chain_extent;
    render_pass_info.clearValueCount = 0;
    render_pass_info.pClearValues = nullptr;
    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );
    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(renderer.swap_chain_extent.width);
    viewport.height = static_cast<float>(renderer.swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = renderer.swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    if (line_vertex_count > 0) {
        vkCmdBindPipeline(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            line_pipeline
        );
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, &offset);
        Matrix<float, 4> view_proj =
            renderer.view_matrix * renderer.projection_matrix;
        vkCmdPushConstants(
            command_buffer,
            line_layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(Matrix<float, 4>),
            &view_proj
        );
        vkCmdDraw(command_buffer, line_vertex_count, 1, 0, 0);
    }
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
    vkCmdEndRenderPass(command_buffer);
}

void ImGuiOverlay::build_panel() {
    if (!show_panel) {
        return;
    }
    ImGui::Begin("Debug overlay", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Checkbox("Actor bounds", &show_actor_bounds);
    ImGui::Checkbox("Light frustums", &show_light_frustums);
    ImGui::Checkbox("Spatial grid", &show_spatial_grid);
    ImGui::Checkbox("Shadows", &shadows_enabled);
    ImGui::Checkbox("Shadow maps", &show_shadow_maps);
    if (show_shadow_maps) {
        ImGui::RadioButton("Directional", &shadow_map_mode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Spot", &shadow_map_mode, 1);
        int max_light = (shadow_map_mode == 0)
            ? static_cast<int>(renderer.directional_light_number)
            : static_cast<int>(renderer.spot_light_number);
        ImGui::SliderInt(
            "Light",
            &shadow_map_light,
            0,
            std::max(0, max_light - 1)
        );
        ImGui::Text(
            "Shadow map #%d of %d",
            shadow_map_light,
            std::max(1, max_light)
        );
    }
    ImGui::Text("Actors: %zu", renderer.actors.size());
    ImGui::Text(
        "Dir lights: %u, Spot lights: %u",
        renderer.directional_light_number,
        renderer.spot_light_number
    );
    ImGui::Separator();
    if (ImGui::Button("Hide panel (F1)")) {
        show_panel = false;
    }
    ImGui::End();
}

void ImGuiOverlay::build_debug_vertices() {
    std::vector<DebugVertex> vertices;
    vertices.reserve(K_MAX_DEBUG_VERTICES);
    line_vertex_count = 0;
    if (show_actor_bounds) {
        const Vector<float, 3> green = {0.0f, 1.0f, 0.0f};
        const Vector<float, 3> red = {1.0f, 0.0f, 0.0f};
        std::vector<Vector<float, 3>> mins;
        std::vector<Vector<float, 3>> maxs;
        mins.reserve(renderer.actors.size());
        maxs.reserve(renderer.actors.size());
        for (size_t i = 0; i < renderer.actors.size(); ++i) {
            RenderActor actor = renderer.actors[i];
            if (actor.mesh_id >= ALL_MESH_MAX_ABSOLUTE_VALUES.size()) {
                mins.push_back({0, 0, 0});
                maxs.push_back({0, 0, 0});
                continue;
            }
            const MeshAxisMaxAbsoluteValues& bounds =
                ALL_MESH_MAX_ABSOLUTE_VALUES[actor.mesh_id];
            const Vector<float, 3> center = {
                bounds.origin_offset_x,
                bounds.origin_offset_y,
                bounds.origin_offset_z
            };
            const Vector<float, 3> half =
                {bounds.absolute_x, bounds.absolute_y, bounds.absolute_z};
            Vector<float, 3> local_corners[8] = {
                center + Vector<float, 3>(-half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], half[1], half[2]),
                center + Vector<float, 3>(-half[0], half[1], half[2])
            };
            Vector<float, 3> mn =
                from_vec4(to_vec4(local_corners[0], 1.0f) * actor.model_matrix);
            Vector<float, 3> mx = mn;
            for (int c = 1; c < 8; ++c) {
                Vector<float, 3> w = from_vec4(
                    to_vec4(local_corners[c], 1.0f) * actor.model_matrix
                );
                for (int a = 0; a < 3; ++a) {
                    mn[a] = std::min(mn[a], w[a]);
                    mx[a] = std::max(mx[a], w[a]);
                }
            }
            mins.push_back(mn);
            maxs.push_back(mx);
        }
        std::vector<bool> collides(renderer.actors.size(), false);
        for (size_t i = 0; i < renderer.actors.size(); ++i) {
            for (size_t j = i + 1; j < renderer.actors.size(); ++j) {
                // Non-strict: the collision system resolves contact by pushing
                // the mover back to exactly touch the target, so overlapping
                // boxes (<) alone misses face-to-face contact.
                if (mins[i][0] <= maxs[j][0] && maxs[i][0] >= mins[j][0]
                    && mins[i][1] <= maxs[j][1] && maxs[i][1] >= mins[j][1]
                    && mins[i][2] <= maxs[j][2] && maxs[i][2] >= mins[j][2]) {
                    collides[i] = true;
                    collides[j] = true;
                }
            }
        }
        for (size_t i = 0; i < renderer.actors.size(); ++i) {
            if (renderer.actors[i].mesh_id
                >= ALL_MESH_MAX_ABSOLUTE_VALUES.size()) {
                continue;
            }
            const MeshAxisMaxAbsoluteValues& bounds =
                ALL_MESH_MAX_ABSOLUTE_VALUES[renderer.actors[i].mesh_id];
            const Vector<float, 3> center = {
                bounds.origin_offset_x,
                bounds.origin_offset_y,
                bounds.origin_offset_z
            };
            const Vector<float, 3> half =
                {bounds.absolute_x, bounds.absolute_y, bounds.absolute_z};
            Vector<float, 3> local_corners[8] = {
                center + Vector<float, 3>(-half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], -half[1], -half[2]),
                center + Vector<float, 3>(half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], half[1], -half[2]),
                center + Vector<float, 3>(-half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], -half[1], half[2]),
                center + Vector<float, 3>(half[0], half[1], half[2]),
                center + Vector<float, 3>(-half[0], half[1], half[2])
            };
            Vector<float, 3> world_corners[8];
            for (int c = 0; c < 8; ++c) {
                world_corners[c] = from_vec4(
                    to_vec4(local_corners[c], 1.0f)
                    * renderer.actors[i].model_matrix
                );
            }
            push_box(vertices, world_corners, collides[i] ? red : green);
        }
    }

    if (show_light_frustums) {
        const Vector<float, 3> yellow = {1.0f, 1.0f, 0.0f};
        for (uint32_t i = 0; i < renderer.directional_light_number; ++i) {
            Vector<float, 3> corners[8];
            Matrix<float, 4> inverse_light =
                inverse_matrix_4x4(renderer.dir_light_space_matrix[i]);
            for (int c = 0; c < 8; ++c) {
                const float s = (c & 4) ? 1.0f : -1.0f; // z (near/far).
                const float u = (c & 2) ? 1.0f : -1.0f; // y.
                const float v = (c & 1) ? 1.0f : -1.0f; // x.
                corners[c] =
                    from_vec4(Vector<float, 4>(u, v, s, 1.0f) * inverse_light);
            }
            push_box(vertices, corners, yellow);
        }
        const Vector<float, 3> cyan = {0.0f, 1.0f, 1.0f};
        for (uint32_t i = 0; i < renderer.spot_light_number; ++i) {
            Vector<float, 3> corners[8];
            Matrix<float, 4> inverse_light =
                inverse_matrix_4x4(renderer.spot_light_space_matrix[i]);
            for (int c = 0; c < 8; ++c) {
                const float s = (c & 4) ? 1.0f : -1.0f;
                const float u = (c & 2) ? 1.0f : -1.0f;
                const float v = (c & 1) ? 1.0f : -1.0f;
                corners[c] =
                    from_vec4(Vector<float, 4>(u, v, s, 1.0f) * inverse_light);
            }
            push_box(vertices, corners, cyan);
        }
    }

    if (show_spatial_grid) {
        const auto& grid = glvm::WORLD.spatial_grid;
        const float half_chunk = grid.grid[0][0][0].SIZE * 0.5f;
        const float cross = 1.5f;
        for (uint32_t z = 0; z < grid.depth; ++z) {
            for (uint32_t y = 0; y < grid.height; ++y) {
                for (uint32_t x = 0; x < grid.width; ++x) {
                    const auto& chunk = grid.grid[z][y][x];
                    const size_t count = chunk.entities.size();
                    if (count == 0) {
                        continue;
                    }
                    // Only occupied cells, colored by entity count.
                    const Vector<float, 3> color = count == 1
                        ? Vector<float, 3>(0.0f, 1.0f, 0.0f)
                        : count <= 3 ? Vector<float, 3>(1.0f, 1.0f, 0.0f)
                                     : Vector<float, 3>(1.0f, 0.0f, 0.0f);
                    const Vector<float, 3> center = chunk.position
                        + Vector<float, 3>(half_chunk, half_chunk, half_chunk);
                    push_line(
                        vertices,
                        center - Vector<float, 3>(cross, 0, 0),
                        center + Vector<float, 3>(cross, 0, 0),
                        color
                    );
                    push_line(
                        vertices,
                        center - Vector<float, 3>(0, cross, 0),
                        center + Vector<float, 3>(0, cross, 0),
                        color
                    );
                    push_line(
                        vertices,
                        center - Vector<float, 3>(0, 0, cross),
                        center + Vector<float, 3>(0, 0, cross),
                        color
                    );
                }
            }
        }
    }

    line_vertex_count = static_cast<uint32_t>(vertices.size());

    if (!vertices.empty() && vertex_buffer_mapped) {
        const size_t bytes = vertices.size() * sizeof(DebugVertex);
        const size_t capacity = K_MAX_DEBUG_VERTICES * sizeof(DebugVertex);
        memcpy(vertex_buffer_mapped, vertices.data(), std::min(bytes, capacity));
    }
}

void ImGuiOverlay::create_render_pass() {
    VkAttachmentDescription color_attachment {};
    color_attachment.format = renderer.swap_chain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference color_reference {};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;
    VkSubpassDependency dependencies[2] {};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = 0;
    VkRenderPassCreateInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 2;
    render_pass_info.pDependencies = dependencies;
    if (vkCreateRenderPass(
            renderer.device,
            &render_pass_info,
            nullptr,
            &render_pass
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create imgui overlay render pass!");
    }
}

void ImGuiOverlay::create_line_pipeline() {
    auto create_shader_module = [&](const char* path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error(
                std::string("failed to open shader: ") + path
            );
        }
        file.seekg(0, std::ios::beg);
        std::vector<char> code(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        VkShaderModuleCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule module;
        if (vkCreateShaderModule(renderer.device, &create_info, nullptr, &module)
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return module;
    };

    VkShaderModule vert = create_shader_module(
        "../../../crates/glvm/assets/shaders/debug/debug_vert.spv"
    );
    VkShaderModule frag = create_shader_module(
        "../../../crates/glvm/assets/shaders/debug/debug_frag.spv"
    );

    VkPipelineShaderStageCreateInfo shader_stages[2] {};
    shader_stages[0].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vert;
    shader_stages[0].pName = "main";
    shader_stages[1].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = frag;
    shader_stages[1].pName = "main";

    VkVertexInputBindingDescription binding_description {};
    binding_description.binding = 0;
    binding_description.stride = sizeof(DebugVertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions {};
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(DebugVertex, x);
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(DebugVertex, r);

    VkPipelineVertexInputStateCreateInfo vertex_input_info {};
    vertex_input_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions =
        attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly {};
    input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state {};
    viewport_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachment {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending {};
    color_blending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    VkPushConstantRange push_constant_range {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(Matrix<float, 4>);

    VkPipelineLayoutCreateInfo layout_info {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 0;
    layout_info.pSetLayouts = nullptr;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &push_constant_range;

    if (vkCreatePipelineLayout(
            renderer.device,
            &layout_info,
            nullptr,
            &line_layout
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create debug line pipeline layout!");
    }

    VkPipelineDepthStencilStateCreateInfo depth_stencil {};
    depth_stencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_FALSE;
    depth_stencil.depthWriteEnable = VK_FALSE;

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamic_state {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states;

    VkGraphicsPipelineCreateInfo pipeline_info {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = line_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;

    if (vkCreateGraphicsPipelines(
            renderer.device,
            VK_NULL_HANDLE,
            1,
            &pipeline_info,
            nullptr,
            &line_pipeline
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create debug line pipeline!");
    }

    vkDestroyShaderModule(renderer.device, vert, nullptr);
    vkDestroyShaderModule(renderer.device, frag, nullptr);
}

void ImGuiOverlay::create_vertex_buffer() {
    const VkDeviceSize buffer_size = K_MAX_DEBUG_VERTICES * sizeof(DebugVertex);
    renderer.create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertex_buffer,
        vertex_buffer_memory
    );
    vkMapMemory(
        renderer.device,
        vertex_buffer_memory,
        0,
        buffer_size,
        0,
        &vertex_buffer_mapped
    );
}

} // namespace glvm

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#endif

namespace glvm {
CVulkanRenderer::CVulkanRenderer() {
    imgui_overlay = new ImGuiOverlay(*this);
}

CVulkanRenderer::~CVulkanRenderer() {
    cleanup();
    delete imgui_overlay;
    imgui_overlay = nullptr;
}

void CVulkanRenderer::draw() {
    imgui_overlay->new_frame();
    main_render_draw_frame();
}

void CVulkanRenderer::set_view_matrix(Matrix<float, 4> new_view_matrix) {
    view_matrix = new_view_matrix;
}

void CVulkanRenderer::set_projection_matrix(
    Matrix<float, 4> new_projection_matrix
) {
    projection_matrix = new_projection_matrix;
}

void CVulkanRenderer::create_texture_image() {
    uint32_t tex_width, tex_height;
    uint32_t tex_channels;

    unsigned int readable_texture_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::RidableTextures]
            .descriptors_bindings_i_ds[0];
    for (unsigned int i = 0; i < initialize_texture_data.size(); ++i) {
        VkDeviceSize image_size {};
        unsigned char* pixels;
        const char* path_to_stb_image = nullptr;

#ifndef STB_IMAGE_IMPLEMENTATION
        image_size = initialize_texture_data[i].dat_length;
        pixels = initialize_texture_data[i].u_i_data;
        tex_width = initialize_texture_data[i].i_width;
        tex_height = initialize_texture_data[i].i_height;
#endif

#ifdef STB_IMAGE_IMPLEMENTATION
        path_to_stb_image = initializeTextureData_[i].path_to_image;
        pixels = stbi_load(
            path_to_stb_image,
            reinterpret_cast<int*>(&tex_width),
            reinterpret_cast<int*>(&tex_height),
            reinterpret_cast<int*>(&tex_channels),
            STBI_rgb_alpha
        );
        image_size = tex_width * tex_height * 4;
#endif

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        create_buffer(
            image_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_buffer_memory
        );

        void* data;
        vkMapMemory(device, staging_buffer_memory, 0, image_size, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(image_size));
        vkUnmapMemory(device, staging_buffer_memory);
        GpuImage texture_image = {
            .image = VkImage {},
            .device_memory = VkDeviceMemory {},
            .view_type = VK_IMAGE_VIEW_TYPE_2D,
            .create_flags = 0,
            .memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .usage_flags =
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .array_layers = 1,
            .width = tex_width,
            .height = tex_height
        };

        create_image(texture_image);

        transition_image_layout(
            texture_image.image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );
        copy_buffer_to_image(
            staging_buffer,
            texture_image.image,
            static_cast<uint32_t>(tex_width),
            static_cast<uint32_t>(tex_height)
        );
        transition_image_layout(
            texture_image.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
        *GPU_DESCRIPTORS
             [DESCRIPTOR_BINDINGS_CONFIG[readable_texture_descriptor_binding_index]
                  .global_descriptor_offset
              + i]
                 .gpu_image = texture_image;

        vkDestroyBuffer(device, staging_buffer, nullptr);
        vkFreeMemory(device, staging_buffer_memory, nullptr);
    }
}

void CVulkanRenderer::recreate_swap_chain() {
    vkDeviceWaitIdle(device);

#ifdef VK_USE_PLATFORM_XCB_KHR
    window->configure_window();
#endif

    imgui_overlay->destroy_swap_chain_resources();
    cleanup_swap_chain();

    create_swap_chain();
    window->width = swap_chain_extent.width;
    window->height = swap_chain_extent.height;
    aspect_rate = (float)window->width / (float)window->height;
    create_image_views();
    create_depth_resources();
    create_directional_light_shadow_map_depth_resources();
    create_spot_light_shadow_map_depth_resources();
    create_point_light_shadow_map_depth_resources();
    create_framebuffers();
    create_main_render_descriptor_sets();
    imgui_overlay->create_swap_chain_resources();
}

void CVulkanRenderer::set_mesh_data(
    std::vector<const char*> paths,
    std::vector<const char*> paths_gltf
) {
    for (unsigned int i = 0; i < paths.size(); ++i) {
        paths_array.push_back(paths[i]);
    }

    for (unsigned int i = 0; i < paths_gltf.size(); ++i) {
        paths_gltf.push_back(paths_gltf[i]);
    }
}

void CVulkanRenderer::run() {
    vk_config_initializer();
    descriptor_set_builder();
    pipeline_builder();
    render_passes_builder();
    render_thread_pool = new ThreadPool(3);
    start_time = std::chrono::steady_clock::now();

    init_window();
    init_vulkan();
}

void CVulkanRenderer::init_window() {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    window = initialize_wayland_window();
    create_wayland_surface_info.display = window->display;
    create_wayland_surface_info.surface = window->wl_surface;
    aspect_rate = (float)window->width / (float)window->height;
    create_wayland_surface_info.sType =
        VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    create_wayland_surface_info.pNext = nullptr;
    create_wayland_surface_info.flags = 0;
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
    window = new glvm::WindowXVulkan();
    createXlibSurfaceInfo.dpy = window->get_display();
    createXlibSurfaceInfo.window = window->get_window();
    aspect_rate = (float)window->width / (float)window->height;

    createXlibSurfaceInfo.sType =
        VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createXlibSurfaceInfo.pNext = nullptr;
    createXlibSurfaceInfo.flags = 0;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    window = new glvm::WindowXCBVulkan();
    createXcbSurfaceInfo.window = window->get_window();
    createXcbSurfaceInfo.connection = window->get_connection();
    aspect_rate = (float)window->width / (float)window->height;

    createXcbSurfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createXcbSurfaceInfo.pNext = nullptr;
    createXcbSurfaceInfo.flags = 0;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    window = new glvm::WindowWinVulkan();
    create_win32_surface_info.hwnd = window->get_modern_window_hwnd();
    aspect_rate = (float)window->width / (float)window->height;

    create_win32_surface_info.sType =
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_win32_surface_info.pNext = nullptr;
    create_win32_surface_info.flags = 0;
#endif
}

void CVulkanRenderer::initialize_game_level_vertices() {
    for (unsigned int m = 0; m < level_generated_vertices.size(); ++m) {
        a_vertices.push_back(level_generated_vertices[m]);
        a_indices.push_back(level_generated_indices[m]);
        joint_matrices_per_mesh.push_back({});
        frames.push_back({});
        for (int i = 0; i < 64; ++i) {
            frames[frames.size() - 1].push_back(0.0f);
        }
        int maximum_joints = 64;
        std::vector<std::vector<Matrix<float, 4>>> joint_matrices;
        for (int i = 0; i < maximum_joints; ++i) {
            std::vector<Matrix<float, 4>> global_all_frame_node_matrix;
            int number_of_frames = 64;
            for (int j = 0; j < number_of_frames; ++j) {
                Matrix<float, 4> unit_matrix(1.0f);
                global_all_frame_node_matrix.push_back(unit_matrix);
            }

            joint_matrices.push_back(global_all_frame_node_matrix);
        }
        joint_matrices_per_mesh[joint_matrices_per_mesh.size() - 1] =
            joint_matrices;

        uint32_t next_index_gltf = wavefront_obj_counter + gltf_counter + m;

        vertex_buffer_container.emplace_back();
        vertex_buffer_memory_container.emplace_back();
        create_vertex_buffer(
            vertex_buffer_container[next_index_gltf],
            vertex_buffer_memory_container[next_index_gltf],
            a_vertices[next_index_gltf]
        );

        index_buffer_container.emplace_back();
        index_buffer_memory_contaner.emplace_back();
        create_index_buffer(
            index_buffer_container[next_index_gltf],
            index_buffer_memory_contaner[next_index_gltf],
            a_indices[next_index_gltf]
        );
    }
}

void CVulkanRenderer::init_vulkan() {
    create_instance();
    setup_debug_messenger();
    create_surface();
    pick_physical_device();
    create_logical_device();
    create_swap_chain();
    create_image_views();
    create_main_render_pass();
    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_command_pool(main_render_command_pool);
    const uint32_t secondary_buffers_command_pools_number = 3;
    secondary_buffers_command_pools.resize(
        secondary_buffers_command_pools_number
    );
    for (uint32_t i = 0; i < secondary_buffers_command_pools.size(); ++i) {
        create_command_pool(secondary_buffers_command_pools[i]);
    }
    create_depth_resources();
    create_directional_light_shadow_map_depth_resources();
    create_spot_light_shadow_map_depth_resources();
    create_point_light_shadow_map_depth_resources();
    create_framebuffers();
    create_texture_image();
    create_texture_image_view();
    create_texture_sampler();
    create_shadow_map_sampler();
    initialize_vertex_buffers_with_wavefront_data();
    initialize_vertex_buffers_with_gltf_data();
    initialize_vertex_buffers_with_font_data();
    create_main_render_uniform_buffers();
    create_main_render_descriptor_pool();
    create_main_render_descriptor_sets();
    imgui_overlay->init();
    set_debug_object_names(
        device,
        vertex_buffer_container,
        index_buffer_container,
        GPU_DESCRIPTORS,
        font_indices_container,
        font_vertex_buffer_container,
        font_index_buffer_container
    );
    const uint32_t main_render_command_buffers_number = 1;
    create_command_buffers(
        main_render_command_pool,
        main_render_command_buffers,
        main_render_command_buffers_number,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY
    );

    create_command_buffers(
        secondary_buffers_command_pools[0],
        directional_light_secondary_command_buffers,
        directional_light_number,
        VK_COMMAND_BUFFER_LEVEL_SECONDARY
    );
    create_command_buffers(
        secondary_buffers_command_pools[1],
        spot_light_secondary_command_buffers,
        spot_light_number,
        VK_COMMAND_BUFFER_LEVEL_SECONDARY
    );
    create_command_buffers(
        secondary_buffers_command_pools[2],
        point_light_secondary_command_buffers,
        point_light_number * 6 * 16,
        VK_COMMAND_BUFFER_LEVEL_SECONDARY
    );
    create_sync_objects(
        image_available_semaphores,
        render_finished_semaphores,
        in_flight_fences
    );
}

void CVulkanRenderer::initialize_vertex_buffers_with_wavefront_data() {
    for (unsigned int m = 0; m < paths_array.size(); ++m) {
        vertex_buffer_container.emplace_back();
        vertex_buffer_memory_container.emplace_back();
        create_vertex_buffer(
            vertex_buffer_container[m],
            vertex_buffer_memory_container[m],
            a_vertices[m]
        );

        index_buffer_container.emplace_back();
        index_buffer_memory_contaner.emplace_back();
        create_index_buffer(
            index_buffer_container[m],
            index_buffer_memory_contaner[m],
            a_indices[m]
        );
        ++wavefront_obj_counter;
    }
}

void CVulkanRenderer::initialize_vertex_buffers_with_gltf_data() {
    for (unsigned int m = 0; m < paths_gltf.size(); ++m) {
        uint32_t next_index_gltf = wavefront_obj_counter + m;
        vertex_buffer_container.emplace_back();
        vertex_buffer_memory_container.emplace_back();
        create_vertex_buffer(
            vertex_buffer_container[next_index_gltf],
            vertex_buffer_memory_container[next_index_gltf],
            a_vertices[next_index_gltf]
        );

        index_buffer_container.emplace_back();
        index_buffer_memory_contaner.emplace_back();
        create_index_buffer(
            index_buffer_container[next_index_gltf],
            index_buffer_memory_contaner[next_index_gltf],
            a_indices[next_index_gltf]
        );
        ++gltf_counter;
    }
}

void CVulkanRenderer::initialize_vertex_buffers_with_font_data() {
    for (unsigned int i = 0; i < symbol_g_vertices_container.size(); ++i) {
        const unsigned int next_buffer_index = font_indices_container[i];
        std::vector<Vertex> symbol_g_vertices = symbol_g_vertices_container[i];

        create_vertex_buffer(
            font_vertex_buffer_container[next_buffer_index],
            font_vertex_buffer_memory_container[next_buffer_index],
            symbol_g_vertices
        );
        create_index_buffer(
            font_index_buffer_container[next_buffer_index],
            font_index_buffer_memory_contaner[next_buffer_index],
            symbol_g_indices
        );
    }
}

void CVulkanRenderer::clear_vk_image(GpuImage* texture_images) {
    vkDestroySampler(device, texture_images->sampler, nullptr);
    for (unsigned int j = 0; j < texture_images->views.size(); ++j) {
        vkDestroyImageView(device, texture_images->views[j], nullptr);
    }

    texture_images->views.clear();

    vkDestroyImage(device, texture_images->image, nullptr);
    vkFreeMemory(device, texture_images->device_memory, nullptr);
}

void CVulkanRenderer::cleanup_swap_chain() {
    vkDeviceWaitIdle(device);
    vkDestroyImageView(device, main_depth_image_view, nullptr);
    vkDestroyImage(device, main_depth_pipeline_image, nullptr);
    vkFreeMemory(device, main_depth_pipeline_image_memory, nullptr);

    for (VkFramebuffer& framebuffer : swap_chain_framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (VkFramebuffer& framebuffer :
         directional_light_shadow_map_frame_buffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (VkFramebuffer& framebuffer : spot_light_shadow_map_frame_buffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (std::vector<VkFramebuffer>& inner_vector :
         point_light_shadow_map_frame_buffers) {
        for (VkFramebuffer& framebuffer : inner_vector) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
    }

    for (VkImageView& image_view : swap_chain_image_views) {
        vkDestroyImageView(device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(device, swap_chain, nullptr);
}

void CVulkanRenderer::cleanup() {
    cleanup_swap_chain();
    imgui_overlay->shutdown();

    for (unsigned int i = 0, j = 0; i < GPU_DESCRIPTORS.size(); ++j) {
        if (DESCRIPTOR_BINDINGS_CONFIG[j].vk_type
            == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            vkDestroyBuffer(
                device,
                GPU_DESCRIPTORS[i].gpu_buffer->buffer,
                nullptr
            );
            vkFreeMemory(
                device,
                GPU_DESCRIPTORS[i].gpu_buffer->device_memory,
                nullptr
            );
            delete GPU_DESCRIPTORS[i].gpu_buffer;
            GPU_DESCRIPTORS[i].gpu_buffer = nullptr;
        } else if (
            DESCRIPTOR_BINDINGS_CONFIG[j].vk_type
            == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        ) {
            for (unsigned int n = i; n
                 < i + DESCRIPTOR_BINDINGS_CONFIG[j].shader_descriptors_number;
                 ++n) {
                if (GPU_DESCRIPTORS[n].gpu_image->views.size()) {
                    clear_vk_image(GPU_DESCRIPTORS[n].gpu_image);
                }

                delete GPU_DESCRIPTORS[n].gpu_image;
                GPU_DESCRIPTORS[n].gpu_image = nullptr;
            }
        }
        i = i + DESCRIPTOR_BINDINGS_CONFIG[j].shader_descriptors_number;
    }

    vkDestroyBuffer(device, hud_uniform_buffer, nullptr);
    vkFreeMemory(device, hud_uniform_buffers_memory, nullptr);
    vkDestroyBuffer(device, font_uniform_buffer, nullptr);
    vkFreeMemory(device, font_uniform_buffers_memory, nullptr);
    vkDestroyBuffer(device, hud_screen_uniform_buffer, nullptr);
    vkFreeMemory(device, hud_screen_uniform_buffers_memory, nullptr);
    vkDestroyBuffer(device, ui_uniform_buffer, nullptr);
    vkFreeMemory(device, ui_uniform_buffers_memory, nullptr);
    vkDestroyBuffer(device, ui_icons_uniform_buffer, nullptr);
    vkFreeMemory(device, ui_icons_uniform_buffers_memory, nullptr);
    vkDestroyBuffer(
        device,
        shadow_map_directional_light_model_matrix_uniform_buffer,
        nullptr
    );
    vkFreeMemory(
        device,
        shadow_map_directional_light_model_matrix_uniform_buffers_memory,
        nullptr
    );
    vkDestroyBuffer(
        device,
        shadow_map_point_light_model_matrix_uniform_buffer,
        nullptr
    );
    vkFreeMemory(
        device,
        shadow_map_point_light_model_matrix_uniform_buffers_memory,
        nullptr
    );
    vkDestroyBuffer(
        device,
        shadow_map_spot_light_model_matrix_uniform_buffer,
        nullptr
    );
    vkFreeMemory(
        device,
        shadow_map_spot_light_model_matrix_uniform_buffers_memory,
        nullptr
    );
    vkDestroyBuffer(device, virtual_textures_uniform_buffer, nullptr);
    vkFreeMemory(device, virtual_textures_uniform_buffer_memory, nullptr);

    for (size_t j = 0; j < vertex_buffer_container.size(); ++j) {
        vkDestroyBuffer(device, vertex_buffer_container[j], nullptr);
        vkFreeMemory(device, vertex_buffer_memory_container[j], nullptr);
    }
    for (size_t j = 0; j < index_buffer_container.size(); ++j) {
        vkDestroyBuffer(device, index_buffer_container[j], nullptr);
        vkFreeMemory(device, index_buffer_memory_contaner[j], nullptr);
    }
    for (size_t j = 0; j < font_indices_container.size(); ++j) {
        vkDestroyBuffer(
            device,
            font_vertex_buffer_container[font_indices_container[j]],
            nullptr
        );
        vkFreeMemory(
            device,
            font_vertex_buffer_memory_container[font_indices_container[j]],
            nullptr
        );
    }
    for (size_t j = 0; j < font_indices_container.size(); ++j) {
        vkDestroyBuffer(
            device,
            font_index_buffer_container[font_indices_container[j]],
            nullptr
        );
        vkFreeMemory(
            device,
            font_index_buffer_memory_contaner[font_indices_container[j]],
            nullptr
        );
    }
    vkDestroyBuffer(device, model_matrix_uniform_buffer, nullptr);
    vkFreeMemory(device, model_matrix_uniform_buffers_memory, nullptr);
    vkDestroyBuffer(device, light_data_uniform_buffer, nullptr);
    vkFreeMemory(device, light_data_uniform_buffers_memory, nullptr);

    vkDeviceWaitIdle(device);

    for (int i = 0; i < SpecificPipeline::PipelinesNumber; ++i) {
        vkDestroyRenderPass(device, RENDER_PASSES[i], nullptr);
    }

    for (unsigned int i = 0; i < DescriptorSetDataLink::DescriptorChunksNumber;
         ++i) {
        vkDestroyDescriptorSetLayout(
            device,
            DESCRIPTOR_SETS_CONFIG[i].set_layout,
            nullptr
        );
    }
    for (unsigned int i = 0; i < SpecificPipeline::PipelinesNumber; ++i) {
        vkDestroyPipeline(device, PIPELINE_CONFIGS[i].pipeline, nullptr);
        vkDestroyPipelineLayout(
            device,
            PIPELINE_CONFIGS[i].pipeline_layout,
            nullptr
        );
    }

    vkDestroySampler(device, texture_sampler, nullptr);
    vkDestroySampler(device, shadow_map_sampler, nullptr);
    for (unsigned int i = 0; i < texture_images.size(); ++i) {
        vkDestroySampler(device, texture_images[i].sampler, nullptr);
        for (unsigned int j = 0; j < texture_images[i].views.size(); ++j) {
            vkDestroyImageView(device, texture_images[i].views[j], nullptr);
        }

        vkDestroyImage(device, texture_images[i].image, nullptr);
        vkFreeMemory(device, texture_images[i].device_memory, nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
        vkDestroyFence(device, in_flight_fences[i], nullptr);
    }

    for (size_t i = 0; i < swap_chain_images.size(); ++i) {
        vkDestroySemaphore(device, render_finished_semaphores[i], nullptr);
    }

    vkDestroyCommandPool(device, directional_light_command_pool, nullptr);
    vkDestroyCommandPool(device, spot_light_command_pool, nullptr);
    vkDestroyCommandPool(device, point_light_command_pool, nullptr);
    vkDestroyCommandPool(device, main_render_command_pool, nullptr);
    vkDestroyCommandPool(device, font_command_pool, nullptr);
    vkDestroyCommandPool(device, hud_command_pool, nullptr);
    vkDestroyCommandPool(device, hud_screen_command_pool, nullptr);
    vkDestroyCommandPool(device, ui_command_pool, nullptr);
    vkDestroyCommandPool(device, ui_icons_command_pool, nullptr);
    vkDestroyCommandPool(device, virtual_textures_command_pool, nullptr);
    for (uint32_t i = 0; i < secondary_buffers_command_pools.size(); ++i) {
        vkDestroyCommandPool(
            device,
            secondary_buffers_command_pools[i],
            nullptr
        );
    }
    vkDestroyDescriptorPool(device, descriptor_pool, nullptr);

    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, nullptr);

    if (ENABLE_VALIDATION_LAYERS) {
        destroy_debug_utils_messenger_ext(instance, debug_messenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    window->close();
}

void CVulkanRenderer::create_instance() {
    if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support()) {
        throw std::runtime_error(
            "validation layers requested, but not available!"
        );
    }

    VkApplicationInfo app_info {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Grey Lane Vertex Machine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    std::vector<const char*> extensions = get_required_extensions();

    create_info.enabledExtensionCount =
        static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
    if (ENABLE_VALIDATION_LAYERS) {
        create_info.enabledLayerCount =
            static_cast<uint32_t>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        populate_debug_messenger_create_info(debug_create_info);
        create_info.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;

        create_info.pNext = nullptr;
    }

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void CVulkanRenderer::populate_debug_messenger_create_info(
    VkDebugUtilsMessengerCreateInfoEXT& create_info
) {
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
}

void CVulkanRenderer::setup_debug_messenger() {
    if (!ENABLE_VALIDATION_LAYERS) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info;
    populate_debug_messenger_create_info(create_info);

    if (create_debug_utils_messenger_ext(
            instance,
            &create_info,
            nullptr,
            &debug_messenger
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void CVulkanRenderer::create_surface() {
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    if (vkCreateWaylandSurfaceKHR(
            instance,
            &create_wayland_surface_info,
            nullptr,
            &surface
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
    if (vkCreateXlibSurfaceKHR(
            instance,
            &createXlibSurfaceInfo,
            nullptr,
            &surface
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    if (vkCreateXcbSurfaceKHR(instance, &createXcbSurfaceInfo, nullptr, &surface)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (vkCreateWin32SurfaceKHR(
            instance,
            &create_win32_surface_info,
            nullptr,
            &surface
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
#endif
}

void CVulkanRenderer::pick_physical_device() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (device_count == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    for (const VkPhysicalDevice& device : devices) {
        VkPhysicalDeviceProperties prop;
        vkGetPhysicalDeviceProperties(device, &prop);

        if (is_device_suitable(device)) {
            physical_device = device;
            break;
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void CVulkanRenderer::create_logical_device() {
    QueueFamilyIndices indices = find_queue_families(physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {
        indices.graphics_family.value(),
        indices.present_family.value()
    };

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features {};
    device_features.samplerAnisotropy = VK_TRUE;
    device_features.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    create_info.queueCreateInfoCount =
        static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();

    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount =
        static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (ENABLE_VALIDATION_LAYERS) {
        create_info.enabledLayerCount =
            static_cast<uint32_t>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &create_info, nullptr, &device)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(
        device,
        indices.graphics_family.value(),
        0,
        &graphics_queue
    );
    vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue);
}

void CVulkanRenderer::create_swap_chain() {
    SwapChainSupportDetails swap_chain_support =
        query_swap_chain_support(physical_device);
    VkSurfaceFormatKHR surface_format =
        choose_swap_surface_format(swap_chain_support.formats);
    VkPresentModeKHR present_mode =
        choose_swap_present_mode(swap_chain_support.present_modes);
    VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities);
    uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0
        && image_count > swap_chain_support.capabilities.maxImageCount) {
        image_count = swap_chain_support.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    QueueFamilyIndices indices = find_queue_families(physical_device);
    uint32_t queue_family_indices[] = {
        indices.graphics_family.value(),
        indices.present_family.value()
    };
    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swap_chain)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }
    vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr);
    swap_chain_images.resize(image_count);
    vkGetSwapchainImagesKHR(
        device,
        swap_chain,
        &image_count,
        swap_chain_images.data()
    );
    swap_chain_image_format = surface_format.format;
    swap_chain_extent = extent;
    window->width = swap_chain_extent.width;
    window->height = swap_chain_extent.height;
}

void CVulkanRenderer::create_image_views() {
    swap_chain_image_views.resize(swap_chain_images.size());
    for (uint32_t i = 0; i < swap_chain_images.size(); i++) {
        GpuImage swap_chain_image = {
            .image = swap_chain_images[i],
            .view_type = VK_IMAGE_VIEW_TYPE_2D,
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
            .format = swap_chain_image_format,
            .red = VK_COMPONENT_SWIZZLE_IDENTITY,
            .green = VK_COMPONENT_SWIZZLE_IDENTITY,
            .blue = VK_COMPONENT_SWIZZLE_IDENTITY,
            .alpha = VK_COMPONENT_SWIZZLE_IDENTITY,
            .array_layers = 1,
            .width = swap_chain_extent.width,
            .height = swap_chain_extent.height
        };
        swap_chain_image_views[i] = create_image_view(swap_chain_image, 0, 1);
    }
}

void CVulkanRenderer::create_main_render_pass() {
    for (unsigned int j = 0; j < SpecificPipeline::PipelinesNumber; ++j) {
        for (unsigned int i = 0;
             i < RENDER_PASS_CONFIGS[j].actual_attachment_description_number;
             ++i) {
            if (RENDER_PASS_CONFIGS[j].attachment_descriptions[i].finalLayout
                    == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                || RENDER_PASS_CONFIGS[j].attachment_descriptions[i].finalLayout
                    == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                RENDER_PASS_CONFIGS[j].attachment_descriptions[i].format =
                    find_depth_format();
            } else {
                RENDER_PASS_CONFIGS[j].attachment_descriptions[i].format =
                    swap_chain_image_format;
            }
        }
        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        for (unsigned int i = 0;
             i < RENDER_PASS_CONFIGS[j].actual_attachment_reference_number;
             ++i) {
            if (RENDER_PASS_CONFIGS[j].attachment_references[i].layout
                == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                subpass.colorAttachmentCount = 1;
                subpass.pColorAttachments =
                    &RENDER_PASS_CONFIGS[j].attachment_references[i];
            } else if (
                RENDER_PASS_CONFIGS[j].attachment_references[i].layout
                == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            ) {
                subpass.pDepthStencilAttachment =
                    &RENDER_PASS_CONFIGS[j].attachment_references[i];
            }
        }
        VkRenderPassCreateInfo render_pass_info {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = static_cast<uint32_t>(
            RENDER_PASS_CONFIGS[j].actual_attachment_description_number
        );
        render_pass_info.pAttachments =
            RENDER_PASS_CONFIGS[j].attachment_descriptions;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount =
            RENDER_PASS_CONFIGS[j].actual_subpass_dependency_number;
        render_pass_info.pDependencies =
            RENDER_PASS_CONFIGS[j].subpass_dependencies;
        if (vkCreateRenderPass(
                device,
                &render_pass_info,
                nullptr,
                &RENDER_PASSES[j]
            )
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
}

void CVulkanRenderer::create_descriptor_set_layout() {
    for (int descriptor_set_counter = 0;
         descriptor_set_counter < DescriptorSetDataLink::DescriptorChunksNumber;
         ++descriptor_set_counter) {
        DescriptorSet& descriptor_set =
            DESCRIPTOR_SETS_CONFIG[descriptor_set_counter];
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (uint32_t j = 0;
             j < descriptor_set.actual_linked_descriptor_bindings_number;
             ++j) {
            uint32_t current_descriptor_binding_id =
                descriptor_set.descriptors_bindings_i_ds[j];
            VkDescriptorSetLayoutBinding model_matrix_ubo_layout {};
            model_matrix_ubo_layout.binding =
                DESCRIPTOR_BINDINGS_CONFIG[current_descriptor_binding_id]
                    .binding;
            model_matrix_ubo_layout.descriptorCount =
                DESCRIPTOR_BINDINGS_CONFIG[current_descriptor_binding_id]
                    .shader_descriptors_number;
            model_matrix_ubo_layout.descriptorType =
                DESCRIPTOR_BINDINGS_CONFIG[current_descriptor_binding_id]
                    .vk_type;
            model_matrix_ubo_layout.pImmutableSamplers = nullptr;
            model_matrix_ubo_layout.stageFlags =
                DESCRIPTOR_BINDINGS_CONFIG[current_descriptor_binding_id]
                    .shader_stage_flag;

            bindings.push_back(model_matrix_ubo_layout);
        }
        VkDescriptorSetLayoutCreateInfo layout_info {};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.flags = 0;
        layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
        layout_info.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(
                device,
                &layout_info,
                nullptr,
                &descriptor_set.set_layout
            )
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
}

void CVulkanRenderer::create_graphics_pipeline() {
    for (int graphics_pipeline_counter = 0;
         graphics_pipeline_counter < SpecificPipeline::PipelinesNumber;
         ++graphics_pipeline_counter) {
        Pipeline& pipeline = PIPELINE_CONFIGS[graphics_pipeline_counter];
        VkRenderPass render_pass = RENDER_PASSES[graphics_pipeline_counter];
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
        VkShaderModule vert_shader_module;
        VkShaderModule frag_shader_module;
        if (pipeline.vert_shader != nullptr) {
            std::vector<char> vert_shader_code =
                read_file(pipeline.vert_shader);
            vert_shader_module = create_shader_module(vert_shader_code);
            VkPipelineShaderStageCreateInfo vert_shader_stage_info {};
            vert_shader_stage_info.sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vert_shader_stage_info.module = vert_shader_module;
            vert_shader_stage_info.pName = "main";
            shader_stages.push_back(vert_shader_stage_info);
        }
        if (pipeline.frag_shader != nullptr) {
            std::vector<char> frag_shader_code =
                read_file(pipeline.frag_shader);
            frag_shader_module = create_shader_module(frag_shader_code);
            VkPipelineShaderStageCreateInfo frag_shader_stage_info {};
            frag_shader_stage_info.sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_shader_stage_info.module = frag_shader_module;
            frag_shader_stage_info.pName = "main";
            shader_stages.push_back(frag_shader_stage_info);
        }

        VkPipelineVertexInputStateCreateInfo vertex_input_info {};
        vertex_input_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 1;
        vertex_input_info.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(pipeline.attribute_descriptions.size());
        vertex_input_info.pVertexBindingDescriptions =
            &pipeline.binding_description;
        vertex_input_info.pVertexAttributeDescriptions =
            pipeline.attribute_descriptions.data();
        VkPipelineInputAssemblyStateCreateInfo input_assembly {};
        input_assembly.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;
        VkPipelineViewportStateCreateInfo viewport_state {};
        viewport_state.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;
        VkPipelineRasterizationStateCreateInfo rasterizer {};
        rasterizer.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        bool is_shadow_map_pipeline = graphics_pipeline_counter
                == SpecificPipeline::DirectionalLightPipeline
            || graphics_pipeline_counter == SpecificPipeline::SpotLightPipeline
            || graphics_pipeline_counter
                == SpecificPipeline::PointLightPipeline;
        // Meshes are CW-wound; main pass keeps outer faces (cull "front"),
        // shadow pass keeps far-side faces (cull "back") so objects do not
        // self-shadow.
        if (is_shadow_map_pipeline) {
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        } else if (
            graphics_pipeline_counter == SpecificPipeline::MainRenderPipeline
        ) {
            rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
        } else {
            // 2D/UI pipelines (HUD, crosshair, fonts, SDF) draw screen-space
            // quads; culling them makes the sprites disappear.
            rasterizer.cullMode = VK_CULL_MODE_NONE;
        }
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        VkPipelineMultisampleStateCreateInfo multisampling {};
        multisampling.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPipelineDepthStencilStateCreateInfo depth_stencil {};
        depth_stencil.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable = VK_TRUE;
        depth_stencil.depthWriteEnable = VK_TRUE;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.stencilTestEnable = VK_FALSE;
        VkPipelineColorBlendAttachmentState color_blend_attachment {};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;
        VkPipelineColorBlendStateCreateInfo color_blending {};
        color_blending.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;
        color_blending.blendConstants[0] = 0.0f;
        color_blending.blendConstants[1] = 0.0f;
        color_blending.blendConstants[2] = 0.0f;
        color_blending.blendConstants[3] = 0.0f;
        std::vector<VkDynamicState> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamic_state {};
        dynamic_state.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount =
            static_cast<uint32_t>(dynamic_states.size());
        dynamic_state.pDynamicStates = dynamic_states.data();
        // Need to access inside pipeline and take ID for specific descriptor
        // set, then with that ID we got descriptor set and take it's layout.
        unsigned int descriptor_layouts_number =
            pipeline.actual_linked_descriptor_sets_number;
        std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
        for (unsigned i = 0; i < descriptor_layouts_number; ++i) {
            descriptor_set_layouts.push_back(
                DESCRIPTOR_SETS_CONFIG[pipeline.linked_descriptor_set_i_ds[i]]
                    .set_layout
            );
        }
        VkPipelineLayoutCreateInfo pipeline_layout_info {};
        pipeline_layout_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = descriptor_layouts_number;
        pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
        if (vkCreatePipelineLayout(
                device,
                &pipeline_layout_info,
                nullptr,
                &pipeline.pipeline_layout
            )
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        VkGraphicsPipelineCreateInfo pipeline_info {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = shader_stages.size();
        pipeline_info.pStages = shader_stages.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState = &multisampling;
        pipeline_info.pDepthStencilState = &depth_stencil;
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDynamicState = &dynamic_state;
        pipeline_info.layout = pipeline.pipeline_layout;
        pipeline_info.renderPass = render_pass;
        pipeline_info.subpass = 0;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        if (vkCreateGraphicsPipelines(
                device,
                VK_NULL_HANDLE,
                1,
                &pipeline_info,
                nullptr,
                &pipeline.pipeline
            )
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        if (pipeline.vert_shader != nullptr) {
            vkDestroyShaderModule(device, vert_shader_module, nullptr);
        }
        if (pipeline.frag_shader != nullptr) {
            vkDestroyShaderModule(device, frag_shader_module, nullptr);
        }
    }
}

void CVulkanRenderer::create_framebuffers() {
    // Main renderer frame buffers initialization.
    swap_chain_framebuffers.resize(swap_chain_image_views.size());
    for (size_t i = 0; i < swap_chain_image_views.size(); ++i) {
        std::vector<VkImageView> main_render_attachments;
        main_render_attachments.push_back(swap_chain_image_views[i]);
        main_render_attachments.push_back(main_depth_image_view);
        create_render_pass_framebuffers(
            main_render_attachments,
            RENDER_PASSES[SpecificPipeline::MainRenderPipeline],
            swap_chain_framebuffers[i],
            swap_chain_extent.width,
            swap_chain_extent.height
        );
    }
    // Directional lights shadow map renderer frame buffers initialization.
    unsigned int directional_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_i_ds[1];
    directional_light_shadow_map_frame_buffers.resize(DIRECTIONAL_LIGHTS_NUMBER);
    for (size_t i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
        std::vector<VkImageView> directional_lights_render_attachments;
        directional_lights_render_attachments.push_back(
            (*GPU_DESCRIPTORS
                  [DESCRIPTOR_BINDINGS_CONFIG
                       [directional_light_descriptor_binding_index]
                           .global_descriptor_offset
                   + i]
                      .gpu_image)
                .views[0]
        );
        create_render_pass_framebuffers(
            directional_lights_render_attachments,
            RENDER_PASSES[SpecificPipeline::DirectionalLightPipeline],
            directional_light_shadow_map_frame_buffers[i],
            FLAT_SHADOW_MAP_SIZE,
            FLAT_SHADOW_MAP_SIZE
        );
    }
    // Spot lights shadow map renderer frame buffers initialization.
    unsigned int spot_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_i_ds[3];
    spot_light_shadow_map_frame_buffers.resize(SPOT_LIGHTS_NUMBER);
    for (size_t i = 0; i < SPOT_LIGHTS_NUMBER; ++i) {
        std::vector<VkImageView> spot_lights_render_attachments;
        spot_lights_render_attachments.push_back(
            (*GPU_DESCRIPTORS
                  [DESCRIPTOR_BINDINGS_CONFIG[spot_light_descriptor_binding_index]
                       .global_descriptor_offset
                   + i]
                      .gpu_image)
                .views[0]
        );
        create_render_pass_framebuffers(
            spot_lights_render_attachments,
            RENDER_PASSES[SpecificPipeline::SpotLightPipeline],
            spot_light_shadow_map_frame_buffers[i],
            FLAT_SHADOW_MAP_SIZE,
            FLAT_SHADOW_MAP_SIZE
        );
    }
    // Point lights shadow map renderer frame buffers initialization.
    unsigned int descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_i_ds[2];
    point_light_shadow_map_frame_buffers.resize(POINT_LIGHTS_NUMBER);
    for (size_t j = 0; j < POINT_LIGHTS_NUMBER; ++j) {
        for (size_t m = 0; m < 6; ++m) {
            std::vector<VkImageView> point_lights_render_attachments;
            point_lights_render_attachments.push_back(
                (*GPU_DESCRIPTORS
                      [DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                           .global_descriptor_offset
                       + j]
                          .gpu_image)
                    .views[m]
            );
            point_light_shadow_map_frame_buffers[j].push_back({});
            create_render_pass_framebuffers(
                point_lights_render_attachments,
                RENDER_PASSES[SpecificPipeline::PointLightPipeline],
                point_light_shadow_map_frame_buffers[j][m],
                SHADOW_MAP_SIZE,
                SHADOW_MAP_SIZE
            );
        }
    }
}

void CVulkanRenderer::create_render_pass_framebuffers(
    std::vector<VkImageView>& attachments,
    VkRenderPass& render_pass,
    VkFramebuffer& swap_chain_framebuffer,
    uint32_t width,
    uint32_t height
) {
    VkFramebufferCreateInfo framebuffer_info {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount =
        static_cast<uint32_t>(attachments.size());
    framebuffer_info.pAttachments = attachments.data();
    framebuffer_info.width = width;
    framebuffer_info.height = height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(
            device,
            &framebuffer_info,
            nullptr,
            &swap_chain_framebuffer
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void CVulkanRenderer::create_command_pool(VkCommandPool& command_pools) {
    QueueFamilyIndices queue_family_indices =
        find_queue_families(physical_device);

    VkCommandPoolCreateInfo pool_info {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

    if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pools)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void CVulkanRenderer::create_depth_resources() {
    VkFormat depth_format = find_depth_format();

    GpuImage depth_image = {
        .image = VkImage {},
        .device_memory = VkDeviceMemory {},
        .view_type = VK_IMAGE_VIEW_TYPE_2D,
        .create_flags = 0,
        .memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT,
        .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT,
        .format = depth_format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .array_layers = 1,
        .width = swap_chain_extent.width,
        .height = swap_chain_extent.height,
    };

    create_image(depth_image);
    main_depth_pipeline_image = depth_image.image;
    main_depth_pipeline_image_memory = depth_image.device_memory;
    main_depth_image_view = create_image_view(depth_image, 0, 1);
}

void CVulkanRenderer::create_directional_light_shadow_map_depth_resources() {
    unsigned int directional_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_i_ds[1];
    for (unsigned int i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
        GpuImage depth_image = {
            .image = VkImage {},
            .device_memory = VkDeviceMemory {},
            .view_type = VK_IMAGE_VIEW_TYPE_2D,
            .create_flags = 0,
            .memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT,
            .format = find_depth_format(),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .array_layers = 1,
            .width = FLAT_SHADOW_MAP_SIZE,
            .height = FLAT_SHADOW_MAP_SIZE,
        };

        create_image(depth_image);

        VkCommandBuffer command_buffer =
            begin_single_time_commands(main_render_command_pool);

        VkImageMemoryBarrier barrier {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = depth_image.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        source_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        vkCmdPipelineBarrier(
            command_buffer,
            source_stage,
            destination_stage,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        end_single_time_commands(main_render_command_pool, command_buffer);

        depth_image.views.push_back(create_image_view(depth_image, 0, 1));
        set_image_debug_object_name(device, depth_image, "directional light");
        *GPU_DESCRIPTORS
             [DESCRIPTOR_BINDINGS_CONFIG[directional_light_descriptor_binding_index]
                  .global_descriptor_offset
              + i]
                 .gpu_image = depth_image;
    }
}

void CVulkanRenderer::create_spot_light_shadow_map_depth_resources() {
    unsigned int spot_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_i_ds[3];
    for (unsigned int i = 0; i < SPOT_LIGHTS_NUMBER; ++i) {
        GpuImage depth_image = {
            .image = VkImage {},
            .device_memory = VkDeviceMemory {},
            .view_type = VK_IMAGE_VIEW_TYPE_2D,
            .create_flags = 0,
            .memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT,
            .format = find_depth_format(),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .array_layers = 1,
            .width = FLAT_SHADOW_MAP_SIZE,
            .height = FLAT_SHADOW_MAP_SIZE,
        };

        create_image(depth_image);

        VkCommandBuffer command_buffer =
            begin_single_time_commands(main_render_command_pool);

        VkImageMemoryBarrier barrier {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = depth_image.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        source_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        vkCmdPipelineBarrier(
            command_buffer,
            source_stage,
            destination_stage,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        end_single_time_commands(main_render_command_pool, command_buffer);

        depth_image.views.push_back(create_image_view(depth_image, 0, 1));
        set_image_debug_object_name(device, depth_image, "spot light");
        *GPU_DESCRIPTORS
             [DESCRIPTOR_BINDINGS_CONFIG[spot_light_descriptor_binding_index]
                  .global_descriptor_offset
              + i]
                 .gpu_image = depth_image;
    }
}

void CVulkanRenderer::create_point_light_shadow_map_depth_resources() {
    unsigned int descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_i_ds[2];
    for (unsigned int i = 0; i < POINT_LIGHTS_NUMBER; ++i) {
        GpuImage depth_image = {
            .image = VkImage {},
            .device_memory = VkDeviceMemory {},
            .view_type = VK_IMAGE_VIEW_TYPE_2D,
            .create_flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
            .memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                | VK_IMAGE_USAGE_SAMPLED_BIT,
            .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT,
            .format = find_depth_format(),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .array_layers = 6,
            .width = SHADOW_MAP_SIZE,
            .height = SHADOW_MAP_SIZE
        };

        create_image(depth_image);

        for (unsigned int j = 0; j < 6; ++j) {
            VkCommandBuffer command_buffer =
                begin_single_time_commands(main_render_command_pool);

            VkImageMemoryBarrier barrier {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = depth_image.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 6;

            VkPipelineStageFlags source_stage;
            VkPipelineStageFlags destination_stage;

            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = 0;

            source_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            vkCmdPipelineBarrier(
                command_buffer,
                source_stage,
                destination_stage,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier
            );

            end_single_time_commands(main_render_command_pool, command_buffer);

            depth_image.views.push_back(create_image_view(depth_image, j, 1));
        }

        set_image_debug_object_name(device, depth_image, "point light laryer");
        *GPU_DESCRIPTORS
             [DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                  .global_descriptor_offset
              + i]
                 .gpu_image = depth_image;
    }

    for (unsigned int i = 0; i < POINT_LIGHTS_NUMBER; ++i) {
        (*GPU_DESCRIPTORS
              [DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                   .global_descriptor_offset
               + i]
                  .gpu_image)
            .view_type = VK_IMAGE_VIEW_TYPE_CUBE;

        set_image_debug_object_name(
            device,
            *GPU_DESCRIPTORS
                 [DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                      .global_descriptor_offset
                  + i]
                     .gpu_image,
            "point light cube"
        );
        (*GPU_DESCRIPTORS
              [DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                   .global_descriptor_offset
               + i]
                  .gpu_image)
            .views.push_back(create_image_view(
                *GPU_DESCRIPTORS
                     [DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                          .global_descriptor_offset
                      + i]
                         .gpu_image,
                0,
                6
            ));
    }
}

VkFormat CVulkanRenderer::find_supported_format(
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features
) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR
            && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (
            tiling == VK_IMAGE_TILING_OPTIMAL
            && (props.optimalTilingFeatures & features) == features
        ) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat CVulkanRenderer::find_depth_format() {
    return find_supported_format(
        {VK_FORMAT_D32_SFLOAT,
         VK_FORMAT_D32_SFLOAT_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool CVulkanRenderer::has_stencil_component(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT
        || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void CVulkanRenderer::create_texture_image_view() {
    unsigned int readable_texture_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::RidableTextures]
            .descriptors_bindings_i_ds[0];
    for (unsigned int i = 0; i < initialize_texture_data.size(); ++i) {
        GpuImage* image = GPU_DESCRIPTORS
                              [DESCRIPTOR_BINDINGS_CONFIG
                                   [readable_texture_descriptor_binding_index]
                                       .global_descriptor_offset
                               + i]
                                  .gpu_image;
        image->views.push_back(create_image_view(*image, 0, 1));
    }
}

void CVulkanRenderer::create_texture_sampler() {
    VkPhysicalDeviceProperties properties {};
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    VkSamplerCreateInfo sampler_info {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    texture_sampler = {}; /// TODO: Is it realy need here?
    if (vkCreateSampler(device, &sampler_info, nullptr, &texture_sampler)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void CVulkanRenderer::create_shadow_map_sampler() {
    VkSamplerCreateInfo sampler_info {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    if (vkCreateSampler(device, &sampler_info, nullptr, &shadow_map_sampler)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create shadow map sampler!");
    }
}

VkImageView CVulkanRenderer::create_image_view(
    GpuImage image,
    uint32_t base_array_layers,
    uint32_t layer_count
) {
    VkImageViewCreateInfo view_info {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image.image;
    view_info.viewType = image.view_type;
    view_info.format = image.format;
    view_info.components.r = image.red;
    view_info.components.g = image.green;
    view_info.components.b = image.blue;
    view_info.components.a = image.alpha;
    view_info.subresourceRange.aspectMask = image.aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = base_array_layers;
    view_info.subresourceRange.layerCount = layer_count;

    VkImageView image_view;
    if (vkCreateImageView(device, &view_info, nullptr, &image_view)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return image_view;
}

void CVulkanRenderer::create_image(GpuImage& image) {
    VkImageCreateInfo image_info {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = image.width;
    image_info.extent.height = image.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = image.array_layers;
    image_info.format = image.format;
    image_info.tiling = image.tiling;
    image_info.usage = image.usage_flags;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = image.create_flags;

    if (vkCreateImage(device, &image_info, nullptr, &image.image)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device, image.image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(
        mem_requirements.memoryTypeBits,
        image.memory_property_flags
    );

    if (vkAllocateMemory(device, &alloc_info, nullptr, &image.device_memory)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image.image, image.device_memory, 0);
}

void CVulkanRenderer::transition_image_layout(
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout
) {
    VkCommandBuffer command_buffer =
        begin_single_time_commands(main_render_command_pool);

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED
        && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (
        old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    ) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        command_buffer,
        source_stage,
        destination_stage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );

    end_single_time_commands(main_render_command_pool, command_buffer);
}

void CVulkanRenderer::transition_shadow_map_image_layout(
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout
) {
    VkCommandBuffer command_buffer =
        begin_single_time_commands(directional_light_command_pool);

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED
        && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (
        old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    ) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        command_buffer,
        source_stage,
        destination_stage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier
    );

    end_single_time_commands(directional_light_command_pool, command_buffer);
}

void CVulkanRenderer::copy_buffer_to_image(
    VkBuffer& buffer,
    VkImage image,
    uint32_t width,
    uint32_t height
) {
    VkCommandBuffer command_buffer =
        begin_single_time_commands(main_render_command_pool);

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        command_buffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    end_single_time_commands(main_render_command_pool, command_buffer);
}

void CVulkanRenderer::create_vertex_buffer(
    VkBuffer& dst_vertex_buffer,
    VkDeviceMemory& dst_vertex_buffer_memory,
    std::vector<Vertex>& vertex_data
) {
    VkDeviceSize buffer_size = sizeof(vertex_data[0]) * vertex_data.size();
    if (vertex_data.size() == 0) {
        buffer_size = 1;
    }

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer,
        staging_buffer_memory
    );

    void* data;
    vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
    if (vertex_data.size() > 0) {
        memcpy(data, vertex_data.data(), (size_t)buffer_size);
    }
    vkUnmapMemory(device, staging_buffer_memory);

    create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        dst_vertex_buffer,
        dst_vertex_buffer_memory
    );

    copy_buffer(staging_buffer, dst_vertex_buffer, buffer_size);

    vkDestroyBuffer(device, staging_buffer, nullptr);
    vkFreeMemory(device, staging_buffer_memory, nullptr);
}

void CVulkanRenderer::create_index_buffer(
    VkBuffer& dst_index_buffer,
    VkDeviceMemory& dst_index_buffer_memory,
    const std::vector<uint32_t>& index_data
) {
    VkDeviceSize buffer_size = sizeof(index_data[0]) * index_data.size();
    if (index_data.empty()) {
        buffer_size = 1;
    }

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer,
        staging_buffer_memory
    );

    void* data;
    vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
    if (!index_data.empty()) {
        memcpy(data, index_data.data(), (size_t)buffer_size);
    }
    vkUnmapMemory(device, staging_buffer_memory);

    create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        dst_index_buffer,
        dst_index_buffer_memory
    );

    copy_buffer(staging_buffer, dst_index_buffer, buffer_size);

    vkDestroyBuffer(device, staging_buffer, nullptr);
    vkFreeMemory(device, staging_buffer_memory, nullptr);
}

void CVulkanRenderer::create_main_render_uniform_buffers() {
    for (unsigned int descriptor_set_config_counter = 0;
         descriptor_set_config_counter
         < DescriptorSetDataLink::DescriptorChunksNumber;
         ++descriptor_set_config_counter) {
        for (unsigned int j = 0;
             j < DESCRIPTOR_SETS_CONFIG[descriptor_set_config_counter]
                     .actual_linked_descriptor_bindings_number;
             ++j) {
            unsigned int descriptor_binding_index =
                DESCRIPTOR_SETS_CONFIG[descriptor_set_config_counter]
                    .descriptors_bindings_i_ds[j];
            VkDescriptorType descriptor_type =
                DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index].vk_type;
            if (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                uint32_t memory =
                    DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                        .ubo_chunk_size
                    * DESCRIPTOR_SETS_CONFIG[descriptor_set_config_counter]
                          .host_descriptor_number;
                create_buffer(
                    memory,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    GPU_DESCRIPTORS
                        [DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                             .global_descriptor_offset]
                            .gpu_buffer->buffer,
                    GPU_DESCRIPTORS
                        [DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                             .global_descriptor_offset]
                            .gpu_buffer->device_memory
                );
            } else {
                continue;
            }
        }
    }
}

void CVulkanRenderer::create_main_render_descriptor_pool() {
    std::array<VkDescriptorPoolSize, 2> pool_sizes {};

    uint32_t descriptor_count = 10000;
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = static_cast<uint32_t>(descriptor_count);
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = static_cast<uint32_t>(descriptor_count);

    VkDescriptorPoolCreateInfo pool_info {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = static_cast<uint32_t>(descriptor_count);

    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void CVulkanRenderer::allocate_descriptor_sets(
    std::vector<VkDescriptorSet>& descriptor_sets,
    VkDescriptorSetLayout set_layout,
    const unsigned int descriptor_sets_number,
    const unsigned int descriptor_offset
) {
    std::vector<VkDescriptorSetLayout> matrix_ubo_layouts(
        descriptor_sets_number,
        set_layout
    );
    VkDescriptorSetAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount =
        static_cast<uint32_t>(descriptor_sets_number);
    alloc_info.pSetLayouts = matrix_ubo_layouts.data();
    if (vkAllocateDescriptorSets(
            device,
            &alloc_info,
            descriptor_sets.data() + descriptor_offset
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}

void CVulkanRenderer::update_descriptor_sets_ubo(
    VkBuffer ubo,
    const VkDeviceSize& ubo_struct_size,
    const unsigned int& ubo_descriptors_number,
    int ubo_binding,
    std::vector<VkDescriptorSet>& ubo_descriptor_sets,
    const unsigned int offset
) {
    for (size_t i = 0; i < ubo_descriptors_number; ++i) {
        VkDescriptorBufferInfo model_matrix_buffer_info =
            create_descriptor_buffer_info(ubo, ubo_struct_size, i);
        std::array<VkWriteDescriptorSet, 1> descriptor_writes {};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet =
            *(DESCRIPTOR_SETS_CHUNKS.data() + offset + i);
        descriptor_writes[0].dstBinding = ubo_binding;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &model_matrix_buffer_info;

        vkUpdateDescriptorSets(
            device,
            static_cast<uint32_t>(descriptor_writes.size()),
            descriptor_writes.data(),
            0,
            nullptr
        );
    }
}

void CVulkanRenderer::update_light_data_descriptor_sets(
    const DescriptorSet& current_descriptor_set1
) {
    const unsigned int linked_descriptor_set_bindings_number =
        current_descriptor_set1.actual_linked_descriptor_bindings_number;
    std::vector<uint32_t> shader_bindings;
    std::vector<uint32_t> descriptor_number_per_binding;
    std::vector<uint32_t> bindings_i_ds;
    for (size_t j = 0; j < linked_descriptor_set_bindings_number; ++j) {
        shader_bindings.push_back(
            DESCRIPTOR_BINDINGS_CONFIG[current_descriptor_set1
                                           .descriptors_bindings_i_ds[j]]
                .binding
        );
        bindings_i_ds.push_back(
            current_descriptor_set1.descriptors_bindings_i_ds[j]
        );
    }

    for (size_t i = 0; i < current_descriptor_set1.host_descriptor_number;
         ++i) {
        std::vector<VkWriteDescriptorSet> descriptor_writes;
        descriptor_writes.resize(shader_bindings.size());
        std::vector<VkDescriptorBufferInfo> descriptor_buffer_infos;
        std::vector<std::vector<VkDescriptorImageInfo>> descriptor_image_infos;
        for (size_t j = 0; j < shader_bindings.size(); ++j) {
            if (DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]].vk_type
                == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                descriptor_buffer_infos.push_back({});

                for (size_t m = 0;
                     m < DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]]
                             .shader_descriptors_number;
                     ++m) {
                    descriptor_buffer_infos[j] = create_descriptor_buffer_info(
                        GPU_DESCRIPTORS
                            [DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]]
                                 .global_descriptor_offset]
                                .gpu_buffer->buffer,
                        DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]]
                            .ubo_chunk_size,
                        i
                    );
                }
                descriptor_writes[j].pBufferInfo =
                    descriptor_buffer_infos.data();
            } else if (
                DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]].vk_type
                == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            ) {
                descriptor_image_infos.push_back({});

                for (size_t m = 0;
                     m < DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]]
                             .shader_descriptors_number;
                     ++m) {
                    uint32_t image_view_index =
                        GPU_DESCRIPTORS
                            [DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]]
                                 .global_descriptor_offset
                             + m]
                                .gpu_image->views.size()
                        - 1;

                    descriptor_image_infos[descriptor_image_infos.size() - 1]
                        .push_back({});
                    descriptor_image_infos[descriptor_image_infos.size() - 1][m] =
                        create_descriptor_image_info(
                            *GPU_DESCRIPTORS
                                 [DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]]
                                      .global_descriptor_offset
                                  + m]
                                     .gpu_image,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                            image_view_index,
                            shadow_map_sampler
                        );
                }
                descriptor_writes[j].pImageInfo =
                    descriptor_image_infos[descriptor_image_infos.size() - 1]
                        .data();
            }
            descriptor_writes[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[j].dstSet =
                *(DESCRIPTOR_SETS_CHUNKS.data()
                  + current_descriptor_set1.descriptor_set_offset + i);
            descriptor_writes[j].dstBinding = shader_bindings[j];
            descriptor_writes[j].dstArrayElement = 0;
            descriptor_writes[j].descriptorType =
                DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]].vk_type;
            descriptor_writes[j].descriptorCount =
                DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]]
                    .shader_descriptors_number;
        }

        vkUpdateDescriptorSets(
            device,
            static_cast<uint32_t>(descriptor_writes.size()),
            descriptor_writes.data(),
            0,
            nullptr
        );
    }
}

void CVulkanRenderer::update_descriptor_sets_combined_image_sampler(
    const DescriptorSet& descriptor_set
) {
    std::vector<uint32_t> bindings_i_ds;
    for (size_t j = 0;
         j < descriptor_set.actual_linked_descriptor_bindings_number;
         ++j) {
        bindings_i_ds.push_back(descriptor_set.descriptors_bindings_i_ds[j]);
    }

    unsigned int readable_texture_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::RidableTextures]
            .descriptors_bindings_i_ds[0];
    for (size_t i = 0; i < descriptor_set.host_descriptor_number; ++i) {
        const unsigned int texture_index = i / 2;
        constexpr unsigned int TEXTURE_VIEW_INDEX = 0;
        VkDescriptorImageInfo image_info = create_descriptor_image_info(
            *GPU_DESCRIPTORS
                 [DESCRIPTOR_BINDINGS_CONFIG
                      [readable_texture_descriptor_binding_index]
                          .global_descriptor_offset
                  + texture_index]
                     .gpu_image,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            TEXTURE_VIEW_INDEX,
            texture_sampler
        );
        std::vector<VkWriteDescriptorSet> descriptor_writes {};

        for (unsigned int j = 0; j < bindings_i_ds.size(); ++j) {
            descriptor_writes.push_back({});
            const unsigned int last_element = descriptor_writes.size() - 1;
            descriptor_writes[last_element].sType =
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[last_element].dstSet =
                *(DESCRIPTOR_SETS_CHUNKS.data()
                  + descriptor_set.descriptor_set_offset + i);
            descriptor_writes[last_element].dstBinding =
                DESCRIPTOR_BINDINGS_CONFIG[bindings_i_ds[j]].binding;
            descriptor_writes[last_element].dstArrayElement = 0;
            descriptor_writes[last_element].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_writes[last_element].descriptorCount = 1;
            descriptor_writes[last_element].pImageInfo = &image_info;
        }
        vkUpdateDescriptorSets(
            device,
            static_cast<uint32_t>(descriptor_writes.size()),
            descriptor_writes.data(),
            0,
            nullptr
        );
    }
}

void CVulkanRenderer::create_descriptor_image_info(
    const unsigned int descriptor_number,
    VkImageLayout image_layout,
    std::vector<GpuImage>& texture_images,
    const unsigned int image_view_index,
    VkDescriptorImageInfo descriptor_image_infos[]
) {
    for (size_t i = 0; i < descriptor_number; ++i) {
        descriptor_image_infos[i] = {};
        descriptor_image_infos[i].imageLayout = image_layout;
        descriptor_image_infos[i].imageView =
            texture_images[i].views[image_view_index];
        descriptor_image_infos[i].sampler = texture_sampler;
    }
}

void CVulkanRenderer::create_main_render_descriptor_sets() {
    vkResetDescriptorPool(device, descriptor_pool, 0);
    for (unsigned int pipeline_counter = 0;
         pipeline_counter < SpecificPipeline::PipelinesNumber;
         ++pipeline_counter) {
        for (unsigned int descriptor_set_counter = 0;
             descriptor_set_counter < PIPELINE_CONFIGS[pipeline_counter]
                                          .actual_linked_descriptor_sets_number;
             ++descriptor_set_counter) {
            const unsigned int linked_descriptor_set_matrix_ubo_id =
                PIPELINE_CONFIGS[pipeline_counter]
                    .linked_descriptor_set_i_ds[descriptor_set_counter];
            const DescriptorSet& current_descriptor_set0 =
                DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_matrix_ubo_id];
            allocate_descriptor_sets(
                DESCRIPTOR_SETS_CHUNKS,
                current_descriptor_set0.set_layout,
                current_descriptor_set0.host_descriptor_number,
                current_descriptor_set0.descriptor_set_offset
            );
            if (current_descriptor_set0.is_texture) {
                update_descriptor_sets_combined_image_sampler(
                    current_descriptor_set0
                );
            } else {
                update_light_data_descriptor_sets(current_descriptor_set0);
            }
        }
    }
}

void CVulkanRenderer::create_buffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& buffer_memory
) {
    VkBufferCreateInfo buffer_info {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    if (size == 0) {
        size = 1;
    }
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device, buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex =
        find_memory_type(mem_requirements.memoryTypeBits, properties);

    int32_t result =
        vkAllocateMemory(device, &alloc_info, nullptr, &buffer_memory);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, buffer_memory, 0);
}

VkCommandBuffer CVulkanRenderer::begin_single_time_commands(
    VkCommandPool& command_pool
) {
    VkCommandBufferAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void CVulkanRenderer::end_single_time_commands(
    VkCommandPool& command_pool,
    VkCommandBuffer& command_buffer
) {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

void CVulkanRenderer::copy_buffer(
    VkBuffer& src_buffer,
    VkBuffer& dst_buffer,
    VkDeviceSize size
) {
    VkCommandBuffer command_buffer =
        begin_single_time_commands(main_render_command_pool);

    VkBufferCopy copy_region {};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    end_single_time_commands(main_render_command_pool, command_buffer);
}

uint32_t CVulkanRenderer::find_memory_type(
    uint32_t type_filter,
    VkMemoryPropertyFlags properties
) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i))
            && (mem_properties.memoryTypes[i].propertyFlags & properties)
                == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void CVulkanRenderer::create_command_buffers(
    VkCommandPool& command_pool,
    std::vector<VkCommandBuffer>& command_buffers,
    uint32_t command_buffers_number,
    VkCommandBufferLevel command_buffer_level_flag
) {
    command_buffers.resize(command_buffers_number * MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = command_buffer_level_flag;
    alloc_info.commandBufferCount = (uint32_t)command_buffers.size();

    if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data())
        != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CVulkanRenderer::execute_secondary_command_buffer(
    VkRenderPass render_pass,
    VkFramebuffer frame_buffer,
    VkExtent2D extent,
    VkCommandBuffer primary_command_buffer,
    VkCommandBuffer secondary_command_buffer
) {
    VkClearValue shadow_map_clear_values[1];
    shadow_map_clear_values[0].depthStencil.depth = 1.0f;
    shadow_map_clear_values[0].depthStencil.stencil = 0;

    VkRenderPassBeginInfo shadow_map_render_pass_info {};
    shadow_map_render_pass_info.sType =
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    shadow_map_render_pass_info.pNext = NULL;
    shadow_map_render_pass_info.renderPass = render_pass;
    shadow_map_render_pass_info.framebuffer = frame_buffer;
    shadow_map_render_pass_info.renderArea.offset.x = 0;
    shadow_map_render_pass_info.renderArea.offset.y = 0;
    shadow_map_render_pass_info.renderArea.extent.width = extent.width;
    shadow_map_render_pass_info.renderArea.extent.height = extent.height;
    shadow_map_render_pass_info.clearValueCount = 1;
    shadow_map_render_pass_info.pClearValues = shadow_map_clear_values;

    vkCmdBeginRenderPass(
        primary_command_buffer,
        &shadow_map_render_pass_info,
        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
    );
    vkCmdExecuteCommands(primary_command_buffer, 1, &secondary_command_buffer);
    vkCmdEndRenderPass(primary_command_buffer);
}

void CVulkanRenderer::update_hud_ubo(
    uint32_t offset,
    bool is_hud_exists,
    float highest_y,
    uint32_t health_counter
) {
    HudUbo hud_ubo {};

    hud_ubo.view = view_matrix;
    hud_ubo.proj = projection_matrix;

    hud_ubo.is_hud_exists = is_hud_exists;
    hud_ubo.current_hp = health_bars[health_counter].current_health;
    hud_ubo.max_hp = health_bars[health_counter].max_health;
    hud_ubo.entity_position = health_bars[health_counter].position;
    hud_ubo.highest_y = highest_y;

    void* hud_matrix_data;
    unsigned int hud_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::HUD]
            .descriptors_bindings_i_ds[0];
    vkMapMemory(
        device,
        GPU_DESCRIPTORS
            [DESCRIPTOR_BINDINGS_CONFIG[hud_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->device_memory,
        sizeof(HudUbo) * offset,
        sizeof(HudUbo),
        0,
        &hud_matrix_data
    );
    memcpy(hud_matrix_data, &hud_ubo, sizeof(HudUbo));
    vkUnmapMemory(
        device,
        GPU_DESCRIPTORS
            [DESCRIPTOR_BINDINGS_CONFIG[hud_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->device_memory
    );
}

void CVulkanRenderer::update_hud_screen_ubo(uint32_t offset, uint32_t crosshair) {
    HudScreenUbo hud_ubo {};
    hud_ubo.model = crosshairs[crosshair].model;

    void* hud_matrix_data;
    unsigned int hud_screen_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::HudScreen]
            .descriptors_bindings_i_ds[0];
    vkMapMemory(
        device,
        GPU_DESCRIPTORS
            [DESCRIPTOR_BINDINGS_CONFIG[hud_screen_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->device_memory,
        sizeof(HudScreenUbo) * offset,
        sizeof(HudScreenUbo),
        0,
        &hud_matrix_data
    );
    memcpy(hud_matrix_data, &hud_ubo, sizeof(HudScreenUbo));
    vkUnmapMemory(
        device,
        GPU_DESCRIPTORS
            [DESCRIPTOR_BINDINGS_CONFIG[hud_screen_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->device_memory
    );
}

void CVulkanRenderer::update_sdf_ubo(uint32_t offset, uint32_t crosshair) {
    SdfUbo hud_ubo {};
    hud_ubo.model = crosshairs[crosshair].model;

    float current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start_time
                         )
                             .count()
        * 0.001;
    hud_ubo.i_time = current_time;

    void* hud_matrix_data;
    unsigned int hud_screen_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::SdfData]
            .descriptors_bindings_i_ds[0];
    vkMapMemory(
        device,
        GPU_DESCRIPTORS
            [DESCRIPTOR_BINDINGS_CONFIG[hud_screen_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->device_memory,
        sizeof(SdfUbo) * offset,
        sizeof(SdfUbo),
        0,
        &hud_matrix_data
    );
    memcpy(hud_matrix_data, &hud_ubo, sizeof(SdfUbo));
    vkUnmapMemory(
        device,
        GPU_DESCRIPTORS
            [DESCRIPTOR_BINDINGS_CONFIG[hud_screen_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->device_memory
    );
}

void CVulkanRenderer::update_ubo_ui(
    const unsigned int current_inventory_row,
    const unsigned int current_inventory_column,
    const unsigned int inventory,
    uint32_t offset
) {
    UiUbo hud_ubo {};

    const unsigned int col_size = inventories[inventory].col;
    hud_ubo.model =
        inventories[inventory]
            .slot_data[col_size * current_inventory_row + current_inventory_column]
            .model;
    hud_ubo.color =
        inventories[inventory]
            .slot_data[col_size * current_inventory_row + current_inventory_column]
            .color;

    void* hud_matrix_data;
    unsigned int ui_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::UI]
            .descriptors_bindings_i_ds[0];
    vkMapMemory(
        device,
        GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG[ui_ubo_descriptor_binding_index]
                            .global_descriptor_offset]
            .gpu_buffer->device_memory,
        sizeof(UiUbo) * offset,
        sizeof(UiUbo),
        0,
        &hud_matrix_data
    );
    memcpy(hud_matrix_data, &hud_ubo, sizeof(UiUbo));
    vkUnmapMemory(
        device,
        GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG[ui_ubo_descriptor_binding_index]
                            .global_descriptor_offset]
            .gpu_buffer->device_memory
    );
}

void CVulkanRenderer::update_ubo_icons_ui(uint32_t offset, uint32_t item) {
    UiUbo hud_ubo {};
    hud_ubo.model = items[item].model;

    void* hud_matrix_data;
    unsigned int ui_icons_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::UiIcons]
            .descriptors_bindings_i_ds[0];
    vkMapMemory(
        device,
        GPU_DESCRIPTORS
            [DESCRIPTOR_BINDINGS_CONFIG[ui_icons_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->device_memory,
        sizeof(UiUbo) * offset,
        sizeof(UiUbo),
        0,
        &hud_matrix_data
    );
    memcpy(hud_matrix_data, &hud_ubo, sizeof(UiUbo));
    vkUnmapMemory(
        device,
        GPU_DESCRIPTORS
            [DESCRIPTOR_BINDINGS_CONFIG[ui_icons_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->device_memory
    );
}

void CVulkanRenderer::hud_record_command_buffer(
    VkCommandBuffer& command_buffer,
    uint32_t image_index
) {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = RENDER_PASSES[SpecificPipeline::HudPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    std::array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount =
        static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        PIPELINE_CONFIGS[SpecificPipeline::HudPipeline].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swap_chain_extent.width;
    viewport.height = (float)swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < health_bars.size(); ++i) {
        unsigned int ui_vertex_id = health_bars[i].mesh_id;
        unsigned int ubo_index = current_frame * hud_ubo_descriptor_number + i;
        update_hud_ubo(ubo_index, true, highest_gltf_y[ui_vertex_id], i);
        const unsigned int linked_descriptor_set_id =
            PIPELINE_CONFIGS[SpecificPipeline::HudPipeline]
                .linked_descriptor_set_i_ds[0];
        const DescriptorSet& current_descriptor_set =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::HudPipeline].pipeline_layout,
            0,
            1,
            &(*(DESCRIPTOR_SETS_CHUNKS.data()
                + current_descriptor_set.descriptor_set_offset + i)),
            0,
            nullptr
        );

        VkBuffer vertex_buffers[] = {vertex_buffer_container[ui_vertex_id]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

        vkCmdBindIndexBuffer(
            command_buffer,
            index_buffer_container[ui_vertex_id],
            0,
            VK_INDEX_TYPE_UINT32
        );

        unsigned int indices_container_size = a_indices[ui_vertex_id].size();

        vkCmdDrawIndexed(
            command_buffer,
            static_cast<uint32_t>(indices_container_size),
            1,
            0,
            0,
            0
        );
    }

    vkCmdEndRenderPass(command_buffer);
}

void CVulkanRenderer::ui_record_command_buffer(
    VkCommandBuffer& command_buffer,
    uint32_t image_index
) {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = RENDER_PASSES[SpecificPipeline::UiPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    std::array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount =
        static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        PIPELINE_CONFIGS[SpecificPipeline::UiPipeline].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swap_chain_extent.width;
    viewport.height = (float)swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < inventories.size(); ++i) {
        RenderInventory inventory = inventories[i];
        unsigned int inventory_texture_id = inventory.inventory_texture_id;
        unsigned int ui_vertex_id = inventory.mesh_id;
        for (unsigned int j = 0; j < inventory.row; ++j) {
            for (unsigned int m = 0; m < inventory.col; ++m) {
                unsigned int ubo_index =
                    current_frame * ui_ubo_descriptors_number
                    + j * inventory.col + m;
                update_ubo_ui(j, m, i, ubo_index);
                const unsigned int linked_descriptor_set_id =
                    PIPELINE_CONFIGS[SpecificPipeline::UiPipeline]
                        .linked_descriptor_set_i_ds[0];
                const DescriptorSet& current_descriptor_set =
                    DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
                vkCmdBindDescriptorSets(
                    command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    PIPELINE_CONFIGS[SpecificPipeline::UiPipeline]
                        .pipeline_layout,
                    0,
                    1,
                    &(*(DESCRIPTOR_SETS_CHUNKS.data()
                        + current_descriptor_set.descriptor_set_offset
                        + ubo_index)),
                    0,
                    nullptr
                );

                const unsigned int linked_descriptor_set_i_d1 =
                    PIPELINE_CONFIGS[SpecificPipeline::UiPipeline]
                        .linked_descriptor_set_i_ds[1];
                const DescriptorSet& current_descriptor_set1 =
                    DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_i_d1];
                vkCmdBindDescriptorSets(
                    command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    PIPELINE_CONFIGS[SpecificPipeline::UiPipeline]
                        .pipeline_layout,
                    1,
                    1,
                    &(*(DESCRIPTOR_SETS_CHUNKS.data()
                        + current_descriptor_set1.descriptor_set_offset
                        + MAX_FRAMES_IN_FLIGHT * inventory_texture_id
                        + current_frame)),
                    0,
                    nullptr
                );

                VkBuffer vertex_buffers[] = {
                    vertex_buffer_container[ui_vertex_id]
                };
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(
                    command_buffer,
                    0,
                    1,
                    vertex_buffers,
                    offsets
                );

                vkCmdBindIndexBuffer(
                    command_buffer,
                    index_buffer_container[ui_vertex_id],
                    0,
                    VK_INDEX_TYPE_UINT32
                );

                unsigned int indices_container_size =
                    a_indices[ui_vertex_id].size();

                vkCmdDrawIndexed(
                    command_buffer,
                    static_cast<uint32_t>(indices_container_size),
                    1,
                    0,
                    0,
                    0
                );
            }
        }
    }

    vkCmdEndRenderPass(command_buffer);
}

void CVulkanRenderer::ui_icons_record_command_buffer(
    VkCommandBuffer& command_buffer,
    uint32_t image_index
) {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass =
        RENDER_PASSES[SpecificPipeline::UiIconsPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    std::array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount =
        static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        PIPELINE_CONFIGS[SpecificPipeline::UiIconsPipeline].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swap_chain_extent.width;
    viewport.height = (float)swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < items.size(); ++i) {
        RenderItem item = items[i];
        unsigned int ui_vertex_id = item.mesh_id;
        unsigned int diffuse_texture_id = item.diffuse_texture_id;
        unsigned int ubo_index = current_frame * items.size() + i;

        update_ubo_icons_ui(ubo_index, i);
        const unsigned int linked_descriptor_set_id =
            PIPELINE_CONFIGS[SpecificPipeline::UiIconsPipeline]
                .linked_descriptor_set_i_ds[0];
        const DescriptorSet& current_descriptor_set =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::UiIconsPipeline].pipeline_layout,
            0,
            1,
            &(*(DESCRIPTOR_SETS_CHUNKS.data()
                + current_descriptor_set.descriptor_set_offset + ubo_index)),
            0,
            nullptr
        );

        const unsigned int linked_descriptor_set_i_d1 =
            PIPELINE_CONFIGS[SpecificPipeline::UiIconsPipeline]
                .linked_descriptor_set_i_ds[1];
        const DescriptorSet& current_descriptor_set1 =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_i_d1];
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::UiIconsPipeline].pipeline_layout,
            1,
            1,
            &(*(DESCRIPTOR_SETS_CHUNKS.data()
                + current_descriptor_set1.descriptor_set_offset
                + MAX_FRAMES_IN_FLIGHT * diffuse_texture_id + current_frame)),
            0,
            nullptr
        );

        VkBuffer vertex_buffers[] = {vertex_buffer_container[ui_vertex_id]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

        vkCmdBindIndexBuffer(
            command_buffer,
            index_buffer_container[ui_vertex_id],
            0,
            VK_INDEX_TYPE_UINT32
        );

        unsigned int indices_container_size = a_indices[ui_vertex_id].size();

        vkCmdDrawIndexed(
            command_buffer,
            static_cast<uint32_t>(indices_container_size),
            1,
            0,
            0,
            0
        );
    }

    vkCmdEndRenderPass(command_buffer);
}

void CVulkanRenderer::hud_screen_record_command_buffer(
    VkCommandBuffer& command_buffer,
    uint32_t image_index
) {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass =
        RENDER_PASSES[SpecificPipeline::HudScreenPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    std::array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount =
        static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        PIPELINE_CONFIGS[SpecificPipeline::HudScreenPipeline].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swap_chain_extent.width;
    viewport.height = (float)swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < crosshairs.size(); ++i) {
        RenderCrosshair crosshair = crosshairs[i];
        unsigned int ui_vertex_id = crosshair.mesh_id;

        unsigned int ubo_index =
            current_frame * hud_screen_ubo_descriptor_number + i;
        update_hud_screen_ubo(ubo_index, i);
        const unsigned int linked_descriptor_set_id =
            PIPELINE_CONFIGS[SpecificPipeline::HudScreenPipeline]
                .linked_descriptor_set_i_ds[0];
        const DescriptorSet& current_descriptor_set =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::HudScreenPipeline]
                .pipeline_layout,
            0,
            1,
            &(*(DESCRIPTOR_SETS_CHUNKS.data()
                + current_descriptor_set.descriptor_set_offset + ubo_index)),
            0,
            nullptr
        );

        VkBuffer vertex_buffers[] = {vertex_buffer_container[ui_vertex_id]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

        vkCmdBindIndexBuffer(
            command_buffer,
            index_buffer_container[ui_vertex_id],
            0,
            VK_INDEX_TYPE_UINT32
        );

        unsigned int indices_container_size = a_indices[ui_vertex_id].size();

        vkCmdDrawIndexed(
            command_buffer,
            static_cast<uint32_t>(indices_container_size),
            1,
            0,
            0,
            0
        );
    }

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CVulkanRenderer::sdf_record_command_buffer(
    VkCommandBuffer& command_buffer,
    uint32_t image_index
) {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = RENDER_PASSES[SpecificPipeline::SdfPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    std::array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount =
        static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        PIPELINE_CONFIGS[SpecificPipeline::SdfPipeline].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swap_chain_extent.width;
    viewport.height = (float)swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < crosshairs.size(); ++i) {
        RenderCrosshair crosshair = crosshairs[i];
        unsigned int ui_vertex_id = crosshair.mesh_id;

        unsigned int ubo_index =
            current_frame * hud_screen_ubo_descriptor_number + i;
        update_sdf_ubo(ubo_index, i);
        const unsigned int linked_descriptor_set_id =
            PIPELINE_CONFIGS[SpecificPipeline::SdfPipeline]
                .linked_descriptor_set_i_ds[0];
        const DescriptorSet& current_descriptor_set =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::SdfPipeline].pipeline_layout,
            0,
            1,
            &(*(DESCRIPTOR_SETS_CHUNKS.data()
                + current_descriptor_set.descriptor_set_offset + ubo_index)),
            0,
            nullptr
        );

        VkBuffer vertex_buffers[] = {vertex_buffer_container[ui_vertex_id]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

        vkCmdBindIndexBuffer(
            command_buffer,
            index_buffer_container[ui_vertex_id],
            0,
            VK_INDEX_TYPE_UINT32
        );
        vkCmdDrawIndexed(command_buffer, 3, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CVulkanRenderer::font_record_command_buffer(
    VkCommandBuffer& command_buffer,
    uint32_t image_index
) {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = RENDER_PASSES[SpecificPipeline::FontPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;
    std::array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount =
        static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        PIPELINE_CONFIGS[SpecificPipeline::FontPipeline].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swap_chain_extent.width;
    viewport.height = (float)swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    for (unsigned int player_counter = 0; player_counter < players.size();
         ++player_counter) {
        player = players[player_counter];
    }
    unsigned int current_actor_memory_offset =
        current_frame * font_ubo_descriptor_number;
    for (unsigned int i = 0; i < fonts.size(); ++i) {
        RenderFont font = fonts[i];
        Vector<float, 3> player_traget_direction =
            font.position - player.position;
        float dot_product = dot(player_traget_direction, player.forward);
        if (dot_product <= 0) {
            continue;
        }

        for (unsigned int j = 0; j < font.font_string.size(); ++j) {
            unsigned int ascii_code =
                static_cast<unsigned int>(font.font_string[j]);
            VkBuffer vertex_buffers[] = {
                font_vertex_buffer_container[ascii_code]
            };
            VkDeviceSize offsets[] = {0};

            vkCmdBindVertexBuffers(
                command_buffer,
                0,
                1,
                vertex_buffers,
                offsets
            );
            vkCmdBindIndexBuffer(
                command_buffer,
                font_index_buffer_container[ascii_code],
                0,
                VK_INDEX_TYPE_UINT32
            );

            unsigned int indices_container_size = symbol_g_indices.size();
            FontUbo font_ubo {};
            Vector<float, 3> result;
            Vector<float, 4> pos = Vector<float, 4>(
                font.position[0],
                font.position[1],
                font.position[2],
                1.0f
            );

            Vector<float, 4> clip_space_position =
                pos * view_matrix * projection_matrix;
            Vector<float, 3> ndc_position = Vector<float, 3>(
                clip_space_position[0] / clip_space_position[3],
                clip_space_position[1] / clip_space_position[3],
                clip_space_position[2] / clip_space_position[3]
            );

            font_ubo.view = view_matrix;
            font_ubo.proj = projection_matrix;

            font_ubo.scale = 0.3f;
            ndc_position[0] += (float)j * 0.17f * font_ubo.scale;
            ndc_position[1] -= font.life_time / 5.0f;
            font_ubo.position = ndc_position;

            void* model_matrix_data;
            unsigned int font_ubo_descriptor_binding_index =
                DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::FontRenderUbo]
                    .descriptors_bindings_i_ds[0];
            vkMapMemory(
                device,
                GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                                    [font_ubo_descriptor_binding_index]
                                        .global_descriptor_offset]
                    .gpu_buffer->device_memory,
                sizeof(font_ubo) * (current_actor_memory_offset + j),
                sizeof(font_ubo),
                0,
                &model_matrix_data
            );
            memcpy(model_matrix_data, &font_ubo, sizeof(font_ubo));
            vkUnmapMemory(
                device,
                GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                                    [font_ubo_descriptor_binding_index]
                                        .global_descriptor_offset]
                    .gpu_buffer->device_memory
            );

            const unsigned int linked_descriptor_set_id =
                PIPELINE_CONFIGS[SpecificPipeline::FontPipeline]
                    .linked_descriptor_set_i_ds[0];
            const DescriptorSet& current_descriptor_set =
                DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
            vkCmdBindDescriptorSets(
                command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                PIPELINE_CONFIGS[SpecificPipeline::FontPipeline].pipeline_layout,
                0,
                1,
                &(*(DESCRIPTOR_SETS_CHUNKS.data()
                    + current_descriptor_set.descriptor_set_offset
                    + current_actor_memory_offset + j)),
                0,
                nullptr
            );
            const unsigned int linked_descriptor_set_i_d1 =
                PIPELINE_CONFIGS[SpecificPipeline::FontPipeline]
                    .linked_descriptor_set_i_ds[1];
            const unsigned int font_atlas_texture_id = 6;
            const DescriptorSet& current_descriptor_set1 =
                DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_i_d1];
            vkCmdBindDescriptorSets(
                command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                PIPELINE_CONFIGS[SpecificPipeline::FontPipeline].pipeline_layout,
                1,
                1,
                &(*(DESCRIPTOR_SETS_CHUNKS.data()
                    + current_descriptor_set1.descriptor_set_offset
                    + MAX_FRAMES_IN_FLIGHT * font_atlas_texture_id
                    + current_frame)),
                0,
                nullptr
            );

            vkCmdDrawIndexed(
                command_buffer,
                static_cast<uint32_t>(indices_container_size),
                1,
                0,
                0,
                0
            );
        }
        current_actor_memory_offset += font.font_string.size();
    }
    vkCmdEndRenderPass(command_buffer);
}

void CVulkanRenderer::record_command_buffer(
    VkCommandBuffer& command_buffer,
    uint32_t image_index
) {
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass =
        RENDER_PASSES[SpecificPipeline::MainRenderPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    for (unsigned int player = 0; player < players.size(); ++player) {
        update_view_position_uniform_buffer(current_frame, player);
    }

    std::array<VkClearValue, 2> clear_values {};
    // Player death screen.
    if (players.size() == 0) {
        clear_values[0].color = {{0.7f, 0.2f, 0.2f, 1.0f}};
    } else {
        clear_values[0].color = {{0.2f, 0.2f, 0.2f, 1.0f}};
    }
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount =
        static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swap_chain_extent.width;
    viewport.height = (float)swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (unsigned int i = 0; i < actors.size(); ++i) {
        RenderActor actor = actors[i];
        unsigned int ui_vertex_id = actor.mesh_id;
        unsigned int diffuse_texture_index = actor.diffuse_texture_index;
        unsigned int specular_texture_index = actor.specular_texture_index;

        unsigned int ubo_index =
            current_frame * matrix_ubo_descriptors_number + i;
        update_matrix_uniform_buffer(ubo_index, i);
        const unsigned int linked_descriptor_set_id =
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .linked_descriptor_set_i_ds[0];
        const DescriptorSet& current_descriptor_set =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .pipeline_layout,
            0,
            1,
            &(*(DESCRIPTOR_SETS_CHUNKS.data()
                + current_descriptor_set.descriptor_set_offset + ubo_index)),
            0,
            nullptr
        );

        const unsigned int linked_descriptor_set_i_d1 =
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .linked_descriptor_set_i_ds[1];
        const DescriptorSet& current_descriptor_set1 =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_i_d1];
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .pipeline_layout,
            1,
            1,
            &(*(DESCRIPTOR_SETS_CHUNKS.data()
                + current_descriptor_set1.descriptor_set_offset
                + current_frame)),
            0,
            nullptr
        );

        VkBuffer vertex_buffers[] = {vertex_buffer_container[ui_vertex_id]};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

        vkCmdBindIndexBuffer(
            command_buffer,
            index_buffer_container[ui_vertex_id],
            0,
            VK_INDEX_TYPE_UINT32
        );

        unsigned int indices_container_size = a_indices[ui_vertex_id].size();

        const unsigned int linked_descriptor_set_i_d2 =
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .linked_descriptor_set_i_ds[2];
        const DescriptorSet& current_descriptor_set2 =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_i_d2];
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .pipeline_layout,
            2,
            1,
            &(*(DESCRIPTOR_SETS_CHUNKS.data()
                + current_descriptor_set2.descriptor_set_offset
                + MAX_FRAMES_IN_FLIGHT * specular_texture_index
                + current_frame)),
            0,
            nullptr
        );
        const unsigned int linked_descriptor_set_i_d3 =
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .linked_descriptor_set_i_ds[3];
        const DescriptorSet& current_descriptor_set3 =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_i_d3];
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .pipeline_layout,
            3,
            1,
            &(*(DESCRIPTOR_SETS_CHUNKS.data()
                + current_descriptor_set3.descriptor_set_offset
                + MAX_FRAMES_IN_FLIGHT * diffuse_texture_index
                + current_frame)),
            0,
            nullptr
        );

        vkCmdDrawIndexed(
            command_buffer,
            static_cast<uint32_t>(indices_container_size),
            1,
            0,
            0,
            0
        );
    }
    vkCmdEndRenderPass(command_buffer);
}

void CVulkanRenderer::create_sync_objects(
    std::vector<VkSemaphore>& image_available_semaphores,
    std::vector<VkSemaphore>& render_finished_semaphores,
    std::vector<VkFence>& in_flight_fences
) {
    image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphores.resize(swap_chain_images.size());
    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphore_info {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(
                device,
                &semaphore_info,
                nullptr,
                &image_available_semaphores[i]
            ) != VK_SUCCESS
            || vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i])
                != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create synchronization objects for a frame!"
            );
        }
    }

    for (size_t i = 0; i < swap_chain_images.size(); ++i) {
        if (vkCreateSemaphore(
                device,
                &semaphore_info,
                nullptr,
                &render_finished_semaphores[i]
            )
            != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to create synchronization objects for a frame!"
            );
        }
    }
}

void CVulkanRenderer::update_directional_light_shadow_map_matrix_ubo(
    uint32_t current_image,
    uint32_t current_light,
    unsigned int actor
) {
    ShadowMapMatrixUBO model_matrix_ubo {};

    model_matrix_ubo.model = actors[actor].model_matrix;
    model_matrix_ubo.light_space_matrix = dir_light_space_matrix[current_light];

    for (unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        model_matrix_ubo.joint_matrices[j] = actors[actor].joint_matrices[j];
    }

    void* model_matrix_data = nullptr;
    unsigned int shadow_map_directional_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapDirectionalLight]
            .descriptors_bindings_i_ds[0];
    vkMapMemory(
        device,
        GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                            [shadow_map_directional_light_descriptor_binding_index]
                                .global_descriptor_offset]
            .gpu_buffer->device_memory,
        current_image * sizeof(model_matrix_ubo),
        sizeof(model_matrix_ubo),
        0,
        &model_matrix_data
    );
    memcpy(model_matrix_data, &model_matrix_ubo, sizeof(model_matrix_ubo));
    vkUnmapMemory(
        device,
        GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                            [shadow_map_directional_light_descriptor_binding_index]
                                .global_descriptor_offset]
            .gpu_buffer->device_memory
    );
}

void CVulkanRenderer::update_spot_light_shadow_map_matrix_ubo(
    uint32_t current_image,
    uint32_t current_light,
    unsigned int actor
) {
    ShadowMapMatrixUBO model_matrix_ubo {};

    model_matrix_ubo.model = actors[actor].model_matrix;
    model_matrix_ubo.light_space_matrix =
        spot_light_space_matrix[current_light];

    for (unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        model_matrix_ubo.joint_matrices[j] = actors[actor].joint_matrices[j];
    }

    void* model_matrix_data;
    unsigned int shadow_map_spot_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapSpotLight]
            .descriptors_bindings_i_ds[0];
    vkMapMemory(
        device,
        GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                            [shadow_map_spot_light_descriptor_binding_index]
                                .global_descriptor_offset]
            .gpu_buffer->device_memory,
        current_image * sizeof(model_matrix_ubo),
        sizeof(model_matrix_ubo),
        0,
        &model_matrix_data
    );
    memcpy(model_matrix_data, &model_matrix_ubo, sizeof(model_matrix_ubo));
    vkUnmapMemory(
        device,
        GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                            [shadow_map_spot_light_descriptor_binding_index]
                                .global_descriptor_offset]
            .gpu_buffer->device_memory
    );
}

void CVulkanRenderer::update_point_light_shadow_map_matrix_ubo(
    uint32_t current_image,
    uint32_t current_light,
    uint32_t layer,
    unsigned int actor
) {
    PointLightShadowMapMatrixUBO model_matrix_ubo {};

    model_matrix_ubo.model = actors[actor].model_matrix;

    model_matrix_ubo.light_space_matrix =
        point_lights[current_light].point_light_space_matrix[layer];
    model_matrix_ubo.far_plane = 100.0f;
    model_matrix_ubo.light_position = point_lights[current_light].position;

    for (unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        model_matrix_ubo.joint_matrices[j] = actors[actor].joint_matrices[j];
    }

    void* model_matrix_data;
    unsigned int shadow_map_point_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapPointLight]
            .descriptors_bindings_i_ds[0];
    vkMapMemory(
        device,
        GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                            [shadow_map_point_light_descriptor_binding_index]
                                .global_descriptor_offset]
            .gpu_buffer->device_memory,
        current_image * sizeof(model_matrix_ubo),
        sizeof(model_matrix_ubo),
        0,
        &model_matrix_data
    );
    memcpy(model_matrix_data, &model_matrix_ubo, sizeof(model_matrix_ubo));
    vkUnmapMemory(
        device,
        GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                            [shadow_map_point_light_descriptor_binding_index]
                                .global_descriptor_offset]
            .gpu_buffer->device_memory
    );
}

void CVulkanRenderer::update_matrix_uniform_buffer(
    uint32_t offset,
    unsigned int actor
) {
    ModelMatrixUBO model_matrix_ubo {};

    model_matrix_ubo.model = actors[actor].model_matrix;

    model_matrix_ubo.view = view_matrix;
    model_matrix_ubo.proj = projection_matrix;

    for (unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        model_matrix_ubo.joint_matrices[j] = actors[actor].joint_matrices[j];
    }

    model_matrix_ubo.ambient = actors[actor].ambient;
    model_matrix_ubo.shininess = actors[actor].shininess;

    for (uint32_t i = 0; i < directional_light_number; ++i) {
        model_matrix_ubo.dir_space_matrix[i] = dir_light_space_matrix[i];
    }

    for (uint32_t i = 0; i < spot_light_number; ++i) {
        model_matrix_ubo.spot_space_matrix[i] = spot_light_space_matrix[i];
    }

    model_matrix_ubo.directional_lights_number = directional_light_number;
    model_matrix_ubo.spot_lights_number = spot_light_number;

    void* model_matrix_data;
    vkMapMemory(
        device,
        GPU_DESCRIPTORS[DescriptorSetDataLink::MainRenderMatrixUbo]
            .gpu_buffer->device_memory,
        sizeof(model_matrix_ubo) * offset,
        sizeof(model_matrix_ubo),
        0,
        &model_matrix_data
    );
    memcpy(model_matrix_data, &model_matrix_ubo, sizeof(model_matrix_ubo));
    vkUnmapMemory(
        device,
        GPU_DESCRIPTORS[DescriptorSetDataLink::MainRenderMatrixUbo]
            .gpu_buffer->device_memory
    );
}

void CVulkanRenderer::update_view_position_uniform_buffer(
    uint32_t current_image,
    uint32_t player
) {
    LightData light_data_ubo {};
    light_data_ubo.view_position = players[player].position;

    DirectionalLight directional_light {};
    directional_light_number = directional_lights.size();
    assert(
        directional_light_number <= 4
        && "Directional lights number greater then 4"
    );
    for (unsigned int i = 0; i < directional_light_number; ++i) {
        RenderDirectionalLight dir_light = directional_lights[i];

        directional_light.position = dir_light.position;
        directional_light.direction = dir_light.direction;
        directional_light.ambient = dir_light.ambient;
        directional_light.diffuse = dir_light.diffuse;
        directional_light.specular = dir_light.specular;

        light_data_ubo.directional_lights[i] = directional_light;
    }
    light_data_ubo.directional_lights_array_size = directional_light_number;

    point_light_number = point_lights.size();
    assert(
        point_light_number <= POINT_LIGHTS_NUMBER
        && "Point lights number greater than 32"
    );
    for (unsigned int i = 0; i < point_light_number; ++i) {
        RenderPointLight point_light = point_lights[i];
        PointLight point_light_ubo {};

        point_light_ubo.position = point_light.position;
        point_light_ubo.ambient = point_light.ambient;
        point_light_ubo.diffuse = point_light.diffuse;
        point_light_ubo.specular = point_light.specular;
        point_light_ubo.constant = point_light.constant;
        point_light_ubo.linear = point_light.linear;
        point_light_ubo.quadratic = point_light.quadratic;

        light_data_ubo.point_lights[i] = point_light_ubo;
    }
    light_data_ubo.point_lights_array_size = point_light_number;
    light_data_ubo.far_plane = 100.0f;

    SpotLight spot_light_ubo {};
    spot_light_number = spot_lights.size();
    assert(spot_light_number <= 8 && "Spot light number greater then 8");
    for (unsigned int i = 0; i < spot_light_number; ++i) {
        RenderSpotLight spot_light = spot_lights[i];

        spot_light_ubo.position = spot_light.position;
        spot_light_ubo.direction = spot_light.direction;
        spot_light_ubo.cut_off = std::cos(radians(spot_light.cut_off));
        spot_light_ubo.outer_cut_off =
            std::cos(radians(spot_light.outer_cut_off));
        spot_light_ubo.ambient = spot_light.ambient;
        spot_light_ubo.diffuse = spot_light.diffuse;
        spot_light_ubo.specular = spot_light.specular;
        spot_light_ubo.constant = spot_light.constant;
        spot_light_ubo.linear = spot_light.linear;
        spot_light_ubo.quadratic = spot_light.quadratic;

        light_data_ubo.spot_lights[i] = spot_light_ubo;
    }
    light_data_ubo.spot_light_array_size = spot_light_number;

    std::random_device rd;
    std::mt19937 mersenne(rd());
    std::uniform_int_distribution<int> distribution_tile_index(
        0,
        INDIRECT_TEXTURE_HEIGHT * INDIRECT_TEXTURE_WIDTH
    );

    if (print == true) {
        for (int i = 0;
             i < INDIRECT_TEXTURE_HEIGHT * INDIRECT_TEXTURE_WIDTH / 4 + 1;
             ++i) {
            for (int j = 0; j < 4; ++j) {
                int random_tile_index = distribution_tile_index(mersenne);
                indirect_texture[i][j] = random_tile_index;
            }
        }
    }
    print = false;
    light_data_ubo.tileset_tiles_count =
        Vector<float, 2>(TILESET_ROW, TILESET_COLUMN);
    light_data_ubo.tiles_raw = 8;
    light_data_ubo.tiles_column = 8;
    light_data_ubo.debug_shadow_mode = imgui_overlay->show_shadow_maps
        ? (imgui_overlay->shadow_map_mode + 1)
        : 0;
    light_data_ubo.debug_shadow_light = imgui_overlay->shadow_map_light;
    light_data_ubo.shadows_enabled = imgui_overlay->shadows_enabled ? 1 : 0;
    for (int i = 0;
         i < INDIRECT_TEXTURE_HEIGHT * INDIRECT_TEXTURE_WIDTH / 4 + 1;
         ++i) {
        light_data_ubo.indirect_texture[i] = indirect_texture[i];
    }

    void* data;
    unsigned int light_data_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_i_ds[0];
    vkMapMemory(
        device,
        GPU_DESCRIPTORS
            [DESCRIPTOR_BINDINGS_CONFIG[light_data_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->device_memory,
        sizeof(light_data_ubo) * current_image,
        sizeof(light_data_ubo),
        0,
        &data
    );
    memcpy(data, &light_data_ubo, sizeof(light_data_ubo));
    vkUnmapMemory(
        device,
        GPU_DESCRIPTORS
            [DESCRIPTOR_BINDINGS_CONFIG[light_data_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->device_memory
    );
}

void CVulkanRenderer::main_render_draw_frame() {
    vkWaitForFences(
        device,
        1,
        &in_flight_fences[current_frame],
        VK_TRUE,
        UINT64_MAX
    );

    uint32_t image_index;
    // vkAcquireNextImageKHR give index of image that WILL BE SOON available for
    // rendering and signal imageAvailablesemaphore when its so. GraphicsQueue
    // waint for this semaphore bacause we pass it in submitInfo.
    VkResult result = vkAcquireNextImageKHR(
        device,
        swap_chain,
        UINT64_MAX,
        image_available_semaphores[current_frame],
        VK_NULL_HANDLE,
        &image_index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swap_chain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(device, 1, &in_flight_fences[current_frame]);
    vkResetCommandBuffer(
        main_render_command_buffers[current_frame],
        0 // VkCommandBufferResetFlagBits.
    );

    auto future1 = render_thread_pool->enqueue([this]() {
        directional_light_record_coomand_buffer(
            directional_light_secondary_command_buffers,
            this->current_frame
        );
    });

    auto future2 = render_thread_pool->enqueue([this]() {
        spot_light_record_command_buffer(
            spot_light_secondary_command_buffers,
            this->current_frame
        );
    });

    auto future3 = render_thread_pool->enqueue([this]() {
        point_light_record_command_buffer(
            point_light_secondary_command_buffers,
            this->current_frame
        );
    });

    future1.wait();
    future2.wait();
    future3.wait();

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(
            main_render_command_buffers[current_frame],
            &begin_info
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    for (uint32_t directional_light_counter = 0;
         directional_light_counter < directional_lights.size();
         ++directional_light_counter) {
        VkExtent2D flat_shadow_map_extent;
        flat_shadow_map_extent.width = FLAT_SHADOW_MAP_SIZE;
        flat_shadow_map_extent.height = FLAT_SHADOW_MAP_SIZE;
        execute_secondary_command_buffer(
            RENDER_PASSES[SpecificPipeline::DirectionalLightPipeline],
            directional_light_shadow_map_frame_buffers[directional_light_counter],
            flat_shadow_map_extent,
            main_render_command_buffers[current_frame],
            directional_light_secondary_command_buffers
                [current_frame * directional_light_number
                 + directional_light_counter]
        );
    }
    for (uint32_t spot_light_counter = 0;
         spot_light_counter < spot_lights.size();
         ++spot_light_counter) {
        VkExtent2D flat_shadow_map_extent;
        flat_shadow_map_extent.width = FLAT_SHADOW_MAP_SIZE;
        flat_shadow_map_extent.height = FLAT_SHADOW_MAP_SIZE;
        execute_secondary_command_buffer(
            RENDER_PASSES[SpecificPipeline::SpotLightPipeline],
            spot_light_shadow_map_frame_buffers[spot_light_counter],
            flat_shadow_map_extent,
            main_render_command_buffers[current_frame],
            spot_light_secondary_command_buffers
                [current_frame * spot_light_number + spot_light_counter]
        );
    }
    for (uint32_t point_light_counter = 0;
         point_light_counter < point_lights.size();
         ++point_light_counter) {
        uint32_t max_cube_map_layers = 6;
        for (uint32_t cube_map_layer_counter = 0;
             cube_map_layer_counter < max_cube_map_layers;
             ++cube_map_layer_counter) {
            VkExtent2D extent;
            extent.width = SHADOW_MAP_SIZE;
            extent.height = SHADOW_MAP_SIZE;
            execute_secondary_command_buffer(
                RENDER_PASSES[SpecificPipeline::PointLightPipeline],
                point_light_shadow_map_frame_buffers[point_light_counter]
                                                    [cube_map_layer_counter],
                extent,
                main_render_command_buffers[current_frame],
                point_light_secondary_command_buffers
                    [current_frame * point_light_number * max_cube_map_layers
                     + point_light_counter * max_cube_map_layers
                     + cube_map_layer_counter]
            );
        }
    }

    record_command_buffer(
        main_render_command_buffers[current_frame],
        image_index
    );
    hud_record_command_buffer(
        main_render_command_buffers[current_frame],
        image_index
    );
    font_record_command_buffer(
        main_render_command_buffers[current_frame],
        image_index
    );
    if (is_inventory_opened) {
        ui_record_command_buffer(
            main_render_command_buffers[current_frame],
            image_index
        );
        ui_icons_record_command_buffer(
            main_render_command_buffers[current_frame],
            image_index
        );
    }

    imgui_overlay->record_command_buffer(
        main_render_command_buffers[current_frame],
        image_index
    );
    hud_screen_record_command_buffer(
        main_render_command_buffers[current_frame],
        image_index
    );

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // GraphicsQueue wait for swapchain image when its become available.
    VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &main_render_command_buffers[current_frame];

    VkSemaphore signal_semaphores[] = {render_finished_semaphores[image_index]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(
            graphics_queue,
            1,
            &submit_info,
            in_flight_fences[current_frame]
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR present_info {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = {swap_chain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;

    present_info.pImageIndices = &image_index;

    result = vkQueuePresentKHR(present_queue, &present_info);

    if (frame_counter == 10) {
        vkDeviceWaitIdle(device);
    }

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR
        || framebuffer_resized) {
        framebuffer_resized = false;
        recreate_swap_chain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CVulkanRenderer::directional_light_shadow_map_draw_frame() {
    vkWaitForFences(
        device,
        1,
        &directional_light_shadow_map_in_flight_fences
            [directional_light_current_frame],
        VK_TRUE,
        UINT64_MAX
    );

    uint32_t image_index = 0;

    vkResetFences(
        device,
        1,
        &directional_light_shadow_map_in_flight_fences
            [directional_light_current_frame]
    );
    vkResetCommandBuffer(
        directional_light_command_buffers[directional_light_current_frame],
        0 // VkCommandBufferResetFlagBits.
    );
    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers =
        &directional_light_command_buffers[directional_light_current_frame];

    if (vkQueueSubmit(
            graphics_queue,
            1,
            &submit_info,
            directional_light_shadow_map_in_flight_fences
                [directional_light_current_frame]
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    directional_light_current_frame =
        (directional_light_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CVulkanRenderer::spot_light_shadow_map_draw_frame() {
    vkWaitForFences(
        device,
        1,
        &spot_light_shadow_map_in_flight_fences[spot_light_current_frame],
        VK_TRUE,
        UINT64_MAX
    );

    uint32_t image_index = 0;

    vkResetFences(
        device,
        1,
        &spot_light_shadow_map_in_flight_fences[spot_light_current_frame]
    );
    vkResetCommandBuffer(
        spot_light_command_buffers[spot_light_current_frame],
        0 // VkCommandBufferResetFlagBits.
    );

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers =
        &spot_light_command_buffers[spot_light_current_frame];

    if (vkQueueSubmit(
            graphics_queue,
            1,
            &submit_info,
            spot_light_shadow_map_in_flight_fences[spot_light_current_frame]
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    spot_light_current_frame =
        (spot_light_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CVulkanRenderer::point_light_shadow_map_draw_frame() {
    vkWaitForFences(
        device,
        1,
        &point_light_shadow_map_in_flight_fences[point_light_current_frame],
        VK_TRUE,
        UINT64_MAX
    );

    uint32_t image_index = 0;

    vkResetFences(
        device,
        1,
        &point_light_shadow_map_in_flight_fences[point_light_current_frame]
    );
    vkResetCommandBuffer(
        point_light_command_buffers[point_light_current_frame],
        0 // VkCommandBufferResetFlagBits.
    );

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers =
        &point_light_command_buffers[point_light_current_frame];

    if (vkQueueSubmit(
            graphics_queue,
            1,
            &submit_info,
            point_light_shadow_map_in_flight_fences[point_light_current_frame]
        )
        != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    point_light_current_frame =
        (point_light_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CVulkanRenderer::directional_light_record_coomand_buffer(
    std::vector<VkCommandBuffer>& command_buffers,
    uint32_t current_frame
) {
    for (uint32_t directional_light_counter = 0;
         directional_light_counter < directional_lights.size();
         ++directional_light_counter) {
        VkCommandBufferInheritanceInfo inheritance_info {};
        inheritance_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritance_info.renderPass =
            RENDER_PASSES[SpecificPipeline::DirectionalLightPipeline];
        inheritance_info.framebuffer =
            directional_light_shadow_map_frame_buffers[directional_light_counter];
        inheritance_info.subpass = 0;

        VkCommandBufferBeginInfo begin_info {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        begin_info.pInheritanceInfo = &inheritance_info;

        VkCommandBuffer command_buffer = command_buffers
            [current_frame * directional_light_number
             + directional_light_counter];
        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to begin recording command buffer!"
            );
        }

        VkViewport shadow_map_view_port;
        shadow_map_view_port.height = FLAT_SHADOW_MAP_SIZE;
        shadow_map_view_port.width = FLAT_SHADOW_MAP_SIZE;
        shadow_map_view_port.minDepth = 0.0f;
        shadow_map_view_port.maxDepth = 1.0f;
        shadow_map_view_port.x = 0;
        shadow_map_view_port.y = 0;
        vkCmdSetViewport(command_buffer, 0, 1, &shadow_map_view_port);

        VkRect2D shadow_map_scissor;
        shadow_map_scissor.extent.width = FLAT_SHADOW_MAP_SIZE;
        shadow_map_scissor.extent.height = FLAT_SHADOW_MAP_SIZE;
        shadow_map_scissor.offset.x = 0;
        shadow_map_scissor.offset.y = 0;
        vkCmdSetScissor(command_buffer, 0, 1, &shadow_map_scissor);

        vkCmdBindPipeline(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::DirectionalLightPipeline].pipeline
        );
        dir_light_space_matrix[directional_light_counter] =
            directional_lights[directional_light_counter]
                .directional_light_space_matrix;

        uint32_t actors_number = actors.size();
        for (unsigned int actor_counter = 0; actor_counter < actors_number;
             ++actor_counter) {
            RenderActor actor = actors[actor_counter];
            unsigned int mesh_id = actor.mesh_id;
            unsigned int ubo_directional_light_index = directional_light_number
                    * actors_number * directional_light_current_frame
                + actors_number * directional_light_counter + actor_counter;

            update_directional_light_shadow_map_matrix_ubo(
                ubo_directional_light_index,
                directional_light_counter,
                actor_counter
            );
            const unsigned int linked_descriptor_set_id =
                PIPELINE_CONFIGS[SpecificPipeline::DirectionalLightPipeline]
                    .linked_descriptor_set_i_ds[0];
            const DescriptorSet& current_descriptor_set =
                DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
            vkCmdBindDescriptorSets(
                command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                PIPELINE_CONFIGS[SpecificPipeline::DirectionalLightPipeline]
                    .pipeline_layout,
                0,
                1,
                &(*(DESCRIPTOR_SETS_CHUNKS.data()
                    + current_descriptor_set.descriptor_set_offset
                    + ubo_directional_light_index)),
                0,
                nullptr
            );

            VkBuffer vertex_buffers[] = {vertex_buffer_container[mesh_id]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(
                command_buffer,
                0,
                1,
                vertex_buffers,
                offsets
            );

            vkCmdBindIndexBuffer(
                command_buffer,
                index_buffer_container[mesh_id],
                0,
                VK_INDEX_TYPE_UINT32
            );

            unsigned int indices_container_size = a_indices[mesh_id].size();
            vkCmdDrawIndexed(
                command_buffer,
                static_cast<uint32_t>(indices_container_size),
                1,
                0,
                0,
                0
            );
        }

        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void CVulkanRenderer::spot_light_record_command_buffer(
    std::vector<VkCommandBuffer>& command_buffers,
    uint32_t current_frame
) {
    for (uint32_t spot_light_counter = 0;
         spot_light_counter < spot_lights.size();
         ++spot_light_counter) {
        VkCommandBufferInheritanceInfo inheritance_info {};
        inheritance_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritance_info.renderPass =
            RENDER_PASSES[SpecificPipeline::SpotLightPipeline];
        inheritance_info.framebuffer =
            spot_light_shadow_map_frame_buffers[spot_light_counter];
        inheritance_info.subpass = 0;

        VkCommandBufferBeginInfo begin_info {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        begin_info.pInheritanceInfo = &inheritance_info;

        VkCommandBuffer command_buffer = command_buffers
            [current_frame * spot_light_number + spot_light_counter];
        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
            throw std::runtime_error(
                "failed to begin recording command buffer!"
            );
        }

        VkViewport spot_light_shadow_map_view_port;
        spot_light_shadow_map_view_port.height = FLAT_SHADOW_MAP_SIZE;
        spot_light_shadow_map_view_port.width = FLAT_SHADOW_MAP_SIZE;
        spot_light_shadow_map_view_port.minDepth = 0.0f;
        spot_light_shadow_map_view_port.maxDepth = 1.0f;
        spot_light_shadow_map_view_port.x = 0;
        spot_light_shadow_map_view_port.y = 0;
        vkCmdSetViewport(command_buffer, 0, 1, &spot_light_shadow_map_view_port);

        VkRect2D spot_light_shadow_map_scissor;
        spot_light_shadow_map_scissor.extent.width = FLAT_SHADOW_MAP_SIZE;
        spot_light_shadow_map_scissor.extent.height = FLAT_SHADOW_MAP_SIZE;
        spot_light_shadow_map_scissor.offset.x = 0;
        spot_light_shadow_map_scissor.offset.y = 0;
        vkCmdSetScissor(command_buffer, 0, 1, &spot_light_shadow_map_scissor);

        vkCmdBindPipeline(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::SpotLightPipeline].pipeline
        );

        spot_light_space_matrix[spot_light_counter] =
            spot_lights[spot_light_counter].spot_ligth_space_matrix;
        uint32_t actors_number = actors.size();
        for (unsigned int actors_counter = 0; actors_counter < actors_number;
             ++actors_counter) {
            RenderActor actor = actors[actors_counter];
            unsigned int mesh_id = actor.mesh_id;
            unsigned int ubo_spot_light_index =
                spot_light_number * actors_number * spot_light_current_frame
                + actors_number * spot_light_counter + actors_counter;

            update_spot_light_shadow_map_matrix_ubo(
                ubo_spot_light_index,
                spot_light_counter,
                actors_counter
            );
            const unsigned int linked_descriptor_set_id =
                PIPELINE_CONFIGS[SpecificPipeline::SpotLightPipeline]
                    .linked_descriptor_set_i_ds[0];
            const DescriptorSet& current_descriptor_set =
                DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
            vkCmdBindDescriptorSets(
                command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                PIPELINE_CONFIGS[SpecificPipeline::SpotLightPipeline]
                    .pipeline_layout,
                0,
                1,
                &(*(DESCRIPTOR_SETS_CHUNKS.data()
                    + current_descriptor_set.descriptor_set_offset
                    + ubo_spot_light_index)),
                0,
                nullptr
            );
            VkBuffer vertex_buffers[] = {vertex_buffer_container[mesh_id]};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(
                command_buffer,
                0,
                1,
                vertex_buffers,
                offsets
            );

            vkCmdBindIndexBuffer(
                command_buffer,
                index_buffer_container[mesh_id],
                0,
                VK_INDEX_TYPE_UINT32
            );

            unsigned int indices_container_size = a_indices[mesh_id].size();
            vkCmdDrawIndexed(
                command_buffer,
                static_cast<uint32_t>(indices_container_size),
                1,
                0,
                0,
                0
            );
        }

        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void CVulkanRenderer::point_light_record_command_buffer(
    std::vector<VkCommandBuffer>& command_buffers,
    uint32_t current_frame
) {
    for (uint32_t point_light_counter = 0;
         point_light_counter < point_lights.size();
         ++point_light_counter) {
        uint32_t max_cube_map_layers = 6;
        // 6 is a number of cube map layers.
        for (uint32_t cube_map_layer_counter = 0;
             cube_map_layer_counter < max_cube_map_layers;
             ++cube_map_layer_counter) {
            VkCommandBufferInheritanceInfo inheritance_info {};
            inheritance_info.sType =
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            inheritance_info.renderPass =
                RENDER_PASSES[SpecificPipeline::PointLightPipeline];
            inheritance_info.framebuffer =
                point_light_shadow_map_frame_buffers[point_light_counter]
                                                    [cube_map_layer_counter];
            inheritance_info.subpass = 0;

            VkCommandBufferBeginInfo begin_info {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            begin_info.pInheritanceInfo = &inheritance_info;

            VkCommandBuffer command_buffer = command_buffers
                [current_frame * point_light_number * max_cube_map_layers
                 + point_light_counter * max_cube_map_layers
                 + cube_map_layer_counter];
            if (vkBeginCommandBuffer(command_buffer, &begin_info)
                != VK_SUCCESS) {
                throw std::runtime_error(
                    "failed to begin recording command buffer!"
                );
            }

            VkViewport point_light_shadow_map_view_port;
            point_light_shadow_map_view_port.height = SHADOW_MAP_SIZE;
            point_light_shadow_map_view_port.width = SHADOW_MAP_SIZE;
            point_light_shadow_map_view_port.minDepth = 0.0f;
            point_light_shadow_map_view_port.maxDepth = 1.0f;
            point_light_shadow_map_view_port.x = 0;
            point_light_shadow_map_view_port.y = 0;
            vkCmdSetViewport(
                command_buffer,
                0,
                1,
                &point_light_shadow_map_view_port
            );

            VkRect2D point_light_shadow_map_scissor;
            point_light_shadow_map_scissor.extent.width = SHADOW_MAP_SIZE;
            point_light_shadow_map_scissor.extent.height = SHADOW_MAP_SIZE;
            point_light_shadow_map_scissor.offset.x = 0;
            point_light_shadow_map_scissor.offset.y = 0;
            vkCmdSetScissor(
                command_buffer,
                0,
                1,
                &point_light_shadow_map_scissor
            );

            vkCmdBindPipeline(
                command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                PIPELINE_CONFIGS[SpecificPipeline::PointLightPipeline].pipeline
            );

            uint32_t actors_number = actors.size();
            for (unsigned int actor_counter = 0; actor_counter < actors_number;
                 ++actor_counter) {
                RenderActor actor = actors[actor_counter];
                unsigned int mesh_id = actor.mesh_id;

                unsigned int ubo_index = point_light_number * actors_number
                        * max_cube_map_layers * point_light_current_frame
                    + actors_number * max_cube_map_layers * point_light_counter
                    + max_cube_map_layers * actor_counter
                    + cube_map_layer_counter;

                update_point_light_shadow_map_matrix_ubo(
                    ubo_index,
                    point_light_counter,
                    cube_map_layer_counter,
                    actor_counter
                );
                const unsigned int linked_descriptor_set_id =
                    PIPELINE_CONFIGS[SpecificPipeline::PointLightPipeline]
                        .linked_descriptor_set_i_ds[0];
                const DescriptorSet& current_descriptor_set =
                    DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
                vkCmdBindDescriptorSets(
                    command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    PIPELINE_CONFIGS[SpecificPipeline::PointLightPipeline]
                        .pipeline_layout,
                    0,
                    1,
                    &(*(DESCRIPTOR_SETS_CHUNKS.data()
                        + current_descriptor_set.descriptor_set_offset
                        + ubo_index)),
                    0,
                    nullptr
                );

                VkBuffer vertex_buffers[] = {vertex_buffer_container[mesh_id]};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(
                    command_buffer,
                    0,
                    1,
                    vertex_buffers,
                    offsets
                );

                vkCmdBindIndexBuffer(
                    command_buffer,
                    index_buffer_container[mesh_id],
                    0,
                    VK_INDEX_TYPE_UINT32
                );

                unsigned int indices_container_size = a_indices[mesh_id].size();
                vkCmdDrawIndexed(
                    command_buffer,
                    static_cast<uint32_t>(indices_container_size),
                    1,
                    0,
                    0,
                    0
                );
            }

            if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }
}

VkShaderModule CVulkanRenderer::create_shader_module(
    const std::vector<char>& code
) {
    VkShaderModuleCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shader_module;
}

VkSurfaceFormatKHR CVulkanRenderer::choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& available_formats
) {
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB
            && available_format.colorSpace
                == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR CVulkanRenderer::choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& available_present_modes
) {
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR
            || available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D CVulkanRenderer::choose_swap_extent(
    const VkSurfaceCapabilitiesKHR& capabilities
) {
    if (capabilities.currentExtent.width
        != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actual_extent {
            .width = window->width,
            .height = window->height
        };
        actual_extent.width = std::clamp(
            actual_extent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        );
        actual_extent.height = std::clamp(
            actual_extent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        );

        return actual_extent;
    }
}

SwapChainSupportDetails CVulkanRenderer::query_swap_chain_support(
    VkPhysicalDevice device
) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device,
        surface,
        &details.capabilities
    );

    uint32_t format_count = 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(
            device,
            surface,
            &format_count,
            nullptr
        )
        != VK_SUCCESS) {
        format_count = 0;
    }

    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device,
            surface,
            &format_count,
            details.formats.data()
        );
    }

    uint32_t present_mode_count = 0;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface,
            &present_mode_count,
            nullptr
        )
        != VK_SUCCESS) {
        present_mode_count = 0;
    }

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface,
            &present_mode_count,
            details.present_modes.data()
        );
    }

    return details;
}

bool CVulkanRenderer::is_device_suitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = find_queue_families(device);

    bool extensions_supported = check_device_extension_support(device);

    bool swap_chain_adequate = false;
    if (extensions_supported) {
        SwapChainSupportDetails swap_chain_support =
            query_swap_chain_support(device);
        swap_chain_adequate = !swap_chain_support.formats.empty()
            && !swap_chain_support.present_modes.empty();
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);

    return indices.is_complete() && extensions_supported && swap_chain_adequate
        && supported_features.samplerAnisotropy
        && supported_features.fillModeNonSolid;
}

bool CVulkanRenderer::check_device_extension_support(VkPhysicalDevice device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extension_count,
        nullptr
    );

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extension_count,
        available_extensions.data()
    );

    std::set<std::string> required_extensions(
        DEVICE_EXTENSIONS.begin(),
        DEVICE_EXTENSIONS.end()
    );

    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

QueueFamilyIndices CVulkanRenderer::find_queue_families(
    VkPhysicalDevice device
) {
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &queue_family_count,
        nullptr
    );

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &queue_family_count,
        queue_families.data()
    );

    int i = 0;
    for (const auto& queue_family : queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,
            i,
            surface,
            &present_support
        );

        if (present_support) {
            indices.present_family = i;
        }

        if (indices.is_complete()) {
            break;
        }

        i++;
    }

    return indices;
}

std::vector<const char*> CVulkanRenderer::get_required_extensions() {
#ifdef VK_USE_PLATFORM_XLIB_KHR
    std::vector<const char*> p_required_extensions = {
        "VK_KHR_xlib_surface",
        "VK_EXT_acquire_xlib_display",
        "VK_KHR_display",
        "VK_KHR_surface",
        "VK_EXT_direct_mode_display",
    };
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    std::vector<const char*> p_required_extensions = {
        "VK_KHR_xcb_surface",
        "VK_KHR_display",
        "VK_KHR_surface",
        "VK_EXT_direct_mode_display"
    };
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    std::vector<const char*> p_required_extensions = {
        "VK_KHR_win32_surface",
        "VK_KHR_surface"
    };
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    std::vector<const char*> p_required_extensions = {
        "VK_KHR_wayland_surface",
        "VK_KHR_display",
        "VK_EXT_direct_mode_display",
        "VK_KHR_surface"
    };
#endif
    if (ENABLE_VALIDATION_LAYERS) {
        p_required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return p_required_extensions;
}

bool CVulkanRenderer::check_validation_layer_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
    for (const char* layer_name : VALIDATION_LAYERS) {
        bool layer_found = false;
        for (const auto& layer_properties : available_layers) {
            if (strcmp(layer_name, layer_properties.layerName) == 0) {
                layer_found = true;
                break;
            }
        }
        if (!layer_found) {
            return false;
        }
    }
    return true;
}

VkDescriptorBufferInfo CVulkanRenderer::create_descriptor_buffer_info(
    VkBuffer ubo,
    const VkDeviceSize& ubo_struct_size,
    const VkDeviceSize& offset_step
) {
    VkDescriptorBufferInfo ubo_buffer_info {};
    ubo_buffer_info.buffer = ubo;
    ubo_buffer_info.offset = offset_step * ubo_struct_size;
    ubo_buffer_info.range = ubo_struct_size;
    return ubo_buffer_info;
}

VkDescriptorImageInfo CVulkanRenderer::create_descriptor_image_info(
    const GpuImage& texture_image,
    VkImageLayout layout,
    unsigned int texture_view_index,
    VkSampler texture_sampler
) {
    VkDescriptorImageInfo image_info {};
    image_info.imageLayout = layout;
    image_info.imageView = texture_image.views[texture_view_index];
    image_info.sampler = texture_sampler;
    return image_info;
}

std::vector<char> CVulkanRenderer::read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t file_size = (size_t)file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}

VKAPI_ATTR VkBool32 VKAPI_CALL CVulkanRenderer::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data
) {
    return VK_FALSE;
}
} // namespace glvm

namespace glvm {

void CJsonParser::read_file(const char* file_path) {
    std::ifstream json_file_input_stream;
    std::stringstream json_file_output_stream;

    json_file_input_stream.open(file_path);
    if (json_file_input_stream.good()) {
        json_file_output_stream << json_file_input_stream.rdbuf();
        json_file_input_stream.close();
        s_json_file_data = json_file_output_stream.str();
    } else {
        return;
    }

    p_json_file_data = s_json_file_data.c_str();
}

void CJsonParser::parse() {
    current_char = p_json_file_data[global_file_counter];

    while (current_char != '\0') {
        current_char = p_json_file_data[global_file_counter];

        if (current_char == '"' && key_flag) {
            last_key = string_parse();
            while (current_char == ' ' || current_char == ':') {
                ++global_file_counter;
                current_char = p_json_file_data[global_file_counter];
            }
        }

        if (current_char == '"') {
            buffer_string = string_parse();

            if (key_flag) {
                JsonValue json_string(buffer_string);
                (*stack_of_json_values.back()->value.object)[last_key] =
                    json_string;
            } else {
                JsonValue json_string(buffer_string);
                stack_of_json_values.back()->value.array->push_back(json_string);
            }
        } else if (
            (current_char >= '0' && current_char <= '9') || current_char == '+'
            || current_char == '-'
        ) {
            buffer_string = number_as_string_parse();
            std::vector<char> vector = string_to_vector_of_chars(buffer_string);
            double f_number = 0.0f;
            int i_number = 0;
            if (is_contain_char(buffer_string, '.')) {
                f_number = parse_floating(vector);

                if (key_flag) {
                    JsonValue json_float(f_number);
                    (*stack_of_json_values.back()->value.object)[last_key] =
                        json_float;
                } else {
                    JsonValue json_float(f_number);
                    stack_of_json_values.back()->value.array->push_back(
                        json_float
                    );
                }
            } else {
                i_number = parse_integer(vector);

                if (key_flag) {
                    JsonValue json_int(i_number);
                    (*stack_of_json_values.back()->value.object)[last_key] =
                        json_int;
                } else {
                    JsonValue json_int(i_number);
                    stack_of_json_values.back()->value.array->push_back(
                        json_int
                    );
                }
            }

        } else if (
            current_char == 't' || current_char == 'f' || current_char == 'n'
        ) {
            std::string bool_or_null_string = bool_or_null_parse();

            if (bool_or_null_string == "true") {
                if (key_flag) {
                    JsonValue json_true(true);
                    (*stack_of_json_values.back()->value.object)[last_key] =
                        json_true;
                } else {
                    JsonValue json_true(true);
                    stack_of_json_values.back()->value.array->push_back(
                        json_true
                    );
                }
            } else if (bool_or_null_string == "false") {
                if (key_flag) {
                    JsonValue json_false(false);
                    (*stack_of_json_values.back()->value.object)[last_key] =
                        json_false;
                } else {
                    JsonValue json_false(false);
                    stack_of_json_values.back()->value.array->push_back(
                        json_false
                    );
                }
            } else if (bool_or_null_string == "null") {
                if (key_flag) {
                    JsonValue json_null;
                    json_null.type = JsonNull;
                    json_null.value.null = NULL;
                    (*stack_of_json_values.back()->value.object)[last_key] =
                        json_null;
                } else {
                    JsonValue json_null;
                    json_null.type = JsonNull;
                    json_null.value.null = NULL;
                    stack_of_json_values.back()->value.array->push_back(
                        json_null
                    );
                }
            }
        } else if (current_char == '{') {
            if (stack_of_json_values.size() == 0) {
                root = new JsonValue;
                *root = create_json_hash_map();
                stack_of_json_values.push_back(root);
            } else if (key_flag) {
                JsonValue json_object = create_json_hash_map();
                (*stack_of_json_values.back()->value.object)[last_key] =
                    json_object;
                stack_of_json_values.push_back(
                    &(*stack_of_json_values.back()->value.object)[last_key]
                );
            } else if (!key_flag) {
                JsonValue json_object = create_json_hash_map();
                stack_of_json_values.back()->value.array->push_back(json_object);
                stack_of_json_values.push_back(
                    &stack_of_json_values.back()->value.array->back()
                );
            }

            key_flag = true;
        } else if (current_char == '[') {
            if (stack_of_json_values.size() == 0) {
                root = new JsonValue;
                *root = create_json_array();
                stack_of_json_values.push_back(root);
            } else if (key_flag) {
                JsonValue json_array = create_json_array();
                (*stack_of_json_values.back()->value.object)[last_key] =
                    json_array;
                stack_of_json_values.push_back(
                    &(*stack_of_json_values.back()->value.object)[last_key]
                );
            } else if (!key_flag) {
                JsonValue json_array = create_json_array();
                stack_of_json_values.back()->value.array->push_back(json_array);
                stack_of_json_values.push_back(
                    &stack_of_json_values.back()->value.array->back()
                );
            }

            key_flag = false;
        } else if (current_char == '}') {
            stack_of_json_values.pop_back();
            if (stack_of_json_values.size()
                && stack_of_json_values.back()->type == JsonObject) {
                key_flag = true;
            } else {
                key_flag = false;
            }

        } else if (current_char == ']') {
            stack_of_json_values.pop_back();
            if (stack_of_json_values.size()
                && stack_of_json_values.back()->type == JsonObject) {
                key_flag = true;
            } else {
                key_flag = false;
            }
        }

        ++global_file_counter;
    }
}

JsonValue CJsonParser::create_json_hash_map() {
    JsonValue json_object;
    json_object.type = JsonObject;
    json_object.value.object = new HashMap<std::string, JsonValue>;
    return json_object;
}

JsonValue CJsonParser::create_json_array() {
    JsonValue json_array;
    json_array.type = JsonArray;
    json_array.value.array = new std::vector<JsonValue>;
    return json_array;
}

std::string CJsonParser::bool_or_null_parse() {
    std::string bool_or_null_string = "";
    while (1) {
        current_char = p_json_file_data[global_file_counter];
        if (current_char >= 'a' && current_char <= 'z') {
            bool_or_null_string.push_back(current_char);
            ++global_file_counter;
        } else {
            return bool_or_null_string;
        }
    }
}

bool CJsonParser::is_contain_char(std::string text, char character) {
    for (unsigned int i = 0; i < text.size(); ++i) {
        if (text[i] == character) {
            return true;
        }
    }

    return false;
}

std::string CJsonParser::number_as_string_parse() {
    std::string number_as_string = "";
    while (1) {
        current_char = p_json_file_data[global_file_counter];
        if ((current_char >= '0' && current_char <= '9') || current_char == '+'
            || current_char == '-' || current_char == 'e') {
            number_as_string.push_back(current_char);
            ++global_file_counter;
        } else if (current_char == '.') {
            number_as_string.push_back(current_char);
            ++global_file_counter;
        } else {
            return number_as_string;
        }
    }
}

std::string CJsonParser::string_parse() {
    ++global_file_counter;
    std::string local_buffer = "";
    while (1) {
        current_char = p_json_file_data[global_file_counter];
        if (current_char == '"') {
            ++global_file_counter;
            current_char = p_json_file_data[global_file_counter];
            return local_buffer;
        } else {
            local_buffer.push_back(current_char);
            ++global_file_counter;
        }
    }
}

std::vector<char> CJsonParser::string_to_vector_of_chars(std::string text) {
    std::vector<char> vector_with_chars;
    for (unsigned int i = 0; i < text.size(); ++i) {
        vector_with_chars.push_back(text[i]);
    }

    return vector_with_chars;
}

int CJsonParser::parse_integer(std::vector<char> digits) {
    std::vector<int> base_container;

    for (unsigned int i = 0; i < digits.size(); ++i) {
        base_container.push_back(digits[i] - 48);
    }

    int i_result = 0;
    bool negate_flag = false;

    unsigned int base_container_size = base_container.size();
    for (unsigned int i = 0; i < base_container_size; ++i) {
        if (base_container[i] == -3 && i == 0) {
            negate_flag = true;
            continue;
        } else if (base_container[i] == -5 && i == 0) {
            continue;
        }

        i_result +=
            base_container[i] * std::pow(10, (base_container_size - 1) - i);
    }

    if (negate_flag) {
        i_result *= -1;
    }

    return i_result;
}

double CJsonParser::parse_floating(std::vector<char> digits) {
    std::vector<int> base_container;

    for (unsigned int i = 0; i < digits.size(); ++i) {
        base_container.push_back(digits[i] - 48);
    }

    int integer_part = 0;
    double floating_part = 0;
    int e_number = 0;
    std::vector<int> integer_part_container;
    std::vector<int> floating_part_container;
    std::vector<int> e_part_container;
    bool dot_flag = false;
    bool negate_flag = false;
    bool e_flag = false;
    // False value equal "+" sign.
    bool e_sign = false;
    unsigned int base_container_size = base_container.size();

    if (base_container[0] == -3) {
        negate_flag = true;
    }

    for (unsigned int i = 0; i < base_container_size; ++i) {
        if (negate_flag && i == 0) {
            continue;
        } else if (base_container[i] == -5 && i == 0) {
            continue;
        } else if (base_container[i] == -2) {
            dot_flag = true;
            continue;
        } else if (base_container[i] == 53) {
            e_flag = true;
            continue;
        }

        if (e_flag) {
            if (base_container[i] == -5) {
                continue;
            } else if (base_container[i] == -3) {
                e_sign = true;
                continue;
            }

            e_part_container.push_back(base_container[i]);
            continue;
        }

        if (base_container[i] >= 0 && base_container[i] <= 9) {
            if (dot_flag) {
                floating_part_container.push_back(base_container[i]);
            } else {
                integer_part_container.push_back(base_container[i]);
            }
        } else {
            return NAN;
        }
    }

    unsigned int e_part_container_size = e_part_container.size();
    for (unsigned int i = 0; i < e_part_container_size; ++i) {
        e_number +=
            e_part_container[i] * std::pow(10, (e_part_container_size - 1) - i);
    }

    unsigned int integer_part_container_size = integer_part_container.size();
    for (unsigned int i = 0; i < integer_part_container_size; ++i) {
        integer_part += integer_part_container[i]
            * std::pow(10, (integer_part_container_size - 1) - i);
    }

    unsigned int floating_part_container_size = floating_part_container.size();
    for (unsigned int i = 0; i < floating_part_container_size; ++i) {
        floating_part += floating_part_container[i] / std::pow(10, i + 1);
    }

    double result = 0;
    result = (double)(integer_part + floating_part);

    if (e_flag) {
        if (e_sign) {
            result /= std::pow(10, e_number);
        } else {
            result *= std::pow(10, e_number);
        }
    }

    if (negate_flag) {
        result *= -1.0f;
    }

    return result;
}

void CJsonParser::search_in_json_array(
    std::vector<JsonValue>* array_value,
    const char* key,
    std::vector<JsonValue>& result_vector
) const {
    for (unsigned int i = 0; i < array_value->size(); ++i) {
        if ((*array_value)[i].type == JsonObject) {
            search_in_json_object(
                (*array_value)[i].value.object,
                key,
                result_vector
            );
        }

        if ((*array_value)[i].type == JsonArray) {
            search_in_json_array(
                (*array_value)[i].value.array,
                key,
                result_vector
            );
        }
    }
}

void CJsonParser::search_in_json_object(
    HashMap<std::string, JsonValue>* map_value,
    const char* key,
    std::vector<JsonValue>& result_vector
) const {
    for (auto& [current_key, current_value] : *map_value) {
        if (current_key == key) {
            result_vector.push_back(current_value);
        }

        if (current_value.type == JsonObject) {
            search_in_json_object(
                current_value.value.object,
                key,
                result_vector
            );
        }

        if (current_value.type == JsonArray) {
            search_in_json_array(current_value.value.array, key, result_vector);
        }
    }
}

std::vector<JsonValue> CJsonParser::search(const char* key) const {
    std::vector<JsonValue> result_vector;
    search_in_json_object(root->value.object, key, result_vector);
    return result_vector;
}

template<typename T>
bool is_element_exist(const T element, const std::vector<T>& array) {
    for (uint32_t n = 0; n < array.size(); ++n) {
        if (element == array[n]) {
            return true;
        }
    }

    return false;
}

template<typename T>
T get_element_index(const T element, const std::vector<T>& array) {
    for (uint32_t n = 0; n < array.size(); ++n) {
        if (element == array[n]) {
            return n;
        }
    }

    return std::numeric_limits<T>::max();
}

void calculate_elements_memory_size(
    const uint32_t indices_elements_count,
    std::string* indices_element_type,
    const uint32_t indices_componet_type,
    uint32_t* indices_buffer_view_byte_length
) {
    if (*indices_element_type == "VEC2") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 2;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 8;
        }
    } else if (*indices_element_type == "VEC3") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 3;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 6;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 12;
        }
    } else if (*indices_element_type == "VEC4") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 8;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 16;
        }
    } else if (*indices_element_type == "SCALAR") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 2;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        }
    } else if (*indices_element_type == "MAT4") {
        *indices_buffer_view_byte_length = indices_elements_count * 64;
    }
}

struct ComponentType {
    enum Type {
        I8 = 5120,
        U8 = 5121,
        I16 = 5122,
        U16 = 5123,
        U32 = 5125,
        F32 = 5126
    };
};

void calculate_byte_step(uint32_t componet_type, unsigned int* byte_step) {
    if (componet_type == ComponentType::I8
        || componet_type == ComponentType::U8) {
        *byte_step = 1;
    } else if (
        componet_type == ComponentType::I16
        || componet_type == ComponentType::U16
    ) {
        *byte_step = 2;
    } else if (
        componet_type == ComponentType::U32
        || componet_type == ComponentType::F32
    ) {
        *byte_step = 4;
    }
}

// Metadata structs to binary buffer with actual data.
struct AccessorMetaData {
    uint32_t buffer_view;
    uint32_t byte_offset;
    uint32_t component_type;
    uint32_t count;
    std::string type;
};

struct BufferViewMetaData {
    uint32_t byte_length;
    uint32_t byte_offset;
};

[[nodiscard]] AccessorMetaData read_accessor_meta_data(
    JsonValue* gltf,
    const uint32_t accessor_index
) {
    AccessorMetaData buffer_meta_data;
    buffer_meta_data.buffer_view =
        (*gltf)["accessors"][accessor_index]["bufferView"].value.i_number;
    buffer_meta_data.count =
        (*gltf)["accessors"][accessor_index]["count"].value.i_number;
    buffer_meta_data.type =
        *(*gltf)["accessors"][accessor_index]["type"].value.string;
    buffer_meta_data.component_type =
        (*gltf)["accessors"][accessor_index]["componentType"].value.i_number;

    buffer_meta_data.byte_offset = 0;
    if ((*gltf)["accessors"][accessor_index].is_object() == JsonObject) {
        HashMap<std::string, JsonValue>* ptr =
            (*gltf)["accessors"][accessor_index].value.object;
        if (ptr->contains("byteOffset")) {
            buffer_meta_data.byte_offset =
                (*gltf)["accessors"][accessor_index]["byteOffset"]
                    .value.i_number;
        }
    }

    return buffer_meta_data;
}

[[nodiscard]] BufferViewMetaData read_buffer_view_meta_data(
    JsonValue* gltf,
    const uint32_t buffer_view_index
) {
    BufferViewMetaData buffer_view_meta_data;
    buffer_view_meta_data.byte_length =
        (*gltf)["bufferViews"][buffer_view_index]["byteLength"].value.i_number;
    buffer_view_meta_data.byte_offset = 0;
    if ((*gltf)["bufferViews"][buffer_view_index].is_object() == JsonObject) {
        HashMap<std::string, JsonValue>* ptr =
            (*gltf)["bufferViews"][buffer_view_index].value.object;
        if (ptr->contains("byteOffset")) {
            buffer_view_meta_data.byte_offset =
                (*gltf)["bufferViews"][buffer_view_index]["byteOffset"]
                    .value.i_number;
        }
    }

    return buffer_view_meta_data;
}

template<typename T>
void read_binary_buffer_data(
    char* buffer,
    AccessorMetaData accessor_meta_data,
    BufferViewMetaData buffer_view_meta_data,
    std::vector<T>& output_data
) {
    uint32_t indices_byte_step = 0;
    calculate_byte_step(accessor_meta_data.component_type, &indices_byte_step);

    unsigned int byte_length = 0;
    calculate_elements_memory_size(
        accessor_meta_data.count,
        &accessor_meta_data.type,
        accessor_meta_data.component_type,
        &byte_length
    );
    for (unsigned int i =
             buffer_view_meta_data.byte_offset + accessor_meta_data.byte_offset;
         i < buffer_view_meta_data.byte_offset + accessor_meta_data.byte_offset
             + byte_length;
         i += indices_byte_step) {
        switch (accessor_meta_data.component_type) {
            case ComponentType::U8:
                output_data.push_back(
                    reinterpret_cast<unsigned char&>(buffer[i])
                );
                break;
            case ComponentType::U16:
                output_data.push_back(
                    reinterpret_cast<unsigned short&>(buffer[i])
                );
                break;
            case ComponentType::U32:
                output_data.push_back(
                    reinterpret_cast<unsigned int&>(buffer[i])
                );
                break;
            case ComponentType::F32:
                output_data.push_back(reinterpret_cast<float&>(buffer[i]));
                break;
        }
    }
}

void CJsonParser::load_gltf(
    const char* paths_gltf,
    std::vector<float>& a_vertexes,
    std::vector<uint32_t>& a_indices,
    std::vector<std::vector<Matrix<float, 4>>>& joint_matrices_per_mesh,
    std::vector<float>& frames,
    bool& no_animations,
    float& top_y
) {
    read_file(paths_gltf);
    parse();
    JsonValue* gltf = get_root();
    std::string binary_path = *(*gltf)["buffers"][0]["uri"].value.string;
    int full_byte_size = (*gltf)["buffers"][0]["byteLength"].value.i_number;
    size_t last_separator = std::string(paths_gltf).find_last_of("/\\");
    std::string binary_full_path = last_separator == std::string::npos
        ? binary_path
        : std::string(paths_gltf).substr(0, last_separator + 1) + binary_path;
    std::ifstream in_stream;
    in_stream.open(binary_full_path, std::ios::binary);
    if (!in_stream.is_open()) {
        throw std::runtime_error(
            "failed to open gltf binary buffer: " + binary_full_path
        );
    }
    char* buffer = new char[full_byte_size];
    in_stream.read(buffer, full_byte_size);
    in_stream.close();
    const uint32_t indices_accessor_index =
        (*gltf)["meshes"][0]["primitives"][0]["indices"].value.i_number;
    AccessorMetaData indices_accessor_meta_data =
        read_accessor_meta_data(gltf, indices_accessor_index);
    BufferViewMetaData indices_buffer_view_meta_data =
        read_buffer_view_meta_data(gltf, indices_accessor_meta_data.buffer_view);
    std::vector<uint32_t> indices;
    read_binary_buffer_data(
        buffer,
        indices_accessor_meta_data,
        indices_buffer_view_meta_data,
        indices
    );
    const uint32_t vertices_position_accessor_index =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["POSITION"]
            .value.i_number;
    AccessorMetaData vertices_position_accessor_meta_data =
        read_accessor_meta_data(gltf, vertices_position_accessor_index);
    BufferViewMetaData vertices_position_buffer_view_meta_data =
        read_buffer_view_meta_data(
            gltf,
            vertices_position_accessor_meta_data.buffer_view
        );
    std::vector<float> vertices_position;
    read_binary_buffer_data(
        buffer,
        vertices_position_accessor_meta_data,
        vertices_position_buffer_view_meta_data,
        vertices_position
    );
    const uint32_t texture_coordinates_accessor_index =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["TEXCOORD_0"]
            .value.i_number;
    AccessorMetaData texture_coordinates_accessor_meta_data =
        read_accessor_meta_data(gltf, texture_coordinates_accessor_index);
    BufferViewMetaData texture_coordinates_buffer_view_meta_data =
        read_buffer_view_meta_data(
            gltf,
            texture_coordinates_accessor_meta_data.buffer_view
        );
    std::vector<float> texture_coordinates;
    read_binary_buffer_data(
        buffer,
        texture_coordinates_accessor_meta_data,
        texture_coordinates_buffer_view_meta_data,
        texture_coordinates
    );
    const uint32_t normals_accessor_index =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["NORMAL"]
            .value.i_number;
    AccessorMetaData normals_accessor_meta_data =
        read_accessor_meta_data(gltf, normals_accessor_index);
    BufferViewMetaData normals_buffer_view_meta_data =
        read_buffer_view_meta_data(gltf, normals_accessor_meta_data.buffer_view);
    std::vector<float> normals;
    read_binary_buffer_data(
        buffer,
        normals_accessor_meta_data,
        normals_buffer_view_meta_data,
        normals
    );
    std::vector<JsonValue> skins = search("skins");
    JsonValue joints;
    std::vector<Matrix<float, 4>> global_transform_joint_node;
    std::vector<Matrix<float, 4>> inverse_bind_matrix_set;
    std::vector<std::vector<Matrix<float, 4>>> joint_matrices;
    std::vector<float> weights_container;
    std::vector<int> joints_indices;
    std::vector<std::vector<int>> children;
    if (skins.size() > 0) {
        no_animations = false;
        joints = (*gltf)["skins"][0]["joints"];
        JsonValue nodes = (*gltf)["nodes"];
        // Loop on joints.
        for (unsigned int i = 0; i < joints.value.array->size(); ++i) {
            unsigned int joint_index_map_to_node =
                (*joints.value.array)[i].value.i_number;
            JsonValue node = nodes[joint_index_map_to_node];
            Quaternion rotation_quaternion;
            Matrix<float, 4> rotation(1.0f);
            Matrix<float, 4> scale(1.0f);
            Matrix<float, 4> translation(1.0f);
            if (node.value.object->contains("rotation")) {
                JsonValue array = (*node.value.object)["rotation"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    switch (i) {
                        case 0:
                            if (array[i].is_interger()) {
                                rotation_quaternion.x = array[i].value.i_number;
                            } else if (array[i].is_float()) {
                                rotation_quaternion.x = array[i].value.f_number;
                            }
                            break;
                        case 1:
                            if (array[i].is_interger()) {
                                rotation_quaternion.y = array[i].value.i_number;
                            } else if (array[i].is_float()) {
                                rotation_quaternion.y = array[i].value.f_number;
                            }
                            break;
                        case 2:
                            if (array[i].is_interger()) {
                                rotation_quaternion.z = array[i].value.i_number;
                            } else if (array[i].is_float()) {
                                rotation_quaternion.z = array[i].value.f_number;
                            }
                            break;
                        case 3:
                            if (array[i].is_interger()) {
                                rotation_quaternion.w = array[i].value.i_number;
                            } else if (array[i].is_float()) {
                                rotation_quaternion.w = array[i].value.f_number;
                            }
                            break;
                    }
                }
                rotation = rotate_quaternion<float, 4>(rotation_quaternion);
                rotation.self_tensor_transpose();
            }
            std::vector<int> local_children;
            // Collect children indices.
            if (node.value.object->contains("children")) {
                JsonValue array = (*node.value.object)["children"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    local_children.push_back(array[i].value.i_number);
                }
                // Linearly put all children to every root joint.
                children.push_back(local_children);
            } else {
                std::vector<int> empty_children;
                // Put empty pack of children if can find a one.
                children.push_back(empty_children);
            }
            if (node.value.object->contains("scale")) {
                JsonValue array = (*node.value.object)["scale"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    if (array[i].is_interger()) {
                        scale[i][i] = array[i].value.i_number;
                    } else if (array[i].is_float()) {
                        scale[i][i] = array[i].value.f_number;
                    }
                }
            }
            if (node.value.object->contains("translation")) {
                JsonValue array = (*node.value.object)["translation"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    if (array[i].is_interger()) {
                        translation[3][i] = array[i].value.i_number;
                    } else if (array[i].is_float()) {
                        translation[3][i] = array[i].value.f_number;
                    }
                }
            }
            // Compute model matrix.
            Matrix<float, 4> model = scale * rotation * translation;
            global_transform_joint_node.push_back(model);
        }
        // Get the inverse bind matrices accessor index.
        const uint32_t inverse_bind_matrices_accessor_index =
            (*gltf)["skins"][0]["inverseBindMatrices"].value.i_number;
        AccessorMetaData inverse_bind_matrices_accessor_meta_data =
            read_accessor_meta_data(gltf, inverse_bind_matrices_accessor_index);
        BufferViewMetaData inveres_bind_matrices_buffer_view_meta_data =
            read_buffer_view_meta_data(
                gltf,
                inverse_bind_matrices_accessor_meta_data.buffer_view
            );
        std::vector<float> inverse_bind_matrices_data;
        read_binary_buffer_data(
            buffer,
            inverse_bind_matrices_accessor_meta_data,
            inveres_bind_matrices_buffer_view_meta_data,
            inverse_bind_matrices_data
        );
        Matrix<float, 4> inverse_bind_matrix(0.0f);
        for (unsigned int n = 0; n < joints.value.array->size(); ++n) {
            for (unsigned int g = 0; g < 4; ++g) {
                for (unsigned int j = 0; j < 4; ++j) {
                    // Put row float data into mat4.
                    inverse_bind_matrix[g][j] =
                        inverse_bind_matrices_data[n * 16 + g * 4 + j];
                }
            }
            inverse_bind_matrix_set.push_back(inverse_bind_matrix);
        }
        const uint32_t joints_accessor_index =
            (*gltf)["meshes"][0]["primitives"][0]["attributes"]["JOINTS_0"]
                .value.i_number;
        AccessorMetaData joints_accessor_meta_data =
            read_accessor_meta_data(gltf, joints_accessor_index);
        BufferViewMetaData joints_buffer_view_meta_data =
            read_buffer_view_meta_data(
                gltf,
                joints_accessor_meta_data.buffer_view
            );
        read_binary_buffer_data(
            buffer,
            joints_accessor_meta_data,
            joints_buffer_view_meta_data,
            joints_indices
        );
        unsigned int weights_accessor_index =
            (*gltf)["meshes"][0]["primitives"][0]["attributes"]["WEIGHTS_0"]
                .value.i_number;
        AccessorMetaData weights_accessor_meta_data =
            read_accessor_meta_data(gltf, weights_accessor_index);
        BufferViewMetaData weights_buffer_view_meta_data =
            read_buffer_view_meta_data(
                gltf,
                weights_accessor_meta_data.buffer_view
            );
        read_binary_buffer_data(
            buffer,
            weights_accessor_meta_data,
            weights_buffer_view_meta_data,
            weights_container
        );
    } else {
        no_animations = true;
    }
    std::vector<JsonValue> animations = search("animations");
    if (animations.size() > 0) {
        std::vector<JsonValue> sampler_indices;
        std::vector<JsonValue> target_nodes;
        std::vector<JsonValue> target_paths;
        JsonValue channels = (*gltf)["animations"][0]["channels"];
        for (unsigned int i = 0; i < channels.value.array->size(); ++i) {
            sampler_indices.push_back(channels[i]["sampler"]);
        }
        for (unsigned int i = 0; i < channels.value.array->size(); ++i) {
            target_nodes.push_back(channels[i]["target"]["node"]);
        }
        for (unsigned int i = 0; i < channels.value.array->size(); ++i) {
            target_paths.push_back(channels[i]["target"]["path"]);
        }
        std::vector<unsigned int> translation_sampler_indices;
        std::vector<unsigned int> rotation_sampler_indices;
        std::vector<unsigned int> scale_sampler_indices;
        std::vector<uint32_t> nodes_map_translations;
        std::vector<uint32_t> nodes_map_rotations;
        std::vector<uint32_t> nodes_map_scales;
        for (unsigned int i = 0; i < sampler_indices.size(); ++i) {
            if (*target_paths[i].value.string == "translation") {
                translation_sampler_indices.push_back(
                    sampler_indices[i].value.i_number
                );
                nodes_map_translations.push_back(target_nodes[i].value.i_number);
            } else if (*target_paths[i].value.string == "rotation") {
                rotation_sampler_indices.push_back(
                    sampler_indices[i].value.i_number
                );
                nodes_map_rotations.push_back(target_nodes[i].value.i_number);
            } else if (*target_paths[i].value.string == "scale") {
                scale_sampler_indices.push_back(
                    sampler_indices[i].value.i_number
                );
                nodes_map_scales.push_back(target_nodes[i].value.i_number);
            }
        }
        JsonValue samplers = (*gltf)["animations"][0]["samplers"];
        std::vector<unsigned int> translation_inputs;
        std::vector<unsigned int> translation_outputs;
        for (unsigned int i = 0; i < translation_sampler_indices.size(); ++i) {
            translation_inputs.push_back(
                samplers[translation_sampler_indices[i]]["input"].value.i_number
            );
        }
        for (unsigned int i = 0; i < translation_sampler_indices.size(); ++i) {
            translation_outputs.push_back(
                samplers[translation_sampler_indices[i]]["output"].value.i_number
            );
        }
        std::vector<std::vector<float>> frame_inputs_translation;
        for (unsigned int i = 0; i < translation_inputs.size(); ++i) {
            AccessorMetaData frame_inputs_translation_accessor_meta_data =
                read_accessor_meta_data(gltf, translation_inputs[i]);
            BufferViewMetaData frame_inputs_translation_buffer_view_meta_data =
                read_buffer_view_meta_data(
                    gltf,
                    frame_inputs_translation_accessor_meta_data.buffer_view
                );
            std::vector<float> temp;
            read_binary_buffer_data(
                buffer,
                frame_inputs_translation_accessor_meta_data,
                frame_inputs_translation_buffer_view_meta_data,
                temp
            );
            frame_inputs_translation.push_back(temp);
        }
        std::vector<std::vector<float>> translations;
        for (unsigned int i = 0; i < translation_outputs.size(); ++i) {
            AccessorMetaData frame_outputs_translation_accessor_meta_data =
                read_accessor_meta_data(gltf, translation_outputs[i]);
            BufferViewMetaData frame_outputs_translation_buffer_view_meta_data =
                read_buffer_view_meta_data(
                    gltf,
                    frame_outputs_translation_accessor_meta_data.buffer_view
                );
            std::vector<float> temp;
            read_binary_buffer_data(
                buffer,
                frame_outputs_translation_accessor_meta_data,
                frame_outputs_translation_buffer_view_meta_data,
                temp
            );
            translations.push_back(temp);
        }
        std::vector<unsigned int> rotation_inputs;
        std::vector<unsigned int> rotation_outputs;
        for (unsigned int i = 0; i < rotation_sampler_indices.size(); ++i) {
            rotation_inputs.push_back(
                samplers[rotation_sampler_indices[i]]["input"].value.i_number
            );
        }
        for (unsigned int i = 0; i < rotation_sampler_indices.size(); ++i) {
            rotation_outputs.push_back(
                samplers[rotation_sampler_indices[i]]["output"].value.i_number
            );
        }
        std::vector<std::vector<float>> frame_inputs_rotation;
        for (unsigned int i = 0; i < rotation_inputs.size(); ++i) {
            AccessorMetaData frame_inputs_rotation_accessor_meta_data =
                read_accessor_meta_data(gltf, rotation_inputs[i]);
            BufferViewMetaData frame_inputs_rotation_buffer_view_meta_data =
                read_buffer_view_meta_data(
                    gltf,
                    frame_inputs_rotation_accessor_meta_data.buffer_view
                );
            std::vector<float> temp;
            read_binary_buffer_data(
                buffer,
                frame_inputs_rotation_accessor_meta_data,
                frame_inputs_rotation_buffer_view_meta_data,
                temp
            );
            frame_inputs_rotation.push_back(temp);
        }
        std::vector<std::vector<float>> rotations;
        for (unsigned int i = 0; i < rotation_outputs.size(); ++i) {
            AccessorMetaData frame_outputs_rotation_accessor_meta_data =
                read_accessor_meta_data(gltf, rotation_outputs[i]);
            BufferViewMetaData frame_outputs_rotation_buffer_view_meta_data =
                read_buffer_view_meta_data(
                    gltf,
                    frame_outputs_rotation_accessor_meta_data.buffer_view
                );
            std::vector<float> temp;
            read_binary_buffer_data(
                buffer,
                frame_outputs_rotation_accessor_meta_data,
                frame_outputs_rotation_buffer_view_meta_data,
                temp
            );
            rotations.push_back(temp);
        }
        std::vector<unsigned int> scale_inputs;
        std::vector<unsigned int> scale_outputs;
        for (unsigned int i = 0; i < scale_sampler_indices.size(); ++i) {
            scale_inputs.push_back(
                samplers[scale_sampler_indices[i]]["input"].value.i_number
            );
        }
        for (unsigned int i = 0; i < scale_sampler_indices.size(); ++i) {
            scale_outputs.push_back(
                samplers[scale_sampler_indices[i]]["output"].value.i_number
            );
        }
        std::vector<std::vector<float>> frame_inputs_scale;
        for (unsigned int i = 0; i < scale_inputs.size(); ++i) {
            AccessorMetaData frame_inputs_scale_accessor_meta_data =
                read_accessor_meta_data(gltf, scale_inputs[i]);
            BufferViewMetaData frame_inputs_scale_buffer_view_meta_data =
                read_buffer_view_meta_data(
                    gltf,
                    frame_inputs_scale_accessor_meta_data.buffer_view
                );
            std::vector<float> temp;
            read_binary_buffer_data(
                buffer,
                frame_inputs_scale_accessor_meta_data,
                frame_inputs_scale_buffer_view_meta_data,
                temp
            );
            frame_inputs_scale.push_back(temp);
        }
        std::vector<std::vector<float>> scales;
        for (unsigned int i = 0; i < scale_outputs.size(); ++i) {
            AccessorMetaData frame_outputs_scale_accessor_meta_data =
                read_accessor_meta_data(gltf, scale_outputs[i]);
            BufferViewMetaData frame_outputs_scale_buffer_view_meta_data =
                read_buffer_view_meta_data(
                    gltf,
                    frame_outputs_scale_accessor_meta_data.buffer_view
                );
            std::vector<float> temp;
            read_binary_buffer_data(
                buffer,
                frame_outputs_scale_accessor_meta_data,
                frame_outputs_scale_buffer_view_meta_data,
                temp
            );
            scales.push_back(temp);
        }
        // Searching for root joins.
        std::vector<int> root_nodes;
        for (unsigned int s = 0; s < joints.value.array->size(); ++s) {
            int current_joint = (*joints.value.array)[s].value.i_number;
            for (unsigned w = 0; w < children.size(); ++w) {
                for (unsigned q = 0; q < children[w].size(); ++q) {
                    if (children[w][q] == current_joint) {
                        goto most_scary_operator_of_all_time;
                    }
                }
            }
            // If we execute this line then this joint index ectualy the root.
            root_nodes.push_back(current_joint);
        most_scary_operator_of_all_time: // Not so scary at all. Am i right?
            continue;
        }
        std::vector<std::vector<unsigned int>> nodes_hierarchy;
        // Loop on parent joints.
        for (unsigned int w = 0; w < root_nodes.size(); ++w) {
            std::vector<std::vector<unsigned int>> nodes_bones;
            unsigned int current_root = root_nodes[w];
            std::vector<uint32_t> node_stack;
            // Start from root joint.
            node_stack.push_back(current_root);
            std::vector<uint32_t> deepness_stack;
            traversal_bones(
                children,
                joints,
                node_stack,
                deepness_stack,
                nodes_bones
            );
            for (unsigned int e = 0; e < nodes_bones.size(); ++e) {
                nodes_hierarchy.push_back(nodes_bones[e]);
            }
        }
        // This logic related to joints that has inverseBindMatrices.
        uint32_t transformations_max = translations.size() > scales.size()
            ? (translations.size() > rotations.size() ? translations.size()
                                                      : rotations.size())
            : (scales.size() > rotations.size() ? scales.size()
                                                : rotations.size());
        const uint32_t num_joints = joints.value.array->size();
        uint32_t translation_frames_number = 0;
        for (uint32_t k = 0; k < frame_inputs_translation.size(); ++k) {
            if (frame_inputs_translation[k].size()
                > translation_frames_number) {
                translation_frames_number = frame_inputs_translation[k].size();
            }
        }
        uint32_t rotation_frames_number = 0;
        for (uint32_t k = 0; k < frame_inputs_rotation.size(); ++k) {
            if (frame_inputs_rotation[k].size() > rotation_frames_number) {
                rotation_frames_number = frame_inputs_rotation[k].size();
            }
        }
        uint32_t scale_frames_number = 0;
        for (uint32_t k = 0; k < frame_inputs_scale.size(); ++k) {
            if (frame_inputs_scale[k].size() > scale_frames_number) {
                scale_frames_number = frame_inputs_scale[k].size();
            }
        }
        const uint32_t frames_max =
            translation_frames_number > scale_frames_number
            ? (translation_frames_number > rotation_frames_number
                   ? translation_frames_number
                   : rotation_frames_number)
            : (scale_frames_number > rotation_frames_number
                   ? scale_frames_number
                   : rotation_frames_number);
        for (uint32_t k = 0; k < frame_inputs_translation.size(); ++k) {
            if (frame_inputs_translation[k].size() > frames.size()) {
                frames = frame_inputs_translation[k];
            }
        }
        for (uint32_t k = 0; k < frame_inputs_rotation.size(); ++k) {
            if (frame_inputs_rotation[k].size() > frames.size()) {
                frames = frame_inputs_rotation[k];
            }
        }
        for (uint32_t k = 0; k < frame_inputs_scale.size(); ++k) {
            if (frame_inputs_scale[k].size() > frames.size()) {
                frames = frame_inputs_scale[k];
            }
        }
        std::vector<int> joint_to_translation_ch;
        std::vector<int> joint_to_rotation_ch;
        std::vector<int> joint_to_scale_ch;
        for (uint32_t k = 0; k < num_joints; ++k) {
            joint_to_translation_ch.push_back(-1);
            joint_to_rotation_ch.push_back(-1);
            joint_to_scale_ch.push_back(-1);
        }
        for (uint32_t k = 0; k < nodes_map_translations.size(); ++k) {
            uint32_t j_idx =
                get_joint_index(joints, (int32_t)nodes_map_translations[k]);
            if (j_idx != UINT32_MAX) {
                joint_to_translation_ch[j_idx] = (int)k;
            }
        }
        for (uint32_t k = 0; k < nodes_map_rotations.size(); ++k) {
            uint32_t j_idx =
                get_joint_index(joints, (int32_t)nodes_map_rotations[k]);
            if (j_idx != UINT32_MAX) {
                joint_to_rotation_ch[j_idx] = (int)k;
            }
        }
        for (uint32_t k = 0; k < nodes_map_scales.size(); ++k) {
            uint32_t j_idx =
                get_joint_index(joints, (int32_t)nodes_map_scales[k]);
            if (j_idx != UINT32_MAX) {
                joint_to_scale_ch[j_idx] = (int)k;
            }
        }
        // Build animatedNodesMatricesAccumulator indexed by joint-index
        // (0..numJoints - 1).
        std::vector<std::vector<Matrix<float, 4>>>
            animated_nodes_matrices_accumulator;
        for (unsigned int j = 0; j < num_joints; ++j) {
            int t_idx = joint_to_translation_ch[j];
            int r_idx = joint_to_rotation_ch[j];
            int s_idx = joint_to_scale_ch[j];
            // Local defaults fresh on every joint, for not make possible to
            // collect data from previous iterations.
            std::vector<float> default_translations;
            std::vector<float> default_rotations;
            std::vector<float> default_scales;
            // Static TRS from node. Using if channel not exists.
            int32_t node_idx = (int32_t)(*joints.value.array)[j].value.i_number;
            float s_tx = 0.f, s_ty = 0.f, s_tz = 0.f;
            float s_rx = 0.f, s_ry = 0.f, s_rz = 0.f, s_rw = 1.f;
            float s_sx = 1.f, s_sy = 1.f, s_sz = 1.f;
            if ((*gltf)["nodes"][node_idx].is_object() == JsonObject) {
                auto* nd = (*gltf)["nodes"][node_idx].value.object;
                if (nd->contains("translation")) {
                    s_tx = (*gltf)["nodes"][node_idx]["translation"][0]
                               .value.f_number;
                    s_ty = (*gltf)["nodes"][node_idx]["translation"][1]
                               .value.f_number;
                    s_tz = (*gltf)["nodes"][node_idx]["translation"][2]
                               .value.f_number;
                }
                if (nd->contains("rotation")) {
                    s_rx =
                        (*gltf)["nodes"][node_idx]["rotation"][0].value.f_number;
                    s_ry =
                        (*gltf)["nodes"][node_idx]["rotation"][1].value.f_number;
                    s_rz =
                        (*gltf)["nodes"][node_idx]["rotation"][2].value.f_number;
                    s_rw =
                        (*gltf)["nodes"][node_idx]["rotation"][3].value.f_number;
                }
                if (nd->contains("scale")) {
                    s_sx =
                        (*gltf)["nodes"][node_idx]["scale"][0].value.f_number;
                    s_sy =
                        (*gltf)["nodes"][node_idx]["scale"][1].value.f_number;
                    s_sz =
                        (*gltf)["nodes"][node_idx]["scale"][2].value.f_number;
                }
            }
            if (t_idx < 0) {
                for (uint32_t f = 0; f < frames_max; ++f) {
                    default_translations.push_back(s_tx);
                    default_translations.push_back(s_ty);
                    default_translations.push_back(s_tz);
                }
            }
            if (r_idx < 0) {
                for (uint32_t f = 0; f < frames_max; ++f) {
                    default_rotations.push_back(s_rx);
                    default_rotations.push_back(s_ry);
                    default_rotations.push_back(s_rz);
                    default_rotations.push_back(s_rw);
                }
            }
            if (s_idx < 0) {
                for (uint32_t f = 0; f < frames_max; ++f) {
                    default_scales.push_back(s_sx);
                    default_scales.push_back(s_sy);
                    default_scales.push_back(s_sz);
                }
            }
            std::vector<float>& bone_t =
                (t_idx >= 0) ? translations[t_idx] : default_translations;
            std::vector<float>& bone_r =
                (r_idx >= 0) ? rotations[r_idx] : default_rotations;
            std::vector<float>& bone_s =
                (s_idx >= 0) ? scales[s_idx] : default_scales;
            // Chennels can has verious number of frames; framesMax - gloabal
            // maximum. Clamp index to last valid chennel frame, for not run out
            // after vectors bounds.
            const uint32_t t_frames = bone_t.size() / 3;
            const uint32_t r_frames = bone_r.size() / 4;
            const uint32_t s_frames = bone_s.size() / 3;
            std::vector<Matrix<float, 4>> per_frame_matrices;
            for (unsigned int i = 0; i < frames_max; ++i) {
                if (t_frames == 0 || r_frames == 0 || s_frames == 0) {
                    // Malformed data; skip joint.
                    std::vector<Matrix<float, 4>> empty;
                    animated_nodes_matrices_accumulator.push_back(empty);
                    continue;
                }
                const uint32_t ti = (i < t_frames) ? i : t_frames - 1;
                const uint32_t ri = (i < r_frames) ? i : r_frames - 1;
                const uint32_t si = (i < s_frames) ? i : s_frames - 1;
                Matrix<float, 4> frame_translation(1.0f);
                Matrix<float, 4> frame_scale(1.0f);
                for (unsigned int q = 0; q < 3; ++q) {
                    frame_translation[3][q] = bone_t[ti * 3 + q];
                    frame_scale[q][q] = bone_s[si * 3 + q];
                }
                Quaternion frame_rotation_quaternion;
                Matrix<float, 4> frame_rotation(1.0f);
                frame_rotation_quaternion.x = bone_r[ri * 4];
                frame_rotation_quaternion.y = bone_r[ri * 4 + 1];
                frame_rotation_quaternion.z = bone_r[ri * 4 + 2];
                frame_rotation_quaternion.w = bone_r[ri * 4 + 3];
                frame_rotation =
                    rotate_quaternion<float, 4>(frame_rotation_quaternion);
                frame_rotation.self_tensor_transpose();
                Matrix<float, 4> local_transform =
                    frame_scale * frame_rotation * frame_translation;
                per_frame_matrices.push_back(local_transform);
            }
            animated_nodes_matrices_accumulator.push_back(per_frame_matrices);
        }
        // Final comstruction of joint-matrices. Both arrays indexed by
        // joint-index now, that's why nodesHierarchy[j][b] address accumulator
        // correctly.
        for (unsigned int j = 0; j < num_joints; ++j) {
            std::vector<Matrix<float, 4>> global_all_frame_node_matrix;
            for (unsigned int i = 0; i < frames_max; ++i) {
                Matrix<float, 4> root_transform(1.0f);
                for (unsigned int b = 0; b < nodes_hierarchy[j].size() - 1;
                     ++b) {
                    root_transform = animated_nodes_matrices_accumulator
                                         [nodes_hierarchy[j][b]][i]
                        * root_transform;
                }
                global_all_frame_node_matrix.push_back(
                    inverse_bind_matrix_set[j]
                    * animated_nodes_matrices_accumulator[j][i] * root_transform
                );
            }
            joint_matrices.push_back(global_all_frame_node_matrix);
        }
    }
    joint_matrices_per_mesh = joint_matrices;
    top_y = -999.999f;
    for (uint32_t i = 0; i < indices.size(); ++i) {
        a_indices.push_back(i);
        unsigned int index = indices[i] * 3;
        if (index + 2 < vertices_position.size()) {
            Vector<float, 3> position = {
                vertices_position[index],
                vertices_position[index + 1],
                vertices_position[index + 2]
            };
            if (position[1] > top_y) {
                top_y = position[1];
            }
            a_vertexes.push_back(position[0]);
            a_vertexes.push_back(position[1]);
            a_vertexes.push_back(position[2]);
        }
        if (index + 2 < normals.size()) {
            Vector<float, 3> normal =
                {normals[index], normals[index + 1], normals[index + 2]};
            a_vertexes.push_back(normal[0]);
            a_vertexes.push_back(normal[1]);
            a_vertexes.push_back(normal[2]);
        }
        index = indices[i] * 2;
        if (index + 1 < texture_coordinates.size()) {
            a_vertexes.push_back(texture_coordinates[index]);
            a_vertexes.push_back(texture_coordinates[index + 1]);
        }
        index = indices[i] * 4;
        if (index + 3 < joints_indices.size()) {
            a_vertexes.push_back(joints_indices[index]);
            a_vertexes.push_back(joints_indices[index + 1]);
            a_vertexes.push_back(joints_indices[index + 2]);
            a_vertexes.push_back(joints_indices[index + 3]);
        }
        if (index + 3 < weights_container.size()) {
            a_vertexes.push_back(weights_container[index]);
            a_vertexes.push_back(weights_container[index + 1]);
            a_vertexes.push_back(weights_container[index + 2]);
            a_vertexes.push_back(weights_container[index + 3]);
        }
    }
    delete[] buffer;
    buffer = nullptr;
}

void CJsonParser::traversal_bones(
    std::vector<std::vector<int>> children,
    JsonValue joints,
    std::vector<uint32_t> node_stack,
    std::vector<uint32_t> deepness_stack,
    std::vector<std::vector<uint32_t>>& result
) {
    uint32_t top_joint_index = 0;
    if (!node_stack.empty()) {
        // Pass array of all joints and root joint and return index of root
        // joint in array.
        top_joint_index = get_joint_index(joints, node_stack.back());
    }
    if (node_stack.size() > deepness_stack.size()) {
        // First 0 level start from.
        uint32_t first_child = 0;
        deepness_stack.push_back(first_child);
    }
    // Main exit check.
    if (deepness_stack.empty()) {
        return;
    }
    uint32_t next_node_index = 0;
    // Check current root joint has any children. Children maps linearly with
    // root joint array index.
    if (top_joint_index != UINT32_MAX && !children[top_joint_index].empty()) {
        // Check if on last child level.
        if (deepness_stack.back() > 0
            && deepness_stack.back() == children[top_joint_index].size()) {
            deepness_stack.pop_back();
            node_stack.pop_back();
            traversal_bones(
                children,
                joints,
                node_stack,
                deepness_stack,
                result
            );
            return;
        }
        // Check if not on last child level.
        if (deepness_stack.back() > 0
            && deepness_stack.back() < children[top_joint_index].size()) {
            next_node_index = children[top_joint_index][deepness_stack.back()];
            node_stack.push_back(next_node_index);
            std::vector<uint32_t> current_node_indices;
            for (uint32_t i = 0; i < node_stack.size(); ++i) {
                uint32_t current_join_index =
                    get_joint_index(joints, node_stack[i]);
                current_node_indices.push_back(current_join_index);
            }
            ++deepness_stack.back();
            traversal_bones(
                children,
                joints,
                node_stack,
                deepness_stack,
                result
            );
            return;
        } else {
            std::vector<uint32_t> current_node_indices;
            for (uint32_t i = 0; i < node_stack.size(); ++i) {
                uint32_t current_join_index =
                    get_joint_index(joints, node_stack[i]);
                current_node_indices.push_back(current_join_index);
            }
            result.push_back(current_node_indices);
            next_node_index = children[top_joint_index][deepness_stack.back()];
            node_stack.push_back(next_node_index);
            ++deepness_stack.back();
            traversal_bones(
                children,
                joints,
                node_stack,
                deepness_stack,
                result
            );
            return;
        }
    } else {
        std::vector<uint32_t> current_node_indices;
        if (top_joint_index == UINT32_MAX) {
            current_node_indices.push_back(node_stack.back());
            result.push_back(current_node_indices);
            return;
        }
        for (uint32_t i = 0; i < node_stack.size(); ++i) {
            uint32_t current_join_index =
                get_joint_index(joints, node_stack[i]);
            current_node_indices.push_back(current_join_index);
        }
        result.push_back(current_node_indices);
        deepness_stack.pop_back();
        node_stack.pop_back();
        traversal_bones(children, joints, node_stack, deepness_stack, result);
        return;
    }
}

std::vector<std::vector<unsigned int>> CJsonParser::make_render_joints_indices(
    std::vector<std::vector<unsigned int>>& input
) {
    std::vector<std::vector<unsigned int>> result;
    bool accumulator_flag = false;
    bool inner_flag = false;
    unsigned int accumulator = input[0][0];
    for (unsigned int i = 0; i < input.size(); ++i) {
        for (unsigned int j = 0; j < input[i].size(); ++j) {
            std::vector<unsigned int> inner;
            for (unsigned int v = 0; v < j + 1; ++v) {
                if (input[i][j] == accumulator && accumulator_flag) {
                    inner_flag = false;
                    continue;
                } else {
                    inner_flag = true;
                    inner.push_back(input[i][v]);

                    if (accumulator_flag == false) {
                        accumulator_flag = true;
                    }
                }
            }
            if (inner_flag) {
                result.push_back(inner);
            }
        }
    }
    return result;
}

bool CJsonParser::contains_element(
    std::vector<std::vector<unsigned int>> container,
    unsigned int element
) {
    bool flag = false;
    for (unsigned int i = 0; i < container.size(); ++i) {
        for (unsigned int j = 0; j < container[i].size(); ++j) {
            if (container[i][j] == element) {
                return true;
            }
        }
    }
    return flag;
}

uint32_t CJsonParser::get_joint_index(JsonValue joints, int32_t searching_index) {
    for (unsigned int i = 0; i < joints.value.array->size(); ++i) {
        int current_joint_index = (*joints.value.array)[i].value.i_number;
        if (current_joint_index == searching_index) {
            return i;
        }
    }
    return -1;
}

CJsonParser::~CJsonParser() {
    delete root;
}
} // namespace glvm

namespace glvm {
MeshManager* MeshManager::p_instance = nullptr;
std::mutex MeshManager::mutex;

MeshManager::MeshManager() {
}

MeshManager::~MeshManager() {
}

void MeshManager::set_mesh(const char* mesh_path) {
    paths_array.push_back(mesh_path);
}

void MeshManager::set_mesh_gltf(const char* path_to_mesh) {
    paths_gltf.push_back(path_to_mesh);
}

MeshManager* MeshManager::get_instance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (p_instance == nullptr) {
        p_instance = new MeshManager();
    }
    return p_instance;
}
} // namespace glvm

namespace glvm {
void ProceduralLevelGeneratingSystem::update() {
    using namespace glvm;
    Engine* glvm = Engine::get_instance();

    // New arch ECS.
    ArchetypeEntityManager* arch_entity_manager =
        ArchetypeEntityManager::get_instance();

    WORLD.search_cache_archetypes(
        player_required_mask,
        &arch_view.cached_player_arch,
        cached_player_arch_number
    );
    components_view.player_transforms =
        (Transform*)arch_view.cached_player_arch
            ->components[ComponentsIndices::TransformComponent];

    while (level_nubmer < 5) {
        std::vector<Vertex> next_level;
        std::vector<uint32_t> indices;
        std::vector<Vertex> transition_bridge_vertices;
        std::vector<uint32_t> transition_bridge_indices;

        if (level_nubmer < 5) {
            std::random_device rd;
            std::mt19937 mersenne(rd());
            std::uniform_int_distribution<int> dist_current_level_y(1, 1);
            unsigned int level_half_y = dist_current_level_y(mersenne);
            std::uniform_int_distribution<int> dist_current_level_x_z(16, 16);
            unsigned int level_half_x = dist_current_level_x_z(mersenne);
            unsigned int level_half_z = dist_current_level_x_z(mersenne);

            // Need to move on half.
            constexpr float TRANSITION_BRIDGE_HALF_WIDTH = 0.5f;
            constexpr float TRANSITION_BRIDGE_HALF_HEIGHT = 1.0f;
            // On first iteration we dont need to define where locate current
            // level depends on previousTransitionBridge.
            if (level_nubmer != 0) {
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

            for (unsigned int i = 0; i < 36; ++i) {
                indices.push_back(BOX_INDICES_FOR_INDEX_BUFFER[i]);
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
            uint64_t game_level_chunk_entity =
                arch_entity_manager->create_entity();

            cached_level_chunk_arch_number = 0;
            // Search and cache one time for LevelChunkArch.
            WORLD.search_cache_archetypes(
                required_mask,
                &arch_view.cached_level_chunk_arch,
                cached_level_chunk_arch_number
            );

            WORLD.add_entity_to_archetype(
                game_level_chunk_entity,
                arch_view.cached_level_chunk_arch
            );
            EntityLocation game_level_chunk_location =
                WORLD.entity_locations[get_id(game_level_chunk_entity)];

            LevelChunkArchetype* level_chunk_arch =
                static_cast<LevelChunkArchetype*>(
                    game_level_chunk_location.arch
                );
            const uint32_t game_level_chunk_index =
                game_level_chunk_location.index;
            TextureHandle game_level_texture = texture_handlers[2];
            if (level_nubmer == 0) {
                // Set up current level position to player position.
                components_view.player_transforms->position = Vector<float, 3>(
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

            for (unsigned int i = 0; i < 36; ++i) {
                transition_bridge_indices.push_back(
                    BOX_INDICES_FOR_INDEX_BUFFER[i]
                );
            }

            mesh_axis_limiting_values.set_to_default_values();

            float half_x = 0.0f;
            float half_y = level_half_y;
            float half_z = 0.0f;
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
            uint64_t transition_bridge_entity =
                arch_entity_manager->create_entity();
            WORLD.add_entity_to_archetype(
                transition_bridge_entity,
                arch_view.cached_level_chunk_arch
            );

            EntityLocation transition_bridge_location =
                WORLD.entity_locations[get_id(transition_bridge_entity)];

            LevelChunkArchetype* transition_bridge_arch =
                static_cast<LevelChunkArchetype*>(
                    transition_bridge_location.arch
                );
            const uint32_t transition_bridge_index =
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

            ++level_nubmer;
        }
        level_generated_vertices.push_back(next_level);
        level_generated_indices.push_back(indices);

        level_generated_vertices.push_back(transition_bridge_vertices);
        level_generated_indices.push_back(transition_bridge_indices);

        bredo_flag = true;
    }
}

void ProceduralLevelGeneratingSystem::set_half_extents_from_direction(
    float& half_x,
    float& half_z,
    const float& transition_bridge_half_width,
    const float& transition_bridge_half_height,
    const float& next_level_transition_direction
) {
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

void ProceduralLevelGeneratingSystem::generate_level(
    const unsigned int level_half_x,
    const unsigned int level_half_y,
    const unsigned int level_half_z,
    const float transition_bridge_half_width,
    const float transition_bridge_half_height
) {
    std::random_device rd;
    std::mt19937 mersenne(rd());
    unsigned int previous_transition_bridge_anchor_point = 0;
    bool valid_level = false;
    while (!valid_level) {
        switch (previous_iteration_transition_bridge_direction) {
            case 1: {
                std::uniform_int_distribution<int>
                    dist_previous_transition_bridge_anchor_point(
                        0,
                        level_half_x * 2 - 1
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
                std::uniform_int_distribution<int>
                    dist_previous_transition_bridge_anchor_point(
                        0,
                        level_half_z * 2 - 1
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
                std::uniform_int_distribution<int>
                    dist_previous_transition_bridge_anchor_point(
                        0,
                        level_half_x * 2 - 1
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
                std::uniform_int_distribution<int>
                    dist_previous_transition_bridge_anchor_point(
                        0,
                        level_half_z * 2 - 1
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
                (4 + previous_iteration_transition_bridge_direction) % 4 + 1;
        } else {
            coordinate_maximum_value_per_direction
                .compare_per_direction_and_set_to_maximum_value_by_module(
                    current_level_position,
                    (float)level_half_x,
                    (float)level_half_y,
                    (float)level_half_z
                );
            valid_level = true;
        }
    }
    current_level_position[1] = 0.0f;
}

void ProceduralLevelGeneratingSystem::generate_transition_bridge(
    const unsigned int level_half_x,
    const unsigned int level_half_y,
    const unsigned int level_half_z,
    const float transition_bridge_half_width,
    const float transition_bridge_half_height
) {
    std::random_device rd;
    std::mt19937 mersenne(rd());
    // 1 - north, 2 - east, 3 - south, 4 - west.
    std::uniform_int_distribution<int> dist_next_level_transition_direction(
        1,
        4
    );
    // Randomly chose direction in where next level will appeared.
    next_level_transition_direction =
        dist_next_level_transition_direction(mersenne);
    unsigned int transition_bridge_anchor_point = 0;
    float transition_bridge_offset_x = 0.0f;
    float transition_bridge_offset_z = 0.0f;
    bool valid_transition_bridge = false;
    while (!valid_transition_bridge) {
        // Choose up (1) or down (3) insert point direction.
        if (next_level_transition_direction == 1
            || next_level_transition_direction == 3) {
            // In what point we connect next transition bridge to current level.
            std::uniform_int_distribution<int>
                dist_transition_bridge_anchor_point(0, level_half_x * 2 - 1);
            transition_bridge_anchor_point =
                dist_transition_bridge_anchor_point(mersenne);
            // Summarize most left position with random value of point where
            // transition bridge will be insert.
            transition_bridge_offset_x =
                -(float)level_half_x + (float)transition_bridge_anchor_point;
            if (next_level_transition_direction == 1) {
                // Move to the bottom level edge.
                transition_bridge_offset_z = level_half_z;
                transition_bridge_position = {
                    current_level_position[0] + transition_bridge_offset_x
                        + transition_bridge_half_width,
                    (float)level_half_y,
                    current_level_position[2] + transition_bridge_offset_z
                        + transition_bridge_half_height
                };
            } else {
                // Move to the upper level edge.
                transition_bridge_offset_z = -(float)level_half_z;
                transition_bridge_position = {
                    current_level_position[0] + transition_bridge_offset_x
                        + transition_bridge_half_width,
                    (float)level_half_y,
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
            std::uniform_int_distribution<int>
                dist_transition_bridge_anchor_point(0, level_half_z * 2 - 1);
            transition_bridge_anchor_point =
                dist_transition_bridge_anchor_point(mersenne);
            // Summarize forward most position with random value of point where
            // transition bridge will be insert.
            transition_bridge_offset_z =
                -(float)level_half_z + (float)transition_bridge_anchor_point;
            if (next_level_transition_direction == 2) {
                // Move to the right level edge.
                transition_bridge_offset_x = level_half_x;
                transition_bridge_position = {
                    current_level_position[0] + transition_bridge_offset_x
                        + transition_bridge_half_height,
                    (float)level_half_y,
                    current_level_position[2] + transition_bridge_offset_z
                        + transition_bridge_half_width
                };
            } else {
                // Move to the left level edge.
                transition_bridge_offset_x = -(float)level_half_x;
                transition_bridge_position = {
                    current_level_position[0] + transition_bridge_offset_x
                        - transition_bridge_half_height,
                    (float)level_half_y,
                    current_level_position[2] + transition_bridge_offset_z
                        + transition_bridge_half_width
                };
            }
        }

        float width = 0;
        float height = 0;
        // Chose transitionBridgeHalfWidth as X and transitionBridgeHalfHeight
        // as Z.
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
                (4 + next_level_transition_direction) % 4 + 1;
        } else {
            // Setting up bounds for all levels.
            coordinate_maximum_value_per_direction
                .compare_per_direction_and_set_to_maximum_value_by_module(
                    transition_bridge_position,
                    (float)width,
                    (float)level_half_y,
                    (float)height
                );
            valid_transition_bridge = true;
        }
    }
    transition_bridge_position[1] = current_level_position[1];
    previous_iteration_transition_bridge_direction =
        next_level_transition_direction;
}

void ProceduralLevelGeneratingSystem::make_cube_object_vertices(
    Vector<float, 4> join_indices,
    Vector<float, 4> weights,
    float half_x,
    float half_y,
    float half_z,
    std::vector<Vertex>& destination_vertices_container
) {
    unsigned int cube_vertices = 8;
    for (unsigned int i = 0; i < cube_vertices; ++i) {
        SVertex vertex;

        switch (i) {
            case 0:
                vertex[0] = half_x;
                vertex[1] = half_y;
                vertex[2] = half_z;
                break;
            case 1:
                vertex[0] = -(float)half_x;
                vertex[1] = half_y;
                vertex[2] = half_z;
                break;
            case 2:
                vertex[0] = -(float)half_x;
                vertex[1] = -(float)half_y;
                vertex[2] = half_z;
                break;
            case 3:
                vertex[0] = half_x;
                vertex[1] = -(float)half_y;
                vertex[2] = half_z;
                break;
            case 4:
                vertex[0] = half_x;
                vertex[1] = half_y;
                vertex[2] = -(float)half_z;
                break;
            case 5:
                vertex[0] = -(float)half_x;
                vertex[1] = half_y;
                vertex[2] = -(float)half_z;
                break;
            case 6:
                vertex[0] = -(float)half_x;
                vertex[1] = -(float)half_y;
                vertex[2] = -(float)half_z;
                break;
            case 7:
                vertex[0] = half_x;
                vertex[1] = -(float)half_y;
                vertex[2] = -(float)half_z;
                break;
        }

        mesh_axis_limiting_values
            .compare_per_direction_and_set_to_maximum_value_by_module(vertex);

        SVertex normal;
        normal[0] = 0;
        normal[1] = 1;
        normal[2] = 0;
        SVertex texture;
        texture[0] = 0;
        texture[1] = 1;

        destination_vertices_container.push_back(
            {{vertex[0], vertex[1], vertex[2]},
             {normal[0], normal[1], normal[2]},
             {texture[0], texture[1]},
             {join_indices[0],
              join_indices[1],
              join_indices[2],
              join_indices[3]},
             {weights[0], weights[1], weights[2], weights[3]}}
        );
    }
}

bool ProceduralLevelGeneratingSystem::
    check_collision_intersection_with_maximum_coordinates(
        Vector<float, 3> position,
        float half_x,
        float half_y,
        float half_z
    ) {
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
} // namespace glvm

#ifdef __linux__
#endif

#ifdef _WIN32
#endif

namespace glvm {
ISoundEngine* CSoundEngineFactory::create_sound_engine() {
#ifdef __linux__
    return new CSoundEngineAlsa;
#endif

#ifdef _WIN32
    return new CSoundEngineWaveform;
#endif
}
#ifdef __linux__
CSoundEngineAlsa::~CSoundEngineAlsa() {
    for (uint32_t i = 0; i < sound_container.size(); ++i) {
        delete sound_container[i];
        sound_container[i] = nullptr;
    }
}

void CSoundEngineAlsa::open_device(const char* device) {
    snd_pcm_open(&pcm_, device, SND_PCM_STREAM_PLAYBACK, 0);
}

void CSoundEngineAlsa::close_device() {
    snd_pcm_drain(pcm_);
    snd_pcm_close(pcm_);
}

void CSoundEngineAlsa::sound_stream() {
    for (unsigned int i = 0; i < sound_container.size(); ++i) {
        playback_sound_sample(*sound_container[i]);
        sound_container.erase(sound_container.begin() + i);
    }
}

void CSoundEngineAlsa::playback_sound_sample(CSoundSample& sample) {
    const snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    const snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
    constexpr unsigned int channels = 2;
    // 0.5 s.
    constexpr unsigned int latency = 500000;
    constexpr unsigned int frame_size = channels * 2;
    constexpr int alsa_frames = 32;

    snd_pcm_set_params(
        pcm_,
        format,
        access,
        channels,
        sample.ui_rate,
        1,
        latency
    );

    FILE* file_descriptor = fopen(sample.k_path_to_file, "r");
    if (file_descriptor == nullptr) {
        return;
    }
    char* buffer = (char*)malloc(alsa_frames * frame_size);
    for (int i = 0; i < 300; ++i) {
        int frames = fread(buffer, frame_size, alsa_frames, file_descriptor);
        if (frames <= 0) {
            break;
        }
        int rest = frames;

        int16_t* samples = reinterpret_cast<int16_t*>(buffer);
        int sample_count = frames * channels;
        for (int j = 0; j < sample_count; ++j) {
            int32_t scaled = static_cast<int32_t>(samples[j] * sample.volume);
            samples[j] =
                static_cast<int16_t>(std::clamp(scaled, -32768, 32767));
        }

        char* data = buffer;
        while (rest > 0) {
            frames = snd_pcm_writei(pcm_, data, rest);
            if (frames <= 0) {
                break;
            }
            rest -= frames;
            data += frames * frame_size;
        }
    }
    free(buffer);
    fclose(file_descriptor);
}

void CSoundEngineAlsa::set_master_volume(long volume_percent) {
    long min_volume = 0;
    long max_volume = 0;
    snd_mixer_t* mixer = nullptr;
    snd_mixer_selem_id_t* sid = nullptr;
    const char* card = "default";
    const char* selem_name = "Master";

    snd_mixer_open(&mixer, 0);
    snd_mixer_attach(mixer, card);
    snd_mixer_selem_register(mixer, NULL, NULL);
    snd_mixer_load(mixer);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* element = snd_mixer_find_selem(mixer, sid);

    snd_mixer_selem_get_playback_volume_range(element, &min_volume, &max_volume);
    snd_mixer_selem_set_playback_volume_all(
        element,
        min_volume + (volume_percent * (max_volume - min_volume)) / 100
    );

    snd_mixer_close(mixer);
}

std::vector<CSoundSample*>& CSoundEngineAlsa::get_sound_container() {
    return sound_container;
}

void CSoundEngineAlsa::create_sound_sample(
    const char* file_path,
    uint32_t duration,
    uint32_t rate,
    float volume
) {
    sound_container.push_back(
        new CSoundSample {file_path, duration, rate, volume}
    );
}
#endif // __linux__

#ifdef _WIN32
void CSoundEngineWaveform::open_device(const char* /* device */) {
}

void CSoundEngineWaveform::close_device() {
}

void CSoundEngineWaveform::create_sound_sample(
    const char* file_path,
    uint32_t duration,
    uint32_t rate,
    float volume
) {
    CSoundSample* sample = new CSoundSample {file_path, duration, rate, volume};
    t_sound_container.push_back(sample);
}
#endif // _WIN32
} // namespace glvm

namespace glvm {
CSystemManager* CSystemManager::p_instance = nullptr;
std::mutex CSystemManager::mutex;

CSystemManager::CSystemManager() {
}

CSystemManager::~CSystemManager() {
    delete p_instance;
    p_instance = nullptr;
}

CSystemManager* CSystemManager::get_instance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (p_instance == nullptr) {
        p_instance = new CSystemManager();
    }
    return p_instance;
}

void CSystemManager::activate_system(ISystem* system) {
    t_system_container.push_back(system);
    ++s_i_system_id;
}

void CSystemManager::deactivate_system(DeactivatedSystems system) {
    deactivated_systems.push_back(system);
}

void CSystemManager::return_system_to_activated_state(
    DeactivatedSystems system
) {
    for (unsigned int i = 0; i < deactivated_systems.size(); ++i) {
        if (system == deactivated_systems[i]) {
            deactivated_systems.erase(deactivated_systems.begin() + i);
            return;
        }
    }
}

void CSystemManager::update() {
    bool removed_system_flag = false;
    for (unsigned int i = 0; i < s_i_system_id; ++i) {
        for (unsigned int j = 0; j < deactivated_systems.size(); ++j) {
            if ((unsigned int)deactivated_systems[j] == i) {
                removed_system_flag = true;
                continue;
            }
        }

        if (removed_system_flag) {
            removed_system_flag = false;
            continue;
        } else {
            t_system_container[i]->update();
        }
    }
}
} // namespace glvm

namespace glvm {
void CCollisionSystem::update() {
    // Spatial grid common data.
    const SpatialGrid& spatial_grid = WORLD.spatial_grid;
    assert(
        spatial_grid.width > 0 && spatial_grid.height > 0
        && spatial_grid.depth > 0
    );
    const float chunk_size = spatial_grid.grid[0][0][0].SIZE;

    const float chunk_half_width = spatial_grid.width * chunk_size * 0.5f;
    const float chunk_half_height = spatial_grid.height * chunk_size * 0.5f;
    const float chunk_half_depth = spatial_grid.depth * chunk_size * 0.5f;

    cached_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        required_mask,
        cached_archetypes,
        cached_archetypes_number
    );

    const float camera_speed = 5.5f * f_delta_time;
    // Outer cycle on every archetype.
    for (uint32_t x = 0; x < cached_archetypes_number; ++x) {
        Archetype* arch = cached_archetypes[x];
        view.backtracking_transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];
        view.backtracking_colliders =
            (Collider*)arch->components[ComponentsIndices::ColliderComponent];
        view.backtracking_collider_flags =
            (ColliderFlags*)
                arch->components[ComponentsIndices::ColliderFlagsComponent];
        view.backtracking_meshes =
            (Mesh*)arch->components[ComponentsIndices::MeshComponent];

        for (unsigned int i = 0; i < arch->entity_count; ++i) {
            // Count on every entity in current outer archetype.
            uint32_t backtracking_entity_id = arch->entities[i];

            uint8_t groud_collision_turn_off_mask =
                (1u << 0) | (0u << 1) | (1u << 2) | (1u << 3);
            if (view.backtracking_collider_flags && view.backtracking_colliders
                && view.backtracking_meshes && view.backtracking_transforms) {
                view.backtracking_collider_flags[i].flags =
                    view.backtracking_collider_flags[i].flags
                    & groud_collision_turn_off_mask;
                view.backtracking_colliders[i].colliders.clear();
                Mesh backtrackin_entity_mesh = view.backtracking_meshes[i];
                MeshHandle backtracking_entity_mesh_handle =
                    backtrackin_entity_mesh.handle;
                Transform* backtracking_transform_component =
                    &view.backtracking_transforms[i];
                Vector<float, 3> backtracking_transform =
                    backtracking_transform_component->position;
                float backtracking_scale =
                    backtracking_transform_component->scale;

                uint64_t move_required_mask =
                    (1ul << ComponentsIndices::MoveComponent);
                // Check if outer current archetype has move component.
                if (matches_required_mask(arch->mask, move_required_mask)) {
                    view.backtracking_move =
                        (Move*)
                            arch->components[ComponentsIndices::MoveComponent];
                    backtracking_transform +=
                        normalize(view.backtracking_move[i].frame_movement)
                        * camera_speed;
                    backtracking_transform += view.backtracking_move[i].gravity;
                }

                // Collect entities from grid chunks.
                MeshAxisMaxAbsoluteValues entity_chunk_bounds =
                    ALL_MESH_MAX_ABSOLUTE_VALUES[backtracking_entity_mesh_handle
                                                     .id];
                std::vector<Vector<float, 3>> entity_box_corner_bound_points =
                    compute_box_corner_bound_points(
                        entity_chunk_bounds,
                        backtracking_transform_component->position,
                        backtracking_transform_component->scale
                    );

                // Result array with collected entities.
                std::vector<uint32_t> collected_entities;
                // Need only left bottom back corner point and right upper front
                // corner point to obtain all box bounds
                const Vector<float, 3> min_entity_position =
                    entity_box_corner_bound_points[0];
                const Vector<float, 3> max_entity_position =
                    entity_box_corner_bound_points[1];

                int index_min_x = static_cast<int>(
                    (min_entity_position[0] + chunk_half_width) / chunk_size
                );
                int index_min_y = static_cast<int>(
                    (min_entity_position[1] + chunk_half_height) / chunk_size
                );
                int index_min_z = static_cast<int>(
                    (min_entity_position[2] + chunk_half_depth) / chunk_size
                );

                int index_max_x = static_cast<int>(
                    (max_entity_position[0] + chunk_half_width) / chunk_size
                );
                int index_max_y = static_cast<int>(
                    (max_entity_position[1] + chunk_half_height) / chunk_size
                );
                int index_max_z = static_cast<int>(
                    (max_entity_position[2] + chunk_half_depth) / chunk_size
                );

                // Entity can legitimately leave the fixed-size world grid -
                // clamp to nearest edge cell instead of crashing.
                index_min_x =
                    std::clamp(index_min_x, 0, (int)spatial_grid.width - 1);
                index_min_y =
                    std::clamp(index_min_y, 0, (int)spatial_grid.height - 1);
                index_min_z =
                    std::clamp(index_min_z, 0, (int)spatial_grid.depth - 1);
                index_max_x =
                    std::clamp(index_max_x, 0, (int)spatial_grid.width - 1);
                index_max_y =
                    std::clamp(index_max_y, 0, (int)spatial_grid.height - 1);
                index_max_z =
                    std::clamp(index_max_z, 0, (int)spatial_grid.depth - 1);

                for (auto i2 = index_min_z; i2 <= index_max_z; ++i2) {
                    for (auto i3 = index_min_y; i3 <= index_max_y; ++i3) {
                        for (auto i4 = index_min_x; i4 <= index_max_x; ++i4) {
                            const std::vector<uint32_t>& chunk_entities =
                                spatial_grid.grid[i2][i3][i4].entities;
                            for (uint32_t i5 = 0; i5 < chunk_entities.size();
                                 ++i5) {
                                const uint32_t entity = chunk_entities[i5];
                                if (!is_exist(collected_entities, entity)) {
                                    collected_entities.push_back(entity);
                                }
                            }
                        }
                    }
                }

                // Inner cycle on every archetype.
                // Count on every entity in current inner archetype.
                for (unsigned int j = 0; j < collected_entities.size(); ++j) {
                    // Check for same entityID and iteration.
                    uint32_t compared_entity_id = collected_entities[j];
                    if (backtracking_entity_id == compared_entity_id) {
                        continue;
                    }

                    EntityLocation compared_entity_location =
                        WORLD.entity_locations[get_id(compared_entity_id)];
                    const uint32_t compared_entity_index =
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
                        view.compared_transforms = &(
                            (Transform*)arch->components
                                [ComponentsIndices::TransformComponent]
                        )[compared_entity_index];
                        view.compared_meshes = &(
                            (
                                Mesh*
                            )arch->components[ComponentsIndices::MeshComponent]
                        )[compared_entity_index];
                        compared_entity_mesh_handle =
                            view.compared_meshes->handle;

                        uint64_t move_required_mask =
                            (1ul << ComponentsIndices::MoveComponent);
                        if (matches_required_mask(
                                compared_entity_location.arch->mask,
                                move_required_mask
                            )) {
                            view.compared_move = &(
                                (Move*)arch
                                    ->components[ComponentsIndices::MoveComponent]
                            )[compared_entity_index];
                        }
                    }

                    Transform* compared_transform_component =
                        view.compared_transforms;
                    Move* compared_move_component = view.compared_move;

                    Vector<float, 3> compared_transform =
                        Vector<float, 3>(0.0f, 0.0f, 0.0f);
                    float compared_scale = 0.0f;
                    compared_transform = compared_transform_component->position;
                    compared_scale = compared_transform_component->scale;

                    Vector<float, 3> gravity_test {};
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
                            ALL_MESH_MAX_ABSOLUTE_VALUES
                                [backtracking_entity_mesh_handle.id];
                    MeshAxisMaxAbsoluteValues
                        compared_mesh_axis_max_absolute_values = {};
                    if (compared_entity_mesh_handle.id
                        < ALL_MESH_MAX_ABSOLUTE_VALUES.size()) {
                        compared_mesh_axis_max_absolute_values =
                            ALL_MESH_MAX_ABSOLUTE_VALUES
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
                        uint8_t groud_collision_turn_on_mask =
                            (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                        view.backtracking_collider_flags[i].flags =
                            view.backtracking_collider_flags[i].flags
                            | groud_collision_turn_on_mask;
                        view.backtracking_colliders[i].colliders.push_back(
                            compared_entity_id
                        );

                        continue;
                    }

                    if (box_collider_flag) {
                        uint8_t wall_collision_turn_on_mask =
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

bool CCollisionSystem::upper_actor_check(
    Vector<float, 3> backtracking_position,
    Vector<float, 3> compared_position,
    float backtracking_scale,
    float compared_scale,
    MeshHandle backtracking_mesh_handle,
    MeshHandle compared_mesh_handle
) {
    MeshAxisMaxAbsoluteValues backtracking_mesh_axis_max_absolute_values =
        ALL_MESH_MAX_ABSOLUTE_VALUES[backtracking_mesh_handle.id];

    MeshAxisMaxAbsoluteValues compared_mesh_axis_max_absolute_values =
        ALL_MESH_MAX_ABSOLUTE_VALUES[compared_mesh_handle.id];

    constexpr float EPSILON = 0.15f;
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
} // namespace glvm

namespace glvm {
void DamageSystem::update() {
    cached_attackable_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        attackable_required_mask,
        arch_view.cached_attackable_archetypes,
        cached_attackable_archetypes_number
    );

    for (uint32_t x = 0; x < cached_attackable_archetypes_number; ++x) {
        Archetype* arch = arch_view.cached_attackable_archetypes[x];
        components_view.attackable_attacks =
            (Attack*)arch->components[ComponentsIndices::AttackComponent];
        components_view.attackable_health =
            (Health*)arch->components[ComponentsIndices::HealthComponent];
        components_view.attackable_fonts =
            (Font*)arch->components[ComponentsIndices::FontComponent];

        for (unsigned int i = 0; i < arch->entity_count; ++i) {
            uint64_t entity = arch->entities[i];
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
                    WORLD.remove_entity(entity);
                }

                Font& font_component = components_view.attackable_fonts[i];
                font_component.font_string.clear();
                font_component.font_string.push_back('4');
                font_component.font_string.push_back('0');
                font_component.life_time = 0;
                font_component.removeble = true;
            }
        }
    }

    cached_font_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        font_required_mask,
        arch_view.cached_font_archetypes,
        cached_font_archetypes_number
    );

    for (uint32_t x = 0; x < cached_font_archetypes_number; ++x) {
        Archetype* arch = arch_view.cached_font_archetypes[x];
        components_view.fonts =
            (Font*)arch->components[ComponentsIndices::FontComponent];

        for (unsigned int i = 0; i < arch->entity_count; ++i) {
            if (components_view.fonts) {
                Font& font_component = components_view.fonts[i];
                if (font_component.removeble) {
                    font_component.life_time += delta_time;
                }
                if (font_component.life_time >= 1.5) {
                }
            }
        }
    }
}
} // namespace glvm

namespace glvm {
void EnemySystem::update() {
    player_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        player_required_mask,
        &arch_view.player_cached_archetype,
        player_archetypes_number
    );
    components_view.player_transforms =
        (Transform*)arch_view.player_cached_archetype
            ->components[ComponentsIndices::TransformComponent];

    enemy_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        enemy_required_mask,
        &arch_view.enemy_cached_archetype,
        enemy_archetypes_number
    );
    components_view.enemy_transforms =
        (Transform*)arch_view.enemy_cached_archetype
            ->components[ComponentsIndices::TransformComponent];
    components_view.enemy_states =
        (State*)arch_view.enemy_cached_archetype
            ->components[ComponentsIndices::StateComponent];
    components_view.enemies =
        (Enemy*)arch_view.enemy_cached_archetype
            ->components[ComponentsIndices::EnemyComponent];

    projectile_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        projectile_required_mask,
        &arch_view.projectile_archetype,
        projectile_archetypes_number
    );

    for (uint32_t j = 0; j < arch_view.player_cached_archetype->entity_count;
         ++j) {
        Transform* player_transform_component =
            &components_view.player_transforms[j];
        for (unsigned int i = 0;
             i < arch_view.enemy_cached_archetype->entity_count;
             ++i) {
            Transform* enemy_transform_component =
                &components_view.enemy_transforms[i];
            State* state_enemy_component = &components_view.enemy_states[i];
            Enemy* enemy_component = &components_view.enemies[i];

            Vector<float, 3> distance = player_transform_component->position
                - enemy_transform_component->position;
            float camera_speed = 5.5f * delta_frame_time;

            if (projectile_cooldown > 0) {
                projectile_cooldown -= camera_speed;
            }
            if (distance.length() > enemy_component->detect_radius
                && state_enemy_component->state == States::ATTACK) {
                float delta_length =
                    distance.length() - enemy_component->detect_radius;
                Vector<float, 3> enemy_move =
                    distance * (delta_length / distance.length());

                enemy_transform_component->position += enemy_move;
            }

            if (distance.length() <= enemy_component->detect_radius) {
                if (projectile_cooldown <= 0) {
                    MeshHandle mesh_handle {};
                    const uint32_t sphere_mesh_handle_index = 2;
                    if (mesh_handlers.size() > 2) {
                        mesh_handle = mesh_handlers[sphere_mesh_handle_index];
                    }

                    TextureHandle texture_handle {};
                    const uint32_t gray_texture_handle = 2;
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
                    uint64_t projectile_entity =
                        arch_entity_manager->create_entity();
                    WORLD.add_entity_to_archetype(
                        projectile_entity,
                        arch_view.projectile_archetype
                    );
                    EntityLocation projectile_location =
                        WORLD.entity_locations[get_id(projectile_entity)];

                    create_projectile(
                        enemy_transform_component->position,
                        player_transform_component->position
                            - enemy_transform_component->position,
                        mesh_handle,
                        material,
                        damage,
                        projectile_location
                    );

                    sound_engine->create_sound_sample(
                        "../../../examples/assets/sounds/pistol.wav",
                        5,
                        22050,
                        0.05
                    );
                    projectile_cooldown = 5.0;
                }

                state_enemy_component->state = States::ATTACK;
            }
        }
    }
}
} // namespace glvm

namespace glvm {
void InventorySystem::update() {
    if (is_inventory_opened) {
        crosshair_archetypes_number = 0;
        WORLD.search_cache_archetypes(
            crosshair_required_mask,
            &arch_view.crosshair_cached_archetype,
            crosshair_archetypes_number
        );
        components_view.crosshair_transforms_view =
            (Transform*)arch_view.crosshair_cached_archetype
                ->components[ComponentsIndices::TransformComponent];

        inventory_archetypes_number = 0;
        WORLD.search_cache_archetypes(
            inventory_required_mask,
            &arch_view.inventory_cached_archetype,
            inventory_archetypes_number
        );

        components_view.inventory_transforms_view =
            (Transform*)arch_view.inventory_cached_archetype
                ->components[ComponentsIndices::TransformComponent];
        components_view.inventory_view =
            (Inventory*)arch_view.inventory_cached_archetype
                ->components[ComponentsIndices::InventoryComponent];
        components_view.inventory_meshes_view =
            (Mesh*)arch_view.inventory_cached_archetype
                ->components[ComponentsIndices::MeshComponent];

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

            const float inventory_slot_scale = inventory_mesh_component->gltf
                ? inventory_component->slot_scale * 2.0f
                : inventory_component->slot_scale;
            const float inventory_slot_half_scale =
                inventory_mesh_component->gltf
                ? inventory_component->slot_scale
                : inventory_component->slot_scale * 0.5f;

            // Take an item from inventory.
            if (*is_item_draged < 0 && is_left_mouse_button_pressed
                && *is_left_mouse_button_released) {
                if (check_crosshair_inventory_intersection(
                        crosshair_transform_component,
                        inventory_transform_component,
                        inventory_component,
                        inventory_slot_scale,
                        inventory_slot_half_scale
                    )) {
                    Point2D<int> intersection_slot =
                        determine_actual_intersection_slot(
                            crosshair_transform_component,
                            inventory_transform_component,
                            inventory_slot_scale,
                            inventory_slot_half_scale
                        );
                    const unsigned int row = intersection_slot.y;
                    const unsigned int column = intersection_slot.x;
                    const unsigned int entity =
                        inventory_component->slots[row][column];
                    // Check slot is not empty and hold an item.
                    if (entity != UINT_MAX && entity >= 0) {
                        EntityLocation item_location =
                            WORLD.entity_locations[get_id(entity)];
                        ItemArchetype* item_arch =
                            static_cast<ItemArchetype*>(item_location.arch);
                        const uint32_t item_index = item_location.index;
                        Item* item_component = &item_arch->items[item_index];

                        if (item_component != nullptr) {
                            for (unsigned int i = 0;
                                 i < item_component->occupied_slots.size();
                                 ++i) {
                                unsigned int row_index =
                                    item_component->occupied_slots[i]
                                    / inventory_component->row;
                                unsigned int col_index =
                                    item_component->occupied_slots[i]
                                    % inventory_component->col;
                                // Need to free all slots that hold an item.
                                inventory_component
                                    ->slots[row_index][col_index] = UINT_MAX;
                            }
                        }
                        // Set currently dragged item entity.
                        *is_item_draged = entity;
                    }
                }
                *is_left_mouse_button_released = false;
            }

            if (*is_item_draged >= 0) {
                if (check_crosshair_inventory_intersection(
                        crosshair_transform_component,
                        inventory_transform_component,
                        inventory_component,
                        inventory_slot_scale,
                        inventory_slot_half_scale
                    )) {
                    Point2D<int> intersection_slot =
                        determine_actual_intersection_slot(
                            crosshair_transform_component,
                            inventory_transform_component,
                            inventory_slot_scale,
                            inventory_slot_half_scale
                        );

                    EntityLocation item_location =
                        WORLD.entity_locations[get_id(*is_item_draged)];
                    ItemArchetype* item_arch =
                        static_cast<ItemArchetype*>(item_location.arch);
                    const uint32_t item_index = item_location.index;
                    Item* item_component = &item_arch->items[item_index];

                    std::vector<unsigned int> potential_occupied_slots;
                    int is_swapable = 0;
                    is_swapable = determine_swappable_status_and_slots(
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
                        is_swapable == -1 || is_swapable >= 0;
                } else {
                    inventory_component->highlighted_slots.clear();
                }
            }

            // Item drop to inventory, swapped or we just can't place.
            if (*is_item_draged >= 0 && is_left_mouse_button_pressed
                && *is_left_mouse_button_released) {
                int is_swapable = 0;
                if (check_crosshair_inventory_intersection(
                        crosshair_transform_component,
                        inventory_transform_component,
                        inventory_component,
                        inventory_slot_scale,
                        inventory_slot_half_scale
                    )) {
                    Point2D<int> intersection_slot =
                        determine_actual_intersection_slot(
                            crosshair_transform_component,
                            inventory_transform_component,
                            inventory_slot_scale,
                            inventory_slot_half_scale
                        );

                    EntityLocation item_location =
                        WORLD.entity_locations[get_id(*is_item_draged)];
                    ItemArchetype* item_arch =
                        static_cast<ItemArchetype*>(item_location.arch);
                    const uint32_t item_index = item_location.index;
                    Item* item_component = &item_arch->items[item_index];

                    std::vector<unsigned int> potential_occupied_slots;
                    is_swapable = determine_swappable_status_and_slots(
                        item_component,
                        inventory_transform_component,
                        potential_occupied_slots,
                        crosshair_transform_component,
                        intersection_slot,
                        inventory_component,
                        inventory_slot_scale
                    );

                    const int item_width = item_component->item_slot_type.width;
                    const int item_height =
                        item_component->item_slot_type.height;

                    // Default value. Just drop item to all empty slots.
                    if (is_swapable == -1) {
                        item_component->occupied_slots =
                            potential_occupied_slots;
                        fill_inventory_slots(
                            item_component,
                            item_width,
                            item_height,
                            inventory_component,
                            *is_item_draged
                        );
                        // Swap one item that we dragging to another one in
                        // inventory.
                    } else if (is_swapable > 0) {
                        EntityLocation item_location =
                            WORLD.entity_locations[get_id(is_swapable)];
                        ItemArchetype* item_arch =
                            static_cast<ItemArchetype*>(item_location.arch);
                        const uint32_t item_index = item_location.index;
                        Item* swaped_item_component =
                            &item_arch->items[item_index];

                        item_component->occupied_slots =
                            potential_occupied_slots;
                        fill_inventory_slots(
                            swaped_item_component,
                            swaped_item_component->item_slot_type.width,
                            swaped_item_component->item_slot_type.height,
                            inventory_component,
                            UINT_MAX
                        );

                        swaped_item_component->occupied_slots.clear();
                        fill_inventory_slots(
                            item_component,
                            item_width,
                            item_height,
                            inventory_component,
                            *is_item_draged
                        );
                    }

                    if (is_swapable == -1) {
                        *is_left_mouse_button_released = false;
                        *is_item_draged = -1;
                        // Already have 2 or more items in potential inventory
                        // slots.
                    } else if (is_swapable == -2) {
                        *is_left_mouse_button_released = false;
                    } else {
                        *is_left_mouse_button_released = false;
                        *is_item_draged = is_swapable;
                    }
                    // Item drop to the ground.
                } else {
                    EntityLocation item_location =
                        WORLD.entity_locations[get_id(*is_item_draged)];
                    ItemArchetype* item_arch =
                        static_cast<ItemArchetype*>(item_location.arch);
                    const uint32_t item_index = item_location.index;
                    item_arch->rigid_bodies[item_index] = {.f_mass = 2.0f};
                    Transform* item_transform =
                        &item_arch->transforms[item_index];
                    Item* item = &item_arch->items[item_index];
                    item->is_actor = true;
                    // Remove this cringe.
                    const uint32_t player = 0;
                    EntityLocation player_location =
                        WORLD.entity_locations[get_id(player)];
                    PlayerArchetype* player_arch =
                        static_cast<PlayerArchetype*>(player_location.arch);
                    const uint32_t player_index = player_location.index;
                    Transform* player_transform =
                        &player_arch->transforms[player_index];
                    item_transform->position = player_transform->position;
                    Vector<float, 3> normalized_forward =
                        normalize(player_transform->forward);
                    item_transform->position[0] += normalized_forward[0] * 2.5f;
                    item_transform->position[1] += normalized_forward[1] * 2.5f;
                    item_transform->position[2] += normalized_forward[2] * 2.5f;
                    item_transform->scale = 0.05f;

                    *is_item_draged = -1;
                    *is_left_mouse_button_released = false;
                }
            }
        }
    }
}

int InventorySystem::determine_swappable_status_and_slots(
    Item* item_component,
    Transform* inventory_transform_component,
    std::vector<unsigned int>& potential_occupied_slots,
    Transform* crosshair_transform_component,
    Point2D<int> intersection_slot,
    Inventory* inventory_component,
    const float inventory_slot_scale
) {
    if (item_component != nullptr) {
        const int item_width = item_component->item_slot_type.width;
        const int item_height = item_component->item_slot_type.height;

        const int row = intersection_slot.y;
        const int column = intersection_slot.x;

        // Find left-upper pivot slot inventory.
        int row_basic_offset = 0;
        int column_basic_offset = 0;

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
            inventory_slot_scale * aspect_rate
        );

        int pivot_row = row - row_basic_offset;
        int pivot_column = column - column_basic_offset;

        pivot_row = clamp<int>(
            0,
            pivot_row,
            static_cast<int>(inventory_component->row) - item_height
        );
        pivot_column = clamp<int>(
            0,
            pivot_column,
            static_cast<int>(inventory_component->col) - item_width
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
    } else {
        // Return -3 as error code means itemComponent is nullptr.
        return -3;
    }
}

void InventorySystem::fill_inventory_slots(
    Item* item_component,
    const int item_width,
    const int item_height,
    Inventory* inventory_component,
    const int fill_value
) {
    for (int i = 0; i < item_height; ++i) {
        for (int j = 0; j < item_width; ++j) {
            const unsigned int slots_row =
                item_component->occupied_slots[i * item_width + j]
                / inventory_component->col;
            const unsigned int slots_column =
                item_component->occupied_slots[i * item_width + j]
                % inventory_component->col;

            inventory_component->slots[slots_row][slots_column] = fill_value;
        }
    }
}

int InventorySystem::determine_swappable_field(
    Item* item_component,
    const int item_width,
    const int item_height,
    int pivot_row,
    int pivot_column,
    Inventory* inventory_component,
    std::vector<unsigned int>& potential_occupied_slots
) {
    item_component->occupied_slots.clear();
    // -1: default value. -2: found two entities in potential slots. Any other
    // value: swappable.
    int is_swapable = -1;
    for (int i = 0; i < item_height; ++i) {
        for (int j = 0; j < item_width; ++j) {
            const unsigned int final_row = pivot_row + i;
            const unsigned int final_column = pivot_column + j;
            if (is_swapable == -1
                && inventory_component->slots[final_row][final_column]
                    != UINT_MAX) {
                is_swapable =
                    inventory_component->slots[final_row][final_column];
            } else if (
                is_swapable > 0
                && inventory_component->slots[final_row][final_column]
                    != UINT_MAX
                && (int)inventory_component->slots[final_row][final_column]
                    != is_swapable
            ) {
                is_swapable = -2;
            }

            potential_occupied_slots.push_back(
                final_row * inventory_component->col + final_column
            );
        }
    }

    return is_swapable;
}

int InventorySystem::calculate_basic_offset(
    const int item_axis_size,
    const float axis_value,
    const float crosshair_axis_position,
    const int axis_slot_index,
    const float inventory_slot_scale
) {
    if (item_axis_size % 2 == 0) {
        const float slot_center_x = axis_value
            + static_cast<float>(axis_slot_index) * inventory_slot_scale;
        if (slot_center_x > crosshair_axis_position) {
            return item_axis_size / 2;
        }
        return item_axis_size / 2 - 1;
    }
    return item_axis_size / 2;
}

bool InventorySystem::check_crosshair_inventory_intersection(
    Transform* crosshair_transform_component,
    Transform* inventory_transform_component,
    Inventory* inventory_component,
    const float inventory_slot_scale,
    const float inventory_slot_half_scale
) {
    return crosshair_transform_component->position[0]
        > inventory_transform_component->position[0] - inventory_slot_half_scale
        && crosshair_transform_component->position[0]
        < inventory_transform_component->position[0] - inventory_slot_half_scale
            + inventory_slot_scale * inventory_component->col
        && crosshair_transform_component->position[1]
        > inventory_transform_component->position[1]
            - inventory_slot_half_scale * aspect_rate
        && crosshair_transform_component->position[1]
        < inventory_transform_component->position[1]
            - inventory_slot_half_scale * aspect_rate
            + inventory_slot_scale * inventory_component->row * aspect_rate;
}

Point2D<int> InventorySystem::determine_actual_intersection_slot(
    Transform* crosshair_transform_component,
    Transform* inventory_transform_component,
    const float inventory_slot_scale,
    const float inventory_slot_half_scale
) {
    float x_delta = crosshair_transform_component->position[0]
        - inventory_transform_component->position[0]
        + inventory_slot_half_scale;
    float y_delta = crosshair_transform_component->position[1]
        - inventory_transform_component->position[1]
        + inventory_slot_half_scale * aspect_rate;

    return Point2D<int> {
        (int)(x_delta / inventory_slot_scale),
        (int)(y_delta / (inventory_slot_scale * aspect_rate))
    };
}
} // namespace glvm

namespace glvm {
// This method is trying to search for suitable slots for the given specific
// type item. It returns true if it finds them and false otherwise.
bool ItemSystem::put_item2x2(
    Inventory* inventory_component,
    unsigned int item_entity
) {
    bool is_slot_found = false;
    unsigned int row = inventory_component->row;
    unsigned int col = inventory_component->col;

    EntityLocation item_location = WORLD.entity_locations[get_id(item_entity)];
    ItemArchetype* item_arch = static_cast<ItemArchetype*>(item_location.arch);
    const uint32_t item_index = item_location.index;
    Item* item_component = &item_arch->items[item_index];

    unsigned int item_width = item_component->item_slot_type.width;
    unsigned int item_height = item_component->item_slot_type.height;
    for (unsigned int i = 0; i < row - item_height + 1; ++i) {
        for (unsigned int j = 0; j < col - item_width + 1; ++j) {
            std::vector<unsigned int> maybe_availabe_slots;
            std::vector<unsigned int> indices_of_maybe_available_slots;
            for (unsigned int m = i; m < i + item_height; ++m) {
                for (unsigned int n = j; n < j + item_width; ++n) {
                    maybe_availabe_slots.push_back(
                        inventory_component->slots[m][n]
                    );
                    indices_of_maybe_available_slots.push_back(m * col + n);
                }
            }

            unsigned int is_all_slots_available = 0;
            for (unsigned int v = 0; v < maybe_availabe_slots.size(); ++v) {
                if (maybe_availabe_slots[v] == UINT_MAX) {
                    ++is_all_slots_available;
                } else {
                    --is_all_slots_available;
                }
            }
            if (maybe_availabe_slots.size() == is_all_slots_available) {
                for (unsigned int w = 0; w < maybe_availabe_slots.size(); ++w) {
                    unsigned int row_index =
                        indices_of_maybe_available_slots[w] / row;
                    unsigned int col_index =
                        indices_of_maybe_available_slots[w] % col;
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

void ItemSystem::update() {
    if (!is_inventory_opened) {
        inventory_archetypes_number = 0;
        WORLD.search_cache_archetypes(
            inventory_required_mask,
            &arch_view.inventory_cached_archetype,
            inventory_archetypes_number
        );
        components_view.inventories_view =
            (Inventory*)arch_view.inventory_cached_archetype
                ->components[ComponentsIndices::InventoryComponent];

        item_archetypes_number = 0;
        WORLD.search_cache_archetypes(
            item_required_mask,
            &arch_view.item_archetype,
            item_archetypes_number
        );
        components_view.items_view =
            (Item*)arch_view.item_archetype
                ->components[ComponentsIndices::ItemComponent];
        components_view.item_colliders_view =
            (Collider*)arch_view.item_archetype
                ->components[ComponentsIndices::ColliderComponent];

        for (unsigned int m = 0;
             m < arch_view.inventory_cached_archetype->entity_count;
             ++m) {
            Inventory* inventory_component =
                &components_view.inventories_view[m];

            for (unsigned int i = 0; i < arch_view.item_archetype->entity_count;
                 ++i) {
                unsigned int item_entity =
                    arch_view.item_archetype->entities[i];
                Collider* item_collider_component =
                    &components_view.item_colliders_view[i];

                for (unsigned int j = 0;
                     j < item_collider_component->colliders.size();
                     ++j) {
                    if (item_collider_component->colliders[j]
                            == inventory_component->entity_owner
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
        WORLD.search_cache_archetypes(
            crosshair_required_mask,
            &arch_view.crosshair_archetype,
            crosshair_archetypes_number
        );
        components_view.crosshair_transforms =
            (Transform*)arch_view.crosshair_archetype
                ->components[ComponentsIndices::TransformComponent];

        item_archetypes_number = 0;
        WORLD.search_cache_archetypes(
            item_required_mask,
            &arch_view.item_archetype,
            item_archetypes_number
        );
        components_view.item_transforms_view =
            (Transform*)arch_view.item_archetype
                ->components[ComponentsIndices::TransformComponent];

        Transform* crosshair_transform_component =
            &components_view.crosshair_transforms[0];
        for (unsigned int i = 0; i < arch_view.item_archetype->entity_count;
             ++i) {
            uint32_t entity_item_containing =
                arch_view.item_archetype->entities[i];
            Transform* item_transform_component =
                &components_view.item_transforms_view[i];
            if (*dragged_item_entity >= 0
                && *dragged_item_entity == (int)entity_item_containing) {
                // Set crosshair position to dragged items.
                item_transform_component->position =
                    crosshair_transform_component->position;
            }
        }
    }
}
} // namespace glvm

namespace glvm {
CMovementSystem::CMovementSystem(CStack& input_stack) :
    input_stack(input_stack) {
}

void CMovementSystem::update() {
    WORLD.search_cache_archetypes(
        player_required_mask,
        &arch_view.player_cached_archetype,
        player_archetypes_number
    );
    components_view.player_moves =
        (Move*)arch_view.player_cached_archetype
            ->components[ComponentsIndices::MoveComponent];
    components_view.player_views =
        (Beholder*)arch_view.player_cached_archetype
            ->components[ComponentsIndices::ViewComponent];
    components_view.player_collider_flags =
        (ColliderFlags*)arch_view.player_cached_archetype
            ->components[ComponentsIndices::ColliderFlagsComponent];
    components_view.player_rigid_body =
        (RigidBody*)arch_view.player_cached_archetype
            ->components[ComponentsIndices::RigidBodyComponent];

    const float camera_speed = 3.0f * delta_frame_time;
    for (unsigned int i = 0;
         i < arch_view.player_cached_archetype->entity_count;
         ++i) {
        const uint64_t entity = arch_view.player_cached_archetype->entities[i];
        EntityLocation& entity_location =
            WORLD.entity_locations[get_id(entity)];
        Beholder* player_view = &components_view.player_views[i];
        Move* player_move = &components_view.player_moves[i];
        ColliderFlags* player_collider_flags =
            &components_view.player_collider_flags[i];
        RigidBody* player_rigid_body = &components_view.player_rigid_body[i];
        for (int n = 0; n < 6; ++n) {
            Vector<float, 3> right;
            Vector<float, 3> forward;
            switch (input_stack[n]) {
                case EEvents::EMoveLeft:
                    right = calculate_vector_rl(*player_view);
                    player_move->frame_movement -= right * camera_speed;
                    entity_location.is_dirty = true;
                    break;
                case EEvents::EMoveRight:
                    right = calculate_vector_rl(*player_view);
                    player_move->frame_movement += right * camera_speed;
                    entity_location.is_dirty = true;
                    break;
                case EEvents::EMoveBackward:
                    forward = calculate_vector_fb(*player_view, G_E_EVENT);
                    player_move->frame_movement -= forward * camera_speed;
                    entity_location.is_dirty = true;
                    break;
                case EEvents::EMoveForward:
                    forward = calculate_vector_fb(*player_view, G_E_EVENT);
                    player_move->frame_movement += forward * camera_speed;
                    entity_location.is_dirty = true;
                    break;
                case EEvents::EJump: {
                    entity_location.is_dirty = true;
                    uint8_t is_groud_collision_mask =
                        (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                    if (player_collider_flags->flags
                        & is_groud_collision_mask) {
                        player_rigid_body->jump_accumulator = 1.5f;
                    }
                } break;
                default:
                    break;
            }
        }
    }

    rigid_body_contained_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        rigid_body_required_mask,
        arch_view.rigid_body_contained_archetypes_cache,
        rigid_body_contained_archetypes_number
    );

    for (uint32_t i0 = 0; i0 < rigid_body_contained_archetypes_number; ++i0) {
        Archetype* current_arch =
            arch_view.rigid_body_contained_archetypes_cache[i0];
        components_view.transforms =
            (Transform*)
                current_arch->components[ComponentsIndices::TransformComponent];
        components_view.rigid_bodies =
            (RigidBody*)
                current_arch->components[ComponentsIndices::RigidBodyComponent];
        components_view.moves =
            (Move*)current_arch->components[ComponentsIndices::MoveComponent];
        components_view.items =
            (Item*)current_arch->components[ComponentsIndices::ItemComponent];

        for (uint32_t i1 = 0; i1 < current_arch->entity_count; ++i1) {
            const uint64_t entity = current_arch->entities[i1];
            EntityLocation& entity_location =
                WORLD.entity_locations[get_id(entity)];
            entity_location.is_dirty = true;

            if (components_view.items && !components_view.items[i1].is_actor) {
                continue;
            }

            Transform* r_transform_component = &components_view.transforms[i1];
            RigidBody* rigid_body_componennt =
                &components_view.rigid_bodies[i1];
            Move* move_component = &components_view.moves[i1];
            r_transform_component->gravity_accumulator += delta_frame_time;
            float gravity = 9.8f * r_transform_component->gravity_accumulator
                * rigid_body_componennt->f_mass * 0.0005;
            if (gravity > 0.2f) {
                gravity = 0.2;
            }

            move_component->gravity[1] -= gravity;
        }
    }
}

Vector<float, 3> CMovementSystem::calculate_vector_rl(Beholder& beholder) {
    Vector<float, 3> normalized_vector =
        normalize(cross(beholder.forward, Vector<float, 3> {0.0f, -1.0f, 0.0}));
    return normalized_vector;
}

Vector<float, 3> CMovementSystem::calculate_vector_fb(
    Beholder& beholder,
    CEvent& event
) {
    Vector<float, 3> forward(0.0f);
    current_x = (float)G_E_EVENT.mouse_pointer_position.offset_x;
    float delta_x = current_x - prev_x;
    const Vector<float, 3> rotate_axis = {0.0, -1.0, 0.0};
    float rotation_angle = delta_x;
    constexpr float ANGLE_SCALE = 0.1f;
    rotation_angle = radians(rotation_angle * ANGLE_SCALE);
    // Quaternions need division by 2.
    constexpr float QUAT_ANGLE_CORRECTION = 0.5f;
    const float sin_rotation_angle =
        sinf(rotation_angle * QUAT_ANGLE_CORRECTION);
    Quaternion rotation_quat = Quaternion(
        cosf(rotation_angle * QUAT_ANGLE_CORRECTION),
        sin_rotation_angle * rotate_axis[0],
        sin_rotation_angle * rotate_axis[1],
        sin_rotation_angle * rotate_axis[2]
    );
    const Quaternion applied_rotation_quat = (rotation_quat
                                              * Quaternion(
                                                  0.0f,
                                                  beholder.forward[0],
                                                  beholder.forward[1],
                                                  beholder.forward[2]
                                              ))
        * conjugate(rotation_quat);

    forward[0] = applied_rotation_quat.x;
    forward[1] = 0.0f;
    forward[2] = applied_rotation_quat.z;
    prev_x = (float)G_E_EVENT.mouse_pointer_position.offset_x;
    forward = normalize(forward);
    return forward;
}
} // namespace glvm

namespace glvm {
namespace {
bool aabb_overlap(
    const Vector<float, 3>& a_position,
    const MeshAxisMaxAbsoluteValues& a_bounds,
    float a_scale,
    const Vector<float, 3>& b_position,
    const MeshAxisMaxAbsoluteValues& b_bounds,
    float b_scale
) {
    return a_position[0] + a_bounds.origin_offset_x * a_scale
            + a_bounds.absolute_x * a_scale
        > b_position[0] + b_bounds.origin_offset_x * b_scale
            - b_bounds.absolute_x * b_scale
        && a_position[0] + a_bounds.origin_offset_x * a_scale
            - a_bounds.absolute_x * a_scale
        < b_position[0] + b_bounds.origin_offset_x * b_scale
            + b_bounds.absolute_x * b_scale
        && a_position[1] + a_bounds.origin_offset_y * a_scale
            + a_bounds.absolute_y * a_scale
        > b_position[1] + b_bounds.origin_offset_y * b_scale
            - b_bounds.absolute_y * b_scale
        && a_position[1] + a_bounds.origin_offset_y * a_scale
            - a_bounds.absolute_y * a_scale
        < b_position[1] + b_bounds.origin_offset_y * b_scale
            + b_bounds.absolute_y * b_scale
        && a_position[2] + a_bounds.origin_offset_z * a_scale
            + a_bounds.absolute_z * a_scale
        > b_position[2] + b_bounds.origin_offset_z * b_scale
            - b_bounds.absolute_z * b_scale
        && a_position[2] + a_bounds.origin_offset_z * a_scale
            - a_bounds.absolute_z * a_scale
        < b_position[2] + b_bounds.origin_offset_z * b_scale
            + b_bounds.absolute_z * b_scale;
}

bool is_above(
    const Vector<float, 3>& a_position,
    const MeshAxisMaxAbsoluteValues& a_bounds,
    float a_scale,
    const Vector<float, 3>& b_position,
    const MeshAxisMaxAbsoluteValues& b_bounds,
    float b_scale
) {
    constexpr float EPSILON = 0.15f;
    return a_position[1] + a_bounds.origin_offset_y * a_scale
        - a_bounds.absolute_y * a_scale + EPSILON
        > b_position[1] + b_bounds.origin_offset_y * b_scale
        + b_bounds.absolute_y * b_scale;
}
} // namespace

// This update searching for referring to colliders entities and check their
// transform components for collision, and if collision detected check if
// backtracking entity had gravity component for call Gravity function.
void CPhysicsSystem::update() {
    cached_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        required_mask,
        arch_view.cached_archetypes,
        cached_archetypes_number
    );

    for (uint32_t x = 0; x < cached_archetypes_number; ++x) {
        Archetype* arch = arch_view.cached_archetypes[x];

        components_view.transforms_view =
            (Transform*)arch_view.cached_archetypes[x]
                ->components[ComponentsIndices::TransformComponent];
        components_view.moves_view =
            (Move*)arch_view.cached_archetypes[x]
                ->components[ComponentsIndices::MoveComponent];
        components_view.rigid_bodies_view =
            (RigidBody*)arch_view.cached_archetypes[x]
                ->components[ComponentsIndices::RigidBodyComponent];
        components_view.collider_flags_view =
            (ColliderFlags*)arch_view.cached_archetypes[x]
                ->components[ComponentsIndices::ColliderFlagsComponent];
        components_view.colliders_view =
            (Collider*)arch_view.cached_archetypes[x]
                ->components[ComponentsIndices::ColliderComponent];
        components_view.meshes_view =
            (Mesh*)arch_view.cached_archetypes[x]
                ->components[ComponentsIndices::MeshComponent];

        float delta_time = 5.5f * f_delta_time;
        for (unsigned int i = 0; i < arch->entity_count; ++i) {
            if (components_view.transforms_view
                && components_view.collider_flags_view
                && components_view.moves_view
                && components_view.rigid_bodies_view) {
                Transform& transform_component =
                    components_view.transforms_view[i];
                Move& move = components_view.moves_view[i];
                ColliderFlags& collider_flags =
                    components_view.collider_flags_view[i];
                uint8_t is_groud_collision_mask =
                    (0u << 0) | (1u << 1) | (0u << 2) | (0u << 3);
                if (collider_flags.flags & is_groud_collision_mask) {
                    move.gravity = 0;
                    transform_component.gravity_accumulator = 0.0f;
                }
                uint8_t is_wall_collision_mask =
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
                            ALL_MESH_MAX_ABSOLUTE_VALUES[meshes[i].handle.id];
                        const Vector<float, 3> player_position =
                            transform_component.position;
                        for (uint32_t c = 0; c < colliders[i].colliders.size();
                             ++c) {
                            const uint32_t collided_entity =
                                colliders[i].colliders[c];
                            EntityLocation& collided_location =
                                WORLD.entity_locations[get_id(collided_entity)];
                            Archetype* collided_arch = collided_location.arch;
                            if (collided_arch == nullptr) {
                                // Entity was removed this frame (e.g. by
                                // DamageSystem) after collision detection.
                                continue;
                            }
                            const uint32_t collided_index =
                                collided_location.index;
                            Transform* collided_transform =
                                (Transform*)collided_arch->components
                                    [ComponentsIndices::TransformComponent];
                            Mesh* collided_mesh =
                                (Mesh*)collided_arch->components
                                    [ComponentsIndices::MeshComponent];
                            if (!collided_transform || !collided_mesh) {
                                continue;
                            }
                            collided_transform += collided_index;
                            collided_mesh += collided_index;
                            const MeshAxisMaxAbsoluteValues collided_bounds =
                                ALL_MESH_MAX_ABSOLUTE_VALUES[collided_mesh
                                                                 ->handle.id];
                            const Vector<float, 3> collided_position =
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
                            for (int axis = 0; axis < 3; ++axis) {
                                Vector<float, 3> candidate = player_position;
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
                    uint8_t wall_collision_turn_off_mask =
                        (0u << 0) | (1u << 1) | (1u << 2) | (1u << 3);
                    collider_flags.flags &= wall_collision_turn_off_mask;
                }
                transform_component.position += move.frame_movement;
                transform_component.position += move.gravity;
                move.gravity = 0.0f;
                move.frame_movement = 0.0f;
                RigidBody& rigid_body = components_view.rigid_bodies_view[i];
                if (rigid_body.jump_accumulator > 0.0f) {
                    rigid_body.jump_accumulator -= delta_time;
                    Vector<float, 3> jump =
                        Vector<float, 3> {0.0f, 5.0f, 0.0f} * delta_time;
                    transform_component.position += jump;
                }
            }
        }
    }
}
} // namespace glvm

namespace glvm {
CProjectileSystem::CProjectileSystem(CStack& input_stack) :
    input_stack(input_stack) {
}

void CProjectileSystem::update() {
    float camera_speed = 5.5f * delta_frame_time;

    player_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        player_required_mask,
        &arch_view.player_cached_archetype,
        player_archetypes_number
    );
    components_view.player_transforms =
        (Transform*)arch_view.player_cached_archetype
            ->components[ComponentsIndices::TransformComponent];
    components_view.player_views =
        (Beholder*)arch_view.player_cached_archetype
            ->components[ComponentsIndices::ViewComponent];

    projectile_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        projectile_required_mask,
        &arch_view.projectile_archetype,
        projectile_archetypes_number
    );

    if (projectile_cooldown > 0) {
        projectile_cooldown -= camera_speed;
    }

    // Iterate on every player and create projectile if "LMB pressed" event
    // found.
    for (unsigned int i = 0;
         i < arch_view.player_cached_archetype->entity_count;
         ++i) {
        Beholder* player_view = &components_view.player_views[i];
        Transform* player_transform = &components_view.player_transforms[i];
        const uint32_t max_event_number = 6;
        for (uint32_t n = 0; n < max_event_number; ++n) {
            if (!is_inventory_opened
                && input_stack.search_element(EEvents::EMouseLeftButton)
                    == EEvents::EMouseLeftButton) {
                if (projectile_cooldown <= 0) {
                    MeshHandle mesh_handle {};
                    const uint32_t sphere_mesh_handle_index = 2;
                    if (mesh_handlers.size() > 2) {
                        mesh_handle = mesh_handlers[sphere_mesh_handle_index];
                    }

                    TextureHandle texture_handle {};
                    const uint32_t gray_texture_handle = 2;
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
                    uint64_t projectile_entity =
                        arch_entity_manager->create_entity();
                    WORLD.add_entity_to_archetype(
                        projectile_entity,
                        arch_view.projectile_archetype
                    );
                    EntityLocation projectile_location =
                        WORLD.entity_locations[get_id(projectile_entity)];

                    create_projectile(
                        player_transform->position,
                        player_view->forward,
                        mesh_handle,
                        material,
                        damage,
                        projectile_location
                    );

                    sound_engine->create_sound_sample(
                        "../../../examples/assets/sounds/pistol.wav",
                        5,
                        22050,
                        0.05
                    );
                    projectile_cooldown = 2.0;
                }
            }
        }
    }

    projectile_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        projectile_required_mask,
        &arch_view.projectile_archetype,
        projectile_archetypes_number
    );

    components_view.projectile_transforms =
        (Transform*)arch_view.projectile_archetype
            ->components[ComponentsIndices::TransformComponent];
    components_view.projectile_collider_flags =
        (ColliderFlags*)arch_view.projectile_archetype
            ->components[ComponentsIndices::ColliderFlagsComponent];
    components_view.projectile_colliders =
        (Collider*)arch_view.projectile_archetype
            ->components[ComponentsIndices::ColliderComponent];
    components_view.projectile_bundles =
        (ProjectileBundle*)arch_view.projectile_archetype
            ->components[ComponentsIndices::ProjectileBundleComponent];
    components_view.projectile_health =
        (Health*)arch_view.projectile_archetype
            ->components[ComponentsIndices::HealthComponent];
    components_view.projectile_attacks =
        (Attack*)arch_view.projectile_archetype
            ->components[ComponentsIndices::AttackComponent];

    // Update position of every projectile.
    for (unsigned int x = 0; x < arch_view.projectile_archetype->entity_count;
         ++x) {
        Transform* projectile_transform =
            &components_view.projectile_transforms[x];
        projectile_transform->position +=
            normalize(projectile_transform->forward) * camera_speed * 2.5;
    }
    // Iterate every projectile, check for collisions with another entities and
    // update damage info if collided entity has attack component.
    for (unsigned int i = 0; i < arch_view.projectile_archetype->entity_count;
         ++i) {
        ColliderFlags* projectile_collider_flags =
            &components_view.projectile_collider_flags[i];
        Health* projectile_health = &components_view.projectile_health[i];
        Attack* projectile_attack = &components_view.projectile_attacks[i];
        const uint8_t wall_collision_bit = 1;
        const uint8_t ground_collistion_bit = (1 << 1);
        if ((projectile_collider_flags->flags & wall_collision_bit)
            || (projectile_collider_flags->flags & ground_collistion_bit)) {
            Damage* projectile_damage =
                &components_view.projectile_bundles[i].damage;
            Collider* projectile_collider =
                &components_view.projectile_colliders[i];
            for (unsigned int j = 0; j < projectile_collider->colliders.size();
                 ++j) {
                unsigned int collided_entity =
                    projectile_collider->colliders[j];

                EntityLocation collided_entity_location =
                    WORLD.entity_locations[get_id(collided_entity)];
                uint64_t required_mask =
                    (1ul << ComponentsIndices::HealthComponent)
                    | (1ul << ComponentsIndices::AttackComponent);

                if ((collided_entity_location.arch != nullptr)
                    && (collided_entity_location.arch->mask & required_mask)
                        == required_mask) {
                    Attack* attacks =
                        (Attack*)collided_entity_location.arch
                            ->components[ComponentsIndices::AttackComponent];
                    attacks[collided_entity_location.index].damage =
                        projectile_damage->maximum_damage;
                    projectile_health->current_health = 0;
                }
            }
        }
    }
}
} // namespace glvm

namespace glvm {
void SpatialGridSystem::update() {
    SpatialGrid& spatial_grid = WORLD.spatial_grid;
    assert(
        spatial_grid.width > 0 && spatial_grid.height > 0
        && spatial_grid.depth > 0
    );
    const float chunk_size = spatial_grid.grid[0][0][0].SIZE;

    const float half_width = spatial_grid.width * chunk_size * 0.5f;
    const float half_height = spatial_grid.height * chunk_size * 0.5f;
    const float half_depth = spatial_grid.depth * chunk_size * 0.5f;

    cached_archetypes_number = 0;
    WORLD.search_cache_archetypes(
        required_mask,
        cached_archetypes,
        cached_archetypes_number
    );

    for (uint32_t i0 = 0; i0 < cached_archetypes_number; ++i0) {
        Archetype* arch = cached_archetypes[i0];
        view.transforms =
            (Transform*)arch->components[ComponentsIndices::TransformComponent];
        view.meshes = (Mesh*)arch->components[ComponentsIndices::MeshComponent];

        for (uint32_t i1 = 0; i1 < arch->entity_count; ++i1) {
            const uint64_t entity = arch->entities[i1];
            EntityLocation& entity_location =
                WORLD.entity_locations[get_id(entity)];
            if (!entity_location.is_dirty && is_initialized) {
                continue;
            }

            if (entity_location.grid_cell_counter > 0) {
                for (uint32_t i2 = 0; i2 < entity_location.grid_cell_counter;
                     ++i2) {
                    uint32_t z = entity_location.grid_cell_indicies[i2][0];
                    uint32_t y = entity_location.grid_cell_indicies[i2][1];
                    uint32_t x = entity_location.grid_cell_indicies[i2][2];
                    std::vector<uint32_t>& chunk_entities =
                        spatial_grid.grid[z][y][x].entities;
                    // Remove by value: the recorded index can be stale after
                    // other removals shifted the cell's vector.
                    for (uint32_t i3 = 0; i3 < chunk_entities.size(); ++i3) {
                        if (chunk_entities[i3] == entity) {
                            chunk_entities.erase(chunk_entities.begin() + i3);
                            break;
                        }
                    }
                }
                entity_location.grid_cell_counter = 0;
            }

            const Transform& transform = view.transforms[i1];
            const Mesh& mesh = view.meshes[i1];

            MeshHandle entity_mesh_handle = mesh.handle;
            MeshAxisMaxAbsoluteValues entity_chunk_bounds =
                ALL_MESH_MAX_ABSOLUTE_VALUES[entity_mesh_handle.id];
            std::vector<Vector<float, 3>> entity_box_corner_bound_points =
                compute_box_corner_bound_points(
                    entity_chunk_bounds,
                    transform.position,
                    transform.scale
                );

            // Need only left bottom back corner point and right upper front
            // corner point to obtain all box bounds.
            const Vector<float, 3> min_entity_position =
                entity_box_corner_bound_points[0];
            const Vector<float, 3> max_entity_position =
                entity_box_corner_bound_points[1];

            int index_min_x = static_cast<int>(
                (min_entity_position[0] + half_width) / chunk_size
            );
            int index_min_y = static_cast<int>(
                (min_entity_position[1] + half_height) / chunk_size
            );
            int index_min_z = static_cast<int>(
                (min_entity_position[2] + half_depth) / chunk_size
            );

            int index_max_x = static_cast<int>(
                (max_entity_position[0] + half_width) / chunk_size
            );
            int index_max_y = static_cast<int>(
                (max_entity_position[1] + half_height) / chunk_size
            );
            int index_max_z = static_cast<int>(
                (max_entity_position[2] + half_depth) / chunk_size
            );

            // Entity can legitimately leave the fixed-size world grid (fell off
            // the world edge, projectile flew away) - clamp to nearest edge
            // cell instead of crashing.
            index_min_x =
                std::clamp(index_min_x, 0, (int)spatial_grid.width - 1);
            index_min_y =
                std::clamp(index_min_y, 0, (int)spatial_grid.height - 1);
            index_min_z =
                std::clamp(index_min_z, 0, (int)spatial_grid.depth - 1);
            index_max_x =
                std::clamp(index_max_x, 0, (int)spatial_grid.width - 1);
            index_max_y =
                std::clamp(index_max_y, 0, (int)spatial_grid.height - 1);
            index_max_z =
                std::clamp(index_max_z, 0, (int)spatial_grid.depth - 1);

            for (auto i2 = index_min_z; i2 <= index_max_z; ++i2) {
                for (auto i3 = index_min_y; i3 <= index_max_y; ++i3) {
                    for (auto i4 = index_min_x; i4 <= index_max_x; ++i4) {
                        std::vector<uint32_t>& chunk_entities =
                            spatial_grid.grid[i2][i3][i4].entities;
                        if (!is_exist<uint32_t>(chunk_entities, entity)) {
                            chunk_entities.push_back(entity);
                            const uint32_t current_grid_cell =
                                entity_location.grid_cell_counter;
                            assert(current_grid_cell < 32);
                            entity_location
                                .grid_cell_indicies[current_grid_cell] =
                                Vector<float, 3>(i2, i3, i4);
                            entity_location
                                .cell_entity_indices[current_grid_cell] =
                                chunk_entities.size() - 1;
                            entity_location.is_dirty = false;
                            ++entity_location.grid_cell_counter;
                        }
                    }
                }
            }
        }
    }
    if (!is_initialized) {
        is_initialized = true;
    }
}

}; // namespace glvm

namespace glvm {
TextureManager* TextureManager::p_instance = nullptr;
std::mutex TextureManager::mutex;

TextureManager::TextureManager() = default;

void TextureManager::bind_texture(
    unsigned int entity_id,
    unsigned int texture_id
) {
    texture_vector[texture_id].entities_owns_this_type_of_texture.push_back(
        entity_id
    );
}

TextureManager* TextureManager::get_instance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (p_instance == nullptr) {
        p_instance = new TextureManager();
    }
    return p_instance;
}

void TextureManager::set_texture_vector(std::vector<Texture> textures) {
    texture_vector = textures;
}

std::vector<Texture>& TextureManager::get_texture_vector() {
    return texture_vector;
}
} // namespace glvm

ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });

                    if (this->stop && this->tasks.empty()) {
                        return;
                    }

                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) {
        worker.join();
    }
}

#ifdef __linux__

namespace glvm {
CTimerX::CTimerX() {
    init_frequency();
    reset();
}

double CTimerX::init_frequency() {
    return frequency_ = 1e+9;
}

double CTimerX::reset() {
    clock_gettime(CLOCK_MONOTONIC, &start_);
    return start_.tv_sec + start_.tv_nsec;
}

double CTimerX::get_elapsed() {
    clock_gettime(CLOCK_MONOTONIC, &now_);
    seconds_ = now_.tv_sec - start_.tv_sec;
    nanoseconds_ = now_.tv_nsec - start_.tv_nsec;
    return seconds_ + nanoseconds_ / frequency_;
}
} // namespace glvm
#endif
namespace glvm {
IChrono* CTimerCreator::create() {
#ifdef __linux__
    return new CTimerX;
#endif

#ifdef _WIN32
    return new CTimerWin;
#endif
}
} // namespace glvm

#ifdef _WIN32

namespace glvm {
CTimerWin::CTimerWin() {
    init_frequency();
    reset();
}

double CTimerWin::init_frequency() {
    QueryPerformanceFrequency((PLARGE_INTEGER)&i64_freq);
    return (double)i64_freq;
}

double CTimerWin::reset() {
    QueryPerformanceCounter((PLARGE_INTEGER)&i64_start);
    return (double)i64_start;
}

double CTimerWin::get_elapsed() {
    QueryPerformanceCounter((PLARGE_INTEGER)&i64_now);
    return (double)(i64_now - i64_start) / i64_freq;
}
} // namespace glvm
#endif // _WIN32

#ifdef _WIN32

namespace glvm {
void CSoundEngineWaveform::sound_stream() {
    for (unsigned int i = 0; i < t_sound_container.size(); ++i) {
        playback_sound_sample(*t_sound_container[i]);
        t_sound_container.erase(t_sound_container.begin() + i);
    }
}

void CSoundEngineWaveform::playback_sound_sample(CSoundSample& sample) {
    HWAVEOUT h_wave_out;
    WAVEHDR lp_wave_hdr {};
    WAVEFORMATEX format;
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = sample.ui_rate;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nChannels * 2;
    // Change this field first if got any problems.
    format.nBlockAlign = 4;
    format.wBitsPerSample = 16;
    format.cbSize = 0;
    // Open a waveform device for output using window callback.
    unsigned int rc = 0;
    rc = waveOutOpen(&h_wave_out, WAVE_MAPPER, &format, 0L, 0L, 0L);
    if (rc != MMSYSERR_NOERROR) {
        std::cerr << "waveOutOpen: " << "error code: " << rc << std::endl;
        std::exit(-1);
    }

    std::ifstream file(
        sample.k_path_to_file,
        std::ios_base::binary | std::ios_base::in
    );
    if (!file) {
        std::cerr << "Fail to open file." << std::endl;
        std::exit(-1);
    }

    char* buf = (char*)malloc(format.nAvgBytesPerSec * 2);
    while (true) {
        file.read(buf, format.nAvgBytesPerSec * 2);
        if (file.gcount() == 0) {
            break;
        }

        lp_wave_hdr.lpData = buf;
        lp_wave_hdr.dwBufferLength = file.gcount();
        lp_wave_hdr.dwFlags = 0L;
        lp_wave_hdr.dwLoops = 0L;
        waveOutPrepareHeader(h_wave_out, &lp_wave_hdr, sizeof(WAVEHDR));
        waveOutWrite(h_wave_out, &lp_wave_hdr, sizeof(WAVEHDR));
        Sleep(
            (lp_wave_hdr.dwBufferLength * 1000) / (format.nAvgBytesPerSec * 2)
        );
        waveOutUnprepareHeader(h_wave_out, &lp_wave_hdr, sizeof(WAVEHDR));
    }

    free(buf);
    waveOutClose(h_wave_out);
}

void CSoundEngineWaveform::set_master_volume(long /* l_volume */) {
}

std::vector<CSoundSample*>& CSoundEngineWaveform::get_sound_container() {
    return t_sound_container;
}
} // namespace glvm
#endif // _WIN32

#ifdef _WIN32

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    // NOLINT(readability-identifier-naming)
    HWND h_wnd,
    UINT msg,
    WPARAM w_param,
    LPARAM l_param
);

namespace glvm {
WindowWinVulkan* WindowWinVulkan::instance = nullptr;

WindowWinVulkan::WindowWinVulkan() {
    instance = this;
    const char* title = "Game";
    int window_width = width / 2, window_height = height / 2;
    // Register the window class for the main window.
    window_class.style = 0;
    window_class.lpfnWndProc = main_wnd_proc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = NULL;
    window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(NULL, NULL);
    window_class.hbrBackground = NULL;
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = "Game";

    RegisterClassA(&window_class);

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
        | WS_MAXIMIZEBOX | WS_THICKFRAME;

    RECT rect;
    SetRect(&rect, 0, 0, window_width, window_height);
    AdjustWindowRect(&rect, style, FALSE);
    int window_x = (width - (rect.right - rect.left)) / 2;
    int window_y = (height - (rect.bottom - rect.top)) / 2;

    // Create the main window.
    p_modern_window = CreateWindowA(
        "Game",
        title,
        style,
        window_x,
        window_y,
        rect.right - rect.left,
        rect.bottom - rect.top,
        (HWND)NULL,
        (HMENU)NULL,
        NULL,
        (LPVOID)NULL
    );

    // Show the window and paint its contents.
    ShowWindow(p_modern_window, SW_SHOWDEFAULT);
    UpdateWindow(p_modern_window);
}

void WindowWinVulkan::swap_buffers() {
}

void WindowWinVulkan::clear_display() {
}

bool WindowWinVulkan::handle_event(CEvent& event) {
    // Create message struct object.
    MSG msg;

    SetWindowLongPtrW(p_modern_window, GWLP_USERDATA, (LONG_PTR)&event);
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        input_stack->control_input(event);
        // DispatchMessage may not have set the event (e.g. a WM_CHAR left
        // over from TranslateMessage, or WM_KEYUP for an unhandled key). The
        // stale value would otherwise be re-pushed by ControlInput on the next
        // message/frame and toggle the cursor a second time.
        event.set_event(EEvents::EDefault);
    }
    return false;
}

void WindowWinVulkan::close() {
    DestroyWindow(p_modern_window);
    PostQuitMessage(0);
}

HWND WindowWinVulkan::get_classic_window_hwnd() {
    return p_classic_window;
}

HWND WindowWinVulkan::get_modern_window_hwnd() {
    return p_modern_window;
}

void WindowWinVulkan::cursor_lock(
    int pointer_x,
    int pointer_y,
    int* out_offset_x,
    int* out_offset_y
) {
    RECT client_rect;
    GetClientRect(p_modern_window, &client_rect);
    const int center_x = client_rect.right / 2;
    const int center_y = client_rect.bottom / 2;
    POINT point_position {center_x, center_y};
    ClientToScreen(p_modern_window, &point_position);
    // Solve a problem with endlessly growing numbers in the start game run.
    if (pointer_x > client_rect.right || pointer_x < 0
        || pointer_y > client_rect.bottom || pointer_y < 0) {
        return;
    }
    int i_offset_x = 0, i_offset_y = 0;
    i_offset_x = pointer_x - previous_x;
    i_offset_y = pointer_y - previous_y;
    previous_x = pointer_x;
    previous_y = pointer_y;

    // The per-frame delta is measured against the cursor's actual position
    // after the previous warp, not against the computed center: the two can
    // differ by a few pixels (DPI rounding), and accumulating that constant
    // error would slowly drift the view until it hits the pitch clamp below.
    // A >250px jump between frames is a cursor teleport, not mouse movement:
    // discard it so the camera doesn't snap toward the new position (startup,
    // refocus, stale sample after the warp, and the first sample after the
    // inventory closes - the cursor was free while the inventory was open).
    // Discard the sample, just re-warp to the center.
    if (i_offset_x > 250 || i_offset_x < -250 || i_offset_y > 250
        || i_offset_y < -250) {
    } else {
        *out_offset_x += i_offset_x;
        *out_offset_y -= i_offset_y;
    }
    // Pitch is limited by angle in Engine::SetViewMatrix(), so this offset may
    // accumulate freely; no pixel clamp here (resolution-independent).
    SetCursorPos(point_position.x, point_position.y);
    SetCursor(NULL);
    // Baseline for the next frame: where the cursor actually ended up after the
    // warp (matches what the next WM_MOUSEMOVE will report).
    POINT actual_position;
    GetCursorPos(&actual_position);
    ScreenToClient(p_modern_window, &actual_position);
    previous_x = actual_position.x;
    previous_y = actual_position.y;
}

// Callback method for events handling.
LRESULT CALLBACK WindowWinVulkan::main_wnd_proc(
    HWND hwnd,
    UINT msg,
    WPARAM w_param,
    LPARAM l_param
) {
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param);
    CEvent* p_event = (CEvent*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if (msg == WM_KEYDOWN && w_param == VK_ESCAPE && p_event != nullptr
        && (l_param & (1 << 30)) == 0) {
        p_event->set_event(EEvents::ECursorReleased);
        return 0;
    }

    if (ImGui::GetCurrentContext()) {
        ImGuiIO& io = ImGui::GetIO();
        const bool is_mouse_message =
            (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
            || msg == WM_MOUSEWHEEL || msg == WM_MOUSEHWHEEL;
        const bool is_keyboard_message =
            (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) || msg == WM_CHAR
            || msg == WM_SYSCHAR || msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP;
        if ((is_mouse_message && io.WantCaptureMouse)
            || (is_keyboard_message && io.WantCaptureKeyboard)) {
            return 0;
        }
    }

    if (p_event == nullptr) {
        return DefWindowProcA(hwnd, msg, w_param, l_param);
    }
    int i_mouse_position_x, i_mouse_position_y;
    switch (msg) {
        case WM_CREATE:
            return 0;
        case WM_SIZE:
            return 0;
        case WM_LBUTTONDOWN:
            p_event->set_event(EEvents::EMouseLeftButton);
            return 0;

        case WM_SETFOCUS:
            if (WindowWinVulkan::instance) {
                WindowWinVulkan::instance->is_focused = true;
            }
            return 0;
        case WM_KILLFOCUS:
            if (WindowWinVulkan::instance) {
                WindowWinVulkan::instance->is_focused = false;
            }
            return 0;
        case WM_LBUTTONUP:
            p_event->set_event(EEvents::EMouseLeftButtonRelease);
            p_event->is_left_mouse_button_released = true;
            return 0;
        case WM_MOUSEMOVE:
            i_mouse_position_x = GET_X_LPARAM(l_param);
            i_mouse_position_y = GET_Y_LPARAM(l_param);
            p_event->set_event(EEvents::EMousePointerPosition);
            p_event->mouse_pointer_position.position_x = i_mouse_position_x;
            p_event->mouse_pointer_position.position_y = i_mouse_position_y;
            return 0;
        case WM_KEYDOWN:
            switch (w_param) {
                case VK_LEFT:
                    break;
                case VK_RIGHT:
                    break;
                case VK_ESCAPE:
                    break;
                case VK_W:
                    p_event->set_event(EEvents::EMoveForward);
                    break;
                case VK_S:
                    p_event->set_event(EEvents::EMoveBackward);
                    break;
                case VK_A:
                    p_event->set_event(EEvents::EMoveLeft);
                    break;
                case VK_D:
                    p_event->set_event(EEvents::EMoveRight);
                    break;
                case VK_SPACE:
                    p_event->set_event(EEvents::EJump);
                    break;
                case VK_I:
                    p_event->set_event(EEvents::EInventory);
                    break;
                case VK_UP:
                    break;
                case VK_DOWN:
                    break;
                case VK_HOME:
                    break;
                case VK_END:
                    break;
                case VK_INSERT:
                    break;
                case VK_DELETE:
                    break;
                case VK_F2:
                    break;
                default:
                    break;
            }
            break;
        case WM_KEYUP:
            switch (w_param) {
                case VK_LEFT:
                    break;
                case VK_RIGHT:
                    break;
                case VK_W:
                    p_event->set_event(EEvents::EKeyreleaseW);
                    break;
                case VK_S:
                    p_event->set_event(EEvents::EKeyreleaseS);
                    break;
                case VK_A:
                    p_event->set_event(EEvents::EKeyreleaseA);
                    break;
                case VK_D:
                    p_event->set_event(EEvents::EKeyreleaseD);
                    break;
                case VK_SPACE:
                    p_event->set_event(EEvents::EKeyreleaseJump);
                    break;
                case VK_I:
                    p_event->set_event(EEvents::EInventoryRelease);
                    break;
                case VK_UP:
                    break;
                case VK_DOWN:
                    break;
                case VK_HOME:
                    break;
                case VK_END:
                    break;
                case VK_INSERT:
                    break;
                case VK_DELETE:
                    break;
                case VK_F2:
                    break;
                default:
                    break;
            }
            break;
        case WM_CLOSE:
            p_event->set_event(EEvents::EGameLoopKill);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        // Process other messages.
        default:
            return DefWindowProc(hwnd, msg, w_param, l_param);
    }
    return 0;
}
} // namespace glvm
#endif // _WIN32

const int K_VERTEX_SIZE = 9;
constexpr float K_WIDTH_OFFSET = 1.0f / 3;

float A_VERTICES[K_VERTEX_SIZE] = {
    -0.5f,
    -0.5f,
    0.5f, // Left vertex.
    -0.5f,
    0.5f,
    0.0f, // Right vertex.
    0.0f,
    0.0f,
    0.0f // Upper vertex.
};

float A_VERTICES2[K_VERTEX_SIZE] = {
    0.5f,
    -0.5f,
    // Left vertex.
    -0.5f,
    0.5f,
    0.5f,
    // Right vertex.
    0.5f,
    0.0f,
    0.0f,
    // Upper vertex.
    -1.0f
};

float A_VERTICES_STATIC_OBJECT[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, 1.0f, 1.0f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f
};

float VERTICES[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, K_WIDTH_OFFSET, 0.75f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,           0.75f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,           1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET, 1.0f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.75f
};

float VERTICES2[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET * 2, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, K_WIDTH_OFFSET * 2, 0.75f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, K_WIDTH_OFFSET,     0.75f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, K_WIDTH_OFFSET,     1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET * 2, 1.0f,
    -0.5f, -0.5f, 0.0f, K_WIDTH_OFFSET,     0.75f
};

float VERTICES3[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    1.0f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.75f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.75f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    1.0f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    1.0f,
    -0.5f,
    -0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.75f
};

float VERTICES4[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET, 0.75f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, K_WIDTH_OFFSET, 0.5f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,           0.5f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,           0.75f, // Up left vertex.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET, 0.75f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f
};

float VERTICES5[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET * 2, 0.75f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, K_WIDTH_OFFSET * 2, 0.5f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, K_WIDTH_OFFSET,     0.5f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, K_WIDTH_OFFSET,     0.75f, // Up left vertex.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET * 2, 0.75f,
    -0.5f, -0.5f, 0.0f, K_WIDTH_OFFSET,     0.5f
};

float VERTICES6[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.75f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.5f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.5f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.75f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.75f,
    -0.5f,
    -0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.5f
};

float VERTICES7[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET, 0.5f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, K_WIDTH_OFFSET, 0.25f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,           0.25f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,           0.5f, // Up left vertex.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET, 0.5f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.25f
};

float VERTICES8[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET * 2, 0.5f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, K_WIDTH_OFFSET * 2, 0.25f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, K_WIDTH_OFFSET,     0.25f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, K_WIDTH_OFFSET,     0.5f, // Up left vertex.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET * 2, 0.5f,
    -0.5f, -0.5f, 0.0f, K_WIDTH_OFFSET,     0.25f
};

float VERTICES9[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.5f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.25f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.25f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.5f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.5f,
    -0.5f,
    -0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.25f
};

float VERTICES10[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET, 0.25f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, K_WIDTH_OFFSET, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,           0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,           0.25f, // Up left vertex.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET, 0.25f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f
};

float VERTICES11[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET * 2, 0.25f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, K_WIDTH_OFFSET * 2, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, K_WIDTH_OFFSET,     0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, K_WIDTH_OFFSET,     0.25f, // Up left vertex.
    0.5f,  0.5f,  0.0f, K_WIDTH_OFFSET * 2, 0.25f,
    -0.5f, -0.5f, 0.0f, K_WIDTH_OFFSET,     0.0f
};

float VERTICES12[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.25f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.0f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.0f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.25f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.25f,
    -0.5f,
    -0.5f,
    0.0f,
    K_WIDTH_OFFSET * 2,
    0.0f
};

int VERTICES_SIZE = sizeof(VERTICES);

namespace glvm {
static bool equals_c_str(const std::vector<char>& v, const char* s) {
    return strcmp(v.data(), s) == 0;
}

CWaveFrontObjParser::CWaveFrontObjParser() {
}

const std::vector<SVertex>& CWaveFrontObjParser::get_coordinate_vertices() const {
    return coordinate_vertices;
}

const std::vector<SVertex>& CWaveFrontObjParser::get_texture_vertices() const {
    return texture_vertices;
}

const std::vector<SVertex>& CWaveFrontObjParser::get_normals() const {
    return normals;
}

const std::vector<SFace>& CWaveFrontObjParser::get_faces() const {
    return faces;
}

void CWaveFrontObjParser::read_file(const char* file_path) {
    std::ifstream wavefront_obj_file_input_stream;
    std::stringstream wavefront_obj_file_output_stream;

    wavefront_obj_file_input_stream.open(file_path);
    if (wavefront_obj_file_input_stream.good()) {
        wavefront_obj_file_output_stream
            << wavefront_obj_file_input_stream.rdbuf();
        wavefront_obj_file_input_stream.close();
        s_wavefront_obj_file_data = wavefront_obj_file_output_stream.str();
    } else {
        return;
    }

    p_wavefront_obj_file_data = s_wavefront_obj_file_data.c_str();
}

void CWaveFrontObjParser::parse_file() {
    while (p_wavefront_obj_file_data[ui_counter] != '\0') {
        std::vector<std::vector<char>> line =
            split(p_wavefront_obj_file_data, ' ', '\n', ui_counter);
        if (equals_c_str(line[0], "v")) {
            SVertex vertex = parse_vertices(line);
            coordinate_vertices.push_back(vertex);
        }
        if (equals_c_str(line[0], "vt")) {
            SVertex vertex = parse_vertices(line);
            texture_vertices.push_back(vertex);
        }
        if (equals_c_str(line[0], "vn")) {
            SVertex vertex = parse_vertices(line);
            normals.push_back(vertex);
        }
        if (equals_c_str(line[0], "f")) {
            SFace face = parse_faces(line);
            faces.push_back(face);
        }
    }
}

std::vector<std::vector<char>> CWaveFrontObjParser::split(
    const char* data,
    const char separator,
    const char exit_symbol,
    unsigned int& position
) {
    std::vector<std::vector<char>> words_container;
    unsigned int outer_index = 0;
    words_container.push_back({});

    for (;; ++position) {
        if (data[position] == '#') {
            while (data[position] != '\n') {
                ++position;
            }
            continue;
        }
        if (data[position] == separator) {
            words_container[outer_index].push_back('\0');
            words_container.push_back({});
            ++outer_index;
            continue;
        }
        if (data[position] == exit_symbol) {
            ++position;
            words_container[outer_index].push_back('\0');
            return words_container;
        }
        words_container[outer_index].push_back(data[position]);
    }
}

SVertex CWaveFrontObjParser::parse_vertices(
    std::vector<std::vector<char>> words
) {
    SVertex vertex;
    unsigned int ui_vertex_index = 0;

    unsigned int ui_words_container_size = words.size();
    for (unsigned int i = 1; i < ui_words_container_size; ++i) {
        float float_number = parse_floating(words[i]);
        vertex[ui_vertex_index++] = float_number;
    }

    return vertex;
}

SFace CWaveFrontObjParser::parse_faces(std::vector<std::vector<char>> words) {
    SFace face;
    std::vector<std::vector<char>> words_inner_container;
    std::vector<char> word;

    unsigned int ui_words_container_size = words.size();

    for (unsigned int i = 1; i < ui_words_container_size; ++i) {
        unsigned int counter = 0;
        words_inner_container = split(words[i].data(), '/', '\0', counter);

        for (unsigned int j = 0; j < words_inner_container.size(); ++j) {
            word = words_inner_container[j];
            int i_value = parse_integer(word);

            face[j].push_back(i_value);
        }
    }
    return face;
}

int CWaveFrontObjParser::parse_integer(std::vector<char> digits) {
    std::vector<int> base_container;

    for (unsigned int i = 0; i < digits.size() - 1; ++i) {
        base_container.push_back(digits[i] - 48);
    }

    int i_result = 0;
    bool negate_flag = false;

    unsigned int base_container_size = base_container.size();
    for (unsigned int i = 0; i < base_container_size; ++i) {
        if (negate_flag && i == 0) {
            continue;
        } else if (base_container[i] == -5 && i == 0) {
            continue;
        }

        i_result +=
            base_container[i] * std::pow(10, (base_container_size - 1) - i);
    }

    return i_result;
}

float CWaveFrontObjParser::parse_floating(std::vector<char> digits) {
    std::vector<int> base_container;

    for (unsigned int i = 0; i < digits.size() - 1; ++i) {
        base_container.push_back(digits[i] - 48);
    }

    int integer_part = 0;
    float floating_part = 0;
    std::vector<int> integer_part_container;
    std::vector<int> floating_part_container;
    bool dot_flag = false;
    bool negate_flag = false;
    unsigned int base_container_size = base_container.size();

    if (base_container[0] == -3) {
        negate_flag = true;
    }

    for (unsigned int i = 0; i < base_container_size; ++i) {
        if (negate_flag && i == 0) {
            continue;
        } else if (base_container[i] == -5 && i == 0) {
            continue;
        } else if (base_container[i] == -2) {
            dot_flag = true;
            continue;
        }

        if (base_container[i] >= 0 && base_container[i] <= 9) {
            if (dot_flag) {
                floating_part_container.push_back(base_container[i]);
            } else {
                integer_part_container.push_back(base_container[i]);
            }
        } else {
            return NAN;
        }
    }

    unsigned int integer_part_container_size = integer_part_container.size();
    for (unsigned int i = 0; i < integer_part_container_size; ++i) {
        integer_part += integer_part_container[i]
            * std::pow(10, (integer_part_container_size - 1) - i);
    }

    unsigned int floating_part_container_size = floating_part_container.size();
    for (unsigned int i = 0; i < floating_part_container_size; ++i) {
        floating_part += floating_part_container[i] / std::pow(10, i + 1);
    }

    float result = 0;
    result = (float)(integer_part + floating_part);

    if (negate_flag) {
        result *= -1.0f;
    }

    return result;
}
} // namespace glvm

#ifdef __linux__

namespace glvm {

static WindowWaylandVulkan wayland_window;

void xdg_surface_configure(
    void* data,
    struct xdg_surface* xdg_surface,
    uint32_t serial
) {
    // The compositor sends a configure event before the surface is shown;
    // it must be acknowledged, otherwise the window never appears.
    WindowWaylandVulkan* config_data = (WindowWaylandVulkan*)data;

    xdg_surface_ack_configure(xdg_surface, serial);
    if (!config_data->pixels) {
        resize(data);
    }
}

void new_frame_callback(
    void* data,
    struct wl_callback* frame_call_back,
    uint32_t callback_data
) {
    wl_callback_destroy(frame_call_back);
    frame_call_back = wl_surface_frame(wayland_window.wl_surface);
    wl_callback_add_listener(
        frame_call_back,
        &wayland_window.callback_listener,
        data
    );
}

void shell_ping(void* data, struct xdg_wm_base* shell, uint32_t serial) {
    xdg_wm_base_pong(shell, serial);
}

void keyboard_keymap(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t format,
    int32_t keymap_file_descriptor,
    uint32_t size
) {
}

void keyboard_enter(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    struct wl_surface* surface,
    struct wl_array* keys
) {
    WindowWaylandVulkan* wayland_window_data = (WindowWaylandVulkan*)data;
    wayland_window_data->is_focused = true;
}

void keyboard_leave(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    struct wl_surface* surface
) {
    WindowWaylandVulkan* wayland_window_data = (WindowWaylandVulkan*)data;
    wayland_window_data->is_focused = false;
}

void push_event(EEvents event_type) {
    G_E_EVENT.set_event(event_type);
    INPUT_STACK.control_input(G_E_EVENT);
}

void keyboard_key(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    uint32_t time,
    uint32_t key,
    uint32_t state
) {
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        if (key == 1) {
            push_event(EEvents::EGameLoopKill);
        }
        if (key == 17) {
            push_event(EEvents::EMoveForward);
        }
        if (key == 31) {
            push_event(EEvents::EMoveBackward);
        }
        if (key == 30) {
            push_event(EEvents::EMoveLeft);
        }
        if (key == 32) {
            push_event(EEvents::EMoveRight);
        }
        if (key == 57) {
            push_event(EEvents::EJump);
        }
        if (key == 23) {
            push_event(EEvents::EInventory);
        }
    }

    if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
        if (key == 17) {
            push_event(EEvents::EKeyreleaseW);
        }
        if (key == 31) {
            push_event(EEvents::EKeyreleaseS);
        }
        if (key == 30) {
            push_event(EEvents::EKeyreleaseA);
        }
        if (key == 32) {
            push_event(EEvents::EKeyreleaseD);
        }
        if (key == 57) {
            push_event(EEvents::EKeyreleaseJump);
        }
        if (key == 23) {
            push_event(EEvents::EInventoryRelease);
        }
    }
}

void keyboard_modifiers(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    uint32_t mods_depressed,
    uint32_t mods_latched,
    uint32_t mods_locked,
    uint32_t group
) {
}

void keyboard_repeat_info(
    void* data,
    struct wl_keyboard* keyboard,
    int32_t rate,
    int32_t delay
) {
}

void pointer_enter(
    void* data,
    struct wl_pointer* pointer,
    uint32_t serial,
    struct wl_surface* surface,
    wl_fixed_t sx,
    wl_fixed_t sy
) {
}

void pointer_leave(
    void* data,
    struct wl_pointer* pointer,
    uint32_t serial,
    struct wl_surface* surface
) {
}

void pointer_motion(
    void* data,
    struct wl_pointer* pointer,
    uint32_t time,
    wl_fixed_t sx,
    wl_fixed_t sy
) {
}

void pointer_axis(
    void* data,
    struct wl_pointer* pointer,
    uint32_t time,
    uint32_t axis,
    wl_fixed_t value
) {
}

void pointer_button(
    void* data,
    struct wl_pointer* pointer,
    uint32_t serial,
    uint32_t time,
    uint32_t button,
    uint32_t state
) {
    if (state == WL_POINTER_BUTTON_STATE_PRESSED && button == 272) {
        push_event(EEvents::EMouseLeftButton);
    }
    if (state == WL_POINTER_BUTTON_STATE_RELEASED && button == 272) {
        push_event(EEvents::EMouseLeftButtonRelease);
        G_E_EVENT.is_left_mouse_button_released = true;
    }
    // Hide the cursor on first opportunity.
    if (!wayland_window.hideAndLockPointer) {
        wayland_window.hideAndLockPointer = true;
        struct wl_buffer* transparent =
            wayland_window.create_transparent_cursor(
                wayland_window.pointer_shared_memory
            );
        wl_surface_attach(wayland_window.pointer_surface, transparent, 0, 0);
        wl_surface_commit(wayland_window.pointer_surface);
        wl_pointer_set_cursor(
            pointer,
            serial,
            wayland_window.pointer_surface,
            0,
            0
        );

        if (!wayland_window.pointer_constraints) {
            return;
        }

        // Lock pointer to the main window surface, not pointer_surface.
        zwp_locked_pointer_v1* locked_pointer =
            zwp_pointer_constraints_v1_lock_pointer(
                wayland_window.pointer_constraints,
                wayland_window.wl_surface,
                pointer,
                NULL,
                ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT
            );

        // Get relative motion.
        wayland_window.relative_pointer =
            zwp_relative_pointer_manager_v1_get_relative_pointer(
                wayland_window.relative_pointer_manager,
                pointer
            );
        zwp_relative_pointer_v1_add_listener(
            wayland_window.relative_pointer,
            &wayland_window.relative_pointer_listener,
            NULL
        );
    }
}

void handle_relative_motion(
    void* data,
    struct zwp_relative_pointer_v1* rel_pointer,
    uint32_t utime_hi,
    uint32_t utime_lo,
    wl_fixed_t dx,
    wl_fixed_t dy,
    wl_fixed_t dx_unaccel,
    wl_fixed_t dy_unaccel
) {
    X_POINTER = wl_fixed_to_int(dx);
    Y_POINTER = wl_fixed_to_int(dy);
}

void seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities) {
    if ((capabilities & WL_SEAT_CAPABILITY_POINTER)
        && !wayland_window.pointer) {
        wayland_window.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(
            wayland_window.pointer,
            &wayland_window.pointer_listener,
            data
        );
    } else if (
        !(capabilities & WL_SEAT_CAPABILITY_POINTER) && wayland_window.pointer
    ) {
        wl_pointer_destroy(wayland_window.pointer);
        wayland_window.pointer = NULL;
    }

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD
        && !wayland_window.keyboard) {
        wayland_window.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(
            wayland_window.keyboard,
            &wayland_window.keyboard_listener,
            data
        );
    }
}

void seat_name(void* data, struct wl_seat* seat, const char* name) {
}

void output_geometry(
    void* data,
    struct wl_output* output,
    int32_t x,
    int32_t y,
    int32_t physical_width,
    int32_t physical_height,
    int32_t subpixel,
    const char* make,
    const char* model,
    int32_t transform
) {
}

void output_mode(
    void* data,
    struct wl_output* output,
    uint32_t flags,
    int32_t width,
    int32_t height,
    int32_t refresh
) {
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        WindowWaylandVulkan* wayland_window_data = (WindowWaylandVulkan*)data;
        wayland_window_data->width = width;
        wayland_window_data->height = height;
    }
}

void output_done(void* data, struct wl_output* output) {
}

void registry_global(
    void* data,
    struct wl_registry* registry,
    uint32_t name,
    const char* interface,
    uint32_t version
) {
    if (!strcmp(interface, wl_compositor_interface.name)) {
        wayland_window.compositor = (wl_compositor*)
            wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (!strcmp(interface, wl_shm_interface.name)) {
        wayland_window.shared_memory =
            (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
        wayland_window.pointer_shared_memory =
            (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (!strcmp(interface, zwp_pointer_constraints_v1_interface.name)) {
        wayland_window.pointer_constraints =
            (zwp_pointer_constraints_v1*)wl_registry_bind(
                registry,
                name,
                &zwp_pointer_constraints_v1_interface,
                1
            );
    } else if (!strcmp(
                   interface,
                   zwp_relative_pointer_manager_v1_interface.name
               )) {
        wayland_window.relative_pointer_manager =
            (zwp_relative_pointer_manager_v1*)wl_registry_bind(
                registry,
                name,
                &zwp_relative_pointer_manager_v1_interface,
                1
            );
    } else if (!strcmp(interface, xdg_wm_base_interface.name)) {
        wayland_window.xdg_shell = (xdg_wm_base*)
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(
            wayland_window.xdg_shell,
            &wayland_window.shell_listener,
            0
        );
    } else if (!strcmp(interface, wl_seat_interface.name)) {
        wayland_window.seat =
            (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(
            wayland_window.seat,
            &wayland_window.seat_lintener,
            data
        );
    } else if (!strcmp(interface, wl_output_interface.name)) {
        struct wl_output* output = (wl_output*)
            wl_registry_bind(registry, name, &wl_output_interface, 1);
        wl_output_add_listener(output, &wayland_window.output_listener, data);
    }
}

void registry_global_remove(
    void* data,
    struct wl_registry* registry,
    uint32_t name
) {
}

int32_t alocate_shared_memory(uint64_t size) {
    char name[8];
    name[0] = '/';
    name[7] = 0;
    for (int8_t i = 1; i < 6; ++i) {
        name[i] = (rand() & 23) + 97;
    }
    int32_t file_descriptor = shm_open(
        name,
        O_RDWR | O_CREAT | O_EXCL,
        S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH
    );
    shm_unlink(name);
    int result = ftruncate(file_descriptor, size);

    return file_descriptor;
}

void resize(void* data) {
    // Rendering goes through the Vulkan swapchain; the CPU shm buffer path is
    // not used. ponytail: drop pixels/buffer members if nothing revives it.
}

void xdg_toplevel_configure(
    void* data,
    struct xdg_toplevel* xdg_toplevel,
    int32_t new_width,
    int32_t new_height,
    struct wl_array* atate
) {
    if (!new_width && !new_height) {
        return;
    }

    WindowWaylandVulkan* toplevel_data = (WindowWaylandVulkan*)data;

    if (toplevel_data->width != new_width
        || toplevel_data->height != new_height) {
        toplevel_data->width = new_width;
        toplevel_data->height = new_height;
        resize(data);
    }
}

void xdg_toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel) {
    WindowWaylandVulkan* toplevel_data = (WindowWaylandVulkan*)data;

    toplevel_data->close_xdg_toplevel = 1;
}

WindowWaylandVulkan::WindowWaylandVulkan() {
}

void WindowWaylandVulkan::init() {
    display = wl_display_connect(0);
    registry = wl_display_get_registry(display);
    wl_registry_add_listener(
        registry,
        &registry_listener,
        (void*)(&wayland_window)
    );
    wl_display_roundtrip(display);
    if (!compositor || !xdg_shell) {
        fprintf(stderr, "Error: compositor or xdg_shell is NULL!\n");
        exit(1);
    }
    wl_surface = wl_compositor_create_surface(compositor);
    pointer_surface = wl_compositor_create_surface(compositor);
    frame_callback = wl_surface_frame(wl_surface);
    wl_callback_add_listener(
        frame_callback,
        &callback_listener,
        (void*)(&wayland_window)
    );
    xdg_surface = xdg_wm_base_get_xdg_surface(xdg_shell, wl_surface);
    xdg_surface_add_listener(
        xdg_surface,
        &xdg_surface_listener,
        (void*)(&wayland_window)
    );
    xdg_topLevel = xdg_surface_get_toplevel(xdg_surface);
    xdg_toplevel_add_listener(
        xdg_topLevel,
        &xdg_toplevel_listener,
        (void*)(&wayland_window)
    );
    xdg_toplevel_set_title(xdg_topLevel, "wayland glvm client");
    wl_surface_commit(wl_surface);
}

bool WindowWaylandVulkan::handle_event(CEvent& event) {
    event.mouse_pointer_position.position_x = X_POINTER;
    event.mouse_pointer_position.position_y = Y_POINTER;
    X_POINTER = 0;
    Y_POINTER = 0;
    wl_display_dispatch(display);
    return close_xdg_toplevel != 0;
}

// Create transparent cursor.
struct wl_buffer* WindowWaylandVulkan::create_transparent_cursor(
    struct wl_shm* shm
) {
    int size = 4 * 64 * 64; // 64x64 RGBA cursor (common size).
    int32_t file_descriptor = alocate_shared_memory(size);
    void* data =
        mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);

    // Fill with transparent pixels.
    for (int i = 0; i < 64 * 64; ++i) {
        ((int*)data)[i] = 0x00000000;
    }

    struct wl_shm_pool* pool = wl_shm_create_pool(shm, file_descriptor, size);
    struct wl_buffer* cursor_buffer = wl_shm_pool_create_buffer(
        pool,
        0,
        64,
        64,
        64 * 4,
        WL_SHM_FORMAT_ARGB8888
    );

    munmap(data, size);
    ::close(file_descriptor);
    wl_shm_pool_destroy(pool);

    return cursor_buffer;
}

void WindowWaylandVulkan::swap_buffers() {
}

void WindowWaylandVulkan::clear_display() {
}

void WindowWaylandVulkan::cursor_lock(
    int pointer_x,
    int pointer_y,
    int* out_offset_x,
    int* out_offset_y
) {
    static int flag = 0;
    if (flag == 0) {
        *out_offset_x = -((int)width / 2);
        *out_offset_y = -((int)height / 2);
        ++flag;
    } else {
        *out_offset_x = pointer_x;
        *out_offset_y = pointer_y;
    }
};

void WindowWaylandVulkan::close() {
    if (keyboard) {
        wl_keyboard_destroy(keyboard);
    }
    // Release the Wayland seat object responsible for managing input devices.
    wl_seat_release(seat);
    if (buffer) {
        wl_buffer_destroy(buffer);
    }
    xdg_toplevel_destroy(xdg_topLevel);
    xdg_surface_destroy(xdg_surface);
    wl_surface_destroy(wl_surface);
    wl_display_disconnect(display);
}

WindowWaylandVulkan* initialize_wayland_window() {
    wayland_window.xdg_toplevel_listener = {
        .configure = xdg_toplevel_configure,
        .close = xdg_toplevel_close,
        .configure_bounds = nullptr,
        .wm_capabilities = nullptr
    };
    wayland_window.xdg_surface_listener = {.configure = xdg_surface_configure};
    wayland_window.callback_listener = {
        // Notify the client when the related request is done.
        .done = new_frame_callback
    };
    wayland_window.shell_listener = {.ping = shell_ping};
    wayland_window.output_listener =
        {.geometry = output_geometry, .mode = output_mode, .done = output_done};
    wayland_window.keyboard_listener = {
        .keymap = keyboard_keymap,
        .enter = keyboard_enter,
        .leave = keyboard_leave,
        .key = keyboard_key,
        .modifiers = keyboard_modifiers,
        .repeat_info = keyboard_repeat_info
    };

    wayland_window.relative_pointer_listener = {
        .relative_motion = handle_relative_motion
    };
    wayland_window.pointer_listener = {
        .enter = pointer_enter,
        .leave = pointer_leave,
        .motion = pointer_motion,
        .button = pointer_button,
        .axis = pointer_axis,
        .frame = nullptr,
        .axis_source = nullptr,
        .axis_stop = nullptr,
        .axis_discrete = nullptr,
        .axis_value120 = nullptr,
        .axis_relative_direction = nullptr
    };
    wayland_window.seat_lintener = {
        .capabilities = seat_capabilities,
        .name = seat_name
    };
    wayland_window.registry_listener = {
        .global = registry_global,
        .global_remove = registry_global_remove
    };

    wayland_window.init();
    return &wayland_window;
}
} // namespace glvm
#endif // __linux__

#ifdef __linux__

#include <X11/Xlib.h>

namespace glvm {
WindowXVulkan::WindowXVulkan() {
    display = XOpenDisplay(nullptr);
    root_window = DefaultRootWindow(display);
    set_window_attributes.event_mask = KeyPressMask | KeyReleaseMask
        | PointerMotionMask | StructureNotifyMask | ButtonPressMask
        | ButtonReleaseMask | FocusChangeMask;

    const int screen_number = XDefaultScreen(display);
    width = DisplayWidth(display, screen_number);
    height = DisplayHeight(display, screen_number);
    // Show the window.
    win = XCreateWindow(
        display,
        root_window,
        0,
        0,
        width,
        height,
        0,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWEventMask,
        &set_window_attributes
    );

    XMapWindow(display, win);

    XWarpPointer(display, None, win, 0, 0, 0, 0, 0, 0);

    Cursor invisible_cursor;
    Pixmap bitmap_no_data;
    XColor black;
    static char no_data[] = {0, 0, 0, 0, 0, 0, 0, 0};
    black.red = black.green = black.blue = 0;

    bitmap_no_data = XCreateBitmapFromData(display, win, no_data, 8, 8);
    invisible_cursor = XCreatePixmapCursor(
        display,
        bitmap_no_data,
        bitmap_no_data,
        &black,
        &black,
        0,
        0
    );
    XDefineCursor(display, win, invisible_cursor);

    XFreeCursor(display, invisible_cursor);
    XFreePixmap(display, bitmap_no_data);

    XGetWindowAttributes(display, win, &x_window_attributes);
}

WindowXVulkan::~WindowXVulkan() = default;

Window WindowXVulkan::get_window() {
    return win;
}

Display* WindowXVulkan::get_display() {
    return display;
}

void WindowXVulkan::cursor_lock(
    int pointer_x,
    int pointer_y,
    int* out_offset_x,
    int* out_offset_y
) {
    *out_offset_x += pointer_x - (int)(width / 2);
    *out_offset_y -= pointer_y - (int)(height / 2);
    // Pitch is limited by angle in Engine::SetViewMatrix(), so this offset may
    // accumulate freely; no pixel clamp here (resolution-independent).
    XWarpPointer(
        display,
        None,
        win,
        0,
        0,
        0,
        0,
        (int)(width / 2),
        (int)(height / 2)
    );
    XFlush(display);
}

void WindowXVulkan::swap_buffers() {
}

void WindowXVulkan::clear_display() {
}

bool WindowXVulkan::handle_event(CEvent& event) {
    XEvent x_event;

    while (XPending(display)) {
        XNextEvent(display, &x_event);
        KeySym key;
        unsigned int mouse_button;
        XMotionEvent motion;

        switch (x_event.type) {
            case MotionNotify:
                motion = x_event.xmotion;

                event.set_event(EEvents::EMousePointerPosition);
                event.mouse_pointer_position.position_x = motion.x;
                event.mouse_pointer_position.position_y = motion.y;
                [[fallthrough]];
            case MapNotify:
                XGrabPointer(
                    display,
                    win,
                    True,
                    PointerMotionMask,
                    GrabModeAsync,
                    GrabModeAsync,
                    win,
                    None,
                    CurrentTime
                );
                break;
            case FocusIn:
                is_focused = true;
                XGrabPointer(
                    display,
                    win,
                    True,
                    PointerMotionMask,
                    GrabModeAsync,
                    GrabModeAsync,
                    win,
                    None,
                    CurrentTime
                );
                break;
            case FocusOut:
                is_focused = false;
                XUngrabPointer(display, CurrentTime);
                break;
            case ButtonPress:
                mouse_button = x_event.xbutton.button;
                switch (mouse_button) {
                    case 1:
                        event.set_event(EEvents::EMouseLeftButton);
                        break;
                }
                break;

            case ButtonRelease:
                mouse_button = x_event.xbutton.button;
                switch (mouse_button) {
                    case 1:
                        event.set_event(EEvents::EMouseLeftButtonRelease);
                        event.is_left_mouse_button_released = true;
                        break;
                }
                break;

            case KeyPress:
                key = XLookupKeysym(&x_event.xkey, 0);
                switch (key) {
                    case XKEY_I:
                        event.set_event(EEvents::EInventory);
                        break;
                    case XKEY_ESCAPE:
                        event.set_event(EEvents::EGameLoopKill);
                        break;
                    case XKEY_A:
                        event.set_event(EEvents::EMoveLeft);
                        break;
                    case XKEY_D:
                        event.set_event(EEvents::EMoveRight);
                        break;
                    case XKEY_S:
                        event.set_event(EEvents::EMoveBackward);
                        break;
                    case XKEY_W:
                        event.set_event(EEvents::EMoveForward);
                        break;
                    case XKEY_SPACE:
                        event.set_event(EEvents::EJump);
                        break;
                }
                break;

            case KeyRelease:
                if (XEventsQueued(display, QueuedAfterReading)) {
                    XEvent x_next_event;
                    XPeekEvent(display, &x_next_event);

                    if (x_next_event.type == KeyPress
                        && x_next_event.xkey.time == x_event.xkey.time
                        && x_next_event.xkey.keycode == x_event.xkey.keycode) {
                        // Key wasn't actually released.
                        XNextEvent(display, &x_next_event);
                        continue;
                    }
                }
                key = XLookupKeysym(&x_event.xkey, 0);
                switch (key) {
                    case XKEY_I:
                        event.set_event(EEvents::EInventoryRelease);
                        break;
                    case XKEY_A:
                        event.set_event(EEvents::EKeyreleaseA);
                        break;
                    case XKEY_D:
                        event.set_event(EEvents::EKeyreleaseD);
                        break;
                    case XKEY_S:
                        event.set_event(EEvents::EKeyreleaseS);
                        break;
                    case XKEY_W:
                        event.set_event(EEvents::EKeyreleaseW);
                        break;
                    case XKEY_SPACE:
                        event.set_event(EEvents::EKeyreleaseJump);
                        break;
                }
                break;
        }

        INPUT_STACK.control_input(event);
    }
    return false;
}

void WindowXVulkan::close() {
    XDestroyWindow(display, win);
    XCloseDisplay(display);
}
} // namespace glvm

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xfixes.h>

namespace glvm {
WindowXCBVulkan::WindowXCBVulkan() {
    // Open the connection to the X server.
    connection = xcb_connect(nullptr, nullptr);
    int error = xcb_connection_has_error(connection);
    if (error) {
        fprintf(stderr, "XCB connection error: %d\n", error);
        // Handle error or exit.
    }

    // Get the first screen.
    const xcb_setup_t* setup = xcb_get_setup(connection);
    assert(connection != nullptr);

    xcb_screen_iterator_t iterator = xcb_setup_roots_iterator(setup);
    screen = iterator.data;

    width = screen->width_in_pixels;
    height = screen->height_in_pixels;

    key_symbols = xcb_key_symbols_alloc(connection);
    assert(key_symbols != nullptr);

    uint32_t event_mask = 0;
    event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t event_flags[2];
    event_flags[0] = screen->black_pixel;
    event_flags[1] = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION
        | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
        | XCB_EVENT_MASK_FOCUS_CHANGE;
    window = xcb_generate_id(connection);
    xcb_create_window(
        connection,
        XCB_COPY_FROM_PARENT,
        window,
        screen->root,
        0,
        0,
        width,
        height,
        10,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        event_mask,
        event_flags
    );

    // Verify the window was created (optional).
    xcb_get_window_attributes_cookie_t attr_cookie =
        xcb_get_window_attributes(connection, window);
    xcb_get_window_attributes_reply_t* attr_reply =
        xcb_get_window_attributes_reply(connection, attr_cookie, nullptr);

    if (!attr_reply) {
        fprintf(stderr, "Failed to query window - maybe it wasn't created.\n");
    } else {
        free(attr_reply);
    }
    // Map the window on the screen.
    xcb_map_window(connection, window);
    // Make sure commands are sent before we pause so that the window gets shown.
    xcb_flush(connection);
    hide_cursor();
}

void WindowXCBVulkan::configure_window() {
    uint16_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
        | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    const uint32_t values[] = {
        320, // x.
        180, // y.
        width,
        height
    };

    xcb_configure_window(connection, window, mask, values);
    xcb_flush(connection);
}

void WindowXCBVulkan::hide_cursor() {
    xcb_pixmap_t foreground_pixmap_id = xcb_generate_id(connection);
    xcb_create_pixmap(connection, 1, foreground_pixmap_id, window, 8, 8);

    // Create graphical context.
    xcb_gcontext_t graphical_context = xcb_generate_id(connection);

    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    uint32_t values_list[2];
    values_list[0] = screen->black_pixel;
    values_list[1] = screen->white_pixel;

    xcb_create_gc(
        connection,
        graphical_context,
        window,
        XCB_GC_FOREGROUND | XCB_GC_BACKGROUND,
        values_list
    );

    const uint8_t pix_map_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00};

    xcb_put_image(
        connection,
        XCB_IMAGE_FORMAT_XY_PIXMAP,
        foreground_pixmap_id,
        graphical_context,
        0,
        0,
        100,
        100,
        0,
        8,
        32,
        pix_map_data
    );

    xcb_cursor_t cursor = xcb_generate_id(connection);
    xcb_create_cursor(
        connection,
        cursor,
        foreground_pixmap_id,
        foreground_pixmap_id,
        0,
        0,
        0,
        0,
        0,
        0,
        8,
        8
    );

    mask = XCB_CW_CURSOR;
    uint32_t value_list = cursor;
    xcb_change_window_attributes(connection, window, mask, &value_list);

    xcb_free_cursor(connection, cursor);
}

xcb_connection_t* WindowXCBVulkan::get_connection() {
    return connection;
}

uint32_t WindowXCBVulkan::get_window() {
    return window;
}

void WindowXCBVulkan::disconnect() {
    xcb_disconnect(connection);
}

void WindowXCBVulkan::swap_buffers() {
}

void WindowXCBVulkan::clear_display() {
}

bool WindowXCBVulkan::handle_event([[maybe_unused]] CEvent& event) {
    xcb_generic_event_t* generic_event;
    bool next_generic_event_flag = false;
    while (next_generic_event_flag
           || (generic_event = xcb_poll_for_event(connection))) {
        next_generic_event_flag = false;
        switch (generic_event->response_type & ~0x80) {
            case XCB_EXPOSE: {
                [[maybe_unused]] xcb_expose_event_t* expose_event =
                    (xcb_expose_event_t*)generic_event;

                if (!is_window_resize_read) {
                    width = expose_event->width;
                    height = expose_event->height;
                    is_window_resize_read = true;
                }
                break;
            }
            case XCB_BUTTON_PRESS: {
                xcb_button_press_event_t* expose_event =
                    (xcb_button_press_event_t*)generic_event;
                switch (expose_event->detail) {
                    case 1:
                        event.set_event(EEvents::EMouseLeftButton);
                        break;
                    case 3:
                        event.set_event(EEvents::EMouseRightButton);
                        break;
                    case 4:
                        break;
                    case 5:
                        break;
                }

                break;
            }
            case XCB_BUTTON_RELEASE: {
                xcb_button_release_event_t* expose_event =
                    (xcb_button_release_event_t*)generic_event;
                switch (expose_event->detail) {
                    case 1:
                        event.set_event(EEvents::EMouseLeftButtonRelease);
                        event.is_left_mouse_button_released = true;
                        break;
                    case 3:
                        event.set_event(EEvents::EMouseRightButtonRelease);
                        break;
                }

                break;
            }
            case XCB_MOTION_NOTIFY: {
                xcb_motion_notify_event_t* expose_event =
                    (xcb_motion_notify_event_t*)generic_event;

                event.set_event(EEvents::EMousePointerPosition);
                event.mouse_pointer_position.position_x = expose_event->event_x;
                event.mouse_pointer_position.position_y = expose_event->event_y;
                [[fallthrough]];
            }
            case XCB_MAP_WINDOW: {
                // Make sure commands are sent before we pause so that the
                // window gets shown.
                xcb_flush(connection);

                xcb_grab_pointer_cookie_t cookie = xcb_grab_pointer(
                    connection,
                    1,
                    window,
                    XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_ASYNC,
                    XCB_GRAB_MODE_ASYNC,
                    window,
                    XCB_NONE,
                    XCB_CURRENT_TIME
                );
                xcb_grab_pointer_reply_t* grab_pointer_reply =
                    xcb_grab_pointer_reply(connection, cookie, nullptr);
                free(grab_pointer_reply);
                break;
            }
            case XCB_ENTER_NOTIFY: {
                [[maybe_unused]] xcb_enter_notify_event_t* expose_event =
                    (xcb_enter_notify_event_t*)generic_event;
                break;
            }
            case XCB_FOCUS_IN:
                is_focused = true;
                xcb_grab_pointer(
                    connection,
                    1,
                    window,
                    XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_ASYNC,
                    XCB_GRAB_MODE_ASYNC,
                    window,
                    XCB_NONE,
                    XCB_CURRENT_TIME
                );
                break;
            case XCB_FOCUS_OUT:
                is_focused = false;
                xcb_ungrab_pointer(connection, XCB_CURRENT_TIME);
                xcb_flush(connection);
                break;
            case XCB_KEY_PRESS: {
                xcb_key_press_event_t* expose_event =
                    (xcb_key_press_event_t*)generic_event;
                xcb_keysym_t keysym =
                    xcb_key_press_lookup_keysym(key_symbols, expose_event, 0);
                switch (keysym) {
                    case 65307:
                        event.set_event(EEvents::EGameLoopKill);
                        break;
                    case 105:
                        event.set_event(EEvents::EInventory);
                        break;
                    case 97:
                        event.set_event(EEvents::EMoveLeft);
                        break;
                    case 100:
                        event.set_event(EEvents::EMoveRight);
                        break;
                    case 115:
                        event.set_event(EEvents::EMoveBackward);
                        break;
                    case 119:
                        event.set_event(EEvents::EMoveForward);
                        break;
                    case 32:
                        event.set_event(EEvents::EJump);
                        break;
                }

                break;
            }
            case XCB_KEY_RELEASE: {
                xcb_key_release_event_t* key_release_event =
                    (xcb_key_release_event_t*)generic_event;
                next_generic_event = xcb_poll_for_event(connection);
                if (next_generic_event != nullptr) {
                    xcb_key_press_event_t* key_press_event =
                        (xcb_key_press_event_t*)next_generic_event;
                    xcb_keysym_t press_keysym = xcb_key_press_lookup_keysym(
                        key_symbols,
                        key_press_event,
                        0
                    );
                    xcb_keysym_t release_keysym = xcb_key_press_lookup_keysym(
                        key_symbols,
                        key_release_event,
                        0
                    );

                    if (next_generic_event->response_type == XCB_KEY_PRESS
                        && key_press_event->time == key_release_event->time
                        && press_keysym == release_keysym) {
                        free(generic_event);
                        generic_event = nullptr;
                        free(next_generic_event);
                        next_generic_event = nullptr;
                        continue;
                    } else {
                        next_generic_event_flag = true;
                    }
                }

                xcb_keysym_t release_keysym = xcb_key_press_lookup_keysym(
                    key_symbols,
                    key_release_event,
                    0
                );
                switch (release_keysym) {
                    case 105:
                        event.set_event(EEvents::EInventoryRelease);
                        break;
                    case 97:
                        event.set_event(EEvents::EKeyreleaseA);
                        break;
                    case 100:
                        event.set_event(EEvents::EKeyreleaseD);
                        break;
                    case 115:
                        event.set_event(EEvents::EKeyreleaseS);
                        break;
                    case 119:
                        event.set_event(EEvents::EKeyreleaseW);
                        break;
                    case 32:
                        event.set_event(EEvents::EKeyreleaseJump);
                        break;
                }

                break;
            }
        }
        if (next_generic_event != nullptr) {
            *generic_event = *next_generic_event;
            free(next_generic_event);
            next_generic_event = nullptr;
        } else {
            free(generic_event);
        }
        INPUT_STACK.control_input(event);
    }
    is_window_resize_read = false;
    return false;
}

void WindowXCBVulkan::close() {
    xcb_key_symbols_free(key_symbols);
    xcb_disconnect(connection);
}

void WindowXCBVulkan::cursor_lock(
    int pointer_x,
    int pointer_y,
    int* out_offset_x,
    int* out_offset_y
) {
    *out_offset_x += pointer_x - (int)(width / 2);
    *out_offset_y -= pointer_y - (int)(height / 2);
    xcb_warp_pointer(
        connection,
        XCB_NONE,
        window,
        0,
        0,
        0,
        0,
        (int)(width / 2),
        (int)(height / 2)
    );
    xcb_flush(connection);
}
} // namespace glvm
#endif // __linux__
