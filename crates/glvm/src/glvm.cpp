#include "glvm/glvm.hpp"

#include "glvm_log/prelude.hpp"
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
#include <math.h>
#include <mutex>
#include <ostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
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
#include <X11/Xlib.h>
#include <poll.h>
#include <strings.h>
#include <vulkan/vulkan_xlib.h>
#include <wayland-client-core.h>
#endif // __linux__

using namespace rusty::prelude;
using namespace glvm_log::prelude;

#ifdef _WIN32
constexpr auto VK_W = 0x57;
constexpr auto VK_S = 0x53;
constexpr auto VK_A = 0x41;
constexpr auto VK_D = 0x44;
constexpr auto VK_I = 0x49;
#endif

namespace glvm {
auto matches_required_mask(const u64 archetype_mask, const u64& system_mask)
    -> bool {
    return (archetype_mask & system_mask) == system_mask;
}

auto make_entity(u32 id, u32 generation) -> u64 {
    return (as<u64>(generation) << ENTITY_ID_BITS) | id;
}

auto get_id(u64 entity) -> u32 {
    return entity & ENTITY_BITS_MASK;
}

auto get_gen(u64 entity) -> u32 {
    return entity >> ENTITY_ID_BITS;
}
}; // namespace glvm

namespace glvm {
World world = {};

World::World() {
    assert(
        spatial_grid.width > 0 && spatial_grid.height > 0
        && spatial_grid.depth > 0
    );

    const auto chunk_size = spatial_grid.grid[0][0][0].SIZE;
    const auto half_world_width = spatial_grid.width * chunk_size * 0.5f;
    const auto half_world_height = spatial_grid.height * chunk_size * 0.5f;
    const auto half_world_depth = spatial_grid.depth * chunk_size * 0.5f;
    const auto half_chunk_size = chunk_size * 0.5f;
    const Vector<f32, 3> pivot = Vector<f32, 3>(
        -half_world_width + half_chunk_size,
        -half_world_height + half_chunk_size,
        -half_world_depth + half_chunk_size
    );
    for (u32 i0 = 0; i0 < SpatialGrid::depth; ++i0) {
        for (u32 i1 = 0; i1 < SpatialGrid::height; ++i1) {
            for (u32 i2 = 0; i2 < SpatialGrid::width; ++i2) {
                spatial_grid.grid[i0][i1][i2].position = Vector<f32, 3>(
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
    for (auto& archetype : archetypes) {
        delete archetype;
        archetype = nullptr;
    }
}

auto World::add_entity_to_archetype(u64 entity, Archetype* arch) -> void {
    const u32 id = get_id(entity);

    if (id >= entity_locations.size()) {
        entity_locations.resize(id + 1);
    }
    EntityLocation& location = entity_locations[id];
    if (location.arch != nullptr) {
        assert(false && "Entity already assigned to archetype");
    }
    const u32 index = arch->add_entity(entity);
    location.arch = arch;
    location.index = index;
}

auto World::remove_entity(u64 entity) -> void {
    const u32 id = get_id(entity);
    EntityLocation& location = entity_locations[id];
    if (location.arch == nullptr) {
        return;
    }
    // Remove entity from spatial grid cells it occupies, otherwise stale
    // references crash collision/physics on later frames.
    if (location.grid_cell_counter > 0) {
        for (u8 i = 0; i < location.grid_cell_counter; ++i) {
            const u32 z = location.grid_cell_indices[i][0];
            const u32 y = location.grid_cell_indices[i][1];
            const u32 x = location.grid_cell_indices[i][2];
            Vec<u32>& chunk_entities = spatial_grid.grid[z][y][x].entities;
            for (u32 k = 0; k < chunk_entities.size(); ++k) {
                if (chunk_entities[k] == entity) {
                    chunk_entities.erase(chunk_entities.begin() + k);
                    break;
                }
            }
        }
        location.grid_cell_counter = 0;
    }
    Archetype* arch = location.arch;
    const u32 index = location.index;
    const u64 moved = arch->remove_entity(index);
    if (moved != entity) {
        const u32 moved_id = get_id(moved);
        entity_locations[moved_id].index = index;
        entity_locations[moved_id].arch = arch;
    }
    location.arch = nullptr;
}

auto World::search_cache_archetypes(
    u64 required_mask,
    Archetype** cached_archetypes,
    u32& cached_archetypes_number
) -> void {
    for (auto* const arch : world.archetypes) {
        if ((arch->mask & required_mask) == required_mask) {
            cached_archetypes[cached_archetypes_number] = arch;
            ++cached_archetypes_number;
        }
    }
}
}; // namespace glvm

namespace glvm {
Array<ComponentTypeInfo, MAX_COMPONENTS> COMPONENT_TYPE_INFOS = {};

auto register_component_move(u32 component_id, ComponentTypeInfo info) -> void {
    if (component_id < MAX_COMPONENTS) {
        COMPONENT_TYPE_INFOS[component_id] = info;
    }
}
} // namespace glvm

namespace glvm {
ArchetypeEntityManager* ArchetypeEntityManager::instance = nullptr;
Mutex ArchetypeEntityManager::mutex;

ArchetypeEntityManager::ArchetypeEntityManager() = default;

ArchetypeEntityManager::~ArchetypeEntityManager() = default;

auto ArchetypeEntityManager::get_instance() -> ArchetypeEntityManager* {
    const MutexGuard<Mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new ArchetypeEntityManager();
    }
    return instance;
}

[[nodiscard]] auto ArchetypeEntityManager::create_entity() -> u64 {
    u32 new_id = 0;
    // Check out whether or not free ID in removed entities registry.
    if (!free_list.empty()) {
        new_id = free_list.back();
        free_list.pop_back();
    } else {
        new_id = next_id++;
        generations.push_back(1);
    }

    return make_entity(new_id, generations[new_id]);
}

auto ArchetypeEntityManager::remove_entity(u64 entity) -> void {
    const u32 id = get_id(entity);
    if (!is_alive(entity)) {
        return;
    }
    generations[id]++;
    free_list.push_back(id);
}

auto ArchetypeEntityManager::is_alive(u64 entity) const -> bool {
    const u32 id = get_id(entity);
    return id < generations.size() && generations[id] == get_gen(entity);
}
}; // namespace glvm

namespace glvm {
auto Archetype::add_entity(u64 entity) -> u32 {
    const u32 index = entity_count++;
    assert(index < CAPACITY);
    entities[index] = entity;
    return index;
}

// Swap-remove.
auto Archetype::remove_entity(u32 index) -> u64 {
    const u32 last = entity_count - 1;
    for (u32 i = 0; i < component_count; ++i) {
        const auto component_id = component_ids[i];
        const ComponentTypeInfo& info = COMPONENT_TYPE_INFOS[component_id];
        if (info.bytes != 0 && info.move_assign != nullptr) {
            auto* base = static_cast<char*>(components[component_id]);
            info.move_assign(
                base + index * info.bytes,
                base + last * info.bytes
            );
        }
    }
    const u64 moved = entities[last];
    entities[index] = moved;
    --entity_count;
    return moved;
}
}; // namespace glvm

namespace glvm {
auto box_collider(
    const Vector<f32, 3> backtracking_pos,
    const Vector<f32, 3> compared_pos,
    const f32 backtracking_scale,
    const f32 compared_scale,
    const MeshAxisMaxAbsoluteValues& backtracking_mesh_axis_max_absolute_values,
    const MeshAxisMaxAbsoluteValues& compared_mesh_axis_max_absolute_values
) -> bool {
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

auto compute_box_corner_bound_points(
    const MeshAxisMaxAbsoluteValues entity_chunk_bounds,
    Vector<f32, 3> entity_position,
    const f32 scale
) -> Vec<Vector<f32, 3>> {
    const auto half_width = entity_chunk_bounds.absolute_x * scale;
    const auto half_height = entity_chunk_bounds.absolute_y * scale;
    const auto half_depth = entity_chunk_bounds.absolute_z * scale;
    const Vector<f32, 3> center_offset = {
        entity_chunk_bounds.origin_offset_x * scale,
        entity_chunk_bounds.origin_offset_y * scale,
        entity_chunk_bounds.origin_offset_z * scale
    };
    Vec<Vector<f32, 3>> result;
    // Left bottom back.
    result.push_back(
        entity_position + center_offset
        + Vector<f32, 3>(-half_width, -half_height, -half_depth)
    );
    // Right upper front.
    result.push_back(
        entity_position + center_offset
        + Vector<f32, 3>(half_width, half_height, half_depth)
    );
    return result;
}

auto set_mesh_bounds(MeshAxisLimitingValues mesh_axis_limiting_values) -> void {
    all_mesh_max_absolute_values.push_back({});

    all_mesh_max_absolute_values[all_mesh_max_absolute_values.size() - 1]
        .absolute_x = (mesh_axis_limiting_values.highest_x
                       - mesh_axis_limiting_values.lowest_x)
        / 2.0f;
    all_mesh_max_absolute_values[all_mesh_max_absolute_values.size() - 1]
        .absolute_y = (mesh_axis_limiting_values.highest_y
                       - mesh_axis_limiting_values.lowest_y)
        / 2.0f;
    all_mesh_max_absolute_values[all_mesh_max_absolute_values.size() - 1]
        .absolute_z = (mesh_axis_limiting_values.highest_z
                       - mesh_axis_limiting_values.lowest_z)
        / 2.0f;

    all_mesh_max_absolute_values[all_mesh_max_absolute_values.size() - 1]
        .origin_offset_x = (mesh_axis_limiting_values.highest_x
                            + mesh_axis_limiting_values.lowest_x)
        / 2.0f;
    all_mesh_max_absolute_values[all_mesh_max_absolute_values.size() - 1]
        .origin_offset_y = (mesh_axis_limiting_values.highest_y
                            + mesh_axis_limiting_values.lowest_y)
        / 2.0f;
    all_mesh_max_absolute_values[all_mesh_max_absolute_values.size() - 1]
        .origin_offset_z = (mesh_axis_limiting_values.highest_z
                            + mesh_axis_limiting_values.lowest_z)
        / 2.0f;
}

} // namespace glvm

namespace glvm {
ComponentManager* ComponentManager::instance = nullptr;
Mutex ComponentManager::mutex;

ComponentManager::ComponentManager() = default;

ComponentManager::~ComponentManager() {
    for (auto& world_sparse_entities_map_to_component :
         world_sparse_entities_map_to_components) {
        delete world_sparse_entities_map_to_component;
        world_sparse_entities_map_to_component = nullptr;
    }
    for (auto& world_dense_components_map_to_entity :
         world_dense_components_map_to_entities) {
        delete world_dense_components_map_to_entity;
        world_dense_components_map_to_entity = nullptr;
    }
}

auto ComponentManager::check_availability(
    Vec<u32>& sparse,
    Vec<u32>& dense,
    u32 entity
) -> bool {
    return entity < sparse.size() && sparse[entity] < dense.size()
        && dense[sparse[entity]] == entity;
}

auto ComponentManager::get_container_id() -> u32 {
    return components_container_id;
}

auto ComponentManager::get_instance() -> ComponentManager* {
    const MutexGuard<Mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new ComponentManager();
    }
    return instance;
}
} // namespace glvm

glvm::EventStack global_input_stack {};

i32 GLOBAL_POINTER_X;
i32 GLOBAL_POINTER_Y;

glvm::Event global_event;
// Contains all maximum absolute axis values.
Vec<glvm::MeshAxisMaxAbsoluteValues> all_mesh_max_absolute_values;

namespace glvm {
Engine* Engine::instance = nullptr;
Mutex Engine::mutex;

auto playback_sound(SoundEngine* sound_engine, AtomicBool& running_sound)
    -> void {
    running_sound = true;
    while (running_sound) {
        sound_engine->sound_stream();
    }
}

auto Engine::register_engine_components() -> void {
    // Base components owned by the engine (Transform/Camera/Mesh/Material,
    // lights, text, animation, physics primitives). Games register their own
    // via glvm::register_component.
    register_component<Transform>(ComponentsIndices::TransformComponent);
    register_component<RigidBody>(ComponentsIndices::RigidBodyComponent);
    register_component<Mesh>(ComponentsIndices::MeshComponent);
    register_component<Font>(ComponentsIndices::FontComponent);
    register_component<Collider>(ComponentsIndices::ColliderComponent);
    register_component<ColliderFlags>(ComponentsIndices::ColliderFlagsComponent);
    register_component<Material>(ComponentsIndices::MaterialComponent);
    register_component<Beholder>(ComponentsIndices::ViewComponent);
    register_component<Animation>(ComponentsIndices::AnimationComponent);
    register_component<DirectionalLightComponent>(
        ComponentsIndices::DirectionalLightComponent
    );
    register_component<SpotLightComponent>(
        ComponentsIndices::SpotLightComponent
    );
    register_component<PointLightComponent>(
        ComponentsIndices::PointLightComponent
    );
    register_component<MeshGeneration>(
        ComponentsIndices::MeshGenerationComponent
    );
}

Engine::Engine() :
    delta_frame_time(0.0f),
    spatial_grid_system(new SpatialGridSystem()) {
    glvm_log::info("glvm", "Engine::Engine start");
    register_engine_components();
    chrono = TimerCreator().create();
    glvm_log::info("glvm", "chrono created");
    sound_engine = SoundEngineFactory().create_sound_engine();
    glvm_log::info("glvm", "sound engine created");
    global_event.set_event(Default);
    // Created here, activated by the game: games choose the activation order
    // via add_system/add_base_systems (order matters, e.g. level generation
    // must run before spatial indexing on the first frame).
    sound_thread = std::thread(
        playback_sound,
        std::ref(sound_engine),
        std::ref(running_sound)
    );
    sound_engine->open_device("default");
    glvm_log::info("glvm", "Engine::Engine end");
}

Engine::~Engine() = default;

auto Engine::get_instance() -> Engine* {
    const MutexGuard<Mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new Engine();
    }
    return instance;
}

auto Engine::add_system(System* system) -> void {
    SystemManager::get_instance()->activate_system(system);
}

auto Engine::add_base_systems() -> void {
    // Generic simulation owned by the engine: spatial indexing. Collision,
    // dynamics and combat systems are game-provided (see examples/tps.cpp).
    // Games call this at the right point of their own schedule.
    SystemManager* system_manager = SystemManager::get_instance();
    system_manager->activate_system(spatial_grid_system);
}

auto Engine::set_pre_update_hook(std::function<void()> hook) -> void {
    pre_update_hook = std::move(hook);
}

auto Engine::set_post_update_hook(std::function<void()> hook) -> void {
    post_update_hook = std::move(hook);
}

auto Engine::set_frame_data_hook(std::function<u32(u32)> hook) -> void {
    frame_data_hook = std::move(hook);
}

auto Engine::set_model_cache_path(const String& path) -> void {
    model_cache_path = path;
}

auto Engine::get_sound_engine() -> SoundEngine* {
    return sound_engine;
}

auto Engine::renderer() -> Renderer* {
    return vulkan_renderer;
}

auto Engine::get_delta_frame_time() const -> f32 {
    return delta_frame_time;
}

auto Engine::get_gravity() const -> f32 {
    return gravity;
}

auto Engine::get_hud_screen_x() const -> f32 {
    return hud_screen_x;
}

auto Engine::get_hud_screen_y() const -> f32 {
    return hud_screen_y;
}

auto Engine::set_hud_screen(f32 x, f32 y) -> void {
    hud_screen_x = x;
    hud_screen_y = y;
}

auto Engine::set_previous_mouse_offsets(f32 x, f32 y) -> void {
    previous_mouse_offset_x = x;
    previous_mouse_offset_y = y;
}

auto Engine::left_mouse_button_pressed() const -> bool {
    return is_left_mouse_button_pressed;
}

auto Engine::game_loop() -> void {
    render_vulkan();
}

auto Engine::event_queue_flush() -> void {
}

auto Engine::render_vulkan() -> void {
    SystemManager* system_manager = SystemManager::get_instance();
    bool game_loop_active = true;
    // Game systems wire themselves via add_system/hooks and the public
    // Engine accessors; the engine only owns generic simulation systems.
    glvm_log::info("glvm", "render_vulkan: creating renderer");
    vulkan_renderer = new Renderer();
    vulkan_renderer->initialize_texture_data = texture_vector;
    vulkan_renderer->paths_array = paths_array;
    vulkan_renderer->paths_gltf = paths_gltf;
    const glvm::MeshManager* mesh_manager = glvm::MeshManager::get_instance();
    vulkan_renderer->set_mesh_data(
        mesh_manager->paths_array,
        mesh_manager->paths_gltf
    );
    directional_light_archetypes_number = 0;
    world.search_cache_archetypes(
        directional_light_required_mask,
        cached_directional_light_archetypes.data(),
        directional_light_archetypes_number
    );
    vulkan_renderer->directional_light_number =
        directional_light_archetypes_number;

    spot_light_archetypes_number = 0;
    world.search_cache_archetypes(
        spot_light_required_mask,
        cached_spot_light_archetypes.data(),
        spot_light_archetypes_number
    );
    vulkan_renderer->spot_light_number = spot_light_archetypes_number;

    point_light_archetypes_number = 0;
    world.search_cache_archetypes(
        point_light_required_mask,
        cached_point_light_archetypes.data(),
        point_light_archetypes_number
    );
    vulkan_renderer->point_light_number = point_light_archetypes_number;
    glvm_log::info("glvm", "render_vulkan: load_wavefront_obj");
    load_wavefront_obj();
    glvm_log::info("glvm", "render_vulkan: initialize_gltf");
    initialize_gltf();
    glvm_log::info("glvm", "render_vulkan: initialize_font_data");
    initialize_font_data();
    glvm_log::info("glvm", "render_vulkan: initialize_math_objects_data");
    initialize_math_objects_data();
    glvm_log::info("glvm", "render_vulkan: run");
    vulkan_renderer->run();
    glvm_log::info("glvm", "render_vulkan: entering game loop");
    vulkan_renderer->window->input_stack = &global_input_stack;
#ifdef _WIN32
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#endif
    while (game_loop_active) {
        delta_frame_time = as<f32>(chrono->get_elapsed());
        chrono->reset();
        if (vulkan_renderer->imgui_overlay->pause_time) {
            delta_frame_time = 0.0f;
        }
        gravity += delta_frame_time;
        vulkan_renderer->window->clear_display();
        vulkan_renderer->window->handle_event(global_event);
        if ((global_input_stack.search_element(EventKind::GameLoopKill))
            == EventKind::GameLoopKill) {
            game_loop_active = false;
        }
        if ((global_input_stack.search_element(EventKind::CursorReleased))
            == EventKind::CursorReleased) {
            vulkan_renderer->is_cursor_released =
                !vulkan_renderer->is_cursor_released;
            global_input_stack.remove(EventKind::CursorReleased);
            global_input_stack.remove(EventKind::CursorReleased);
            // The click-to-relock must not trigger on the button that was
            // already held while Esc was pressed.
            global_input_stack.remove(EventKind::MouseLeftButton);
        }
        if (vulkan_renderer->is_cursor_released
            && (global_input_stack.search_element(EventKind::MouseLeftButton))
                == EventKind::MouseLeftButton
            && !vulkan_renderer->imgui_overlay->wants_mouse()) {
            vulkan_renderer->is_cursor_released = false;
        }
        is_left_mouse_button_pressed =
            (global_input_stack.search_element(EventKind::MouseLeftButton))
            == EventKind::MouseLeftButton;
        // Games handle their own UI toggles (e.g. inventory) in the
        // pre-update hook.
        global_event.set_last_event(global_input_stack);
        if (vulkan_renderer->window_system == WindowSystem::WAYLAND) {
            if (vulkan_renderer->window->is_focused) {
                vulkan_renderer->window->cursor_lock(
                    global_event.mouse_pointer_position.position_x,
                    global_event.mouse_pointer_position.position_y,
                    &global_event.mouse_pointer_position.offset_x,
                    &global_event.mouse_pointer_position.offset_y
                );
            }
        } else {
            const bool cursor_should_be_hidden =
                !vulkan_renderer->is_inventory_opened
                && vulkan_renderer->window->is_focused
                && !vulkan_renderer->is_cursor_released
                && !vulkan_renderer->imgui_overlay->wants_mouse();
            if (cursor_should_be_hidden && !is_cursor_hidden) {
#ifdef _WIN32
                ShowCursor(FALSE);
#endif
                is_cursor_hidden = true;
            } else if (!cursor_should_be_hidden && is_cursor_hidden) {
#ifdef _WIN32
                ShowCursor(TRUE);
#endif
                is_cursor_hidden = false;
            }
            if (cursor_should_be_hidden) {
                vulkan_renderer->window->cursor_lock(
                    global_event.mouse_pointer_position.position_x,
                    global_event.mouse_pointer_position.position_y,
                    &global_event.mouse_pointer_position.offset_x,
                    &global_event.mouse_pointer_position.offset_y
                );
            }
            // Games reset their own mouse/camera state on UI close in the
            // pre-update hook.
        }
        compute_hud_screen_coordinates();
        if (pre_update_hook) {
            pre_update_hook();
        }
        enlarge_frame_accumulator(delta_frame_time);
        system_manager->update();
        if (post_update_hook) {
            post_update_hook();
        }
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
    glvm_log::info("glvm", "render_vulkan: loop exited, deleting renderer");
    delete vulkan_renderer;
    glvm_log::info("glvm", "render_vulkan: renderer deleted");
}

auto Engine::enlarge_frame_accumulator(f32 value) -> void {
    animation_archetypes_number = 0;
    for (auto* arch : world.archetypes) {
        const u64 required_mask = (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::AnimationComponent);
        if (matches_required_mask(arch->mask, required_mask)) {
            cached_animation_archetypes[animation_archetypes_number] = arch;
            ++animation_archetypes_number;
        }
    }
    for (u32 n = 0; n < animation_archetypes_number; ++n) {
        Archetype* arch = cached_animation_archetypes[n];
        auto* animation_view = as<Animation*>(
            arch->components[ComponentsIndices::AnimationComponent]
        );
        auto* mesh_view =
            as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);
        if (arch != nullptr && animation_view != nullptr
            && mesh_view != nullptr) {
            for (u32 i = 0; i < cached_animation_archetypes[n]->entity_count;
                 ++i) {
                if (&mesh_view[i] != nullptr && &animation_view[i] != nullptr) {
                    const u32 mesh_id = mesh_view[i].handle.id;
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

auto Engine::set_view_matrix() -> void {
    // Generic camera follow: any entity with the base Camera (Beholder)
    // component drives the view. Games decide which entities those are.
    camera_archetypes_number = 0;
    world.search_cache_archetypes(
        camera_required_mask,
        cached_camera_archetypes.data(),
        camera_archetypes_number
    );
    for (u32 n = 0; n < camera_archetypes_number; ++n) {
        Archetype* arch = cached_camera_archetypes[n];
        auto* views =
            as<Beholder*>(arch->components[ComponentsIndices::ViewComponent]);
        auto* transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        for (u32 x = 0; x < arch->entity_count; ++x) {
            Beholder* camera_component = &views[x];
            Transform* player_transform = &transforms[x];

            Matrix<f32, 4> view_matrix(1.0f);
            const auto SENSITIVITY = 0.1f;
            yaw = global_event.mouse_pointer_position.offset_x;
            pitch = global_event.mouse_pointer_position.offset_y;
            yaw *= SENSITIVITY;
            pitch *= SENSITIVITY;
            global_event.mouse_pointer_position.pitch = pitch;
            global_event.mouse_pointer_position.yaw = yaw;
            vulkan_renderer->current_x =
                as<f32>(global_event.mouse_pointer_position.offset_x);
            vulkan_renderer->current_y =
                as<f32>(global_event.mouse_pointer_position.offset_y);
            f32 delta_x = 0.0f;
            f32 delta_y = 0.0f;
            if (!vulkan_renderer->is_inventory_opened) {
                if (vulkan_renderer->window_system == WindowSystem::WAYLAND) {
                    delta_x = vulkan_renderer->current_x;
                    delta_y = vulkan_renderer->current_y;
                } else {
                    delta_x =
                        vulkan_renderer->current_x - vulkan_renderer->prev_x;
                    delta_y =
                        vulkan_renderer->current_y - vulkan_renderer->prev_y;
                    delta_y *= -1.0f;
                }
            }
            const Vector<f32, 3> right_vec = cross(
                camera_component->forward,
                Vector<f32, 3>(0.0f, -1.0f, 0.0f)
            );
            const Vector<f32, 3> new_up_vec =
                cross(right_vec, camera_component->forward);
            // 1. The mouse direction determines the "intended direction of
            // rotation" for the object.
            // 2. The camera is "looking forward."
            // 3. To make the object "rotate as if the mouse is pushing it,"
            // you need to rotate it around an axis that is perpendicular to
            // both the view direction and the mouse movement.
            const Vector<f32, 3> rotate_axis = normalize(cross(
                camera_component->forward,
                right_vec * delta_x + new_up_vec * delta_y
            ));
            if (vec_length(rotate_axis) >= 0.001f) {
                // A vector in the screen's tangent plane: it indicates the
                // direction in which the mouse moved, but expressed in world
                // (or 3D) space.
                f32 rotation_angle =
                    std::sqrt((delta_y * delta_y) + (delta_x * delta_x));
                constexpr auto ANGLE_SCALE = 0.05f;
                rotation_angle = radians(rotation_angle * ANGLE_SCALE);
                // Quaternions need division by 2.
                constexpr auto QUAT_ANGLE_CORRECTION = 0.5f;
                const Point applied_rotation_point =
                    exp(rotation_angle,
                        RLine {
                            .rx = -rotate_axis.elements[0],
                            .ry = -rotate_axis.elements[1],
                            .rz = -rotate_axis.elements[2]
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
            constexpr auto MAX_PITCH_SIN = 1.0f;
            if (camera_component->forward[1] > MAX_PITCH_SIN) {
                camera_component->forward[1] = MAX_PITCH_SIN;
            } else if (camera_component->forward[1] < -MAX_PITCH_SIN) {
                camera_component->forward[1] = -MAX_PITCH_SIN;
            }
            camera_component->forward = normalize(camera_component->forward);
            player_transform->forward = camera_component->forward;
            Matrix<f32, 4> view = look_at_main(
                camera_component->position + player_transform->position,
                camera_component->position + player_transform->position
                    + camera_component->forward,
                Vector<f32, 3>(0.0f, -1.0f, 0.0f)
            );
            for (u32 i = 0; i < 4; ++i) {
                for (u32 j = 0; j < 4; ++j) {
                    view_matrix[i][j] = view[i][j];
                }
            }
            vulkan_renderer->view_matrix = view_matrix;
            vulkan_renderer->prev_y =
                as<f32>(global_event.mouse_pointer_position.offset_y);
            vulkan_renderer->prev_x =
                as<f32>(global_event.mouse_pointer_position.offset_x);
        }
    }
}

auto Engine::set_projection_matrix() -> void {
    const Matrix<f32, 4> t_projection_matrix = perspective<f32>(
        radians<f32>(90.0f),
        vulkan_renderer->aspect_ratio,
        0.1f,
        100.0f
    );
    vulkan_renderer->projection_matrix = t_projection_matrix;
    vulkan_renderer->projection_matrix[1][1] *= 1.0f;
}

[[nodiscard]] auto Engine::update_animation_frames(
    Animation* animation_component,
    u32 mesh_id
) -> Vec<Matrix<f32, 4>> {
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

    u32 joint_matrices_data_size {};
    if (vulkan_renderer->joint_matrices_per_mesh.size() > 0) {
        joint_matrices_data_size =
            vulkan_renderer->joint_matrices_per_mesh[mesh_id].size();
    }

    Vec<Matrix<f32, 4>> joint_matrices;
    if (joint_matrices_data_size == 0) {
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (u32 i = 0; i < MAX_JOINTS_NUMBER; ++i) {
            const Matrix<f32, 4> unit_matrix(1.0f);
            joint_matrices[i] = unit_matrix;
        }

    } else {
        joint_matrices.resize(MAX_JOINTS_NUMBER);
        for (u32 i = 0; i < joint_matrices_data_size; ++i) {
            if (mesh_id >= vulkan_renderer->joint_matrices_per_mesh.size()) {
                throw std::runtime_error(
                    "mesh_id out of range of joint matrices"
                );
            }
            if (i >= vulkan_renderer->joint_matrices_per_mesh[mesh_id].size()) {
                throw std::runtime_error(
                    "joint index out of range of joint matrices"
                );
            }
            if (animation_component->current_animation_frame
                >= vulkan_renderer->joint_matrices_per_mesh[mesh_id][i].size()) {
                throw std::runtime_error(
                    "animation frame out of range of joint matrices"
                );
            }
            joint_matrices[i] =
                vulkan_renderer->joint_matrices_per_mesh
                    [mesh_id][i][animation_component->current_animation_frame];
        }
        for (u32 j = joint_matrices_data_size; j < MAX_JOINTS_NUMBER; ++j) {
            const Matrix<f32, 4> unit_matrix(1.0f);
            joint_matrices[j] = unit_matrix;
        }
    }
    return joint_matrices;
}

auto Engine::update_directional_light_space_matrix_shadow_map_ubo(
    DirectionalLightComponent* light
) -> Matrix<f32, 4> {
    const f32 near_plane_flat_shadow_map = 5.5f;
    const f32 far_plane_flat_shadow_map = 100.0f;
    const Matrix<f32, 4> directional_projection_matrix_light = ortho<f32>(
        -50.0f,
        50.0f,
        -50.0f,
        50.0f,
        near_plane_flat_shadow_map,
        far_plane_flat_shadow_map
    );
    const Vector<f32, 3> position_vector_light = light->position;
    const Vector<f32, 3> direction_vector_light = light->direction;
    Matrix<f32, 4> view_matrix_light = look_at_main(
        position_vector_light,
        direction_vector_light,
        {0.0f, -1.0f, 0.0f}
    );
    return view_matrix_light * directional_projection_matrix_light;
}

auto Engine::update_spot_light_space_matrix_shadow_map_ubo(
    SpotLightComponent* light
) -> Matrix<f32, 4> {
    const f32 near_plane_flat_shadow_map = 0.5f;
    const f32 far_plane_flat_shadow_map = 100.0f;
    const Matrix<f32, 4> spot_projection_matrix_light = perspective<f32>(
        radians<f32>(90.0f),
        as<f32>(SHADOW_MAP_SIZE) / as<f32>(SHADOW_MAP_SIZE),
        near_plane_flat_shadow_map,
        far_plane_flat_shadow_map
    );
    const Vector<f32, 3> position_vector_light = light->position;
    const Vector<f32, 3> direction_vector_light = light->direction;
    Matrix<f32, 4> view_matrix_light = look_at_main(
        position_vector_light,
        direction_vector_light,
        {0.0f, -1.0f, 0.0f}
    );
    return view_matrix_light * spot_projection_matrix_light;
}

auto Engine::update_point_light_space_matrix_shadow_map_ubo(
    PointLightComponent* light,
    u32 layer
) -> Matrix<f32, 4> {
    const Vector<f32, 3> position_vector_light = light->position;
    Vector<f32, 3> directional_vector_light = Vector<f32, 3>(0.0f, 0.0f, 0.0f);
    Vector<f32, 3> up_vector = {0.0f, 0.0f, 0.0f};
    switch (layer) {
        case 0:
            // Positive X.
            directional_vector_light =
                position_vector_light + Vector<f32, 3>(1.0f, 0.0f, 0.0f);
            up_vector = Vector<f32, 3>(0.0f, -1.0f, 0.0f);
            break;
        case 1:
            // Negative X.
            directional_vector_light =
                position_vector_light + Vector<f32, 3>(-1.0f, 0.0f, 0.0f);
            up_vector = Vector<f32, 3>(0.0f, -1.0f, 0.0f);
            break;
        case 2:
            // Positive Y.
            directional_vector_light =
                position_vector_light + Vector<f32, 3>(0.0f, 1.0f, 0.0f);
            up_vector = Vector<f32, 3>(0.0f, 0.0f, 1.0f);
            break;
        case 3:
            // Negative Y.
            directional_vector_light =
                position_vector_light + Vector<f32, 3>(0.0f, -1.0f, 0.0f);
            up_vector = Vector<f32, 3>(0.0f, 0.0f, -1.0f);
            break;
        case 4:
            // Positive Z.
            directional_vector_light =
                position_vector_light + Vector<f32, 3>(0.0f, 0.0f, 1.0f);
            up_vector = Vector<f32, 3>(0.0f, -1.0f, 0.0f);
            break;
            // Negative Z.
        case 5:
            directional_vector_light =
                position_vector_light + Vector<f32, 3>(0.0f, 0.0f, -1.0f);
            up_vector = Vector<f32, 3>(0.0f, -1.0f, 0.0f);
            break;
        default:
            break;
    }
    const Matrix<f32, 4> projection_matrix_cube_shadow_map = perspective<f32>(
        radians<f32>(90.0f),
        as<f32>(SHADOW_MAP_SIZE) / as<f32>(SHADOW_MAP_SIZE),
        0.3f,
        100.0f
    );
    Matrix<f32, 4> view_matrix_light =
        look_at_main(position_vector_light, directional_vector_light, up_vector);
    return view_matrix_light * projection_matrix_cube_shadow_map;
}

auto Engine::set_frame_data() -> void {
    vulkan_renderer->directional_lights.clear();
    directional_light_archetypes_number = 0;
    world.search_cache_archetypes(
        directional_light_required_mask,
        cached_directional_light_archetypes.data(),
        directional_light_archetypes_number
    );
    u32 directional_light_counter = 0;
    for (u32 x = 0; x < directional_light_archetypes_number; ++x) {
        Archetype* arch = cached_directional_light_archetypes[x];
        auto* directional_lights = as<DirectionalLightComponent*>(
            arch->components[ComponentsIndices::DirectionalLightComponent]
        );
        for (u32 x1 = 0; x1 < arch->entity_count; ++x1) {
            if (directional_lights != nullptr) {
                vulkan_renderer->directional_lights.push_back({});
                DirectionalLightComponent* light = &directional_lights[x1];
                vulkan_renderer->directional_lights[directional_light_counter]
                    .directional_light_space_matrix =
                    update_directional_light_space_matrix_shadow_map_ubo(light);
                vulkan_renderer->directional_lights[directional_light_counter]
                    .position = Vector<f32, 4>(
                    light->position[0],
                    light->position[1],
                    light->position[2],
                    0.0f
                );
                vulkan_renderer->directional_lights[directional_light_counter]
                    .direction = Vector<f32, 4>(
                    light->direction[0],
                    light->direction[1],
                    light->direction[2],
                    0.0f
                );
                vulkan_renderer->directional_lights[directional_light_counter]
                    .ambient = Vector<f32, 4>(
                    light->ambient[0],
                    light->ambient[1],
                    light->ambient[2],
                    0.0f
                );
                vulkan_renderer->directional_lights[directional_light_counter]
                    .diffuse = Vector<f32, 4>(
                    light->diffuse[0],
                    light->diffuse[1],
                    light->diffuse[2],
                    0.0f
                );
                vulkan_renderer->directional_lights[directional_light_counter]
                    .specular = Vector<f32, 4>(
                    light->specular[0],
                    light->specular[1],
                    light->specular[2],
                    0.0f
                );
                ++directional_light_counter;
            }
        }
    }
    vulkan_renderer->spot_lights.clear();
    spot_light_archetypes_number = 0;
    world.search_cache_archetypes(
        spot_light_required_mask,
        cached_spot_light_archetypes.data(),
        spot_light_archetypes_number
    );
    u32 spot_light_counter = 0;
    for (u32 x = 0; x < spot_light_archetypes_number; ++x) {
        Archetype* arch = cached_spot_light_archetypes[x];
        auto* spot_lights = as<SpotLightComponent*>(
            arch->components[ComponentsIndices::SpotLightComponent]
        );
        for (u32 x1 = 0; x1 < arch->entity_count; ++x1) {
            if (spot_lights != nullptr) {
                vulkan_renderer->spot_lights.push_back({});
                SpotLightComponent* light = &spot_lights[x1];
                vulkan_renderer->spot_lights[spot_light_counter]
                    .spot_light_space_matrix =
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
    world.search_cache_archetypes(
        point_light_required_mask,
        cached_point_light_archetypes.data(),
        point_light_archetypes_number
    );
    u32 point_light_counter = 0;
    for (u32 x = 0; x < point_light_archetypes_number; ++x) {
        Archetype* arch = cached_point_light_archetypes[x];
        auto* point_lights = as<PointLightComponent*>(
            arch->components[ComponentsIndices::PointLightComponent]
        );
        for (u32 x1 = 0; x1 < arch->entity_count; ++x1) {
            if (point_lights != nullptr) {
                vulkan_renderer->point_lights.push_back({});
                PointLightComponent* light = &point_lights[x1];
                // 6 is a number of cube map layers.
                const u32 max_cube_map_layers = 6;
                for (u32 cube_map_layer_counter = 0;
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
    // Health bars are game UI: filled by the game via frame_data_hook
    // (see examples/tps.cpp), same as the other game actor categories.
    vulkan_renderer->health_bars.clear();
    vulkan_renderer->fonts.clear();
    fonts_archetypes_number = 0;
    world.search_cache_archetypes(
        font_required_mask,
        cached_fonts_archetypes.data(),
        fonts_archetypes_number
    );

    u32 font_counter = 0;
    for (u32 x = 0; x < fonts_archetypes_number; ++x) {
        Archetype* arch = cached_fonts_archetypes[x];
        auto* font_transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        auto* fonts =
            as<Font*>(arch->components[ComponentsIndices::FontComponent]);
        for (u32 i = 0; i < arch->entity_count; ++i) {
            vulkan_renderer->fonts.push_back({});
            const Font* font_component = &fonts[i];
            const Transform* transform_component = &font_transforms[i];
            vulkan_renderer->fonts[font_counter].position =
                transform_component->position;
            vulkan_renderer->fonts[font_counter].font_string =
                font_component->font_string;
            vulkan_renderer->fonts[font_counter].lifetime =
                font_component->lifetime;
            ++font_counter;
        }
    }
    // Game UI (inventories, items, crosshair) is filled by the game via
    // frame_data_hook. The engine only clears the buffers here so a game
    // without the hook still renders a clean frame.
    vulkan_renderer->inventories.clear();
    vulkan_renderer->items.clear();
    vulkan_renderer->crosshairs.clear();
    vulkan_renderer->math_objects.clear();
    vulkan_renderer->actors.clear();
    // Game actor categories (animated meshes, static meshes, level chunks,
    // projectiles, items) and game UI are appended by the game via
    // frame_data_hook.
    u32 game_actors_counter = 0;
    if (frame_data_hook) {
        game_actors_counter = frame_data_hook(game_actors_counter);
    }
    vulkan_renderer->players.clear();
    camera_archetypes_number = 0;
    world.search_cache_archetypes(
        camera_required_mask,
        cached_camera_archetypes.data(),
        camera_archetypes_number
    );
    u32 player_entity_count = 0;
    for (u32 x = 0; x < camera_archetypes_number; ++x) {
        Archetype* arch = cached_camera_archetypes[x];
        auto* player_transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        for (u32 n = 0; n < arch->entity_count; ++n) {
            vulkan_renderer->players.push_back({});
            const Transform* player_transform_component = &player_transforms[n];
            if (&player_transforms[n] != nullptr) {
                vulkan_renderer->players[player_entity_count].position =
                    player_transform_component->position;
                vulkan_renderer->players[player_entity_count].forward =
                    player_transform_component->forward;
            }
            ++player_entity_count;
        }
    }
}

auto Engine::load_wavefront_obj() -> void {
    for (u32 m = 0; m < paths_array.size(); ++m) {
        WavefrontObjParser parser;
        WavefrontObjParser* wavefront_obj_parser = &parser;
        wavefront_obj_parser->read_file(paths_array[m]);
        wavefront_obj_parser->parse_file();
        vulkan_renderer->indices.emplace_back();
        vulkan_renderer->vertices.emplace_back();
        vulkan_renderer->highest_gltf_y.emplace_back();
        vulkan_renderer->highest_gltf_y[m] = -999.999f;
        vulkan_renderer->frames.emplace_back();
        vulkan_renderer->joint_matrices_per_mesh.emplace_back();
        u32 vertex_index = 0;
        u32 texture_index = 0;
        u32 normal_index = 0;
        const u32 face_vertices_size = wavefront_obj_parser->get_faces().size();
        vulkan_renderer->mesh_axis_limiting_values.set_to_default_values();
        for (u32 i = 0; i < face_vertices_size; ++i) {
            for (i32 j = 0; j < 3; ++j) {
                vertex_index = wavefront_obj_parser->get_faces()[i][0][j] - 1;
                vulkan_renderer->indices[m].push_back((i * 3) + j);
                Position vertex = wavefront_obj_parser
                                      ->get_coordinate_vertices()[vertex_index];
                texture_index = wavefront_obj_parser->get_faces()[i][1][j] - 1;
                Position texture =
                    wavefront_obj_parser->get_texture_vertices()[texture_index];
                normal_index = wavefront_obj_parser->get_faces()[i][2][j] - 1;
                Position normal =
                    wavefront_obj_parser->get_normals()[normal_index];
                Vector<f32, 4> joint_indices;
                Vector<f32, 4> weights;
                vulkan_renderer->highest_gltf_y[m] =
                    std::max(vertex[1], vulkan_renderer->highest_gltf_y[m]);
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
                weights[3] = 1.0f;
                vulkan_renderer->vertices[m].push_back(
                    {.pos = {vertex[0], vertex[1], vertex[2]},
                     .color = {normal[0], normal[1], normal[2]},
                     .tex_coord = {texture[0], texture[1]},
                     .joint_indices =
                         {joint_indices[0], joint_indices[1], joint_indices[2]},
                     .weights = {weights[0], weights[1], weights[2]}}
                );
            }
        }
        set_mesh_bounds(vulkan_renderer->mesh_axis_limiting_values);
        ++wavefront_obj_counter;
    }
}

auto Engine::calculate_mesh_bounds(const Vector<f32, 4>& animated_vertex)
    -> void {
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

auto Engine::is_model_cache_exists(const String& model_file_path) -> bool {
    // No cache path configured (the default): compute bounds every time.
    if (model_cache_path.empty()) {
        return false;
    }
    std::ofstream models_cache(model_cache_path, std::ios::app);
    if (!models_cache.is_open()) {
        std::cerr << "Error opening the models cache file" << std::endl;
        throw std::runtime_error("Failed to load mesh cache");
    }
    std::ifstream file(model_cache_path);
    String line;
    while (std::getline(file, line)) {
        if (line.contains(model_file_path)) {
            std::istringstream iss(line);
            String keyword;
            f32 highest_x = NAN;
            f32 lowest_x = NAN;
            f32 highest_y = NAN;
            f32 lowest_y = NAN;
            f32 highest_z = NAN;
            f32 lowest_z = NAN;
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

auto Engine::write_models_cache(const String& model_file_path) -> void {
    if (model_cache_path.empty()) {
        return;
    }
    std::ofstream models_cache(model_cache_path, std::ios::app);
    if (!models_cache.is_open()) {
        std::cerr << "Error opening the models cache file" << '\n';
        throw std::runtime_error("Failed to load mesh cache");
    }
    models_cache << model_file_path;
    models_cache << " " << vulkan_renderer->mesh_axis_limiting_values.highest_x
                 << " " << vulkan_renderer->mesh_axis_limiting_values.lowest_x
                 << " " << vulkan_renderer->mesh_axis_limiting_values.highest_y
                 << " " << vulkan_renderer->mesh_axis_limiting_values.lowest_y
                 << " " << vulkan_renderer->mesh_axis_limiting_values.highest_z
                 << " " << vulkan_renderer->mesh_axis_limiting_values.lowest_z
                 << '\n';
    models_cache.close();
}

auto Engine::initialize_gltf() -> void {
    Vec<bool> animation_flags;
    for (u32 m = 0; m < paths_gltf.size(); ++m) {
        JsonParser json_parser;
        vulkan_renderer->vertices_temp.emplace_back();
        vulkan_renderer->indices.emplace_back();
        vulkan_renderer->frames.emplace_back();
        vulkan_renderer->joint_matrices_per_mesh.emplace_back();
        animation_flags.push_back({});
        vulkan_renderer->highest_gltf_y.emplace_back();
        const u32 next_index_gltf = wavefront_obj_counter + m;
        bool animation_flag = false;
        json_parser.load_gltf(
            paths_gltf[m],
            vulkan_renderer->vertices_temp[m],
            vulkan_renderer->indices[next_index_gltf],
            vulkan_renderer->joint_matrices_per_mesh[next_index_gltf],
            vulkan_renderer->frames[next_index_gltf],
            animation_flag,
            vulkan_renderer->highest_gltf_y[next_index_gltf]
        );
        animation_flags[m] = animation_flag;
        glvm_log::info(
            "glvm",
            "gltf parsed {}: raw_floats={} indices={} joints={} framesets={} animated={}",
            paths_gltf[m],
            vulkan_renderer->vertices_temp[m].size(),
            vulkan_renderer->indices[next_index_gltf].size(),
            vulkan_renderer->joint_matrices_per_mesh[next_index_gltf].size(),
            vulkan_renderer->frames[next_index_gltf].size(),
            animation_flag
        );
    }
    for (u32 m = 0; m < paths_gltf.size(); ++m) {
        vulkan_renderer->vertices.emplace_back();
        vulkan_renderer->mesh_axis_limiting_values.set_to_default_values();
        is_already_cached = false;
        is_model_cache_exists(paths_gltf[m]);
        i32 step_offset = 0;
        if (animation_flags[m]) {
            step_offset = 8;
        } else {
            step_offset = 16;
        }
        for (u32 n = 0; n < vulkan_renderer->vertices_temp[m].size();
             n += step_offset) {
            Position vertex {};
            vertex[0] = vulkan_renderer->vertices_temp[m][n];
            vertex[1] = vulkan_renderer->vertices_temp[m][n + 1];
            vertex[2] = vulkan_renderer->vertices_temp[m][n + 2];
            Position normal {};
            normal[0] = vulkan_renderer->vertices_temp[m][n + 3];
            normal[1] = vulkan_renderer->vertices_temp[m][n + 4];
            normal[2] = vulkan_renderer->vertices_temp[m][n + 5];
            Position texture {};
            texture[0] = vulkan_renderer->vertices_temp[m][n + 6];
            texture[1] = vulkan_renderer->vertices_temp[m][n + 7];
            Vector<f32, 4> joint_indices;
            Vector<f32, 4> weights;
            if (animation_flags[m]) {
                joint_indices[0] = -1;
                joint_indices[1] = -1;
                joint_indices[2] = -1;
                joint_indices[3] = -1;
                weights[0] = 1;
                weights[1] = 1;
                weights[2] = 1;
                weights[3] = 1;
            } else {
                joint_indices[0] = vulkan_renderer->vertices_temp[m][n + 8];
                joint_indices[1] = vulkan_renderer->vertices_temp[m][n + 9];
                joint_indices[2] = vulkan_renderer->vertices_temp[m][n + 10];
                joint_indices[3] = vulkan_renderer->vertices_temp[m][n + 11];

                weights[0] = vulkan_renderer->vertices_temp[m][n + 12];
                weights[1] = vulkan_renderer->vertices_temp[m][n + 13];
                weights[2] = vulkan_renderer->vertices_temp[m][n + 14];
                weights[3] = vulkan_renderer->vertices_temp[m][n + 15];
            }
            const u32 next_index_gltf = wavefront_obj_counter + m;
            vulkan_renderer->vertices[next_index_gltf].push_back(
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
            if (is_already_cached) {
                continue;
            }
            Vector<f32, 4> animated_vertex =
                Vector<f32, 4>(vertex[0], vertex[1], vertex[2], 1.0f);
            if (!animation_flags[m]
                && vulkan_renderer->joint_matrices_per_mesh[next_index_gltf]
                        .size()
                    > 0) {
                for (u32 frame = 0;
                     frame < vulkan_renderer
                                 ->joint_matrices_per_mesh[next_index_gltf][0]
                                 .size();
                     ++frame) {
                    const Matrix<f32, 4> skin_matrix =
                        (vulkan_renderer->joint_matrices_per_mesh
                             [next_index_gltf][as<i32>(joint_indices[0])][frame]
                         * weights[0])
                        + (vulkan_renderer->joint_matrices_per_mesh
                               [next_index_gltf][as<i32>(joint_indices[1])]
                               [frame]
                           * weights[1])
                        + (vulkan_renderer->joint_matrices_per_mesh
                               [next_index_gltf][as<i32>(joint_indices[2])]
                               [frame]
                           * weights[2])
                        + (vulkan_renderer->joint_matrices_per_mesh
                               [next_index_gltf][as<i32>(joint_indices[3])]
                               [frame]
                           * weights[3]);

                    animated_vertex =
                        Vector<f32, 4>(vertex[0], vertex[1], vertex[2], 1.0f)
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

auto Engine::initialize_font_data() -> void {
    constexpr auto FONT_STEP = 1.0f / 12;
    constexpr auto GLYPH_ROW = 7;
    constexpr auto GLYPH_COLUMN = 12;
    vulkan_renderer->font_vertex_buffer_container.resize(128);
    vulkan_renderer->font_vertex_buffer_memory_container.resize(128);
    vulkan_renderer->font_index_buffer_container.resize(128);
    vulkan_renderer->font_index_buffer_memory_container.resize(128);
    for (u32 i = 0; i < GLYPH_ROW; ++i) {
        for (u32 j = 0; j < GLYPH_COLUMN; ++j) {
            Vec<Vertex> symbol_g_vertices;
            symbol_g_vertices.push_back(
                {.pos = {-0.5f, 0.5f, 0.0f},
                 .color = {0.0f, 1.0f, 0.0f},
                 .tex_coord = {FONT_STEP * j, (FONT_STEP * i) + FONT_STEP},
                 .joint_indices = {0.0f, 0.0f, 0.0f, 0.0f},
                 .weights = {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.push_back(
                {.pos = {0.5f, 0.5f, 0.0f},
                 .color = {1.0f, 1.0f, 0.0f},
                 .tex_coord =
                     {(FONT_STEP * j) + FONT_STEP, (FONT_STEP * i) + FONT_STEP},
                 .joint_indices = {0.0f, 0.0f, 0.0f, 0.0f},
                 .weights = {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.push_back(
                {.pos = {-0.5f, -0.5f, 0.0f},
                 .color = {0.0f, 0.0f, 0.0f},
                 .tex_coord = {FONT_STEP * j, FONT_STEP * i},
                 .joint_indices = {0.0f, 0.0f, 0.0f, 0.0f},
                 .weights = {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            symbol_g_vertices.push_back(
                {.pos = {0.5f, -0.5f, 0.0f},
                 .color = {1.0f, 0.0f, 0.0f},
                 .tex_coord = {(FONT_STEP * j) + FONT_STEP, FONT_STEP * i},
                 .joint_indices = {0.0f, 0.0f, 0.0f, 0.0f},
                 .weights = {1.0f, 0.0f, 0.0f, 0.0f}}
            );
            const u32 current_buffer_index = (i * GLYPH_COLUMN) + j;
            bool exit_flag = false;
            const auto next_buffer_index =
                as<u32>(vulkan_renderer->glyphs[current_buffer_index]);
            // TODO: Fix garbage algorithm.
            for (const auto n : vulkan_renderer->font_indices_container) {
                if (next_buffer_index == n) {
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

auto Engine::initialize_math_objects_data() -> void {
    math_object_archetypes_number = 0;
    world.search_cache_archetypes(
        math_object_required_mask,
        cached_math_object_archetypes.data(),
        math_object_archetypes_number
    );
    for (u32 archetype_index = 0;
         archetype_index < math_object_archetypes_number;
         ++archetype_index) {
        Archetype* arch = cached_math_object_archetypes[archetype_index];
        auto* generated_meshes = as<MeshGeneration*>(
            arch->components[ComponentsIndices::MeshGenerationComponent]
        );
        for (u32 entity_index = 0; entity_index < arch->entity_count;
             ++entity_index) {
            MeshGeneration& generated_mesh = generated_meshes[entity_index];
            generated_mesh.mesh_id =
                as<u32>(vulkan_renderer->math_objects_vertices.size());
            Vec<u32> indices;
            const u32 vertices_number = as<u32>(generated_mesh.vertices.size());
            if (vertices_number == 3) {
                for (const i32 index : TRIANGLE_INDEX_BUFFER_DATA) {
                    indices.push_back(as<u32>(index));
                }
            } else if (vertices_number == 8) {
                for (const i32 index : BOX_INDEX_BUFFER_DATA_LINE_MODE) {
                    indices.push_back(as<u32>(index));
                }
            } else if (vertices_number == 2) {
                for (const i32 index : VECTOR_INDEX_BUFFER_DATA) {
                    indices.push_back(as<u32>(index));
                }
            } else if (vertices_number == 4) {
                for (const i32 index : PLANE_INDEX_BUFFER_DATA) {
                    indices.push_back(as<u32>(index));
                }
            } else {
                panic(
                    "wrong vertices number for a math object mesh: {}",
                    vertices_number
                );
            }
            vulkan_renderer->math_objects_indices.push_back(indices);
            Vec<Vertex> mesh_vertices;
            for (const Vector<f32, 3>& vertex : generated_mesh.vertices) {
                mesh_vertices.push_back(
                    {.pos = vertex,
                     .color = {0.0f, 1.0f, 0.0f},
                     .tex_coord = {0.0f, 1.0f},
                     .joint_indices = {-1.0f, -1.0f, -1.0f, -1.0f},
                     .weights = {1.0f, 1.0f, 1.0f, 1.0f}}
                );
            }
            vulkan_renderer->math_objects_vertices.push_back(mesh_vertices);
        }
    }
}

auto Engine::compute_model_matrix(Transform* transform, f32 yaw)
    -> Matrix<f32, 4> {
    Matrix<f32, 4> scaling_matrix(1.0f);
    Matrix<f32, 4> translation_matrix(1.0f);
    scaling_matrix[0][0] = transform->scale;
    scaling_matrix[1][1] = transform->scale;
    scaling_matrix[2][2] = transform->scale;
    translation_matrix[3][0] = transform->position[0];
    translation_matrix[3][1] = transform->position[1];
    translation_matrix[3][2] = transform->position[2];
    translation_matrix[3][3] = 1.0f;
    // Mesh facing: yaw rotates the model about Y. Identity at zero, so
    // existing content renders exactly as before.
    const auto half_yaw = yaw * 0.5f;
    const Quaternion
        rotation_quaternion(std::cos(half_yaw), 0.0f, std::sin(half_yaw), 0.0f);
    auto rotation_matrix = rotate_quaternion<f32, 4>(rotation_quaternion);
    rotation_matrix.self_tensor_transpose();
    return scaling_matrix * rotation_matrix * translation_matrix;
}

auto Engine::compute_hud_screen_coordinates() -> void {
    if (vulkan_renderer->window_system == WindowSystem::WAYLAND) {
        hud_screen_y -= global_event.mouse_pointer_position.offset_y
            / as<f32>(vulkan_renderer->window->height);
        hud_screen_x += global_event.mouse_pointer_position.offset_x
            / as<f32>(vulkan_renderer->window->width);
    } else {
        if (vulkan_renderer->is_inventory_opened
            || vulkan_renderer->is_cursor_released) {
            // Cursor is free while the inventory is open or the cursor is
            // released: track its real position instead of the locked-mouse
            // offsets.
            hud_screen_x = 1.0f
                - (global_event.mouse_pointer_position.position_x
                   / (as<f32>(vulkan_renderer->window->width) / 2.0f));
            hud_screen_y =
                -((global_event.mouse_pointer_position.position_y
                   / (as<f32>(vulkan_renderer->window->height) / 2.0f))
                  - 1.0f);
        } else {
            hud_screen_y -= (previous_mouse_offset_y
                             - global_event.mouse_pointer_position.offset_y)
                / as<f32>(vulkan_renderer->window->height);
            hud_screen_x += (previous_mouse_offset_x
                             - global_event.mouse_pointer_position.offset_x)
                / as<f32>(vulkan_renderer->window->width);
        }
        previous_mouse_offset_x = global_event.mouse_pointer_position.offset_x;
        previous_mouse_offset_y = global_event.mouse_pointer_position.offset_y;
    }
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

auto Engine::load_texture_from_file(const char* path_to_texture)
    -> TextureHandle {
    const u32 texture_id = texture_vector.size();
    TextureHandle texture_handle {};
    texture_handle.id = texture_id;
    texture_vector.push_back({.path_to_image = path_to_texture});
    texture_handlers.push_back(texture_handle);
    return texture_handle;
}

auto Engine::load_texture_from_address(
    u32 width,
    u32 height,
    u32 data_length,
    u8* data
) -> TextureHandle {
    glvm_log::info(
        "glvm",
        "load_texture_from_address {}x{} len {}",
        width,
        height,
        data_length
    );
    const u32 texture_id = texture_vector.size();
    TextureHandle texture_handle {};
    texture_handle.id = texture_id;
    texture_vector.push_back(
        {.width = width,
         .height = height,
         .data_length = data_length,
         .data = data}
    );
    texture_handlers.push_back(texture_handle);
    return texture_handle;
}

auto Engine::load_mesh_from_obj(const char* mesh_path) -> MeshHandle {
    MeshHandle mesh_handle {};
    mesh_handle.id = mesh_id;
    paths_array.push_back(mesh_path);
    mesh_handles.push_back(mesh_handle);
    ++mesh_id;
    return mesh_handle;
}

auto Engine::load_mesh_from_gltf(const char* path_to_mesh) -> MeshHandle {
    glvm_log::info("glvm", "load_mesh_from_gltf {}", path_to_mesh);
    MeshHandle mesh_handle {};
    mesh_handle.id = mesh_id;
    paths_gltf.push_back(path_to_mesh);
    mesh_handles.push_back(mesh_handle);
    ++mesh_id;
    return mesh_handle;
}

auto Engine::load_mesh() -> MeshHandle {
    MeshHandle mesh_handle {};
    mesh_handle.id = mesh_id;
    mesh_handles.push_back(mesh_handle);
    ++mesh_id;
    return mesh_handle;
}

auto Engine::game_kill() -> void {
    running_sound = false;
    // Join the sound thread before touching the sound engine: it may still
    // be inside sound_stream().
    if (sound_thread.joinable()) {
        sound_thread.join();
    }
    sound_engine->close_device();
    delete sound_engine;
    sound_engine = nullptr;
    delete chrono;
    chrono = nullptr;
    // Only generic systems are owned by the engine; games delete their own.
    delete spatial_grid_system;
    spatial_grid_system = nullptr;
    glvm_log::info("glvm", "game_kill done");
}
} // namespace glvm

namespace glvm {
EntityManager* EntityManager::instance = nullptr;
Mutex EntityManager::mutex;

EntityManager::EntityManager() = default;

EntityManager::~EntityManager() = default;

auto EntityManager::get_instance() -> EntityManager* {
    const MutexGuard<Mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new EntityManager();
    }
    return instance;
}

[[nodiscard]] auto EntityManager::create_entity() -> u32 {
    u32 new_id = 0;
    // Check out whether or not free ID in removed entities registry.
    if (!removed_entity_registry.empty()) {
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

// No need to delete the real component in this method, because systems don't
// work with components lacking indices in the ordered container.
auto EntityManager::remove_entity(
    u32& entity_id,
    ComponentManager* component_manager
) -> void {
    component_manager->remove_all_components(entity_id);
    active_entity_registry[entity_id] = INVALID_ENTITY_ID;
    removed_entity_registry.push_back(entity_id);
    is_entities_collection_changed = true;
}
} // namespace glvm

namespace glvm {
Event::Event() = default;

auto Event::get_event() -> EventKind& {
    return event;
}

auto Event::set_event(EventKind new_event) -> void {
    event = new_event;
}

auto Event::set_next_event(EventKind new_event) -> void {
    next_event = new_event;
}

auto Event::get_next_event() -> EventKind {
    return next_event;
}

auto Event::set_last_event(EventStack stack) -> void {
    switch (stack.pop()) {
        case glvm::MoveRight:
            set_event(glvm::EventKind::MoveRight);
            break;
        case glvm::MoveLeft:
            set_event(glvm::EventKind::MoveLeft);
            break;
        case glvm::MoveBackward:
            set_event(glvm::EventKind::MoveBackward);
            break;
        case glvm::MoveForward:
            set_event(glvm::EventKind::MoveForward);
            break;
        case glvm::MouseLeftButton:
            set_event(glvm::EventKind::MouseLeftButton);
            break;
        default:
            break;
    }
}
} // namespace glvm

namespace glvm {
Vec<VkDescriptorSet> DESCRIPTOR_SETS_CHUNKS;
Vec<VkRenderPass> RENDER_PASSES;
Vec<Descriptor> GPU_DESCRIPTORS;
} // namespace glvm

namespace glvm {
auto descriptor_set_builder() -> void {
    // Counts ds bindings indexes inside ds.
    static u32 DS_GLOBAL_BINDINGS_COUNTER = 0;
    // Counts host data ds.
    static u32 DS_HOST_NUMBER = 0;
    // Counts offsets data descriptors.
    static u32 GLOBAL_DESCRIPTORS_OFFSET = 0;
    for (u32 ds_counter = 0;
         ds_counter < DescriptorSetDataLink::DescriptorChunksNumber;
         ++ds_counter) {
        // Offset for indexing inside descriptorSetsChunks.
        DESCRIPTOR_SETS_CONFIG[ds_counter].descriptor_set_offset =
            DS_HOST_NUMBER;
        DS_HOST_NUMBER +=
            DESCRIPTOR_SETS_CONFIG[ds_counter].host_descriptor_number;
        for (u32 ds_local_bindings_counter = 0; ds_local_bindings_counter
             < DESCRIPTOR_SETS_CONFIG[ds_counter]
                   .actual_linked_descriptor_bindings_number;
             ++ds_local_bindings_counter) {
            const auto ds_sum_bindings_counter =
                DS_GLOBAL_BINDINGS_COUNTER + ds_local_bindings_counter;
            // Global offset for descriptors inside ds binding.
            DESCRIPTOR_BINDINGS_CONFIG[ds_sum_bindings_counter]
                .global_descriptor_offset = GLOBAL_DESCRIPTORS_OFFSET;
            // Index for ds bindings inside ds.
            DESCRIPTOR_SETS_CONFIG[ds_counter]
                .descriptors_bindings_ids[ds_local_bindings_counter] =
                ds_sum_bindings_counter;
            if (DESCRIPTOR_BINDINGS_CONFIG[ds_sum_bindings_counter].vk_type
                == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                for (u32 descriptor_counter = 0; descriptor_counter
                     < DESCRIPTOR_BINDINGS_CONFIG[ds_sum_bindings_counter]
                           .shader_descriptors_number;
                     ++descriptor_counter) {
                    GPU_DESCRIPTORS.emplace_back();
                    GPU_DESCRIPTORS[GPU_DESCRIPTORS.size() - 1].gpu_buffer =
                        new GpuBuffer;
                    ++GLOBAL_DESCRIPTORS_OFFSET;
                }
            } else if (
                DESCRIPTOR_BINDINGS_CONFIG[ds_sum_bindings_counter].vk_type
                == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            ) {
                for (u32 descriptor_counter = 0; descriptor_counter
                     < DESCRIPTOR_BINDINGS_CONFIG[ds_sum_bindings_counter]
                           .shader_descriptors_number;
                     ++descriptor_counter) {
                    GPU_DESCRIPTORS.emplace_back();
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

auto pipeline_builder() -> void {
    static u32 DESCRIPTOR_SETS_LAYOUT_ID_COUNTER = 0;
    for (u32 pipeline_counter = 0;
         pipeline_counter < SpecificPipeline::PipelinesNumber;
         ++pipeline_counter) {
        for (u32 linked_ds_layout_counter = 0; linked_ds_layout_counter
             < PIPELINE_CONFIGS[pipeline_counter]
                   .actual_linked_descriptor_sets_number;
             ++linked_ds_layout_counter) {
            PIPELINE_CONFIGS[pipeline_counter]
                .linked_descriptor_set_ids[linked_ds_layout_counter] =
                DESCRIPTOR_SETS_LAYOUT_ID_COUNTER + linked_ds_layout_counter;
        }
        DESCRIPTOR_SETS_LAYOUT_ID_COUNTER +=
            PIPELINE_CONFIGS[pipeline_counter]
                .actual_linked_descriptor_sets_number;
    }
}

auto render_passes_builder() -> void {
    RENDER_PASSES.resize(SpecificPipeline::PipelinesNumber);
}
}; // namespace glvm

namespace glvm {
auto create_debug_utils_messenger_ext(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger
) -> VkResult {
    static auto FUNC = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
    );
    if (FUNC != nullptr) {
        return FUNC(instance, create_info, allocator, debug_messenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

auto create_begin_debug_utils_label_ext(
    VkInstance instance,
    VkCommandBuffer command_buffer,
    const VkDebugUtilsLabelEXT* label_info
) -> void {
#ifndef NDEBUG
    static auto FUNC = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
        vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT")
    );
    FUNC(command_buffer, label_info);
#endif
}

auto create_end_debug_utils_label_ext(
    VkInstance instance,
    VkCommandBuffer command_buffer
) -> void {
#ifndef NDEBUG
    static auto FUNC = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
        vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT")
    );
    FUNC(command_buffer);
#endif
}

auto destroy_debug_utils_messenger_ext(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* allocator
) -> void {
    static auto FUNC = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
    );
    if (FUNC != nullptr) {
        FUNC(instance, debug_messenger, allocator);
    }
}

auto set_debug_object_name(
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* object_name_info
) -> VkResult {
    static auto FUNC = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
        vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT")
    );
    if (FUNC != nullptr) {
        return FUNC(device, object_name_info);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

auto set_image_debug_object_name(
    VkDevice device,
    GpuImage image,
    String image_name
) -> void {
    VkDebugUtilsObjectNameInfoEXT image_object_info {};
    image_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String image_name1 = String(VK_DEBUG_IMAGE_SET_RED) + " \x1b[31m"
        + image_name + " pipeline #\x1b[0m " + std::to_string(0);
    const char* str_image_name = image_name1.c_str();
    image_object_info.pObjectName = str_image_name;
    image_object_info.objectType = VK_OBJECT_TYPE_IMAGE;
    image_object_info.objectHandle = reinterpret_cast<u64>(image.image);
    set_debug_object_name(device, &image_object_info);
}

auto set_pipeline_debug_object_name(
    VkDevice device,
    VkPipeline pipeline,
    String pipeline_name
) -> void {
    VkDebugUtilsObjectNameInfoEXT main_pipeline_object_info {};
    main_pipeline_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String main_pipeline_image_name = String(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31m" + pipeline_name + " pipeline #\x1b[0m "
        + std::to_string(0);
    const char* main_pipeline_str_image_name = main_pipeline_image_name.c_str();
    main_pipeline_object_info.pObjectName = main_pipeline_str_image_name;
    main_pipeline_object_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    main_pipeline_object_info.objectHandle = reinterpret_cast<u64>(pipeline);
    set_debug_object_name(device, &main_pipeline_object_info);
}

auto set_descriptor_set_object_name(
    VkDevice device,
    VkDescriptorSet descriptor_set,
    String descriptor_set_name,
    u32 index
) -> void {
    VkDebugUtilsObjectNameInfoEXT descriptor_set_object_info {};
    descriptor_set_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String name = String(VK_DEBUG_DESCRIPTOR_SET_RED) + " \x1b[31m"
        + descriptor_set_name + " descriptor set #\x1b[0m "
        + std::to_string(index);
    const char* str_name = name.c_str();
    descriptor_set_object_info.pObjectName = str_name;
    descriptor_set_object_info.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    descriptor_set_object_info.objectHandle =
        reinterpret_cast<u64>(descriptor_set);
    set_debug_object_name(device, &descriptor_set_object_info);
}

auto set_debug_object_names(
    VkDevice device,
    const Vec<VkBuffer>& vertex_buffer_container,
    const Vec<VkBuffer>& index_buffer_container,
    const Vec<Descriptor>& gpu_descriptors,
    const Vec<u32>& font_indices_container,
    const Vec<VkBuffer>& font_vertex_buffer_container,
    const Vec<VkBuffer>& font_index_buffer_container
) -> void {
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
    const String main_pipeline_image_name = String(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mMain pipeline #\x1b[0m " + std::to_string(0);
    const char* main_pipeline_str_image_name = main_pipeline_image_name.c_str();
    main_pipeline_object_info.pObjectName = main_pipeline_str_image_name;
    main_pipeline_object_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    main_pipeline_object_info.objectHandle = reinterpret_cast<u64>(
        PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline].pipeline
    );
    set_debug_object_name(device, &main_pipeline_object_info);
    VkDebugUtilsObjectNameInfoEXT main_pipeline_layout_object_info {};
    main_pipeline_layout_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String main_pipeline_layout_image_name =
        String(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mMain pipeline layout #\x1b[0m " + std::to_string(0);
    const char* main_pipeline_layout_str_image_name =
        main_pipeline_layout_image_name.c_str();
    main_pipeline_layout_object_info.pObjectName =
        main_pipeline_layout_str_image_name;
    main_pipeline_layout_object_info.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    main_pipeline_layout_object_info.objectHandle = reinterpret_cast<u64>(
        PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline].pipeline_layout
    );
    set_debug_object_name(device, &main_pipeline_object_info);
    VkDebugUtilsObjectNameInfoEXT directional_light_pipeline_object_info {};
    directional_light_pipeline_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String directional_light_pipeline_image_name =
        String(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mDirectional light pipeline #\x1b[0m " + std::to_string(0);
    const char* directional_light_pipeline_str_image_name =
        directional_light_pipeline_image_name.c_str();
    directional_light_pipeline_object_info.pObjectName =
        directional_light_pipeline_str_image_name;
    directional_light_pipeline_object_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    directional_light_pipeline_object_info.objectHandle = reinterpret_cast<u64>(
        PIPELINE_CONFIGS[SpecificPipeline::DirectionalLightPipeline].pipeline
    );
    set_debug_object_name(device, &directional_light_pipeline_object_info);
    VkDebugUtilsObjectNameInfoEXT
        directional_light_pipeline_layout_object_info {};
    directional_light_pipeline_layout_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String directional_light_pipeline_layout_image_name =
        String(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mDirectional light pipeline layout #\x1b[0m "
        + std::to_string(0);
    const char* directional_light_pipeline_layout_str_image_name =
        directional_light_pipeline_layout_image_name.c_str();
    directional_light_pipeline_layout_object_info.pObjectName =
        directional_light_pipeline_layout_str_image_name;
    directional_light_pipeline_layout_object_info.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    directional_light_pipeline_layout_object_info.objectHandle =
        reinterpret_cast<u64>(
            PIPELINE_CONFIGS[SpecificPipeline::DirectionalLightPipeline]
                .pipeline_layout
        );
    set_debug_object_name(
        device,
        &directional_light_pipeline_layout_object_info
    );
    VkDebugUtilsObjectNameInfoEXT spot_light_pipeline_object_info {};
    spot_light_pipeline_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String spot_light_pipeline_image_name = String(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mSpot light pipeline #\x1b[0m " + std::to_string(0);
    const char* spot_light_pipeline_str_image_name =
        spot_light_pipeline_image_name.c_str();
    spot_light_pipeline_object_info.pObjectName =
        spot_light_pipeline_str_image_name;
    spot_light_pipeline_object_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    spot_light_pipeline_object_info.objectHandle = reinterpret_cast<u64>(
        PIPELINE_CONFIGS[SpecificPipeline::SpotLightPipeline].pipeline
    );
    set_debug_object_name(device, &spot_light_pipeline_object_info);
    VkDebugUtilsObjectNameInfoEXT spot_light_pipeline_layout_object_info {};
    spot_light_pipeline_layout_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String spot_light_pipeline_layout_image_name =
        String(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mSpot light pipeline layout #\x1b[0m " + std::to_string(0);
    const char* spot_light_pipeline_layout_str_image_name =
        spot_light_pipeline_layout_image_name.c_str();
    spot_light_pipeline_layout_object_info.pObjectName =
        spot_light_pipeline_layout_str_image_name;
    spot_light_pipeline_layout_object_info.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    spot_light_pipeline_layout_object_info.objectHandle = reinterpret_cast<u64>(
        PIPELINE_CONFIGS[SpecificPipeline::SpotLightPipeline].pipeline_layout
    );
    set_debug_object_name(device, &spot_light_pipeline_layout_object_info);
    VkDebugUtilsObjectNameInfoEXT point_light_pipeline_object_info {};
    point_light_pipeline_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String point_light_pipeline_image_name = String(VK_DEBUG_PIPELINE_RED)
        + " \x1b[31mPoint light pipeline #\x1b[0m " + std::to_string(0);
    const char* point_light_pipeline_str_image_name =
        point_light_pipeline_image_name.c_str();
    point_light_pipeline_object_info.pObjectName =
        point_light_pipeline_str_image_name;
    point_light_pipeline_object_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    point_light_pipeline_object_info.objectHandle = reinterpret_cast<u64>(
        PIPELINE_CONFIGS[SpecificPipeline::PointLightPipeline].pipeline
    );
    set_debug_object_name(device, &point_light_pipeline_object_info);
    VkDebugUtilsObjectNameInfoEXT point_light_pipeline_layout_object_info {};
    point_light_pipeline_layout_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String point_light_pipeline_layout_image_name =
        String(VK_DEBUG_PIPELINE_LAYOUT_RED)
        + " \x1b[31mPoint light pipeline layout #\x1b[0m " + std::to_string(0);
    const char* point_light_pipeline_layout_str_image_name =
        point_light_pipeline_layout_image_name.c_str();
    point_light_pipeline_layout_object_info.pObjectName =
        point_light_pipeline_layout_str_image_name;
    point_light_pipeline_layout_object_info.objectType =
        VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    point_light_pipeline_layout_object_info.objectHandle = reinterpret_cast<u64>(
        PIPELINE_CONFIGS[SpecificPipeline::PointLightPipeline].pipeline_layout
    );
    set_debug_object_name(device, &point_light_pipeline_layout_object_info);
    VkDebugUtilsObjectNameInfoEXT hud_uniform_buffer_object_info {};
    hud_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String hud_image_name = String(VK_DEBUG_IMAGE_SET_RED)
        + " Hud uniform buffer # " + std::to_string(0);
    const char* hud_str_image_name = hud_image_name.c_str();
    hud_uniform_buffer_object_info.pObjectName = hud_str_image_name;
    hud_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    const u32 hud_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::HUD]
            .descriptors_bindings_ids[0];
    hud_uniform_buffer_object_info.objectHandle = reinterpret_cast<u64>(
        gpu_descriptors
            [DESCRIPTOR_BINDINGS_CONFIG[hud_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->buffer
    );
    set_debug_object_name(device, &hud_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT font_uniform_buffer_object_info {};
    font_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String font_image_name = String(VK_DEBUG_IMAGE_SET_RED)
        + " Font uniform buffer # " + std::to_string(0);
    const char* font_str_image_name = font_image_name.c_str();
    font_uniform_buffer_object_info.pObjectName = font_str_image_name;
    font_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    const u32 font_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::FontRenderUbo]
            .descriptors_bindings_ids[0];
    font_uniform_buffer_object_info.objectHandle = reinterpret_cast<u64>(
        gpu_descriptors
            [DESCRIPTOR_BINDINGS_CONFIG[font_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->buffer
    );
    set_debug_object_name(device, &font_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT ui_uniform_buffer_object_info {};
    ui_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String ui_image_name = String(VK_DEBUG_IMAGE_SET_RED)
        + " UI uniform buffer # " + std::to_string(0);
    const char* ui_str_image_name = ui_image_name.c_str();
    ui_uniform_buffer_object_info.pObjectName = ui_str_image_name;
    ui_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    const u32 ui_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::UI]
            .descriptors_bindings_ids[0];
    ui_uniform_buffer_object_info.objectHandle = reinterpret_cast<u64>(
        gpu_descriptors[DESCRIPTOR_BINDINGS_CONFIG[ui_ubo_descriptor_binding_index]
                            .global_descriptor_offset]
            .gpu_buffer->buffer
    );
    set_debug_object_name(device, &ui_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT ui_icons_uniform_buffer_object_info {};
    ui_icons_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String ui_icons_image_name = String(VK_DEBUG_IMAGE_SET_RED)
        + " UI icons uniform buffer # " + std::to_string(0);
    const char* ui_icons_str_image_name = ui_icons_image_name.c_str();
    ui_icons_uniform_buffer_object_info.pObjectName = ui_icons_str_image_name;
    ui_icons_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    const u32 ui_icons_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::UiIcons]
            .descriptors_bindings_ids[0];
    ui_icons_uniform_buffer_object_info.objectHandle = reinterpret_cast<u64>(
        gpu_descriptors
            [DESCRIPTOR_BINDINGS_CONFIG[ui_icons_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->buffer
    );
    set_debug_object_name(device, &ui_icons_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT
        directional_light_uniform_buffer_object_info {};
    directional_light_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String directional_light_image_name = String(VK_DEBUG_IMAGE_SET_RED)
        + " Shadow map directional light model matrix uniform buffer # "
        + std::to_string(0);
    const char* directional_light_str_image_name =
        directional_light_image_name.c_str();
    directional_light_uniform_buffer_object_info.pObjectName =
        directional_light_str_image_name;
    directional_light_uniform_buffer_object_info.objectType =
        VK_OBJECT_TYPE_BUFFER;
    const u32 shadow_map_directional_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapDirectionalLight]
            .descriptors_bindings_ids[0];
    directional_light_uniform_buffer_object_info.objectHandle =
        reinterpret_cast<u64>(
            gpu_descriptors
                [DESCRIPTOR_BINDINGS_CONFIG
                     [shadow_map_directional_light_descriptor_binding_index]
                         .global_descriptor_offset]
                    .gpu_buffer->buffer
        );
    set_debug_object_name(device, &directional_light_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT point_light_uniform_buffer_object_info {};
    point_light_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String point_light_image_name = String(VK_DEBUG_IMAGE_SET_RED)
        + " Shadow map point light model matrix uniform buffer # "
        + std::to_string(0);
    const char* point_light_str_image_name = point_light_image_name.c_str();
    point_light_uniform_buffer_object_info.pObjectName =
        point_light_str_image_name;
    point_light_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    const u32 shadow_map_point_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapPointLight]
            .descriptors_bindings_ids[0];
    point_light_uniform_buffer_object_info.objectHandle = reinterpret_cast<u64>(
        gpu_descriptors[DESCRIPTOR_BINDINGS_CONFIG
                            [shadow_map_point_light_descriptor_binding_index]
                                .global_descriptor_offset]
            .gpu_buffer->buffer
    );
    set_debug_object_name(device, &point_light_uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT spot_light_uniform_buffer_object_info {};
    spot_light_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String spot_light_image_name = String(VK_DEBUG_IMAGE_SET_RED)
        + " Shadow map spot light model matrix uniform buffer # "
        + std::to_string(0);
    const char* spot_light_str_image_name = spot_light_image_name.c_str();
    spot_light_uniform_buffer_object_info.pObjectName =
        spot_light_str_image_name;
    spot_light_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    const u32 shadow_map_spot_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapSpotLight]
            .descriptors_bindings_ids[0];
    spot_light_uniform_buffer_object_info.objectHandle = reinterpret_cast<u64>(
        gpu_descriptors[DESCRIPTOR_BINDINGS_CONFIG
                            [shadow_map_spot_light_descriptor_binding_index]
                                .global_descriptor_offset]
            .gpu_buffer->buffer
    );
    set_debug_object_name(device, &spot_light_uniform_buffer_object_info);
    for (usize i = 0; i < vertex_buffer_container.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniform_buffer_object_info {};
        uniform_buffer_object_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        const String image_name = String(VK_DEBUG_IMAGE_SET_RED)
            + " Vertex uniform buffer # " + std::to_string(i);
        const char* str_image_name = image_name.c_str();
        uniform_buffer_object_info.pObjectName = str_image_name;
        uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
        uniform_buffer_object_info.objectHandle =
            reinterpret_cast<u64>(vertex_buffer_container[i]);
        set_debug_object_name(device, &uniform_buffer_object_info);
    }
    for (usize i = 0; i < index_buffer_container.size(); ++i) {
        VkDebugUtilsObjectNameInfoEXT uniform_buffer_object_info {};
        uniform_buffer_object_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        const String image_name = String(VK_DEBUG_IMAGE_SET_RED)
            + " Index uniform buffer # " + std::to_string(i);
        const char* str_image_name = image_name.c_str();
        uniform_buffer_object_info.pObjectName = str_image_name;
        uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
        uniform_buffer_object_info.objectHandle =
            reinterpret_cast<u64>(index_buffer_container[i]);
        set_debug_object_name(device, &uniform_buffer_object_info);
    }
    for (const auto i : font_indices_container) {
        VkDebugUtilsObjectNameInfoEXT uniform_buffer_object_info {};
        uniform_buffer_object_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        const String image_name = String(VK_DEBUG_IMAGE_SET_RED)
            + " Font vertex uniform buffer # " + std::to_string(i);
        const char* str_image_name = image_name.c_str();
        uniform_buffer_object_info.pObjectName = str_image_name;
        uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
        uniform_buffer_object_info.objectHandle =
            reinterpret_cast<u64>(font_vertex_buffer_container[i]);
        set_debug_object_name(device, &uniform_buffer_object_info);
    }
    for (const auto i : font_indices_container) {
        VkDebugUtilsObjectNameInfoEXT uniform_buffer_object_info {};
        uniform_buffer_object_info.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        const String image_name = String(VK_DEBUG_IMAGE_SET_RED)
            + " Font index uniform buffer # " + std::to_string(i);
        const char* str_image_name = image_name.c_str();
        uniform_buffer_object_info.pObjectName = str_image_name;
        uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
        uniform_buffer_object_info.objectHandle =
            reinterpret_cast<u64>(font_index_buffer_container[i]);
        set_debug_object_name(device, &uniform_buffer_object_info);
    }
    VkDebugUtilsObjectNameInfoEXT uniform_buffer_object_info {};
    uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String image_name = String(VK_DEBUG_IMAGE_SET_RED)
        + " Model matrix uniform buffer # " + std::to_string(0);
    const char* str_image_name = image_name.c_str();
    uniform_buffer_object_info.pObjectName = str_image_name;
    uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    uniform_buffer_object_info.objectHandle = reinterpret_cast<u64>(
        gpu_descriptors[DescriptorSetDataLink::MainRenderMatrixUbo]
            .gpu_buffer->buffer
    );
    set_debug_object_name(device, &uniform_buffer_object_info);
    VkDebugUtilsObjectNameInfoEXT light_data_uniform_buffer_object_info {};
    light_data_uniform_buffer_object_info.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    const String light_data_image_name = String(VK_DEBUG_IMAGE_SET_RED)
        + " Light data uniform buffer # " + std::to_string(0);
    const char* light_data_str_image_name = light_data_image_name.c_str();
    light_data_uniform_buffer_object_info.pObjectName =
        light_data_str_image_name;
    light_data_uniform_buffer_object_info.objectType = VK_OBJECT_TYPE_BUFFER;
    const u32 light_data_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_ids[0];
    light_data_uniform_buffer_object_info.objectHandle = reinterpret_cast<u64>(
        gpu_descriptors
            [DESCRIPTOR_BINDINGS_CONFIG[light_data_ubo_descriptor_binding_index]
                 .global_descriptor_offset]
                .gpu_buffer->buffer
    );
    set_debug_object_name(device, &light_data_uniform_buffer_object_info);
}
}; // namespace glvm

namespace glvm {
namespace {
// 16k verts, shared line + quad buffer.
constexpr auto MAX_DEBUG_VERTICES = 1 << 14;

auto push_line(
    Vec<DebugVertex>& out,
    const Vector<f32, 3>& a,
    const Vector<f32, 3>& b,
    const Vector<f32, 3>& color
) -> void {
    out.push_back(
        {.x = a[0],
         .y = a[1],
         .z = a[2],
         .r = color[0],
         .g = color[1],
         .b = color[2]}
    );
    out.push_back(
        {.x = b[0],
         .y = b[1],
         .z = b[2],
         .r = color[0],
         .g = color[1],
         .b = color[2]}
    );
}

auto push_cross(
    Vec<DebugVertex>& out,
    const Vector<f32, 3>& center,
    f32 size,
    const Vector<f32, 3>& color
) -> void {
    push_line(
        out,
        {center[0] - size, center[1], center[2]},
        {center[0] + size, center[1], center[2]},
        color
    );
    push_line(
        out,
        {center[0], center[1] - size, center[2]},
        {center[0], center[1] + size, center[2]},
        color
    );
    push_line(
        out,
        {center[0], center[1], center[2] - size},
        {center[0], center[1], center[2] + size},
        color
    );
}

auto push_axis(
    Vec<DebugVertex>& out,
    const Vector<f32, 3>& origin,
    const Vector<f32, 3>& target,
    f32 length,
    const Vector<f32, 3>& color
) -> void {
    const f32 dx = target[0] - origin[0];
    const f32 dy = target[1] - origin[1];
    const f32 dz = target[2] - origin[2];
    const f32 len = std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
    if (len > 0.0001f) {
        const Vector<f32, 3> tip = {
            origin[0] + (dx / len * length),
            origin[1] + (dy / len * length),
            origin[2] + (dz / len * length)
        };
        push_line(out, origin, tip, color);
    }
}

auto push_box(
    Vec<DebugVertex>& out,
    const Array<Vector<f32, 3>, 8>& corners,
    const Vector<f32, 3>& color
) -> void {
    static const Array<Array<u32, 2>, 12> EDGES = {
        {{0, 1},
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
         {3, 7}}
    };
    for (const auto& edge : EDGES) {
        push_line(out, corners[edge[0]], corners[edge[1]], color);
    }
}

auto to_vec4(const Vector<f32, 3>& v, f32 w) -> Vector<f32, 4> {
    return {v[0], v[1], v[2], w};
}

auto from_vec4(const Vector<f32, 4>& v) -> Vector<f32, 3> {
    return {v[0], v[1], v[2]};
}
} // namespace

ImGuiOverlay::ImGuiOverlay(Renderer& renderer) : renderer(renderer) {
}

auto ImGuiOverlay::wants_mouse() const -> bool {
    if (!initialized) {
        return false;
    }
    const ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse || io.WantCaptureKeyboard;
}

auto ImGuiOverlay::init() -> void {
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
    ImGui_ImplWin32_Init(
        static_cast<WindowWinVulkan*>(renderer.window)->get_modern_window_hwnd()
    );
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
    init_info.ImageCount = as<u32>(renderer.swap_chain_images.size());
    init_info.PipelineInfoMain.RenderPass = render_pass;
    init_info.PipelineInfoMain.Subpass = 0;
    if (!ImGui_ImplVulkan_Init(&init_info)) {
        throw std::runtime_error("failed to init ImGui vulkan backend!");
    }
    initialized = true;
    create_swap_chain_resources();
}

auto ImGuiOverlay::shutdown() -> void {
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

auto ImGuiOverlay::create_swap_chain_resources() -> void {
    if (!initialized) {
        return;
    }
    framebuffers.resize(renderer.swap_chain_image_views.size());
    for (usize i = 0; i < framebuffers.size(); ++i) {
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

auto ImGuiOverlay::destroy_swap_chain_resources() -> void {
    for (VkFramebuffer& framebuffer : framebuffers) {
        vkDestroyFramebuffer(renderer.device, framebuffer, nullptr);
    }
    framebuffers.clear();
}

auto ImGuiOverlay::new_frame() -> void {
    if (!initialized) {
        return;
    }
#ifdef _WIN32
    ImGui_ImplWin32_NewFrame();
#else
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(
        as<f32>(renderer.window->width),
        as<f32>(renderer.window->height)
    );
    static auto last_frame_time = std::chrono::steady_clock::now();
    const auto now_time = std::chrono::steady_clock::now();
    io.DeltaTime =
        std::chrono::duration<f32>(now_time - last_frame_time).count();
    last_frame_time = now_time;
    // No OS cursor plumbing yet: let ImGui draw its own cursor.
    io.MouseDrawCursor = true;
    io.AddMousePosEvent(
        global_event.mouse_pointer_position.offset_x,
        global_event.mouse_pointer_position.offset_y
    );
    io.AddMouseButtonEvent(
        ImGuiMouseButton_Left,
        !global_event.is_left_mouse_button_released
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

auto ImGuiOverlay::record_command_buffer(
    VkCommandBuffer command_buffer,
    u32 image_index
) -> void {
    if (!initialized) {
        return;
    }
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = framebuffers[image_index];
    render_pass_info.renderArea.offset = {.x = 0, .y = 0};
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
    viewport.width = as<f32>(renderer.swap_chain_extent.width);
    viewport.height = as<f32>(renderer.swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    VkRect2D scissor {};
    scissor.offset = {.x = 0, .y = 0};
    scissor.extent = renderer.swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    if (line_vertex_count > 0) {
        vkCmdBindPipeline(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            line_pipeline
        );
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, &offset);
        Matrix<f32, 4> view_proj =
            renderer.view_matrix * renderer.projection_matrix;
        vkCmdPushConstants(
            command_buffer,
            line_layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(Matrix<f32, 4>),
            &view_proj
        );
        vkCmdDraw(command_buffer, line_vertex_count, 1, 0, 0);
    }
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
    vkCmdEndRenderPass(command_buffer);
}

auto ImGuiOverlay::build_panel() -> void {
    if (!show_panel) {
        return;
    }
    ImGui::Begin("Debug overlay", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Checkbox("Show colliders", &show_colliders);
    ImGui::Checkbox("Actor bounds", &show_actor_bounds);
    ImGui::Checkbox("Light frustums", &show_light_frustums);
    ImGui::Checkbox("Light gizmos", &show_light_gizmos);
    ImGui::Checkbox("Spatial grid", &show_spatial_grid);
    ImGui::Checkbox("Ambient light", &ambient_enabled);
    ImGui::Checkbox("Directional light", &directional_enabled);
    ImGui::Checkbox("Point lights", &point_enabled);
    ImGui::Checkbox("Spot lights", &spot_enabled);
    ImGui::Checkbox("Shadows", &shadows_enabled);
    ImGui::Checkbox("Shadow maps", &show_shadow_maps);
    if (show_shadow_maps) {
        ImGui::RadioButton("Directional", &shadow_map_mode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Spot", &shadow_map_mode, 1);
        const i32 max_light = (shadow_map_mode == 0)
            ? as<i32>(renderer.directional_light_number)
            : as<i32>(renderer.spot_light_number);
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
    ImGui::Checkbox("Wireframe", &wireframe_enabled);
    if (ImGui::Checkbox("VSync", &vsync_enabled)) {
        renderer.recreate_swap_chain();
    }
    ImGui::Checkbox("Pause time", &pause_time);
    ImGui::Checkbox("Stats", &show_stats);
    if (show_stats) {
        ImGui::Separator();
        const u32 draw_calls = as<u32>(renderer.actors.size());
        usize triangle_count = 0;
        for (auto& actor : renderer.actors) {
            const u32 mesh_id = actor.mesh_id;
            if (mesh_id < renderer.indices.size()) {
                triangle_count += renderer.indices[mesh_id].size() / 3;
            }
        }
        const f32 frame_ms = ImGui::GetIO().DeltaTime * 1000.0f;
        frame_time_history.push_back(frame_ms);
        if (frame_time_history.size() > 120) {
            frame_time_history.erase(frame_time_history.begin());
        }
        f32 total_ms = 0.0f;
        for (const auto i : frame_time_history) {
            total_ms += i;
        }
        const f32 avg_ms = frame_time_history.empty()
            ? 0.0f
            : total_ms / as<f32>(frame_time_history.size());
        const f32 avg_fps = avg_ms > 0.0f ? 1000.0f / avg_ms : 0.0f;
        ImGui::Text("Frame Time: %.1f ms (avg %.1f ms)", frame_ms, avg_ms);
        ImGui::Text(
            "FPS: %.0f (avg %.0f)",
            as<f64>(ImGui::GetIO().Framerate),
            as<f64>(avg_fps)
        );
        ImGui::PlotLines(
            "##frame_time",
            [](void* data, int idx) -> float {
                auto* history = as<Vec<f32>*>(data);
                return (*history)[as<usize>(idx)];
            },
            &frame_time_history,
            as<i32>(frame_time_history.size())
        );
        ImGui::Text("Draw Calls: %u", draw_calls);
        ImGui::Text("Triangles: %zu", triangle_count);
        ImGui::Text("Actors: %zu", renderer.actors.size());
        ImGui::Text(
            "Dir lights: %u, Spot lights: %u",
            renderer.directional_light_number,
            renderer.spot_light_number
        );
    }
    ImGui::Separator();
    if (ImGui::Button("Hide panel (F1)")) {
        show_panel = false;
    }
    ImGui::End();
}

auto ImGuiOverlay::build_debug_vertices() -> void {
    Vec<DebugVertex> vertices;
    vertices.reserve(MAX_DEBUG_VERTICES);
    line_vertex_count = 0;
    if (show_actor_bounds || show_colliders) {
        const Vector<f32, 3> green = {0.0f, 1.0f, 0.0f};
        const Vector<f32, 3> red = {1.0f, 0.0f, 0.0f};
        const Vector<f32, 3> orange = {1.0f, 0.5f, 0.0f};
        Vec<Vector<f32, 3>> mins;
        Vec<Vector<f32, 3>> maxs;
        mins.reserve(renderer.actors.size());
        maxs.reserve(renderer.actors.size());
        for (const auto& actor : renderer.actors) {
            if (actor.mesh_id >= all_mesh_max_absolute_values.size()) {
                mins.emplace_back(0, 0, 0);
                maxs.emplace_back(0, 0, 0);
                continue;
            }
            const MeshAxisMaxAbsoluteValues& bounds =
                all_mesh_max_absolute_values[actor.mesh_id];
            const Vector<f32, 3> center = {
                bounds.origin_offset_x,
                bounds.origin_offset_y,
                bounds.origin_offset_z
            };
            const Vector<f32, 3> half =
                {bounds.absolute_x, bounds.absolute_y, bounds.absolute_z};
            Array<Vector<f32, 3>, 8> local_corners = {
                center + Vector<f32, 3>(-half[0], -half[1], -half[2]),
                center + Vector<f32, 3>(half[0], -half[1], -half[2]),
                center + Vector<f32, 3>(half[0], half[1], -half[2]),
                center + Vector<f32, 3>(-half[0], half[1], -half[2]),
                center + Vector<f32, 3>(-half[0], -half[1], half[2]),
                center + Vector<f32, 3>(half[0], -half[1], half[2]),
                center + Vector<f32, 3>(half[0], half[1], half[2]),
                center + Vector<f32, 3>(-half[0], half[1], half[2])
            };
            Vector<f32, 3> mn =
                from_vec4(to_vec4(local_corners[0], 1.0f) * actor.model_matrix);
            Vector<f32, 3> mx = mn;
            for (i32 c = 1; c < 8; ++c) {
                Vector<f32, 3> w = from_vec4(
                    to_vec4(local_corners[c], 1.0f) * actor.model_matrix
                );
                for (i32 a = 0; a < 3; ++a) {
                    mn[a] = std::min(mn[a], w[a]);
                    mx[a] = std::max(mx[a], w[a]);
                }
            }
            mins.push_back(mn);
            maxs.push_back(mx);
        }
        Vec<bool> collides(renderer.actors.size(), false);
        for (usize i = 0; i < renderer.actors.size(); ++i) {
            for (usize j = i + 1; j < renderer.actors.size(); ++j) {
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
        for (usize i = 0; i < renderer.actors.size(); ++i) {
            if (renderer.actors[i].mesh_id
                >= all_mesh_max_absolute_values.size()) {
                continue;
            }
            const MeshAxisMaxAbsoluteValues& bounds =
                all_mesh_max_absolute_values[renderer.actors[i].mesh_id];
            const Vector<f32, 3> center = {
                bounds.origin_offset_x,
                bounds.origin_offset_y,
                bounds.origin_offset_z
            };
            const Vector<f32, 3> half =
                {bounds.absolute_x, bounds.absolute_y, bounds.absolute_z};
            Array<Vector<f32, 3>, 8> local_corners = {
                center + Vector<f32, 3>(-half[0], -half[1], -half[2]),
                center + Vector<f32, 3>(half[0], -half[1], -half[2]),
                center + Vector<f32, 3>(half[0], half[1], -half[2]),
                center + Vector<f32, 3>(-half[0], half[1], -half[2]),
                center + Vector<f32, 3>(-half[0], -half[1], half[2]),
                center + Vector<f32, 3>(half[0], -half[1], half[2]),
                center + Vector<f32, 3>(half[0], half[1], half[2]),
                center + Vector<f32, 3>(-half[0], half[1], half[2])
            };
            Array<Vector<f32, 3>, 8> world_corners;
            for (i32 c = 0; c < 8; ++c) {
                world_corners[c] = from_vec4(
                    to_vec4(local_corners[c], 1.0f)
                    * renderer.actors[i].model_matrix
                );
            }
            if (show_actor_bounds) {
                push_box(vertices, world_corners, collides[i] ? red : green);
            } else if (collides[i]) {
                push_box(vertices, world_corners, orange);
            }
        }
    }

    if (show_light_frustums) {
        const Vector<f32, 3> yellow = {1.0f, 1.0f, 0.0f};
        for (u32 i = 0; i < renderer.directional_light_number; ++i) {
            Array<Vector<f32, 3>, 8> corners;
            const Matrix<f32, 4> inverse_light =
                inverse_matrix_4x4(renderer.dir_light_space_matrix[i]);
            for (i32 c = 0; c < 8; ++c) {
                const auto s = ((c & 4) != 0) ? 1.0f : -1.0f; // z (near/far).
                const auto u = ((c & 2) != 0) ? 1.0f : -1.0f; // y.
                const auto v = ((c & 1) != 0) ? 1.0f : -1.0f; // x.
                corners[c] =
                    from_vec4(Vector<f32, 4>(u, v, s, 1.0f) * inverse_light);
            }
            push_box(vertices, corners, yellow);
        }
        const Vector<f32, 3> cyan = {0.0f, 1.0f, 1.0f};
        for (u32 i = 0; i < renderer.spot_light_number; ++i) {
            Array<Vector<f32, 3>, 8> corners;
            const Matrix<f32, 4> inverse_light =
                inverse_matrix_4x4(renderer.spot_light_space_matrix[i]);
            for (i32 c = 0; c < 8; ++c) {
                const auto s = ((c & 4) != 0) ? 1.0f : -1.0f;
                const auto u = ((c & 2) != 0) ? 1.0f : -1.0f;
                const auto v = ((c & 1) != 0) ? 1.0f : -1.0f;
                corners[c] =
                    from_vec4(Vector<f32, 4>(u, v, s, 1.0f) * inverse_light);
            }
            push_box(vertices, corners, cyan);
        }
    }
    if (show_light_gizmos) {
        const f32 gizmo_size = 0.5f;
        const f32 gizmo_axis_length = 3.0f;
        const Vector<f32, 3> white = {1.0f, 1.0f, 1.0f};
        const Vector<f32, 3> magenta = {1.0f, 0.0f, 1.0f};
        const Vector<f32, 3> orange = {1.0f, 0.5f, 0.0f};
        for (auto& directional_light : renderer.directional_lights) {
            Vector<f32, 4> raw_pos = directional_light.position;
            const Vector<f32, 3> pos = {raw_pos[0], raw_pos[1], raw_pos[2]};
            push_cross(vertices, pos, gizmo_size, white);
            Vector<f32, 4> raw_target = directional_light.direction;
            const Vector<f32, 3> target =
                {raw_target[0], raw_target[1], raw_target[2]};
            push_axis(vertices, pos, target, gizmo_axis_length, white);
        }
        for (auto& point_light : renderer.point_lights) {
            const Vector<f32, 3> pos = point_light.position;
            push_cross(vertices, pos, gizmo_size, magenta);
        }
        for (auto& spot_light : renderer.spot_lights) {
            const Vector<f32, 3> pos = spot_light.position;
            push_cross(vertices, pos, gizmo_size, orange);
            const Vector<f32, 3> target = spot_light.direction;
            push_axis(vertices, pos, target, gizmo_axis_length, orange);
        }
    }
    if (show_spatial_grid) {
        const auto& grid = glvm::world.spatial_grid;
        const auto half_chunk = grid.grid[0][0][0].SIZE * 0.5f;
        const auto cross = 1.5f;
        for (const auto& z : grid.grid) {
            for (const auto& y : z) {
                for (const auto& chunk : y) {
                    const auto count = chunk.entities.size();
                    if (count == 0) {
                        continue;
                    }
                    // Only occupied cells, colored by entity count.
                    const Vector<f32, 3> color = count == 1
                        ? Vector<f32, 3>(0.0f, 1.0f, 0.0f)
                        : count <= 3 ? Vector<f32, 3>(1.0f, 1.0f, 0.0f)
                                     : Vector<f32, 3>(1.0f, 0.0f, 0.0f);
                    const Vector<f32, 3> center = chunk.position
                        + Vector<f32, 3>(half_chunk, half_chunk, half_chunk);
                    push_line(
                        vertices,
                        center - Vector<f32, 3>(cross, 0, 0),
                        center + Vector<f32, 3>(cross, 0, 0),
                        color
                    );
                    push_line(
                        vertices,
                        center - Vector<f32, 3>(0, cross, 0),
                        center + Vector<f32, 3>(0, cross, 0),
                        color
                    );
                    push_line(
                        vertices,
                        center - Vector<f32, 3>(0, 0, cross),
                        center + Vector<f32, 3>(0, 0, cross),
                        color
                    );
                }
            }
        }
    }

    line_vertex_count = as<u32>(vertices.size());

    if (!vertices.empty() && vertex_buffer_mapped) {
        const auto bytes = vertices.size() * sizeof(DebugVertex);
        const auto capacity = MAX_DEBUG_VERTICES * sizeof(DebugVertex);
        memcpy(vertex_buffer_mapped, vertices.data(), std::min(bytes, capacity));
    }
}

auto ImGuiOverlay::create_render_pass() -> void {
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
    Array<VkSubpassDependency, 2> dependencies {};
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
    render_pass_info.pDependencies = dependencies.data();
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

auto ImGuiOverlay::create_line_pipeline() -> void {
    auto create_shader_module = [&](const char* path) -> VkShaderModule {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error(String("failed to open shader: ") + path);
        }
        file.seekg(0, std::ios::beg);
        Vec<char> code(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        VkShaderModuleCreateInfo create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const u32*>(code.data());
        VkShaderModule module = nullptr;
        if (vkCreateShaderModule(renderer.device, &create_info, nullptr, &module)
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return module;
    };

    VkShaderModule vert =
        create_shader_module(GLVM_SHADER_DIR "/debug/debug_vert.spv");
    VkShaderModule frag =
        create_shader_module(GLVM_SHADER_DIR "/debug/debug_frag.spv");

    Array<VkPipelineShaderStageCreateInfo, 2> shader_stages {};
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

    Array<VkVertexInputAttributeDescription, 2> attribute_descriptions {};
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
        as<u32>(attribute_descriptions.size());
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
    push_constant_range.size = sizeof(Matrix<f32, 4>);

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

    Array<VkDynamicState, 2> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamic_state {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states.data();

    VkGraphicsPipelineCreateInfo pipeline_info {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages.data();
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

auto ImGuiOverlay::create_vertex_buffer() -> void {
    const VkDeviceSize buffer_size = MAX_DEBUG_VERTICES * sizeof(DebugVertex);
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
Renderer::Renderer() : imgui_overlay(new ImGuiOverlay(*this)) {
}

Renderer::~Renderer() {
    cleanup();
    delete imgui_overlay;
    imgui_overlay = nullptr;
}

auto Renderer::draw() -> void {
    imgui_overlay->new_frame();
    main_render_draw_frame();
}

auto Renderer::set_view_matrix(Matrix<f32, 4> new_view_matrix) -> void {
    view_matrix = new_view_matrix;
}

auto Renderer::set_projection_matrix(Matrix<f32, 4> new_projection_matrix)
    -> void {
    projection_matrix = new_projection_matrix;
}

auto Renderer::extract_frustum(const Matrix<f32, 4>& vp) -> Frustum {
    Frustum frustum;
    frustum.planes[as<usize>(PlaneIndex::Left)] = {
        .x = vp[0][3] + vp[0][0],
        .y = vp[1][3] + vp[1][0],
        .z = vp[2][3] + vp[2][0],
        .w = vp[3][3] + vp[3][0]
    };
    frustum.planes[as<usize>(PlaneIndex::Right)] = {
        .x = vp[0][3] - vp[0][0],
        .y = vp[1][3] - vp[1][0],
        .z = vp[2][3] - vp[2][0],
        .w = vp[3][3] - vp[3][0]
    };
    frustum.planes[as<usize>(PlaneIndex::Bottom)] = {
        .x = vp[0][3] + vp[0][1],
        .y = vp[1][3] + vp[1][1],
        .z = vp[2][3] + vp[2][1],
        .w = vp[3][3] + vp[3][1]
    };
    frustum.planes[as<usize>(PlaneIndex::Top)] = {
        .x = vp[0][3] - vp[0][1],
        .y = vp[1][3] - vp[1][1],
        .z = vp[2][3] - vp[2][1],
        .w = vp[3][3] - vp[3][1]
    };
    frustum.planes[as<usize>(PlaneIndex::Near)] = {
        .x = vp[0][3] + vp[0][2],
        .y = vp[1][3] + vp[1][2],
        .z = vp[2][3] + vp[2][2],
        .w = vp[3][3] + vp[3][2]
    };
    frustum.planes[as<usize>(PlaneIndex::Far)] = {
        .x = vp[0][3] - vp[0][2],
        .y = vp[1][3] - vp[1][2],
        .z = vp[2][3] - vp[2][2],
        .w = vp[3][3] - vp[3][2]
    };
    for (Plane& plane : frustum.planes) {
        plane = normalize(plane);
    }
    return frustum;
}

auto Renderer::create_texture_image() -> void {
    u32 tex_width = 0;
    u32 tex_height = 0;
    const u32 tex_channels = 0;

    const u32 readable_texture_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ReadableTextures]
            .descriptors_bindings_ids[0];
    for (u32 i = 0; i < initialize_texture_data.size(); ++i) {
        VkDeviceSize image_size {};
        const u8* pixels = nullptr;
        const char* path_to_stb_image = nullptr;

#ifndef STB_IMAGE_IMPLEMENTATION
        image_size = initialize_texture_data[i].data_length;
        pixels = initialize_texture_data[i].data;
        tex_width = initialize_texture_data[i].width;
        tex_height = initialize_texture_data[i].height;
#endif

#ifdef STB_IMAGE_IMPLEMENTATION
        path_to_stb_image = initializeTextureData_[i].path_to_image;
        pixels = stbi_load(
            path_to_stb_image,
            reinterpret_cast<i32*>(&tex_width),
            reinterpret_cast<i32*>(&tex_height),
            reinterpret_cast<i32*>(&tex_channels),
            STBI_rgb_alpha
        );
        image_size = tex_width * tex_height * 4;
#endif

        if (pixels == nullptr) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer staging_buffer = nullptr;
        VkDeviceMemory staging_buffer_memory = nullptr;
        create_buffer(
            image_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_buffer_memory
        );

        void* data = nullptr;
        vkMapMemory(device, staging_buffer_memory, 0, image_size, 0, &data);
        memcpy(data, pixels, as<usize>(image_size));
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
            as<u32>(tex_width),
            as<u32>(tex_height)
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

auto Renderer::recreate_swap_chain() -> void {
    vkDeviceWaitIdle(device);

#ifdef VK_USE_PLATFORM_XCB_KHR
    if (window_system == WindowSystem::XCB) {
        static_cast<WindowXCBVulkan*>(window)->configure_window();
    }
#endif

    imgui_overlay->destroy_swap_chain_resources();
    cleanup_swap_chain();

    create_swap_chain();
    window->width = swap_chain_extent.width;
    window->height = swap_chain_extent.height;
    aspect_ratio = as<f32>(window->width) / as<f32>(window->height);
    create_image_views();
    create_depth_resources();
    create_directional_light_shadow_map_depth_resources();
    create_spot_light_shadow_map_depth_resources();
    create_point_light_shadow_map_depth_resources();
    create_framebuffers();
    create_main_render_descriptor_sets();
    imgui_overlay->create_swap_chain_resources();
}

auto Renderer::set_mesh_data(Vec<const char*> paths, Vec<const char*> paths_gltf)
    -> void {
    for (const auto* path : paths) {
        paths_array.push_back(path);
    }

    for (u32 i = 0; i < paths_gltf.size(); ++i) {
        paths_gltf.push_back(paths_gltf[i]);
    }
}

auto Renderer::run() -> void {
    vk_config_initializer();
    descriptor_set_builder();
    pipeline_builder();
    render_passes_builder();
    render_thread_pool = new ThreadPool(3);
    start_time = std::chrono::steady_clock::now();

    init_window();
    init_vulkan();
}

#ifdef __linux__
struct WindowXVulkan: public WindowInterface {
private:
    XWindowAttributes x_window_attributes;
    Window root_window;
    XSetWindowAttributes set_window_attributes;

public:
    ::Display* display;
    Window win;

    WindowXVulkan();
    ~WindowXVulkan();

    auto get_window() -> Window;
    auto get_display() -> ::Display*;
    auto cursor_lock(
        i32 pointer_x,
        i32 pointer_y,
        i32* out_offset_x,
        i32* out_offset_y
    ) -> void override;
    auto swap_buffers() -> void override;
    auto clear_display() -> void override;
    auto handle_event(Event& event) -> bool override;
    auto close() -> void override;
};

// Kept out of the header with WindowXVulkan: glvm.hpp must not pull in Xlib,
// whose global Font typedef and None/Bool macros break consumers.
static VkXlibSurfaceCreateInfoKHR create_xlib_surface_info {};

namespace {
auto env_matches(const char* name, const char* value) -> bool {
    const char* env = std::getenv(name);
    return env != nullptr && strcasecmp(env, value) == 0;
}

// Wayland wins over XWayland; GLVM_WINDOW_SYSTEM=wayland|x11|xcb forces one.
auto detect_window_system() -> WindowSystem {
    if (env_matches("GLVM_WINDOW_SYSTEM", "wayland")) {
        return WindowSystem::WAYLAND;
    }
    if (env_matches("GLVM_WINDOW_SYSTEM", "x11")) {
        return WindowSystem::X11;
    }
    if (env_matches("GLVM_WINDOW_SYSTEM", "xcb")) {
        return WindowSystem::XCB;
    }
    if (std::getenv("WAYLAND_DISPLAY") != nullptr) {
        return WindowSystem::WAYLAND;
    }
    if (std::getenv("DISPLAY") != nullptr) {
        return WindowSystem::XCB;
    }
    return WindowSystem::WAYLAND;
}
} // namespace
#endif

auto Renderer::init_window() -> void {
#ifdef _WIN32
    window_system = WindowSystem::WINDOWS;
    auto* win32 = new WindowWinVulkan();
    window = win32;
    create_win32_surface_info.hwnd = win32->get_modern_window_hwnd();
    create_win32_surface_info.sType =
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_win32_surface_info.pNext = nullptr;
    create_win32_surface_info.flags = 0;
#endif

#ifdef __linux__
    window_system = detect_window_system();
    switch (window_system) {
        case WindowSystem::WAYLAND: {
            auto* wayland = initialize_wayland_window();
            window = wayland;
            create_wayland_surface_info.display = wayland->display;
            create_wayland_surface_info.surface = wayland->wl_surface;
            create_wayland_surface_info.sType =
                VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            create_wayland_surface_info.pNext = nullptr;
            create_wayland_surface_info.flags = 0;
            break;
        }
        case WindowSystem::X11: {
            auto* x11 = new WindowXVulkan();
            window = x11;
            create_xlib_surface_info.dpy = x11->get_display();
            create_xlib_surface_info.window = x11->get_window();
            create_xlib_surface_info.sType =
                VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
            create_xlib_surface_info.pNext = nullptr;
            create_xlib_surface_info.flags = 0;
            break;
        }
        case WindowSystem::XCB: {
            auto* xcb = new WindowXCBVulkan();
            window = xcb;
            create_xcb_surface_info.window = xcb->get_window();
            create_xcb_surface_info.connection = xcb->get_connection();
            create_xcb_surface_info.sType =
                VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            create_xcb_surface_info.pNext = nullptr;
            create_xcb_surface_info.flags = 0;
            break;
        }
        case WindowSystem::WINDOWS:
            break;
    }
#endif

    aspect_ratio = as<f32>(window->width) / as<f32>(window->height);
}

auto Renderer::initialize_game_level_vertices() -> void {
    for (u32 m = 0; m < level_generated_vertices.size(); ++m) {
        vertices.push_back(level_generated_vertices[m]);
        indices.push_back(level_generated_indices[m]);
        joint_matrices_per_mesh.emplace_back();
        frames.emplace_back();
        for (i32 i = 0; i < 64; ++i) {
            frames[frames.size() - 1].push_back(0.0f);
        }
        const i32 maximum_joints = 64;
        Vec<Vec<Matrix<f32, 4>>> joint_matrices;
        for (i32 i = 0; i < maximum_joints; ++i) {
            Vec<Matrix<f32, 4>> global_all_frame_node_matrix;
            const i32 number_of_frames = 64;
            for (i32 j = 0; j < number_of_frames; ++j) {
                const Matrix<f32, 4> unit_matrix(1.0f);
                global_all_frame_node_matrix.push_back(unit_matrix);
            }

            joint_matrices.push_back(global_all_frame_node_matrix);
        }
        joint_matrices_per_mesh[joint_matrices_per_mesh.size() - 1] =
            joint_matrices;

        const u32 next_index_gltf = wavefront_obj_counter + gltf_counter + m;

        vertex_buffer_container.emplace_back();
        vertex_buffer_memory_container.emplace_back();
        create_vertex_buffer(
            vertex_buffer_container[next_index_gltf],
            vertex_buffer_memory_container[next_index_gltf],
            vertices[next_index_gltf]
        );

        index_buffer_container.emplace_back();
        index_buffer_memory_container.emplace_back();
        create_index_buffer(
            index_buffer_container[next_index_gltf],
            index_buffer_memory_container[next_index_gltf],
            indices[next_index_gltf]
        );
    }
}

auto Renderer::init_vulkan() -> void {
    if (volkInitialize() != VK_SUCCESS) {
        throw std::runtime_error("failed to initialize volk!");
    }
    create_instance();
    volkLoadInstance(instance);
    setup_debug_messenger();
    create_surface();
    pick_physical_device();
    create_logical_device();
    volkLoadDevice(device);
    create_swap_chain();
    create_image_views();
    create_main_render_pass();
    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_command_pool(main_render_command_pool);
    const auto secondary_buffers_command_pools_number = 3;
    secondary_buffers_command_pools.resize(
        secondary_buffers_command_pools_number
    );
    for (auto& secondary_buffers_command_pool :
         secondary_buffers_command_pools) {
        create_command_pool(secondary_buffers_command_pool);
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
    initialize_vertex_buffers_with_math_objects_data();
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
    const auto main_render_command_buffers_number = 1;
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

    map_memory_ubo(
        DescriptorSetDataLink::ShadowMapDirectionalLight,
        sizeof(ShadowMapMatrixUBO)
    );
    map_memory_ubo(
        DescriptorSetDataLink::ShadowMapSpotLight,
        sizeof(ShadowMapMatrixUBO)
    );
    map_memory_ubo(
        DescriptorSetDataLink::ShadowMapPointLight,
        sizeof(PointLightShadowMapMatrixUBO)
    );
    map_memory_ubo(
        DescriptorSetDataLink::MainRenderMatrixUbo,
        sizeof(ModelMatrixUBO)
    );
    map_memory_ubo(DescriptorSetDataLink::HUD, sizeof(HudUbo));
    map_memory_ubo(DescriptorSetDataLink::FontRenderUbo, sizeof(FontUbo));
    map_memory_ubo(DescriptorSetDataLink::HudScreen, sizeof(HudScreenUbo));
    map_memory_ubo(DescriptorSetDataLink::UI, sizeof(UiUbo));
    map_memory_ubo(DescriptorSetDataLink::UiIcons, sizeof(UiUbo));
    map_memory_ubo(
        DescriptorSetDataLink::MathObjectsDebugData,
        sizeof(MathObjectDebugUbo)
    );
    map_memory_ubo(
        DescriptorSetDataLink::MainRenderLightDataUbo,
        sizeof(LightData)
    );
}

auto Renderer::initialize_vertex_buffers_with_wavefront_data() -> void {
    for (u32 m = 0; m < paths_array.size(); ++m) {
        vertex_buffer_container.emplace_back();
        vertex_buffer_memory_container.emplace_back();
        create_vertex_buffer(
            vertex_buffer_container[m],
            vertex_buffer_memory_container[m],
            vertices[m]
        );

        index_buffer_container.emplace_back();
        index_buffer_memory_container.emplace_back();
        create_index_buffer(
            index_buffer_container[m],
            index_buffer_memory_container[m],
            indices[m]
        );
        ++wavefront_obj_counter;
    }
}

auto Renderer::initialize_vertex_buffers_with_gltf_data() -> void {
    for (u32 m = 0; m < paths_gltf.size(); ++m) {
        const u32 next_index_gltf = wavefront_obj_counter + m;
        glvm_log::info(
            "glvm",
            "gltf upload {}: vertices={} indices={}",
            paths_gltf[m],
            vertices[next_index_gltf].size(),
            indices[next_index_gltf].size()
        );
        vertex_buffer_container.emplace_back();
        vertex_buffer_memory_container.emplace_back();
        create_vertex_buffer(
            vertex_buffer_container[next_index_gltf],
            vertex_buffer_memory_container[next_index_gltf],
            vertices[next_index_gltf]
        );

        index_buffer_container.emplace_back();
        index_buffer_memory_container.emplace_back();
        create_index_buffer(
            index_buffer_container[next_index_gltf],
            index_buffer_memory_container[next_index_gltf],
            indices[next_index_gltf]
        );
        ++gltf_counter;
    }
}

auto Renderer::initialize_vertex_buffers_with_font_data() -> void {
    for (u32 i = 0; i < symbol_g_vertices_container.size(); ++i) {
        const auto next_buffer_index = font_indices_container[i];
        Vec<Vertex> symbol_g_vertices = symbol_g_vertices_container[i];

        create_vertex_buffer(
            font_vertex_buffer_container[next_buffer_index],
            font_vertex_buffer_memory_container[next_buffer_index],
            symbol_g_vertices
        );
        create_index_buffer(
            font_index_buffer_container[next_buffer_index],
            font_index_buffer_memory_container[next_buffer_index],
            symbol_g_indices
        );
    }
}

auto Renderer::initialize_vertex_buffers_with_math_objects_data() -> void {
    for (u32 m = 0; m < math_objects_vertices.size(); ++m) {
        math_objects_vertex_buffer_container.emplace_back();
        math_objects_vertex_buffer_memory_container.emplace_back();
        create_vertex_buffer(
            math_objects_vertex_buffer_container[m],
            math_objects_vertex_buffer_memory_container[m],
            math_objects_vertices[m]
        );
        math_objects_index_buffer_container.emplace_back();
        math_objects_index_buffer_memory_container.emplace_back();
        create_index_buffer(
            math_objects_index_buffer_container[m],
            math_objects_index_buffer_memory_container[m],
            math_objects_indices[m]
        );
    }
}

auto Renderer::clear_vk_image(GpuImage* texture_images) -> void {
    vkDestroySampler(device, texture_images->sampler, nullptr);
    for (auto& view : texture_images->views) {
        vkDestroyImageView(device, view, nullptr);
    }

    texture_images->views.clear();

    vkDestroyImage(device, texture_images->image, nullptr);
    vkFreeMemory(device, texture_images->device_memory, nullptr);
}

auto Renderer::cleanup_swap_chain() -> void {
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

    for (const Vec<VkFramebuffer>& inner_vector :
         point_light_shadow_map_frame_buffers) {
        for (const VkFramebuffer& framebuffer : inner_vector) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
    }

    for (VkImageView& image_view : swap_chain_image_views) {
        vkDestroyImageView(device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(device, swap_chain, nullptr);
}

auto Renderer::cleanup() -> void {
    glvm_log::info("glvm", "cleanup start");
    cleanup_swap_chain();
    glvm_log::info("glvm", "cleanup: swap chain done");
    imgui_overlay->shutdown();
    glvm_log::info("glvm", "cleanup: imgui done");

    usize descriptor_index = 0;
    for (u32 binding_index = 0; descriptor_index < GPU_DESCRIPTORS.size()
         && binding_index < std::size(DESCRIPTOR_BINDINGS_CONFIG);
         ++binding_index) {
        const DescriptorBinding& binding =
            DESCRIPTOR_BINDINGS_CONFIG[binding_index];
        for (u32 descriptor_counter = 0;
             descriptor_counter < binding.shader_descriptors_number;
             ++descriptor_counter, ++descriptor_index) {
            if (binding.vk_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                const GpuBuffer* gpu_buffer =
                    GPU_DESCRIPTORS[descriptor_index].gpu_buffer;
                if (gpu_buffer->mapped_data_ptr != nullptr) {
                    vkUnmapMemory(device, gpu_buffer->device_memory);
                }
                vkDestroyBuffer(device, gpu_buffer->buffer, nullptr);
                vkFreeMemory(device, gpu_buffer->device_memory, nullptr);
                delete gpu_buffer;
                GPU_DESCRIPTORS[descriptor_index].gpu_buffer = nullptr;
            } else if (
                binding.vk_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            ) {
                GpuImage* gpu_image =
                    GPU_DESCRIPTORS[descriptor_index].gpu_image;
                if (gpu_image->views.size()) {
                    clear_vk_image(gpu_image);
                }
                delete gpu_image;
                GPU_DESCRIPTORS[descriptor_index].gpu_image = nullptr;
            }
        }
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

    for (usize j = 0; j < vertex_buffer_container.size(); ++j) {
        vkDestroyBuffer(device, vertex_buffer_container[j], nullptr);
        vkFreeMemory(device, vertex_buffer_memory_container[j], nullptr);
    }
    for (usize j = 0; j < index_buffer_container.size(); ++j) {
        vkDestroyBuffer(device, index_buffer_container[j], nullptr);
        vkFreeMemory(device, index_buffer_memory_container[j], nullptr);
    }
    for (const auto j : font_indices_container) {
        vkDestroyBuffer(device, font_vertex_buffer_container[j], nullptr);
        vkFreeMemory(device, font_vertex_buffer_memory_container[j], nullptr);
    }
    for (const auto j : font_indices_container) {
        vkDestroyBuffer(device, font_index_buffer_container[j], nullptr);
        vkFreeMemory(device, font_index_buffer_memory_container[j], nullptr);
    }
    for (usize j = 0; j < math_objects_vertex_buffer_container.size(); ++j) {
        vkDestroyBuffer(
            device,
            math_objects_vertex_buffer_container[j],
            nullptr
        );
        vkFreeMemory(
            device,
            math_objects_vertex_buffer_memory_container[j],
            nullptr
        );
    }
    for (usize j = 0; j < math_objects_index_buffer_container.size(); ++j) {
        vkDestroyBuffer(device, math_objects_index_buffer_container[j], nullptr);
        vkFreeMemory(
            device,
            math_objects_index_buffer_memory_container[j],
            nullptr
        );
    }
    vkDestroyBuffer(device, model_matrix_uniform_buffer, nullptr);
    vkFreeMemory(device, model_matrix_uniform_buffers_memory, nullptr);
    vkDestroyBuffer(device, light_data_uniform_buffer, nullptr);
    vkFreeMemory(device, light_data_uniform_buffers_memory, nullptr);
    glvm_log::info("glvm", "cleanup: descriptors and uniform buffers done");

    vkDeviceWaitIdle(device);

    for (i32 i = 0; i < SpecificPipeline::PipelinesNumber; ++i) {
        vkDestroyRenderPass(device, RENDER_PASSES[i], nullptr);
    }

    for (u32 i = 0; i < DescriptorSetDataLink::DescriptorChunksNumber; ++i) {
        vkDestroyDescriptorSetLayout(
            device,
            DESCRIPTOR_SETS_CONFIG[i].set_layout,
            nullptr
        );
    }
    if (main_wireframe_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, main_wireframe_pipeline, nullptr);
        main_wireframe_pipeline = VK_NULL_HANDLE;
    }
    for (u32 i = 0; i < SpecificPipeline::PipelinesNumber; ++i) {
        vkDestroyPipeline(device, PIPELINE_CONFIGS[i].pipeline, nullptr);
        vkDestroyPipelineLayout(
            device,
            PIPELINE_CONFIGS[i].pipeline_layout,
            nullptr
        );
    }

    vkDestroySampler(device, texture_sampler, nullptr);
    vkDestroySampler(device, shadow_map_sampler, nullptr);
    for (auto& texture_image : texture_images) {
        vkDestroySampler(device, texture_image.sampler, nullptr);
        for (auto& view : texture_image.views) {
            vkDestroyImageView(device, view, nullptr);
        }

        vkDestroyImage(device, texture_image.image, nullptr);
        vkFreeMemory(device, texture_image.device_memory, nullptr);
    }

    for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
        vkDestroyFence(device, in_flight_fences[i], nullptr);
    }

    for (usize i = 0; i < swap_chain_images.size(); ++i) {
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
    for (auto& secondary_buffers_command_pool :
         secondary_buffers_command_pools) {
        vkDestroyCommandPool(device, secondary_buffers_command_pool, nullptr);
    }
    vkDestroyDescriptorPool(device, descriptor_pool, nullptr);

    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, nullptr);

    if (enable_validation_layers) {
        destroy_debug_utils_messenger_ext(instance, debug_messenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glvm_log::info("glvm", "cleanup done, closing window");
    window->close();
}

auto Renderer::create_instance() -> void {
    if (enable_validation_layers && !check_validation_layer_support()) {
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

    Vec<const char*> extensions = get_required_extensions();

    create_info.enabledExtensionCount = as<u32>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
    if (enable_validation_layers) {
        create_info.enabledLayerCount = as<u32>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        populate_debug_messenger_create_info(debug_create_info);
        create_info.pNext = &debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;

        create_info.pNext = nullptr;
    }

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

auto Renderer::populate_debug_messenger_create_info(
    VkDebugUtilsMessengerCreateInfoEXT& create_info
) -> void {
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

auto Renderer::setup_debug_messenger() -> void {
    if (!enable_validation_layers) {
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

auto Renderer::create_surface() -> void {
    switch (window_system) {
        case WindowSystem::WAYLAND:
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
            break;
        case WindowSystem::X11:
#ifdef VK_USE_PLATFORM_XLIB_KHR
            if (vkCreateXlibSurfaceKHR(
                    instance,
                    &create_xlib_surface_info,
                    nullptr,
                    &surface
                )
                != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }
#endif
            break;
        case WindowSystem::XCB:
#ifdef VK_USE_PLATFORM_XCB_KHR
            if (vkCreateXcbSurfaceKHR(
                    instance,
                    &create_xcb_surface_info,
                    nullptr,
                    &surface
                )
                != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }
#endif
            break;
        case WindowSystem::WINDOWS:
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
            break;
    }
}

auto Renderer::pick_physical_device() -> void {
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (device_count == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    Vec<VkPhysicalDevice> devices(device_count);
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

auto Renderer::create_logical_device() -> void {
    QueueFamilyIndices indices = find_queue_families(physical_device);

    Vec<VkDeviceQueueCreateInfo> queue_create_infos;
    const BTreeSet<u32> unique_queue_families = {
        indices.graphics_family.value(),
        indices.present_family.value()
    };

    f32 queue_priority = 1.0f;
    for (const u32 queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities =
            reinterpret_cast<const float*>(&queue_priority);
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features {};
    device_features.samplerAnisotropy = VK_TRUE;
    device_features.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    create_info.queueCreateInfoCount = as<u32>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();

    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = as<u32>(DEVICE_EXTENSIONS.size());
    create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (enable_validation_layers) {
        create_info.enabledLayerCount = as<u32>(VALIDATION_LAYERS.size());
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

auto Renderer::create_swap_chain() -> void {
    const SwapChainSupportDetails swap_chain_support =
        query_swap_chain_support(physical_device);
    const VkSurfaceFormatKHR surface_format =
        choose_swap_surface_format(swap_chain_support.formats);
    const VkPresentModeKHR present_mode =
        choose_swap_present_mode(swap_chain_support.present_modes);
    const VkExtent2D extent =
        choose_swap_extent(swap_chain_support.capabilities);
    u32 image_count = swap_chain_support.capabilities.minImageCount + 1;
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
    Array<u32, 2> queue_family_indices = {
        indices.graphics_family.value(),
        indices.present_family.value()
    };
    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices.data();
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

auto Renderer::create_image_views() -> void {
    swap_chain_image_views.resize(swap_chain_images.size());
    for (u32 i = 0; i < swap_chain_images.size(); i++) {
        const GpuImage swap_chain_image = {
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

auto Renderer::create_main_render_pass() -> void {
    for (u32 j = 0; j < SpecificPipeline::PipelinesNumber; ++j) {
        for (u32 i = 0;
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
        for (u32 i = 0;
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
        render_pass_info.attachmentCount = as<u32>(
            RENDER_PASS_CONFIGS[j].actual_attachment_description_number
        );
        render_pass_info.pAttachments =
            RENDER_PASS_CONFIGS[j].attachment_descriptions.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount =
            RENDER_PASS_CONFIGS[j].actual_subpass_dependency_number;
        render_pass_info.pDependencies =
            RENDER_PASS_CONFIGS[j].subpass_dependencies.data();
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

auto Renderer::create_descriptor_set_layout() -> void {
    for (i32 descriptor_set_counter = 0;
         descriptor_set_counter < DescriptorSetDataLink::DescriptorChunksNumber;
         ++descriptor_set_counter) {
        DescriptorSet& descriptor_set =
            DESCRIPTOR_SETS_CONFIG[descriptor_set_counter];
        Vec<VkDescriptorSetLayoutBinding> bindings;
        for (u32 j = 0;
             j < descriptor_set.actual_linked_descriptor_bindings_number;
             ++j) {
            const u32 current_descriptor_binding_id =
                descriptor_set.descriptors_bindings_ids[j];
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
        layout_info.bindingCount = as<u32>(bindings.size());
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

auto Renderer::create_graphics_pipeline() -> void {
    for (i32 graphics_pipeline_counter = 0;
         graphics_pipeline_counter < SpecificPipeline::PipelinesNumber;
         ++graphics_pipeline_counter) {
        Pipeline& pipeline = PIPELINE_CONFIGS[graphics_pipeline_counter];
        VkRenderPass render_pass = RENDER_PASSES[graphics_pipeline_counter];
        Vec<VkPipelineShaderStageCreateInfo> shader_stages;
        VkShaderModule vert_shader_module;
        VkShaderModule frag_shader_module;
        if (pipeline.vert_shader != nullptr) {
            const Vec<char> vert_shader_code = read_file(pipeline.vert_shader);
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
            const Vec<char> frag_shader_code = read_file(pipeline.frag_shader);
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
            as<u32>(pipeline.attribute_descriptions.size());
        vertex_input_info.pVertexBindingDescriptions =
            &pipeline.binding_description;
        vertex_input_info.pVertexAttributeDescriptions =
            pipeline.attribute_descriptions.data();
        VkPipelineInputAssemblyStateCreateInfo input_assembly {};
        input_assembly.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = pipeline.topology;
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
        rasterizer.polygonMode = pipeline.polygon_mode;
        rasterizer.lineWidth = 1.0f;
        const bool is_shadow_map_pipeline = graphics_pipeline_counter
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
        Vec<VkDynamicState> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamic_state {};
        dynamic_state.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = as<u32>(dynamic_states.size());
        dynamic_state.pDynamicStates = dynamic_states.data();
        // Need to access inside the pipeline and take the ID of a specific
        // descriptor set, then with that ID we get the descriptor set and take
        // its layout.
        const u32 descriptor_layouts_number =
            pipeline.actual_linked_descriptor_sets_number;
        Vec<VkDescriptorSetLayout> descriptor_set_layouts;
        for (u32 i = 0; i < descriptor_layouts_number; ++i) {
            descriptor_set_layouts.push_back(
                DESCRIPTOR_SETS_CONFIG[pipeline.linked_descriptor_set_ids[i]]
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
        if (graphics_pipeline_counter == SpecificPipeline::MainRenderPipeline) {
            rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
            if (vkCreateGraphicsPipelines(
                    device,
                    VK_NULL_HANDLE,
                    1,
                    &pipeline_info,
                    nullptr,
                    &main_wireframe_pipeline
                )
                != VK_SUCCESS) {
                throw std::runtime_error("failed to create wireframe pipeline!");
            }
            rasterizer.polygonMode = pipeline.polygon_mode;
        }
        if (pipeline.vert_shader != nullptr) {
            vkDestroyShaderModule(device, vert_shader_module, nullptr);
        }
        if (pipeline.frag_shader != nullptr) {
            vkDestroyShaderModule(device, frag_shader_module, nullptr);
        }
    }
}

auto Renderer::create_framebuffers() -> void {
    // Main renderer framebuffers initialization.
    swap_chain_framebuffers.resize(swap_chain_image_views.size());
    for (usize i = 0; i < swap_chain_image_views.size(); ++i) {
        Vec<VkImageView> main_render_attachments;
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
    // Directional lights shadow map renderer framebuffers initialization.
    const u32 directional_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_ids[1];
    directional_light_shadow_map_frame_buffers.resize(DIRECTIONAL_LIGHTS_NUMBER);
    for (usize i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
        Vec<VkImageView> directional_lights_render_attachments;
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
    const u32 spot_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_ids[3];
    spot_light_shadow_map_frame_buffers.resize(SPOT_LIGHTS_NUMBER);
    for (usize i = 0; i < SPOT_LIGHTS_NUMBER; ++i) {
        Vec<VkImageView> spot_lights_render_attachments;
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
    const u32 descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_ids[2];
    point_light_shadow_map_frame_buffers.resize(POINT_LIGHTS_NUMBER);
    for (usize j = 0; j < POINT_LIGHTS_NUMBER; ++j) {
        for (usize m = 0; m < 6; ++m) {
            Vec<VkImageView> point_lights_render_attachments;
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

auto Renderer::create_render_pass_framebuffers(
    Vec<VkImageView>& attachments,
    VkRenderPass& render_pass,
    VkFramebuffer& swap_chain_framebuffer,
    u32 width,
    u32 height
) -> void {
    VkFramebufferCreateInfo framebuffer_info {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = as<u32>(attachments.size());
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

auto Renderer::create_command_pool(VkCommandPool& command_pools) -> void {
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

auto Renderer::create_depth_resources() -> void {
    const VkFormat depth_format = find_depth_format();

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

auto Renderer::create_directional_light_shadow_map_depth_resources() -> void {
    const u32 directional_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_ids[1];
    for (u32 i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
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

auto Renderer::create_spot_light_shadow_map_depth_resources() -> void {
    const u32 spot_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_ids[3];
    for (u32 i = 0; i < SPOT_LIGHTS_NUMBER; ++i) {
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

auto Renderer::create_point_light_shadow_map_depth_resources() -> void {
    const u32 descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_ids[2];
    for (u32 i = 0; i < POINT_LIGHTS_NUMBER; ++i) {
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

        for (u32 j = 0; j < 6; ++j) {
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

        set_image_debug_object_name(device, depth_image, "point light layer");
        *GPU_DESCRIPTORS
             [DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                  .global_descriptor_offset
              + i]
                 .gpu_image = depth_image;
    }

    for (u32 i = 0; i < POINT_LIGHTS_NUMBER; ++i) {
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

auto Renderer::find_supported_format(
    const Vec<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features
) -> VkFormat {
    for (const VkFormat format : candidates) {
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

auto Renderer::find_depth_format() -> VkFormat {
    return find_supported_format(
        {VK_FORMAT_D32_SFLOAT,
         VK_FORMAT_D32_SFLOAT_S8_UINT,
         VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

auto Renderer::has_stencil_component(VkFormat format) -> bool {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT
        || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

auto Renderer::create_texture_image_view() -> void {
    const u32 readable_texture_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ReadableTextures]
            .descriptors_bindings_ids[0];
    for (u32 i = 0; i < initialize_texture_data.size(); ++i) {
        GpuImage* image = GPU_DESCRIPTORS
                              [DESCRIPTOR_BINDINGS_CONFIG
                                   [readable_texture_descriptor_binding_index]
                                       .global_descriptor_offset
                               + i]
                                  .gpu_image;
        image->views.push_back(create_image_view(*image, 0, 1));
    }
}

auto Renderer::create_texture_sampler() -> void {
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

    texture_sampler = {}; /// TODO: Is it really need here?
    if (vkCreateSampler(device, &sampler_info, nullptr, &texture_sampler)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

auto Renderer::create_shadow_map_sampler() -> void {
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

auto Renderer::create_image_view(
    GpuImage image,
    u32 base_array_layers,
    u32 layer_count
) -> VkImageView {
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

auto Renderer::create_image(GpuImage& image) -> void {
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

auto Renderer::transition_image_layout(
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout
) -> void {
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

auto Renderer::transition_shadow_map_image_layout(
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout
) -> void {
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

auto Renderer::copy_buffer_to_image(
    VkBuffer& buffer,
    VkImage image,
    u32 width,
    u32 height
) -> void {
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

auto Renderer::create_vertex_buffer(
    VkBuffer& dst_vertex_buffer,
    VkDeviceMemory& dst_vertex_buffer_memory,
    Vec<Vertex>& vertex_data
) -> void {
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
        memcpy(data, vertex_data.data(), as<usize>(buffer_size));
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

auto Renderer::create_index_buffer(
    VkBuffer& dst_index_buffer,
    VkDeviceMemory& dst_index_buffer_memory,
    const Vec<u32>& index_data
) -> void {
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
        memcpy(data, index_data.data(), as<usize>(buffer_size));
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

auto Renderer::create_main_render_uniform_buffers() -> void {
    for (u32 descriptor_set_config_counter = 0; descriptor_set_config_counter
         < DescriptorSetDataLink::DescriptorChunksNumber;
         ++descriptor_set_config_counter) {
        for (u32 j = 0;
             j < DESCRIPTOR_SETS_CONFIG[descriptor_set_config_counter]
                     .actual_linked_descriptor_bindings_number;
             ++j) {
            const u32 descriptor_binding_index =
                DESCRIPTOR_SETS_CONFIG[descriptor_set_config_counter]
                    .descriptors_bindings_ids[j];
            const VkDescriptorType descriptor_type =
                DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index].vk_type;
            if (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                const u32 memory =
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

auto Renderer::create_main_render_descriptor_pool() -> void {
    Array<VkDescriptorPoolSize, 2> pool_sizes {};

    const u32 descriptor_count = 10000;
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = as<u32>(descriptor_count);
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = as<u32>(descriptor_count);

    VkDescriptorPoolCreateInfo pool_info {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = as<u32>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = as<u32>(descriptor_count);

    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

auto Renderer::allocate_descriptor_sets(
    Vec<VkDescriptorSet>& descriptor_sets,
    VkDescriptorSetLayout set_layout,
    const u32 descriptor_sets_number,
    const u32 descriptor_offset
) -> void {
    Vec<VkDescriptorSetLayout> matrix_ubo_layouts(
        descriptor_sets_number,
        set_layout
    );
    VkDescriptorSetAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = as<u32>(descriptor_sets_number);
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

auto Renderer::update_descriptor_sets_ubo(
    VkBuffer ubo,
    const VkDeviceSize& ubo_struct_size,
    const u32& ubo_descriptors_number,
    i32 ubo_binding,
    Vec<VkDescriptorSet>& ubo_descriptor_sets,
    const u32 offset
) -> void {
    for (usize i = 0; i < ubo_descriptors_number; ++i) {
        const VkDescriptorBufferInfo model_matrix_buffer_info =
            create_descriptor_buffer_info(ubo, ubo_struct_size, i);
        Array<VkWriteDescriptorSet, 1> descriptor_writes {};

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
            as<u32>(descriptor_writes.size()),
            descriptor_writes.data(),
            0,
            nullptr
        );
    }
}

auto Renderer::update_light_data_descriptor_sets(
    const DescriptorSet& current_descriptor_set1
) -> void {
    const auto linked_descriptor_set_bindings_number =
        current_descriptor_set1.actual_linked_descriptor_bindings_number;
    Vec<u32> shader_bindings;
    const Vec<u32> descriptor_number_per_binding;
    Vec<u32> bindings_ids;
    for (usize j = 0; j < linked_descriptor_set_bindings_number; ++j) {
        shader_bindings.push_back(
            DESCRIPTOR_BINDINGS_CONFIG[current_descriptor_set1
                                           .descriptors_bindings_ids[j]]
                .binding
        );
        bindings_ids.push_back(
            current_descriptor_set1.descriptors_bindings_ids[j]
        );
    }

    for (usize i = 0; i < current_descriptor_set1.host_descriptor_number; ++i) {
        Vec<VkWriteDescriptorSet> descriptor_writes;
        descriptor_writes.resize(shader_bindings.size());
        Vec<VkDescriptorBufferInfo> descriptor_buffer_infos;
        Vec<Vec<VkDescriptorImageInfo>> descriptor_image_infos;
        for (usize j = 0; j < shader_bindings.size(); ++j) {
            if (DESCRIPTOR_BINDINGS_CONFIG[bindings_ids[j]].vk_type
                == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                descriptor_buffer_infos.push_back({});

                for (usize m = 0;
                     m < DESCRIPTOR_BINDINGS_CONFIG[bindings_ids[j]]
                             .shader_descriptors_number;
                     ++m) {
                    descriptor_buffer_infos[j] = create_descriptor_buffer_info(
                        GPU_DESCRIPTORS
                            [DESCRIPTOR_BINDINGS_CONFIG[bindings_ids[j]]
                                 .global_descriptor_offset]
                                .gpu_buffer->buffer,
                        DESCRIPTOR_BINDINGS_CONFIG[bindings_ids[j]]
                            .ubo_chunk_size,
                        i
                    );
                }
                descriptor_writes[j].pBufferInfo =
                    descriptor_buffer_infos.data();
            } else if (
                DESCRIPTOR_BINDINGS_CONFIG[bindings_ids[j]].vk_type
                == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            ) {
                descriptor_image_infos.push_back({});

                for (usize m = 0;
                     m < DESCRIPTOR_BINDINGS_CONFIG[bindings_ids[j]]
                             .shader_descriptors_number;
                     ++m) {
                    const u32 image_view_index =
                        GPU_DESCRIPTORS
                            [DESCRIPTOR_BINDINGS_CONFIG[bindings_ids[j]]
                                 .global_descriptor_offset
                             + m]
                                .gpu_image->views.size()
                        - 1;

                    descriptor_image_infos[descriptor_image_infos.size() - 1]
                        .push_back({});
                    descriptor_image_infos[descriptor_image_infos.size() - 1][m] =
                        create_descriptor_image_info(
                            *GPU_DESCRIPTORS
                                 [DESCRIPTOR_BINDINGS_CONFIG[bindings_ids[j]]
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
                DESCRIPTOR_BINDINGS_CONFIG[bindings_ids[j]].vk_type;
            descriptor_writes[j].descriptorCount =
                DESCRIPTOR_BINDINGS_CONFIG[bindings_ids[j]]
                    .shader_descriptors_number;
        }

        vkUpdateDescriptorSets(
            device,
            as<u32>(descriptor_writes.size()),
            descriptor_writes.data(),
            0,
            nullptr
        );
    }
}

auto Renderer::update_descriptor_sets_combined_image_sampler(
    const DescriptorSet& descriptor_set
) -> void {
    Vec<u32> bindings_ids;
    for (usize j = 0;
         j < descriptor_set.actual_linked_descriptor_bindings_number;
         ++j) {
        bindings_ids.push_back(descriptor_set.descriptors_bindings_ids[j]);
    }

    const u32 readable_texture_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ReadableTextures]
            .descriptors_bindings_ids[0];
    // Slots beyond the loaded textures reuse the last valid one: every
    // descriptor array entry must reference a real image view, no matter
    // how few textures a game loads.
    const usize loaded_textures = initialize_texture_data.size();
    if (loaded_textures == 0) {
        return;
    }
    for (usize i = 0; i < descriptor_set.host_descriptor_number; ++i) {
        const auto texture_index = std::min<usize>(i / 2, loaded_textures - 1);
        constexpr auto TEXTURE_VIEW_INDEX = 0;
        const VkDescriptorImageInfo image_info = create_descriptor_image_info(
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
        Vec<VkWriteDescriptorSet> descriptor_writes {};

        for (const auto bindings_id : bindings_ids) {
            descriptor_writes.push_back({});
            const auto last_element = descriptor_writes.size() - 1;
            descriptor_writes[last_element].sType =
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[last_element].dstSet =
                *(DESCRIPTOR_SETS_CHUNKS.data()
                  + descriptor_set.descriptor_set_offset + i);
            descriptor_writes[last_element].dstBinding =
                DESCRIPTOR_BINDINGS_CONFIG[bindings_id].binding;
            descriptor_writes[last_element].dstArrayElement = 0;
            descriptor_writes[last_element].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_writes[last_element].descriptorCount = 1;
            descriptor_writes[last_element].pImageInfo = &image_info;
        }
        vkUpdateDescriptorSets(
            device,
            as<u32>(descriptor_writes.size()),
            descriptor_writes.data(),
            0,
            nullptr
        );
    }
}

auto Renderer::create_descriptor_image_info(
    const u32 descriptor_number,
    VkImageLayout image_layout,
    Vec<GpuImage>& texture_images,
    const u32 image_view_index,
    VkDescriptorImageInfo* descriptor_image_infos
) -> void {
    for (usize i = 0; i < descriptor_number; ++i) {
        descriptor_image_infos[i] = {};
        descriptor_image_infos[i].imageLayout = image_layout;
        descriptor_image_infos[i].imageView =
            texture_images[i].views[image_view_index];
        descriptor_image_infos[i].sampler = texture_sampler;
    }
}

auto Renderer::create_main_render_descriptor_sets() -> void {
    vkResetDescriptorPool(device, descriptor_pool, 0);
    for (u32 pipeline_counter = 0;
         pipeline_counter < SpecificPipeline::PipelinesNumber;
         ++pipeline_counter) {
        for (u32 descriptor_set_counter = 0;
             descriptor_set_counter < PIPELINE_CONFIGS[pipeline_counter]
                                          .actual_linked_descriptor_sets_number;
             ++descriptor_set_counter) {
            const auto linked_descriptor_set_matrix_ubo_id =
                PIPELINE_CONFIGS[pipeline_counter]
                    .linked_descriptor_set_ids[descriptor_set_counter];
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

auto Renderer::create_buffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& buffer_memory
) -> void {
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

    const i32 result =
        vkAllocateMemory(device, &alloc_info, nullptr, &buffer_memory);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, buffer_memory, 0);
}

auto Renderer::begin_single_time_commands(VkCommandPool& command_pool)
    -> VkCommandBuffer {
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

auto Renderer::end_single_time_commands(
    VkCommandPool& command_pool,
    VkCommandBuffer& command_buffer
) -> void {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

auto Renderer::copy_buffer(
    VkBuffer& src_buffer,
    VkBuffer& dst_buffer,
    VkDeviceSize size
) -> void {
    VkCommandBuffer command_buffer =
        begin_single_time_commands(main_render_command_pool);

    VkBufferCopy copy_region {};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    end_single_time_commands(main_render_command_pool, command_buffer);
}

auto Renderer::find_memory_type(
    u32 type_filter,
    VkMemoryPropertyFlags properties
) -> u32 {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (u32 i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i))
            && (mem_properties.memoryTypes[i].propertyFlags & properties)
                == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

auto Renderer::create_command_buffers(
    VkCommandPool& command_pool,
    Vec<VkCommandBuffer>& command_buffers,
    u32 command_buffers_number,
    VkCommandBufferLevel command_buffer_level_flag
) -> void {
    command_buffers.resize(command_buffers_number * MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = command_buffer_level_flag;
    alloc_info.commandBufferCount = as<u32>(command_buffers.size());

    if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data())
        != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

auto Renderer::execute_secondary_command_buffer(
    VkRenderPass render_pass,
    VkFramebuffer frame_buffer,
    VkExtent2D extent,
    VkCommandBuffer primary_command_buffer,
    VkCommandBuffer secondary_command_buffer
) -> void {
    Array<VkClearValue, 1> shadow_map_clear_values;
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
    shadow_map_render_pass_info.pClearValues = shadow_map_clear_values.data();

    vkCmdBeginRenderPass(
        primary_command_buffer,
        &shadow_map_render_pass_info,
        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
    );
    vkCmdExecuteCommands(primary_command_buffer, 1, &secondary_command_buffer);
    vkCmdEndRenderPass(primary_command_buffer);
}

auto Renderer::map_memory_ubo(
    DescriptorSetDataLink descriptor_set_link,
    u32 ubo_data_size
) -> void {
    const u32 descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[descriptor_set_link].descriptors_bindings_ids[0];
    GpuBuffer* gpu_buffer =
        GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG[descriptor_binding_index]
                            .global_descriptor_offset]
            .gpu_buffer;
    const u32 descriptor_number =
        DESCRIPTOR_SETS_CONFIG[descriptor_set_link].host_descriptor_number;
    vkMapMemory(
        device,
        gpu_buffer->device_memory,
        0,
        ubo_data_size * descriptor_number,
        0,
        &gpu_buffer->mapped_data_ptr
    );
}

auto Renderer::update_hud_ubo(
    u32 offset,
    bool hud_exists,
    f32 highest_y,
    u32 health_counter
) -> void {
    HudUbo hud_ubo {};

    hud_ubo.view = view_matrix;
    hud_ubo.proj = projection_matrix;

    hud_ubo.hud_exists = hud_exists;
    hud_ubo.current_hp = health_bars[health_counter].current_health;
    hud_ubo.max_hp = health_bars[health_counter].max_health;
    hud_ubo.entity_position = health_bars[health_counter].position;
    hud_ubo.highest_y = highest_y;

    const u32 hud_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::HUD]
            .descriptors_bindings_ids[0];
    HudUbo* hud_matrix_data =
        as<HudUbo*>(
            GPU_DESCRIPTORS
                [DESCRIPTOR_BINDINGS_CONFIG[hud_ubo_descriptor_binding_index]
                     .global_descriptor_offset]
                    .gpu_buffer->mapped_data_ptr
        )
        + offset;
    memcpy(hud_matrix_data, &hud_ubo, sizeof(HudUbo));
}

auto Renderer::update_hud_screen_ubo(u32 offset, u32 crosshair) -> void {
    HudScreenUbo hud_ubo {};
    hud_ubo.model = crosshairs[crosshair].model;

    const u32 hud_screen_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::HudScreen]
            .descriptors_bindings_ids[0];
    HudScreenUbo* hud_matrix_data =
        as<HudScreenUbo*>(
            GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                                [hud_screen_ubo_descriptor_binding_index]
                                    .global_descriptor_offset]
                .gpu_buffer->mapped_data_ptr
        )
        + offset;
    memcpy(hud_matrix_data, &hud_ubo, sizeof(HudScreenUbo));
}

auto Renderer::update_sdf_ubo(u32 offset, u32 crosshair) -> void {
    SdfUbo hud_ubo {};
    hud_ubo.model = crosshairs[crosshair].model;

    const f32 current_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time
        )
            .count()
        * 0.001f;
    hud_ubo.time = current_time;

    void* hud_matrix_data;
    const u32 hud_screen_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::SdfData]
            .descriptors_bindings_ids[0];
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

auto Renderer::update_math_objects_debug_ubo(u32 offset, u32 math_object)
    -> void {
    MathObjectDebugUbo math_object_ubo {};
    math_object_ubo.model = math_objects[math_object].model_matrix;
    math_object_ubo.view = view_matrix;
    math_object_ubo.projection = projection_matrix;

    const u32 math_object_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MathObjectsDebugData]
            .descriptors_bindings_ids[0];
    MathObjectDebugUbo* math_object_data =
        as<MathObjectDebugUbo*>(
            GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                                [math_object_ubo_descriptor_binding_index]
                                    .global_descriptor_offset]
                .gpu_buffer->mapped_data_ptr
        )
        + offset;
    memcpy(math_object_data, &math_object_ubo, sizeof(MathObjectDebugUbo));
}

auto Renderer::update_ubo_ui(
    const u32 current_inventory_row,
    const u32 current_inventory_column,
    const u32 inventory,
    u32 offset
) -> void {
    UiUbo hud_ubo {};

    const auto col_size = inventories[inventory].col;
    hud_ubo.model =
        inventories[inventory]
            .slot_data[col_size * current_inventory_row + current_inventory_column]
            .model;
    hud_ubo.color =
        inventories[inventory]
            .slot_data[col_size * current_inventory_row + current_inventory_column]
            .color;

    const u32 ui_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::UI]
            .descriptors_bindings_ids[0];
    UiUbo* hud_matrix_data =
        as<UiUbo*>(
            GPU_DESCRIPTORS
                [DESCRIPTOR_BINDINGS_CONFIG[ui_ubo_descriptor_binding_index]
                     .global_descriptor_offset]
                    .gpu_buffer->mapped_data_ptr
        )
        + offset;
    memcpy(hud_matrix_data, &hud_ubo, sizeof(UiUbo));
}

auto Renderer::update_ubo_icons_ui(u32 offset, u32 item) -> void {
    UiUbo hud_ubo {};
    hud_ubo.model = items[item].model;

    const u32 ui_icons_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::UiIcons]
            .descriptors_bindings_ids[0];
    UiUbo* hud_matrix_data =
        as<UiUbo*>(GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                                       [ui_icons_ubo_descriptor_binding_index]
                                           .global_descriptor_offset]
                       .gpu_buffer->mapped_data_ptr)
        + offset;
    memcpy(hud_matrix_data, &hud_ubo, sizeof(UiUbo));
}

auto Renderer::hud_record_command_buffer(
    VkCommandBuffer& command_buffer,
    u32 image_index
) -> void {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = RENDER_PASSES[SpecificPipeline::HudPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    Array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = as<u32>(clear_values.size());
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
    viewport.width = as<f32>(swap_chain_extent.width);
    viewport.height = as<f32>(swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (u32 i = 0; i < health_bars.size(); ++i) {
        const u32 ui_vertex_id = health_bars[i].mesh_id;
        const u32 ubo_index = current_frame * hud_ubo_descriptor_number + i;
        update_hud_ubo(ubo_index, true, highest_gltf_y[ui_vertex_id], i);
        const auto linked_descriptor_set_id =
            PIPELINE_CONFIGS[SpecificPipeline::HudPipeline]
                .linked_descriptor_set_ids[0];
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

        Array<VkBuffer, 1> vertex_buffers = {
            vertex_buffer_container[ui_vertex_id]
        };
        Array<VkDeviceSize, 1> offsets = {0};
        vkCmdBindVertexBuffers(
            command_buffer,
            0,
            1,
            vertex_buffers.data(),
            offsets.data()
        );

        vkCmdBindIndexBuffer(
            command_buffer,
            index_buffer_container[ui_vertex_id],
            0,
            VK_INDEX_TYPE_UINT32
        );

        const u32 indices_container_size = indices[ui_vertex_id].size();

        vkCmdDrawIndexed(
            command_buffer,
            as<u32>(indices_container_size),
            1,
            0,
            0,
            0
        );
    }

    vkCmdEndRenderPass(command_buffer);
}

auto Renderer::ui_record_command_buffer(
    VkCommandBuffer& command_buffer,
    u32 image_index
) -> void {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = RENDER_PASSES[SpecificPipeline::UiPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    Array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = as<u32>(clear_values.size());
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
    viewport.width = as<f32>(swap_chain_extent.width);
    viewport.height = as<f32>(swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (u32 i = 0; i < inventories.size(); ++i) {
        const RenderInventory inventory = inventories[i];
        const u32 inventory_texture_id = inventory.inventory_texture_id;
        const u32 ui_vertex_id = inventory.mesh_id;
        for (u32 j = 0; j < inventory.row; ++j) {
            for (u32 m = 0; m < inventory.col; ++m) {
                const u32 ubo_index = current_frame * ui_ubo_descriptors_number
                    + j * inventory.col + m;
                update_ubo_ui(j, m, i, ubo_index);
                const auto linked_descriptor_set_id =
                    PIPELINE_CONFIGS[SpecificPipeline::UiPipeline]
                        .linked_descriptor_set_ids[0];
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

                const auto linked_descriptor_set_id1 =
                    PIPELINE_CONFIGS[SpecificPipeline::UiPipeline]
                        .linked_descriptor_set_ids[1];
                const DescriptorSet& current_descriptor_set1 =
                    DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id1];
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

                Array<VkBuffer, 1> vertex_buffers = {
                    vertex_buffer_container[ui_vertex_id]
                };
                Array<VkDeviceSize, 1> offsets = {0};
                vkCmdBindVertexBuffers(
                    command_buffer,
                    0,
                    1,
                    vertex_buffers.data(),
                    offsets.data()
                );

                vkCmdBindIndexBuffer(
                    command_buffer,
                    index_buffer_container[ui_vertex_id],
                    0,
                    VK_INDEX_TYPE_UINT32
                );

                const u32 indices_container_size = indices[ui_vertex_id].size();

                vkCmdDrawIndexed(
                    command_buffer,
                    as<u32>(indices_container_size),
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

auto Renderer::ui_icons_record_command_buffer(
    VkCommandBuffer& command_buffer,
    u32 image_index
) -> void {
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

    Array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = as<u32>(clear_values.size());
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
    viewport.width = as<f32>(swap_chain_extent.width);
    viewport.height = as<f32>(swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (u32 i = 0; i < items.size(); ++i) {
        const RenderItem item = items[i];
        const u32 ui_vertex_id = item.mesh_id;
        const u32 diffuse_texture_id = item.diffuse_texture_id;
        const u32 ubo_index = current_frame * items.size() + i;

        update_ubo_icons_ui(ubo_index, i);
        const auto linked_descriptor_set_id =
            PIPELINE_CONFIGS[SpecificPipeline::UiIconsPipeline]
                .linked_descriptor_set_ids[0];
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

        const auto linked_descriptor_set_id1 =
            PIPELINE_CONFIGS[SpecificPipeline::UiIconsPipeline]
                .linked_descriptor_set_ids[1];
        const DescriptorSet& current_descriptor_set1 =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id1];
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

        Array<VkBuffer, 1> vertex_buffers = {
            vertex_buffer_container[ui_vertex_id]
        };
        Array<VkDeviceSize, 1> offsets = {0};
        vkCmdBindVertexBuffers(
            command_buffer,
            0,
            1,
            vertex_buffers.data(),
            offsets.data()
        );

        vkCmdBindIndexBuffer(
            command_buffer,
            index_buffer_container[ui_vertex_id],
            0,
            VK_INDEX_TYPE_UINT32
        );

        const u32 indices_container_size = indices[ui_vertex_id].size();

        vkCmdDrawIndexed(
            command_buffer,
            as<u32>(indices_container_size),
            1,
            0,
            0,
            0
        );
    }

    vkCmdEndRenderPass(command_buffer);
}

auto Renderer::hud_screen_record_command_buffer(
    VkCommandBuffer& command_buffer,
    u32 image_index
) -> void {
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

    Array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = as<u32>(clear_values.size());
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
    viewport.width = as<f32>(swap_chain_extent.width);
    viewport.height = as<f32>(swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (u32 i = 0; i < crosshairs.size(); ++i) {
        const RenderCrosshair crosshair = crosshairs[i];
        const u32 ui_vertex_id = crosshair.mesh_id;

        const u32 ubo_index =
            current_frame * hud_screen_ubo_descriptor_number + i;
        update_hud_screen_ubo(ubo_index, i);
        const auto linked_descriptor_set_id =
            PIPELINE_CONFIGS[SpecificPipeline::HudScreenPipeline]
                .linked_descriptor_set_ids[0];
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

        Array<VkBuffer, 1> vertex_buffers = {
            vertex_buffer_container[ui_vertex_id]
        };
        Array<VkDeviceSize, 1> offsets = {0};
        vkCmdBindVertexBuffers(
            command_buffer,
            0,
            1,
            vertex_buffers.data(),
            offsets.data()
        );

        vkCmdBindIndexBuffer(
            command_buffer,
            index_buffer_container[ui_vertex_id],
            0,
            VK_INDEX_TYPE_UINT32
        );

        const u32 indices_container_size = indices[ui_vertex_id].size();

        vkCmdDrawIndexed(
            command_buffer,
            as<u32>(indices_container_size),
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

auto Renderer::sdf_record_command_buffer(
    VkCommandBuffer& command_buffer,
    u32 image_index
) -> void {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = RENDER_PASSES[SpecificPipeline::SdfPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    Array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = as<u32>(clear_values.size());
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
    viewport.width = as<f32>(swap_chain_extent.width);
    viewport.height = as<f32>(swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (u32 i = 0; i < crosshairs.size(); ++i) {
        const RenderCrosshair crosshair = crosshairs[i];
        const u32 ui_vertex_id = crosshair.mesh_id;

        const u32 ubo_index =
            current_frame * hud_screen_ubo_descriptor_number + i;
        update_sdf_ubo(ubo_index, i);
        const auto linked_descriptor_set_id =
            PIPELINE_CONFIGS[SpecificPipeline::SdfPipeline]
                .linked_descriptor_set_ids[0];
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

        Array<VkBuffer, 1> vertex_buffers = {
            vertex_buffer_container[ui_vertex_id]
        };
        Array<VkDeviceSize, 1> offsets = {0};
        vkCmdBindVertexBuffers(
            command_buffer,
            0,
            1,
            vertex_buffers.data(),
            offsets.data()
        );

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

auto Renderer::math_objects_debug_record_command_buffer(
    VkCommandBuffer& command_buffer,
    u32 image_index
) -> void {
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass =
        RENDER_PASSES[SpecificPipeline::MathObjectsDebugPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    Array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = as<u32>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        PIPELINE_CONFIGS[SpecificPipeline::MathObjectsDebugPipeline].pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = as<f32>(swap_chain_extent.width);
    viewport.height = as<f32>(swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    const auto linked_descriptor_set_id =
        PIPELINE_CONFIGS[SpecificPipeline::MathObjectsDebugPipeline]
            .linked_descriptor_set_ids[0];
    const DescriptorSet& current_descriptor_set =
        DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id];
    for (u32 i = 0; i < math_objects.size(); ++i) {
        const RenderMathObject math_object = math_objects[i];
        const u32 ui_vertex_id = math_object.mesh_id;

        const u32 ubo_index =
            current_frame * current_descriptor_set.host_descriptor_number + i;
        update_math_objects_debug_ubo(ubo_index, i);
        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PIPELINE_CONFIGS[SpecificPipeline::MathObjectsDebugPipeline]
                .pipeline_layout,
            0,
            1,
            &(*(DESCRIPTOR_SETS_CHUNKS.data()
                + current_descriptor_set.descriptor_set_offset + ubo_index)),
            0,
            nullptr
        );

        Array<VkBuffer, 1> vertex_buffers = {
            math_objects_vertex_buffer_container[ui_vertex_id]
        };
        Array<VkDeviceSize, 1> offsets = {0};
        vkCmdBindVertexBuffers(
            command_buffer,
            0,
            1,
            vertex_buffers.data(),
            offsets.data()
        );

        vkCmdBindIndexBuffer(
            command_buffer,
            math_objects_index_buffer_container[ui_vertex_id],
            0,
            VK_INDEX_TYPE_UINT32
        );

        const u32 indices_container_size =
            as<u32>(math_objects_indices[ui_vertex_id].size());
        vkCmdDrawIndexed(command_buffer, indices_container_size, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(command_buffer);
}

auto Renderer::font_record_command_buffer(
    VkCommandBuffer& command_buffer,
    u32 image_index
) -> void {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = RENDER_PASSES[SpecificPipeline::FontPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;
    Array<VkClearValue, 2> clear_values {};
    clear_values[0].color = {{0.5f, 0.2f, 0.2f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = as<u32>(clear_values.size());
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
    viewport.width = as<f32>(swap_chain_extent.width);
    viewport.height = as<f32>(swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    for (const auto& p : players) {
        player = p;
    }
    u32 current_actor_memory_offset =
        current_frame * font_ubo_descriptor_number;
    for (auto font : fonts) {
        const Vector<f32, 3> player_target_direction =
            font.position - player.position;
        const f32 dot_product = dot(player_target_direction, player.forward);
        if (dot_product <= 0) {
            continue;
        }

        for (u32 j = 0; j < font.font_string.size(); ++j) {
            const u32 ascii_code = as<u32>(font.font_string[j]);
            Array<VkBuffer, 1> vertex_buffers = {
                font_vertex_buffer_container[ascii_code]
            };
            Array<VkDeviceSize, 1> offsets = {0};

            vkCmdBindVertexBuffers(
                command_buffer,
                0,
                1,
                vertex_buffers.data(),
                offsets.data()
            );
            vkCmdBindIndexBuffer(
                command_buffer,
                font_index_buffer_container[ascii_code],
                0,
                VK_INDEX_TYPE_UINT32
            );

            const u32 indices_container_size = symbol_g_indices.size();
            FontUbo font_ubo {};
            const Vector<f32, 3> result;
            Vector<f32, 4> pos = Vector<f32, 4>(
                font.position[0],
                font.position[1],
                font.position[2],
                1.0f
            );

            Vector<f32, 4> clip_space_position =
                pos * view_matrix * projection_matrix;
            Vector<f32, 3> ndc_position = Vector<f32, 3>(
                clip_space_position[0] / clip_space_position[3],
                clip_space_position[1] / clip_space_position[3],
                clip_space_position[2] / clip_space_position[3]
            );

            font_ubo.view = view_matrix;
            font_ubo.proj = projection_matrix;

            font_ubo.scale = 0.3f;
            ndc_position[0] += as<f32>(j) * 0.17f * font_ubo.scale;
            ndc_position[1] -= font.lifetime / 5.0f;
            font_ubo.position = ndc_position;

            const u32 font_ubo_descriptor_binding_index =
                DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::FontRenderUbo]
                    .descriptors_bindings_ids[0];
            FontUbo* model_matrix_data =
                as<FontUbo*>(
                    GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                                        [font_ubo_descriptor_binding_index]
                                            .global_descriptor_offset]
                        .gpu_buffer->mapped_data_ptr
                )
                + current_actor_memory_offset + j;
            memcpy(model_matrix_data, &font_ubo, sizeof(font_ubo));

            const auto linked_descriptor_set_id =
                PIPELINE_CONFIGS[SpecificPipeline::FontPipeline]
                    .linked_descriptor_set_ids[0];
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
            const auto linked_descriptor_set_id1 =
                PIPELINE_CONFIGS[SpecificPipeline::FontPipeline]
                    .linked_descriptor_set_ids[1];
            const auto font_atlas_texture_id = 6;
            const DescriptorSet& current_descriptor_set1 =
                DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id1];
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
                as<u32>(indices_container_size),
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

auto Renderer::record_command_buffer(
    VkCommandBuffer& command_buffer,
    u32 image_index
) -> void {
    VkRenderPassBeginInfo render_pass_info {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass =
        RENDER_PASSES[SpecificPipeline::MainRenderPipeline];
    render_pass_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent.height = swap_chain_extent.height;
    render_pass_info.renderArea.extent.width = swap_chain_extent.width;

    for (u32 player = 0; player < players.size(); ++player) {
        update_view_position_uniform_buffer(current_frame, player);
    }

    Array<VkClearValue, 2> clear_values {};
    // Player death screen.
    if (players.size() == 0) {
        clear_values[0].color = {{0.7f, 0.2f, 0.2f, 1.0f}};
    } else {
        clear_values[0].color = {{0.2f, 0.2f, 0.2f, 1.0f}};
    }
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = as<u32>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE
    );

    VkPipeline main_pipeline =
        PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline].pipeline;
    if (imgui_overlay->wireframe_enabled
        && main_wireframe_pipeline != VK_NULL_HANDLE) {
        main_pipeline = main_wireframe_pipeline;
    }
    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        main_pipeline
    );

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = as<f32>(swap_chain_extent.width);
    viewport.height = as<f32>(swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (u32 i = 0; i < actors.size(); ++i) {
        const RenderActor actor = actors[i];
        const u32 ui_vertex_id = actor.mesh_id;
        const u32 diffuse_texture_index = actor.diffuse_texture_index;
        const u32 specular_texture_index = actor.specular_texture_index;

        const u32 ubo_index = current_frame * matrix_ubo_descriptors_number + i;
        update_matrix_uniform_buffer(ubo_index, i);
        const auto linked_descriptor_set_id =
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .linked_descriptor_set_ids[0];
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

        const auto linked_descriptor_set_id1 =
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .linked_descriptor_set_ids[1];
        const DescriptorSet& current_descriptor_set1 =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id1];
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

        Array<VkBuffer, 1> vertex_buffers = {
            vertex_buffer_container[ui_vertex_id]
        };
        Array<VkDeviceSize, 1> offsets = {0};
        vkCmdBindVertexBuffers(
            command_buffer,
            0,
            1,
            vertex_buffers.data(),
            offsets.data()
        );

        vkCmdBindIndexBuffer(
            command_buffer,
            index_buffer_container[ui_vertex_id],
            0,
            VK_INDEX_TYPE_UINT32
        );

        const u32 indices_container_size = indices[ui_vertex_id].size();

        const auto linked_descriptor_set_id2 =
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .linked_descriptor_set_ids[2];
        const DescriptorSet& current_descriptor_set2 =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id2];
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
        const auto linked_descriptor_set_id3 =
            PIPELINE_CONFIGS[SpecificPipeline::MainRenderPipeline]
                .linked_descriptor_set_ids[3];
        const DescriptorSet& current_descriptor_set3 =
            DESCRIPTOR_SETS_CONFIG[linked_descriptor_set_id3];
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
            as<u32>(indices_container_size),
            1,
            0,
            0,
            0
        );
    }
    vkCmdEndRenderPass(command_buffer);
}

auto Renderer::create_sync_objects(
    Vec<VkSemaphore>& image_available_semaphores,
    Vec<VkSemaphore>& render_finished_semaphores,
    Vec<VkFence>& in_flight_fences
) -> void {
    image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphores.resize(swap_chain_images.size());
    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphore_info {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
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

    for (usize i = 0; i < swap_chain_images.size(); ++i) {
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

auto Renderer::update_directional_light_shadow_map_matrix_ubo(
    u32 current_image,
    u32 current_light,
    u32 actor
) -> void {
    const u32 shadow_map_directional_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapDirectionalLight]
            .descriptors_bindings_ids[0];
    ShadowMapMatrixUBO* model_matrix_ubo =
        as<ShadowMapMatrixUBO*>(
            GPU_DESCRIPTORS
                [DESCRIPTOR_BINDINGS_CONFIG
                     [shadow_map_directional_light_descriptor_binding_index]
                         .global_descriptor_offset]
                    .gpu_buffer->mapped_data_ptr
        )
        + current_image;

    model_matrix_ubo->model = actors[actor].model_matrix;
    model_matrix_ubo->light_space_matrix =
        dir_light_space_matrix[current_light];

    for (u32 j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        model_matrix_ubo->joint_matrices[j] = actors[actor].joint_matrices[j];
    }
}

auto Renderer::update_spot_light_shadow_map_matrix_ubo(
    u32 current_image,
    u32 current_light,
    u32 actor
) -> void {
    const u32 shadow_map_spot_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapSpotLight]
            .descriptors_bindings_ids[0];
    ShadowMapMatrixUBO* model_matrix_ubo =
        as<ShadowMapMatrixUBO*>(
            GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                                [shadow_map_spot_light_descriptor_binding_index]
                                    .global_descriptor_offset]
                .gpu_buffer->mapped_data_ptr
        )
        + current_image;

    model_matrix_ubo->model = actors[actor].model_matrix;
    model_matrix_ubo->light_space_matrix =
        spot_light_space_matrix[current_light];

    for (u32 j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        model_matrix_ubo->joint_matrices[j] = actors[actor].joint_matrices[j];
    }
}

auto Renderer::update_point_light_shadow_map_matrix_ubo(
    u32 current_image,
    u32 current_light,
    u32 layer,
    u32 actor
) -> void {
    const u32 shadow_map_point_light_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::ShadowMapPointLight]
            .descriptors_bindings_ids[0];
    PointLightShadowMapMatrixUBO* model_matrix_ubo =
        as<PointLightShadowMapMatrixUBO*>(
            GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                                [shadow_map_point_light_descriptor_binding_index]
                                    .global_descriptor_offset]
                .gpu_buffer->mapped_data_ptr
        )
        + current_image;

    model_matrix_ubo->model = actors[actor].model_matrix;

    model_matrix_ubo->light_space_matrix =
        point_lights[current_light].point_light_space_matrix[layer];
    model_matrix_ubo->far_plane = 100.0f;
    model_matrix_ubo->light_position = point_lights[current_light].position;

    for (u32 j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        model_matrix_ubo->joint_matrices[j] = actors[actor].joint_matrices[j];
    }
}

auto Renderer::update_matrix_uniform_buffer(u32 offset, u32 actor) -> void {
    const u32 main_render_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderMatrixUbo]
            .descriptors_bindings_ids[0];
    ModelMatrixUBO* model_matrix_ubo =
        as<ModelMatrixUBO*>(
            GPU_DESCRIPTORS
                [DESCRIPTOR_BINDINGS_CONFIG[main_render_descriptor_binding_index]
                     .global_descriptor_offset]
                    .gpu_buffer->mapped_data_ptr
        )
        + offset;

    model_matrix_ubo->model = actors[actor].model_matrix;

    model_matrix_ubo->view = view_matrix;
    model_matrix_ubo->proj = projection_matrix;

    for (u32 j = 0; j < MAX_JOINTS_NUMBER; ++j) {
        model_matrix_ubo->joint_matrices[j] = actors[actor].joint_matrices[j];
    }

    model_matrix_ubo->ambient = actors[actor].ambient;
    model_matrix_ubo->shininess = actors[actor].shininess;

    for (u32 i = 0; i < directional_light_number; ++i) {
        model_matrix_ubo->dir_space_matrix[i] = dir_light_space_matrix[i];
    }

    for (u32 i = 0; i < spot_light_number; ++i) {
        model_matrix_ubo->spot_space_matrix[i] = spot_light_space_matrix[i];
    }

    model_matrix_ubo->directional_lights_number = directional_light_number;
    model_matrix_ubo->spot_lights_number = spot_light_number;
}

auto Renderer::update_view_position_uniform_buffer(u32 current_image, u32 player)
    -> void {
    const u32 light_data_ubo_descriptor_binding_index =
        DESCRIPTOR_SETS_CONFIG[DescriptorSetDataLink::MainRenderLightDataUbo]
            .descriptors_bindings_ids[0];
    LightData* light_data_ubo =
        as<LightData*>(
            GPU_DESCRIPTORS[DESCRIPTOR_BINDINGS_CONFIG
                                [light_data_ubo_descriptor_binding_index]
                                    .global_descriptor_offset]
                .gpu_buffer->mapped_data_ptr
        )
        + current_image;

    light_data_ubo->view_position = players[player].position;

    directional_light_number = directional_lights.size();
    if (!imgui_overlay->directional_enabled) {
        directional_light_number = 0;
    }
    assert(
        directional_light_number <= 4
        && "Directional lights number greater then 4"
    );
    for (u32 i = 0; i < directional_light_number; ++i) {
        const RenderDirectionalLight dir_light = directional_lights[i];

        light_data_ubo->directional_lights[i].position = dir_light.position;
        light_data_ubo->directional_lights[i].direction = dir_light.direction;
        light_data_ubo->directional_lights[i].ambient = dir_light.ambient;
        if (!imgui_overlay->ambient_enabled) {
            light_data_ubo->directional_lights[i].ambient =
                Vector<f32, 4>(0.0f, 0.0f, 0.0f, 0.0f);
        }
        light_data_ubo->directional_lights[i].diffuse = dir_light.diffuse;
        light_data_ubo->directional_lights[i].specular = dir_light.specular;
    }
    light_data_ubo->directional_lights_array_size = directional_light_number;

    point_light_number = point_lights.size();
    if (!imgui_overlay->point_enabled) {
        point_light_number = 0;
    }
    assert(
        point_light_number <= POINT_LIGHTS_NUMBER
        && "Point lights number greater than 32"
    );
    for (u32 i = 0; i < point_light_number; ++i) {
        const RenderPointLight point_light = point_lights[i];

        light_data_ubo->point_lights[i].position = point_light.position;
        light_data_ubo->point_lights[i].ambient = point_light.ambient;
        if (!imgui_overlay->ambient_enabled) {
            light_data_ubo->point_lights[i].ambient =
                Vector<f32, 3>(0.0f, 0.0f, 0.0f);
        }
        light_data_ubo->point_lights[i].diffuse = point_light.diffuse;
        light_data_ubo->point_lights[i].specular = point_light.specular;
        light_data_ubo->point_lights[i].constant = point_light.constant;
        light_data_ubo->point_lights[i].linear = point_light.linear;
        light_data_ubo->point_lights[i].quadratic = point_light.quadratic;
    }
    light_data_ubo->point_lights_array_size = point_light_number;
    light_data_ubo->far_plane = 100.0f;

    spot_light_number = spot_lights.size();
    if (!imgui_overlay->spot_enabled) {
        spot_light_number = 0;
    }
    assert(spot_light_number <= 8 && "Spot light number greater then 8");
    for (u32 i = 0; i < spot_light_number; ++i) {
        const RenderSpotLight spot_light = spot_lights[i];

        light_data_ubo->spot_lights[i].position = spot_light.position;
        light_data_ubo->spot_lights[i].direction = spot_light.direction;
        light_data_ubo->spot_lights[i].cut_off =
            std::cos(radians(spot_light.cut_off));
        light_data_ubo->spot_lights[i].outer_cut_off =
            std::cos(radians(spot_light.outer_cut_off));
        light_data_ubo->spot_lights[i].ambient = spot_light.ambient;
        if (!imgui_overlay->ambient_enabled) {
            light_data_ubo->spot_lights[i].ambient =
                Vector<f32, 3>(0.0f, 0.0f, 0.0f);
        }
        light_data_ubo->spot_lights[i].diffuse = spot_light.diffuse;
        light_data_ubo->spot_lights[i].specular = spot_light.specular;
        light_data_ubo->spot_lights[i].constant = spot_light.constant;
        light_data_ubo->spot_lights[i].linear = spot_light.linear;
        light_data_ubo->spot_lights[i].quadratic = spot_light.quadratic;
    }
    light_data_ubo->spot_light_array_size = spot_light_number;

    std::random_device rd;
    std::mt19937 mersenne(rd());
    std::uniform_int_distribution<i32> distribution_tile_index(
        0,
        INDIRECT_TEXTURE_HEIGHT * INDIRECT_TEXTURE_WIDTH
    );

    if (print == true) {
        for (auto& i : indirect_texture) {
            for (i32 j = 0; j < 4; ++j) {
                const i32 random_tile_index = distribution_tile_index(mersenne);
                i[j] = random_tile_index;
            }
        }
    }
    print = false;
    light_data_ubo->tileset_tiles_count =
        Vector<f32, 2>(TILESET_ROW, TILESET_COLUMN);
    light_data_ubo->tiles_row = 8;
    light_data_ubo->tiles_column = 8;
    light_data_ubo->debug_shadow_mode = imgui_overlay->show_shadow_maps
        ? (imgui_overlay->shadow_map_mode + 1)
        : 0;
    light_data_ubo->debug_shadow_light = imgui_overlay->shadow_map_light;
    light_data_ubo->shadows_enabled = imgui_overlay->shadows_enabled ? 1 : 0;
    for (i32 i = 0;
         i < INDIRECT_TEXTURE_HEIGHT * INDIRECT_TEXTURE_WIDTH / 4 + 1;
         ++i) {
        light_data_ubo->indirect_texture[i] = indirect_texture[i];
    }
}

auto Renderer::main_render_draw_frame() -> void {
    vkWaitForFences(
        device,
        1,
        &in_flight_fences[current_frame],
        VK_TRUE,
        UINT64_MAX
    );

    u32 image_index;
    // vkAcquireNextImageKHR gives the index of the image that WILL SOON be
    // available for rendering and signals imageAvailableSemaphore when it is so.
    // GraphicsQueue waits for this semaphore because we pass it in submitInfo.
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

    auto future1 = render_thread_pool->enqueue([this]() -> void {
        directional_light_record_command_buffer(
            directional_light_secondary_command_buffers,
            this->current_frame
        );
    });

    auto future2 = render_thread_pool->enqueue([this]() -> void {
        spot_light_record_command_buffer(
            spot_light_secondary_command_buffers,
            this->current_frame
        );
    });

    auto future3 = render_thread_pool->enqueue([this]() -> void {
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
    for (u32 directional_light_counter = 0;
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
    for (u32 spot_light_counter = 0; spot_light_counter < spot_lights.size();
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
    for (u32 point_light_counter = 0; point_light_counter < point_lights.size();
         ++point_light_counter) {
        const u32 max_cube_map_layers = 6;
        for (u32 cube_map_layer_counter = 0;
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
    math_objects_debug_record_command_buffer(
        main_render_command_buffers[current_frame],
        image_index
    );
    hud_screen_record_command_buffer(
        main_render_command_buffers[current_frame],
        image_index
    );

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // GraphicsQueue waits for the swapchain image when it becomes available.
    Array<VkSemaphore, 1> wait_semaphores = {
        image_available_semaphores[current_frame]
    };
    Array<VkPipelineStageFlags, 1> wait_stages = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores.data();
    submit_info.pWaitDstStageMask = wait_stages.data();

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &main_render_command_buffers[current_frame];

    Array<VkSemaphore, 1> signal_semaphores = {
        render_finished_semaphores[image_index]
    };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores.data();

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
    present_info.pWaitSemaphores = signal_semaphores.data();

    Array<VkSwapchainKHR, 1> swap_chains = {swap_chain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains.data();

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

auto Renderer::directional_light_shadow_map_draw_frame() -> void {
    vkWaitForFences(
        device,
        1,
        &directional_light_shadow_map_in_flight_fences
            [directional_light_current_frame],
        VK_TRUE,
        UINT64_MAX
    );

    const u32 image_index = 0;

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

auto Renderer::spot_light_shadow_map_draw_frame() -> void {
    vkWaitForFences(
        device,
        1,
        &spot_light_shadow_map_in_flight_fences[spot_light_current_frame],
        VK_TRUE,
        UINT64_MAX
    );

    const u32 image_index = 0;

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

auto Renderer::point_light_shadow_map_draw_frame() -> void {
    vkWaitForFences(
        device,
        1,
        &point_light_shadow_map_in_flight_fences[point_light_current_frame],
        VK_TRUE,
        UINT64_MAX
    );

    const u32 image_index = 0;

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

auto Renderer::directional_light_record_command_buffer(
    Vec<VkCommandBuffer>& command_buffers,
    u32 current_frame
) -> void {
    for (u32 directional_light_counter = 0;
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

        const u32 actors_number = actors.size();
        for (u32 actor_counter = 0; actor_counter < actors_number;
             ++actor_counter) {
            const RenderActor actor = actors[actor_counter];
            const u32 mesh_id = actor.mesh_id;
            const u32 ubo_directional_light_index = directional_light_number
                    * actors_number * directional_light_current_frame
                + actors_number * directional_light_counter + actor_counter;

            update_directional_light_shadow_map_matrix_ubo(
                ubo_directional_light_index,
                directional_light_counter,
                actor_counter
            );
            const auto linked_descriptor_set_id =
                PIPELINE_CONFIGS[SpecificPipeline::DirectionalLightPipeline]
                    .linked_descriptor_set_ids[0];
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

            Array<VkBuffer, 1> vertex_buffers = {
                vertex_buffer_container[mesh_id]
            };
            Array<VkDeviceSize, 1> offsets = {0};
            vkCmdBindVertexBuffers(
                command_buffer,
                0,
                1,
                vertex_buffers.data(),
                offsets.data()
            );

            vkCmdBindIndexBuffer(
                command_buffer,
                index_buffer_container[mesh_id],
                0,
                VK_INDEX_TYPE_UINT32
            );

            const u32 indices_container_size = indices[mesh_id].size();
            vkCmdDrawIndexed(
                command_buffer,
                as<u32>(indices_container_size),
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

auto Renderer::spot_light_record_command_buffer(
    Vec<VkCommandBuffer>& command_buffers,
    u32 current_frame
) -> void {
    for (u32 spot_light_counter = 0; spot_light_counter < spot_lights.size();
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
            spot_lights[spot_light_counter].spot_light_space_matrix;
        const u32 actors_number = actors.size();
        for (u32 actors_counter = 0; actors_counter < actors_number;
             ++actors_counter) {
            const RenderActor actor = actors[actors_counter];
            const u32 mesh_id = actor.mesh_id;
            const u32 ubo_spot_light_index =
                spot_light_number * actors_number * spot_light_current_frame
                + actors_number * spot_light_counter + actors_counter;

            update_spot_light_shadow_map_matrix_ubo(
                ubo_spot_light_index,
                spot_light_counter,
                actors_counter
            );
            const auto linked_descriptor_set_id =
                PIPELINE_CONFIGS[SpecificPipeline::SpotLightPipeline]
                    .linked_descriptor_set_ids[0];
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
            Array<VkBuffer, 1> vertex_buffers = {
                vertex_buffer_container[mesh_id]
            };
            Array<VkDeviceSize, 1> offsets = {0};
            vkCmdBindVertexBuffers(
                command_buffer,
                0,
                1,
                vertex_buffers.data(),
                offsets.data()
            );

            vkCmdBindIndexBuffer(
                command_buffer,
                index_buffer_container[mesh_id],
                0,
                VK_INDEX_TYPE_UINT32
            );

            const u32 indices_container_size = indices[mesh_id].size();
            vkCmdDrawIndexed(
                command_buffer,
                as<u32>(indices_container_size),
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

auto Renderer::point_light_record_command_buffer(
    Vec<VkCommandBuffer>& command_buffers,
    u32 current_frame
) -> void {
    for (u32 point_light_counter = 0; point_light_counter < point_lights.size();
         ++point_light_counter) {
        const u32 max_cube_map_layers = 6;
        // 6 is a number of cube map layers.
        for (u32 cube_map_layer_counter = 0;
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

            const u32 actors_number = actors.size();
            for (u32 actor_counter = 0; actor_counter < actors_number;
                 ++actor_counter) {
                const RenderActor actor = actors[actor_counter];
                const u32 mesh_id = actor.mesh_id;

                const u32 ubo_index = point_light_number * actors_number
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
                const auto linked_descriptor_set_id =
                    PIPELINE_CONFIGS[SpecificPipeline::PointLightPipeline]
                        .linked_descriptor_set_ids[0];
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

                Array<VkBuffer, 1> vertex_buffers = {
                    vertex_buffer_container[mesh_id]
                };
                Array<VkDeviceSize, 1> offsets = {0};
                vkCmdBindVertexBuffers(
                    command_buffer,
                    0,
                    1,
                    vertex_buffers.data(),
                    offsets.data()
                );

                vkCmdBindIndexBuffer(
                    command_buffer,
                    index_buffer_container[mesh_id],
                    0,
                    VK_INDEX_TYPE_UINT32
                );

                const u32 indices_container_size = indices[mesh_id].size();
                vkCmdDrawIndexed(
                    command_buffer,
                    as<u32>(indices_container_size),
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

auto Renderer::create_shader_module(const Vec<char>& code) -> VkShaderModule {
    VkShaderModuleCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const u32*>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module)
        != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shader_module;
}

auto Renderer::choose_swap_surface_format(
    const Vec<VkSurfaceFormatKHR>& available_formats
) -> VkSurfaceFormatKHR {
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB
            && available_format.colorSpace
                == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

auto Renderer::choose_swap_present_mode(
    const Vec<VkPresentModeKHR>& available_present_modes
) -> VkPresentModeKHR {
    if (imgui_overlay != nullptr && imgui_overlay->vsync_enabled) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return available_present_mode;
        }
    }
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

auto Renderer::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
    -> VkExtent2D {
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
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

auto Renderer::query_swap_chain_support(VkPhysicalDevice device)
    -> SwapChainSupportDetails {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device,
        surface,
        &details.capabilities
    );

    u32 format_count = 0;
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

    u32 present_mode_count = 0;
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

auto Renderer::is_device_suitable(VkPhysicalDevice device) -> bool {
    QueueFamilyIndices indices = find_queue_families(device);

    const bool extensions_supported = check_device_extension_support(device);

    bool swap_chain_adequate = false;
    if (extensions_supported) {
        const SwapChainSupportDetails swap_chain_support =
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

auto Renderer::check_device_extension_support(VkPhysicalDevice device) -> bool {
    u32 extension_count;
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extension_count,
        nullptr
    );

    Vec<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extension_count,
        available_extensions.data()
    );

    BTreeSet<String> required_extensions(
        DEVICE_EXTENSIONS.begin(),
        DEVICE_EXTENSIONS.end()
    );

    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

auto Renderer::find_queue_families(VkPhysicalDevice device)
    -> QueueFamilyIndices {
    QueueFamilyIndices indices;

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &queue_family_count,
        nullptr
    );

    Vec<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &queue_family_count,
        queue_families.data()
    );

    i32 i = 0;
    for (const auto& queue_family : queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = Option<u32>(static_cast<u32>(i));
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,
            i,
            surface,
            &present_support
        );

        if (present_support) {
            indices.present_family = Option<u32>(static_cast<u32>(i));
        }

        if (indices.is_complete()) {
            break;
        }

        i++;
    }

    return indices;
}

auto Renderer::get_required_extensions() -> Vec<const char*> {
    Vec<const char*> required_extensions = {"VK_KHR_surface"};
    switch (window_system) {
        case WindowSystem::WAYLAND:
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
            required_extensions.push_back("VK_KHR_wayland_surface");
#endif
            break;
        case WindowSystem::X11:
#ifdef VK_USE_PLATFORM_XLIB_KHR
            required_extensions.push_back("VK_KHR_xlib_surface");
#endif
            break;
        case WindowSystem::XCB:
#ifdef VK_USE_PLATFORM_XCB_KHR
            required_extensions.push_back("VK_KHR_xcb_surface");
#endif
            break;
        case WindowSystem::WINDOWS:
#ifdef VK_USE_PLATFORM_WIN32_KHR
            required_extensions.push_back("VK_KHR_win32_surface");
#endif
            break;
    }
    if (enable_validation_layers) {
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return required_extensions;
}

auto Renderer::check_validation_layer_support() -> bool {
    u32 layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    Vec<VkLayerProperties> available_layers(layer_count);
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

auto Renderer::create_descriptor_buffer_info(
    VkBuffer ubo,
    const VkDeviceSize& ubo_struct_size,
    const VkDeviceSize& offset_step
) -> VkDescriptorBufferInfo {
    VkDescriptorBufferInfo ubo_buffer_info {};
    ubo_buffer_info.buffer = ubo;
    ubo_buffer_info.offset = offset_step * ubo_struct_size;
    ubo_buffer_info.range = ubo_struct_size;
    return ubo_buffer_info;
}

auto Renderer::create_descriptor_image_info(
    const GpuImage& texture_image,
    VkImageLayout layout,
    u32 texture_view_index,
    VkSampler texture_sampler
) -> VkDescriptorImageInfo {
    VkDescriptorImageInfo image_info {};
    image_info.imageLayout = layout;
    image_info.imageView = texture_image.views[texture_view_index];
    image_info.sampler = texture_sampler;
    return image_info;
}

auto Renderer::read_file(const String& filename) -> Vec<char> {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    const usize file_size = as<usize>(file.tellg());
    Vec<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}

VKAPI_ATTR auto VKAPI_CALL Renderer::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data
) -> VkBool32 {
    return VK_FALSE;
}
} // namespace glvm

namespace glvm {

auto JsonParser::read_file(const char* file_path) -> void {
    std::ifstream json_file_input_stream;
    std::stringstream json_file_output_stream;

    json_file_input_stream.open(file_path);
    if (json_file_input_stream.good()) {
        json_file_output_stream << json_file_input_stream.rdbuf();
        json_file_input_stream.close();
        json_file_data = json_file_output_stream.str();
    } else {
        return;
    }

    json_file_data_ptr = json_file_data.c_str();
}

auto JsonParser::parse() -> void {
    current_char = json_file_data_ptr[file_counter];

    while (current_char != '\0') {
        current_char = json_file_data_ptr[file_counter];

        if (current_char == '"' && key_flag) {
            last_key = parse_string();
            while (current_char == ' ' || current_char == ':') {
                ++file_counter;
                current_char = json_file_data_ptr[file_counter];
            }
        }

        if (current_char == '"') {
            buffer_string = parse_string();

            if (key_flag) {
                const JsonValue json_string(buffer_string);
                (*stack_of_json_values.back()->value.object)[last_key] =
                    json_string;
            } else {
                const JsonValue json_string(buffer_string);
                stack_of_json_values.back()->value.array->push_back(json_string);
            }
        } else if (
            (current_char >= '0' && current_char <= '9') || current_char == '+'
            || current_char == '-'
        ) {
            buffer_string = parse_number_as_string();
            const Vec<char> vector = string_to_vector_of_chars(buffer_string);
            f64 float_number = 0.0f;
            i32 int_number = 0;
            if (contains_char(buffer_string, '.')) {
                float_number = parse_float(vector);

                if (key_flag) {
                    const JsonValue json_float(float_number);
                    (*stack_of_json_values.back()->value.object)[last_key] =
                        json_float;
                } else {
                    const JsonValue json_float(float_number);
                    stack_of_json_values.back()->value.array->push_back(
                        json_float
                    );
                }
            } else {
                int_number = parse_integer(vector);

                if (key_flag) {
                    const JsonValue json_int(int_number);
                    (*stack_of_json_values.back()->value.object)[last_key] =
                        json_int;
                } else {
                    const JsonValue json_int(int_number);
                    stack_of_json_values.back()->value.array->push_back(
                        json_int
                    );
                }
            }

        } else if (
            current_char == 't' || current_char == 'f' || current_char == 'n'
        ) {
            const String bool_or_null_string = parse_bool_or_null();

            if (bool_or_null_string == "true") {
                if (key_flag) {
                    const JsonValue json_true(true);
                    (*stack_of_json_values.back()->value.object)[last_key] =
                        json_true;
                } else {
                    const JsonValue json_true(true);
                    stack_of_json_values.back()->value.array->push_back(
                        json_true
                    );
                }
            } else if (bool_or_null_string == "false") {
                if (key_flag) {
                    const JsonValue json_false(false);
                    (*stack_of_json_values.back()->value.object)[last_key] =
                        json_false;
                } else {
                    const JsonValue json_false(false);
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
                const JsonValue json_object = create_json_hash_map();
                (*stack_of_json_values.back()->value.object)[last_key] =
                    json_object;
                stack_of_json_values.push_back(
                    &(*stack_of_json_values.back()->value.object)[last_key]
                );
            } else if (!key_flag) {
                const JsonValue json_object = create_json_hash_map();
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
                const JsonValue json_array = create_json_array();
                (*stack_of_json_values.back()->value.object)[last_key] =
                    json_array;
                stack_of_json_values.push_back(
                    &(*stack_of_json_values.back()->value.object)[last_key]
                );
            } else if (!key_flag) {
                const JsonValue json_array = create_json_array();
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

        ++file_counter;
    }
}

auto JsonParser::create_json_hash_map() -> JsonValue {
    JsonValue json_object;
    json_object.type = JsonObject;
    json_object.value.object = new HashMap<String, JsonValue>;
    return json_object;
}

auto JsonParser::create_json_array() -> JsonValue {
    JsonValue json_array;
    json_array.type = JsonArray;
    json_array.value.array = new Vec<JsonValue>;
    return json_array;
}

auto JsonParser::parse_bool_or_null() -> String {
    String bool_or_null_string = "";
    while (1) {
        current_char = json_file_data_ptr[file_counter];
        if (current_char >= 'a' && current_char <= 'z') {
            bool_or_null_string.push_back(current_char);
            ++file_counter;
        } else {
            return bool_or_null_string;
        }
    }
}

auto JsonParser::contains_char(String text, char character) -> bool {
    for (const auto i : text) {
        if (i == character) {
            return true;
        }
    }

    return false;
}

auto JsonParser::parse_number_as_string() -> String {
    String number_as_string = "";
    while (1) {
        current_char = json_file_data_ptr[file_counter];
        if ((current_char >= '0' && current_char <= '9') || current_char == '+'
            || current_char == '-' || current_char == 'e') {
            number_as_string.push_back(current_char);
            ++file_counter;
        } else if (current_char == '.') {
            number_as_string.push_back(current_char);
            ++file_counter;
        } else {
            return number_as_string;
        }
    }
}

auto JsonParser::parse_string() -> String {
    ++file_counter;
    String local_buffer = "";
    while (1) {
        current_char = json_file_data_ptr[file_counter];
        if (current_char == '"') {
            ++file_counter;
            current_char = json_file_data_ptr[file_counter];
            return local_buffer;
        } else {
            local_buffer.push_back(current_char);
            ++file_counter;
        }
    }
}

auto JsonParser::string_to_vector_of_chars(String text) -> Vec<char> {
    Vec<char> vector_with_chars;
    for (const auto i : text) {
        vector_with_chars.push_back(i);
    }

    return vector_with_chars;
}

auto JsonParser::parse_integer(Vec<char> digits) -> i32 {
    Vec<i32> base_container;

    for (const auto digit : digits) {
        base_container.push_back(digit - 48);
    }

    i32 result = 0;
    bool negate_flag = false;

    const u32 base_container_size = base_container.size();
    for (u32 i = 0; i < base_container_size; ++i) {
        if (base_container[i] == -3 && i == 0) {
            negate_flag = true;
            continue;
        } else if (base_container[i] == -5 && i == 0) {
            continue;
        }

        result +=
            base_container[i] * std::pow(10, (base_container_size - 1) - i);
    }

    if (negate_flag) {
        result *= -1;
    }

    return result;
}

auto JsonParser::parse_float(Vec<char> digits) -> f64 {
    Vec<i32> base_container;

    for (const auto digit : digits) {
        base_container.push_back(digit - 48);
    }

    i32 integer_part = 0;
    f64 floating_part = 0;
    i32 exponent = 0;
    Vec<i32> integer_part_container;
    Vec<i32> floating_part_container;
    Vec<i32> exponent_digits;
    bool dot_flag = false;
    bool negate_flag = false;
    bool has_exponent = false;
    // False value equal "+" sign.
    bool exponent_negative = false;
    const u32 base_container_size = base_container.size();

    if (base_container[0] == -3) {
        negate_flag = true;
    }

    for (u32 i = 0; i < base_container_size; ++i) {
        if (negate_flag && i == 0) {
            continue;
        } else if (base_container[i] == -5 && i == 0) {
            continue;
        } else if (base_container[i] == -2) {
            dot_flag = true;
            continue;
        } else if (base_container[i] == 53) {
            has_exponent = true;
            continue;
        }

        if (has_exponent) {
            if (base_container[i] == -5) {
                continue;
            } else if (base_container[i] == -3) {
                exponent_negative = true;
                continue;
            }

            exponent_digits.push_back(base_container[i]);
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

    const u32 exponent_digit_count = exponent_digits.size();
    for (u32 i = 0; i < exponent_digit_count; ++i) {
        exponent +=
            exponent_digits[i] * std::pow(10, (exponent_digit_count - 1) - i);
    }

    const u32 integer_part_container_size = integer_part_container.size();
    for (u32 i = 0; i < integer_part_container_size; ++i) {
        integer_part += integer_part_container[i]
            * std::pow(10, (integer_part_container_size - 1) - i);
    }

    const u32 floating_part_container_size = floating_part_container.size();
    for (u32 i = 0; i < floating_part_container_size; ++i) {
        floating_part += floating_part_container[i] / std::pow(10, i + 1);
    }

    f64 result = 0;
    result = as<f64>((integer_part + floating_part));

    if (has_exponent) {
        if (exponent_negative) {
            result /= std::pow(10, exponent);
        } else {
            result *= std::pow(10, exponent);
        }
    }

    if (negate_flag) {
        result *= -1.0f;
    }

    return result;
}

auto JsonParser::search_in_json_array(
    Vec<JsonValue>* array_value,
    const char* key,
    Vec<JsonValue>& result_vector
) const -> void {
    for (const auto& i : *array_value) {
        if (i.type == JsonObject) {
            search_in_json_object(i.value.object, key, result_vector);
        }

        if (i.type == JsonArray) {
            search_in_json_array(i.value.array, key, result_vector);
        }
    }
}

auto JsonParser::search_in_json_object(
    HashMap<String, JsonValue>* map_value,
    const char* key,
    Vec<JsonValue>& result_vector
) const -> void {
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

auto JsonParser::search(const char* key) const -> Vec<JsonValue> {
    Vec<JsonValue> result_vector;
    search_in_json_object(root->value.object, key, result_vector);
    return result_vector;
}

template<typename T>
auto is_element_exist(const T element, const Vec<T>& array) -> bool {
    for (u32 n = 0; n < array.size(); ++n) {
        if (element == array[n]) {
            return true;
        }
    }

    return false;
}

template<typename T>
auto get_element_index(const T element, const Vec<T>& array) -> T {
    for (u32 n = 0; n < array.size(); ++n) {
        if (element == array[n]) {
            return n;
        }
    }

    return std::numeric_limits<T>::max();
}

auto calculate_elements_memory_size(
    const u32 indices_elements_count,
    String* indices_element_type,
    const u32 indices_component_type,
    u32* indices_buffer_view_byte_length
) -> void {
    if (*indices_element_type == "VEC2") {
        if (indices_component_type == 5120 || indices_component_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 2;
        } else if (
            indices_component_type == 5122 || indices_component_type == 5123
        ) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        } else if (
            indices_component_type == 5125 || indices_component_type == 5126
        ) {
            *indices_buffer_view_byte_length = indices_elements_count * 8;
        }
    } else if (*indices_element_type == "VEC3") {
        if (indices_component_type == 5120 || indices_component_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 3;
        } else if (
            indices_component_type == 5122 || indices_component_type == 5123
        ) {
            *indices_buffer_view_byte_length = indices_elements_count * 6;
        } else if (
            indices_component_type == 5125 || indices_component_type == 5126
        ) {
            *indices_buffer_view_byte_length = indices_elements_count * 12;
        }
    } else if (*indices_element_type == "VEC4") {
        if (indices_component_type == 5120 || indices_component_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        } else if (
            indices_component_type == 5122 || indices_component_type == 5123
        ) {
            *indices_buffer_view_byte_length = indices_elements_count * 8;
        } else if (
            indices_component_type == 5125 || indices_component_type == 5126
        ) {
            *indices_buffer_view_byte_length = indices_elements_count * 16;
        }
    } else if (*indices_element_type == "SCALAR") {
        if (indices_component_type == 5120 || indices_component_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count;
        } else if (
            indices_component_type == 5122 || indices_component_type == 5123
        ) {
            *indices_buffer_view_byte_length = indices_elements_count * 2;
        } else if (
            indices_component_type == 5125 || indices_component_type == 5126
        ) {
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

auto calculate_byte_step(u32 component_type, u32* byte_step) -> void {
    if (component_type == ComponentType::I8
        || component_type == ComponentType::U8) {
        *byte_step = 1;
    } else if (
        component_type == ComponentType::I16
        || component_type == ComponentType::U16
    ) {
        *byte_step = 2;
    } else if (
        component_type == ComponentType::U32
        || component_type == ComponentType::F32
    ) {
        *byte_step = 4;
    }
}

// Metadata structs to binary buffer with actual data.
struct AccessorMetaData {
    u32 buffer_view;
    u32 byte_offset;
    u32 component_type;
    u32 count;
    String type;
};

struct BufferViewMetaData {
    u32 byte_length;
    u32 byte_offset;
};

[[nodiscard]] auto read_accessor_meta_data(
    JsonValue* gltf,
    const u32 accessor_index
) -> AccessorMetaData {
    AccessorMetaData buffer_meta_data;
    buffer_meta_data.buffer_view =
        (*gltf)["accessors"][accessor_index]["bufferView"].value.int_number;
    buffer_meta_data.count =
        (*gltf)["accessors"][accessor_index]["count"].value.int_number;
    buffer_meta_data.type =
        *(*gltf)["accessors"][accessor_index]["type"].value.string;
    buffer_meta_data.component_type =
        (*gltf)["accessors"][accessor_index]["componentType"].value.int_number;

    buffer_meta_data.byte_offset = 0;
    if ((*gltf)["accessors"][accessor_index].is_object() == JsonObject) {
        const HashMap<String, JsonValue>* ptr =
            (*gltf)["accessors"][accessor_index].value.object;
        if (ptr->contains("byteOffset")) {
            buffer_meta_data.byte_offset =
                (*gltf)["accessors"][accessor_index]["byteOffset"]
                    .value.int_number;
        }
    }

    return buffer_meta_data;
}

[[nodiscard]] auto read_buffer_view_meta_data(
    JsonValue* gltf,
    const u32 buffer_view_index
) -> BufferViewMetaData {
    BufferViewMetaData buffer_view_meta_data;
    buffer_view_meta_data.byte_length =
        (*gltf)["bufferViews"][buffer_view_index]["byteLength"].value.int_number;
    buffer_view_meta_data.byte_offset = 0;
    if ((*gltf)["bufferViews"][buffer_view_index].is_object() == JsonObject) {
        const HashMap<String, JsonValue>* ptr =
            (*gltf)["bufferViews"][buffer_view_index].value.object;
        if (ptr->contains("byteOffset")) {
            buffer_view_meta_data.byte_offset =
                (*gltf)["bufferViews"][buffer_view_index]["byteOffset"]
                    .value.int_number;
        }
    }

    return buffer_view_meta_data;
}

template<typename T>
auto read_binary_buffer_data(
    char* buffer,
    AccessorMetaData accessor_meta_data,
    BufferViewMetaData buffer_view_meta_data,
    Vec<T>& output_data
) -> void {
    u32 indices_byte_step = 0;
    calculate_byte_step(accessor_meta_data.component_type, &indices_byte_step);

    u32 byte_length = 0;
    calculate_elements_memory_size(
        accessor_meta_data.count,
        &accessor_meta_data.type,
        accessor_meta_data.component_type,
        &byte_length
    );
    for (u32 i =
             buffer_view_meta_data.byte_offset + accessor_meta_data.byte_offset;
         i < buffer_view_meta_data.byte_offset + accessor_meta_data.byte_offset
             + byte_length;
         i += indices_byte_step) {
        switch (accessor_meta_data.component_type) {
            case ComponentType::U8:
                output_data.push_back(reinterpret_cast<u8&>(buffer[i]));
                break;
            case ComponentType::U16:
                output_data.push_back(reinterpret_cast<u16&>(buffer[i]));
                break;
            case ComponentType::U32:
                output_data.push_back(reinterpret_cast<u32&>(buffer[i]));
                break;
            case ComponentType::F32:
                output_data.push_back(reinterpret_cast<f32&>(buffer[i]));
                break;
        }
    }
}

auto JsonParser::load_gltf(
    const char* paths_gltf,
    Vec<f32>& vertices,
    Vec<u32>& indices,
    Vec<Vec<Matrix<f32, 4>>>& joint_matrices_per_mesh,
    Vec<f32>& frames,
    bool& no_animations,
    f32& top_y
) -> void {
    read_file(paths_gltf);
    parse();
    JsonValue* gltf = get_root();
    const String binary_path = *(*gltf)["buffers"][0]["uri"].value.string;
    const i32 full_byte_size =
        (*gltf)["buffers"][0]["byteLength"].value.int_number;
    const usize last_separator = String(paths_gltf).find_last_of("/\\");
    const String binary_full_path = last_separator == String::npos
        ? binary_path
        : String(paths_gltf).substr(0, last_separator + 1) + binary_path;
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
    const auto indices_accessor_index =
        (*gltf)["meshes"][0]["primitives"][0]["indices"].value.int_number;
    const AccessorMetaData indices_accessor_meta_data =
        read_accessor_meta_data(gltf, indices_accessor_index);
    const BufferViewMetaData indices_buffer_view_meta_data =
        read_buffer_view_meta_data(gltf, indices_accessor_meta_data.buffer_view);
    Vec<u32> mesh_indices;
    read_binary_buffer_data(
        buffer,
        indices_accessor_meta_data,
        indices_buffer_view_meta_data,
        mesh_indices
    );
    const auto vertices_position_accessor_index =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["POSITION"]
            .value.int_number;
    const AccessorMetaData vertices_position_accessor_meta_data =
        read_accessor_meta_data(gltf, vertices_position_accessor_index);
    const BufferViewMetaData vertices_position_buffer_view_meta_data =
        read_buffer_view_meta_data(
            gltf,
            vertices_position_accessor_meta_data.buffer_view
        );
    Vec<f32> vertices_position;
    read_binary_buffer_data(
        buffer,
        vertices_position_accessor_meta_data,
        vertices_position_buffer_view_meta_data,
        vertices_position
    );
    const auto texture_coordinates_accessor_index =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["TEXCOORD_0"]
            .value.int_number;
    const AccessorMetaData texture_coordinates_accessor_meta_data =
        read_accessor_meta_data(gltf, texture_coordinates_accessor_index);
    const BufferViewMetaData texture_coordinates_buffer_view_meta_data =
        read_buffer_view_meta_data(
            gltf,
            texture_coordinates_accessor_meta_data.buffer_view
        );
    Vec<f32> texture_coordinates;
    read_binary_buffer_data(
        buffer,
        texture_coordinates_accessor_meta_data,
        texture_coordinates_buffer_view_meta_data,
        texture_coordinates
    );
    const auto normals_accessor_index =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["NORMAL"]
            .value.int_number;
    const AccessorMetaData normals_accessor_meta_data =
        read_accessor_meta_data(gltf, normals_accessor_index);
    const BufferViewMetaData normals_buffer_view_meta_data =
        read_buffer_view_meta_data(gltf, normals_accessor_meta_data.buffer_view);
    Vec<f32> normals;
    read_binary_buffer_data(
        buffer,
        normals_accessor_meta_data,
        normals_buffer_view_meta_data,
        normals
    );
    const Vec<JsonValue> skins = search("skins");
    JsonValue joints;
    Vec<Matrix<f32, 4>> global_transform_joint_node;
    Vec<Matrix<f32, 4>> inverse_bind_matrix_set;
    Vec<Vec<Matrix<f32, 4>>> joint_matrices;
    Vec<f32> weights_container;
    Vec<i32> joints_indices;
    Vec<Vec<i32>> children;
    if (skins.size() > 0) {
        no_animations = false;
        joints = (*gltf)["skins"][0]["joints"];
        JsonValue nodes = (*gltf)["nodes"];
        // Loop on joints.
        for (auto& i : *joints.value.array) {
            const u32 joint_index_map_to_node = i.value.int_number;
            const JsonValue node = nodes[joint_index_map_to_node];
            Quaternion rotation_quaternion;
            Matrix<f32, 4> rotation(1.0f);
            Matrix<f32, 4> scale(1.0f);
            Matrix<f32, 4> translation(1.0f);
            if (node.value.object->contains("rotation")) {
                JsonValue array = (*node.value.object)["rotation"];
                for (u32 i = 0; i < array.value.array->size(); ++i) {
                    switch (i) {
                        case 0:
                            if (array[i].is_integer()) {
                                rotation_quaternion.x =
                                    array[i].value.int_number;
                            } else if (array[i].is_float()) {
                                rotation_quaternion.x =
                                    as<f32>(array[i].value.float_number);
                            }
                            break;
                        case 1:
                            if (array[i].is_integer()) {
                                rotation_quaternion.y =
                                    array[i].value.int_number;
                            } else if (array[i].is_float()) {
                                rotation_quaternion.y =
                                    as<f32>(array[i].value.float_number);
                            }
                            break;
                        case 2:
                            if (array[i].is_integer()) {
                                rotation_quaternion.z =
                                    array[i].value.int_number;
                            } else if (array[i].is_float()) {
                                rotation_quaternion.z =
                                    as<f32>(array[i].value.float_number);
                            }
                            break;
                        case 3:
                            if (array[i].is_integer()) {
                                rotation_quaternion.w =
                                    array[i].value.int_number;
                            } else if (array[i].is_float()) {
                                rotation_quaternion.w =
                                    as<f32>(array[i].value.float_number);
                            }
                            break;
                    }
                }
                rotation = rotate_quaternion<f32, 4>(rotation_quaternion);
                rotation.self_tensor_transpose();
            }
            Vec<i32> local_children;
            // Collect children indices.
            if (node.value.object->contains("children")) {
                JsonValue array = (*node.value.object)["children"];
                for (u32 i = 0; i < array.value.array->size(); ++i) {
                    local_children.push_back(array[i].value.int_number);
                }
                // Linearly append all children to every root joint.
                children.push_back(local_children);
            } else {
                const Vec<i32> empty_children;
                // Put an empty set of children if none can be found.
                children.push_back(empty_children);
            }
            if (node.value.object->contains("scale")) {
                JsonValue array = (*node.value.object)["scale"];
                for (u32 i = 0; i < array.value.array->size(); ++i) {
                    if (array[i].is_integer()) {
                        scale[i][i] = array[i].value.int_number;
                    } else if (array[i].is_float()) {
                        scale[i][i] = as<f32>(array[i].value.float_number);
                    }
                }
            }
            if (node.value.object->contains("translation")) {
                JsonValue array = (*node.value.object)["translation"];
                for (u32 i = 0; i < array.value.array->size(); ++i) {
                    if (array[i].is_integer()) {
                        translation[3][i] = array[i].value.int_number;
                    } else if (array[i].is_float()) {
                        translation[3][i] =
                            as<f32>(array[i].value.float_number);
                    }
                }
            }
            // Compute model matrix.
            const Matrix<f32, 4> model = scale * rotation * translation;
            global_transform_joint_node.push_back(model);
        }
        // Get the inverse bind matrices accessor index.
        const auto inverse_bind_matrices_accessor_index =
            (*gltf)["skins"][0]["inverseBindMatrices"].value.int_number;
        const AccessorMetaData inverse_bind_matrices_accessor_meta_data =
            read_accessor_meta_data(gltf, inverse_bind_matrices_accessor_index);
        const BufferViewMetaData inverse_bind_matrices_buffer_view_meta_data =
            read_buffer_view_meta_data(
                gltf,
                inverse_bind_matrices_accessor_meta_data.buffer_view
            );
        Vec<f32> inverse_bind_matrices_data;
        read_binary_buffer_data(
            buffer,
            inverse_bind_matrices_accessor_meta_data,
            inverse_bind_matrices_buffer_view_meta_data,
            inverse_bind_matrices_data
        );
        Matrix<f32, 4> inverse_bind_matrix(0.0f);
        for (u32 n = 0; n < joints.value.array->size(); ++n) {
            for (u32 g = 0; g < 4; ++g) {
                for (u32 j = 0; j < 4; ++j) {
                    // Put row-major f32 data into mat4.
                    inverse_bind_matrix[g][j] =
                        inverse_bind_matrices_data[n * 16 + g * 4 + j];
                }
            }
            inverse_bind_matrix_set.push_back(inverse_bind_matrix);
        }
        const auto joints_accessor_index =
            (*gltf)["meshes"][0]["primitives"][0]["attributes"]["JOINTS_0"]
                .value.int_number;
        const AccessorMetaData joints_accessor_meta_data =
            read_accessor_meta_data(gltf, joints_accessor_index);
        const BufferViewMetaData joints_buffer_view_meta_data =
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
        const u32 weights_accessor_index =
            (*gltf)["meshes"][0]["primitives"][0]["attributes"]["WEIGHTS_0"]
                .value.int_number;
        const AccessorMetaData weights_accessor_meta_data =
            read_accessor_meta_data(gltf, weights_accessor_index);
        const BufferViewMetaData weights_buffer_view_meta_data =
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
    const Vec<JsonValue> animations = search("animations");
    if (animations.size() > 0) {
        Vec<JsonValue> sampler_indices;
        Vec<JsonValue> target_nodes;
        Vec<JsonValue> target_paths;
        JsonValue channels = (*gltf)["animations"][0]["channels"];
        for (u32 i = 0; i < channels.value.array->size(); ++i) {
            sampler_indices.push_back(channels[i]["sampler"]);
        }
        for (u32 i = 0; i < channels.value.array->size(); ++i) {
            target_nodes.push_back(channels[i]["target"]["node"]);
        }
        for (u32 i = 0; i < channels.value.array->size(); ++i) {
            target_paths.push_back(channels[i]["target"]["path"]);
        }
        Vec<u32> translation_sampler_indices;
        Vec<u32> rotation_sampler_indices;
        Vec<u32> scale_sampler_indices;
        Vec<u32> nodes_map_translations;
        Vec<u32> nodes_map_rotations;
        Vec<u32> nodes_map_scales;
        for (u32 i = 0; i < sampler_indices.size(); ++i) {
            if (*target_paths[i].value.string == "translation") {
                translation_sampler_indices.push_back(
                    sampler_indices[i].value.int_number
                );
                nodes_map_translations.push_back(
                    target_nodes[i].value.int_number
                );
            } else if (*target_paths[i].value.string == "rotation") {
                rotation_sampler_indices.push_back(
                    sampler_indices[i].value.int_number
                );
                nodes_map_rotations.push_back(target_nodes[i].value.int_number);
            } else if (*target_paths[i].value.string == "scale") {
                scale_sampler_indices.push_back(
                    sampler_indices[i].value.int_number
                );
                nodes_map_scales.push_back(target_nodes[i].value.int_number);
            }
        }
        JsonValue samplers = (*gltf)["animations"][0]["samplers"];
        Vec<u32> translation_inputs;
        Vec<u32> translation_outputs;
        for (const auto translation_sampler_index :
             translation_sampler_indices) {
            translation_inputs.push_back(
                samplers[translation_sampler_index]["input"].value.int_number
            );
        }
        for (const auto translation_sampler_index :
             translation_sampler_indices) {
            translation_outputs.push_back(
                samplers[translation_sampler_index]["output"].value.int_number
            );
        }
        Vec<Vec<f32>> frame_inputs_translation;
        for (const auto translation_input : translation_inputs) {
            const AccessorMetaData frame_inputs_translation_accessor_meta_data =
                read_accessor_meta_data(gltf, translation_input);
            const BufferViewMetaData
                frame_inputs_translation_buffer_view_meta_data =
                    read_buffer_view_meta_data(
                        gltf,
                        frame_inputs_translation_accessor_meta_data.buffer_view
                    );
            Vec<f32> temp;
            read_binary_buffer_data(
                buffer,
                frame_inputs_translation_accessor_meta_data,
                frame_inputs_translation_buffer_view_meta_data,
                temp
            );
            frame_inputs_translation.push_back(temp);
        }
        Vec<Vec<f32>> translations;
        for (const auto translation_output : translation_outputs) {
            const AccessorMetaData frame_outputs_translation_accessor_meta_data =
                read_accessor_meta_data(gltf, translation_output);
            const BufferViewMetaData
                frame_outputs_translation_buffer_view_meta_data =
                    read_buffer_view_meta_data(
                        gltf,
                        frame_outputs_translation_accessor_meta_data.buffer_view
                    );
            Vec<f32> temp;
            read_binary_buffer_data(
                buffer,
                frame_outputs_translation_accessor_meta_data,
                frame_outputs_translation_buffer_view_meta_data,
                temp
            );
            translations.push_back(temp);
        }
        Vec<u32> rotation_inputs;
        Vec<u32> rotation_outputs;
        for (const auto rotation_sampler_index : rotation_sampler_indices) {
            rotation_inputs.push_back(
                samplers[rotation_sampler_index]["input"].value.int_number
            );
        }
        for (const auto rotation_sampler_index : rotation_sampler_indices) {
            rotation_outputs.push_back(
                samplers[rotation_sampler_index]["output"].value.int_number
            );
        }
        Vec<Vec<f32>> frame_inputs_rotation;
        for (const auto rotation_input : rotation_inputs) {
            const AccessorMetaData frame_inputs_rotation_accessor_meta_data =
                read_accessor_meta_data(gltf, rotation_input);
            const BufferViewMetaData frame_inputs_rotation_buffer_view_meta_data =
                read_buffer_view_meta_data(
                    gltf,
                    frame_inputs_rotation_accessor_meta_data.buffer_view
                );
            Vec<f32> temp;
            read_binary_buffer_data(
                buffer,
                frame_inputs_rotation_accessor_meta_data,
                frame_inputs_rotation_buffer_view_meta_data,
                temp
            );
            frame_inputs_rotation.push_back(temp);
        }
        Vec<Vec<f32>> rotations;
        for (const auto rotation_output : rotation_outputs) {
            const AccessorMetaData frame_outputs_rotation_accessor_meta_data =
                read_accessor_meta_data(gltf, rotation_output);
            const BufferViewMetaData
                frame_outputs_rotation_buffer_view_meta_data =
                    read_buffer_view_meta_data(
                        gltf,
                        frame_outputs_rotation_accessor_meta_data.buffer_view
                    );
            Vec<f32> temp;
            read_binary_buffer_data(
                buffer,
                frame_outputs_rotation_accessor_meta_data,
                frame_outputs_rotation_buffer_view_meta_data,
                temp
            );
            rotations.push_back(temp);
        }
        Vec<u32> scale_inputs;
        Vec<u32> scale_outputs;
        for (const auto scale_sampler_index : scale_sampler_indices) {
            scale_inputs.push_back(
                samplers[scale_sampler_index]["input"].value.int_number
            );
        }
        for (const auto scale_sampler_index : scale_sampler_indices) {
            scale_outputs.push_back(
                samplers[scale_sampler_index]["output"].value.int_number
            );
        }
        Vec<Vec<f32>> frame_inputs_scale;
        for (const auto scale_input : scale_inputs) {
            const AccessorMetaData frame_inputs_scale_accessor_meta_data =
                read_accessor_meta_data(gltf, scale_input);
            const BufferViewMetaData frame_inputs_scale_buffer_view_meta_data =
                read_buffer_view_meta_data(
                    gltf,
                    frame_inputs_scale_accessor_meta_data.buffer_view
                );
            Vec<f32> temp;
            read_binary_buffer_data(
                buffer,
                frame_inputs_scale_accessor_meta_data,
                frame_inputs_scale_buffer_view_meta_data,
                temp
            );
            frame_inputs_scale.push_back(temp);
        }
        Vec<Vec<f32>> scales;
        for (const auto scale_output : scale_outputs) {
            const AccessorMetaData frame_outputs_scale_accessor_meta_data =
                read_accessor_meta_data(gltf, scale_output);
            const BufferViewMetaData frame_outputs_scale_buffer_view_meta_data =
                read_buffer_view_meta_data(
                    gltf,
                    frame_outputs_scale_accessor_meta_data.buffer_view
                );
            Vec<f32> temp;
            read_binary_buffer_data(
                buffer,
                frame_outputs_scale_accessor_meta_data,
                frame_outputs_scale_buffer_view_meta_data,
                temp
            );
            scales.push_back(temp);
        }
        // Searching for root joints.
        Vec<i32> root_nodes;
        for (auto& s : *joints.value.array) {
            const i32 current_joint = s.value.int_number;
            for (auto& w : children) {
                for (const auto q : w) {
                    if (q == current_joint) {
                        goto most_scary_operator_of_all_time;
                    }
                }
            }
            // If this line executes, then this joint index is actually the root.
            root_nodes.push_back(current_joint);
        most_scary_operator_of_all_time: // Not so scary at all. Am I right?
            continue;
        }
        Vec<Vec<u32>> nodes_hierarchy;
        // Loop on parent joints.
        for (const auto current_root : root_nodes) {
            Vec<Vec<u32>> nodes_bones;
            Vec<u32> node_stack;
            // Start from root joint.
            node_stack.push_back(current_root);
            const Vec<u32> depth_stack;
            traverse_bones(
                children,
                joints,
                node_stack,
                depth_stack,
                nodes_bones
            );
            for (const auto& nodes_bone : nodes_bones) {
                nodes_hierarchy.push_back(nodes_bone);
            }
        }
        // This logic related to joints that has inverseBindMatrices.
        const u32 transformations_max = translations.size() > scales.size()
            ? (translations.size() > rotations.size() ? translations.size()
                                                      : rotations.size())
            : (scales.size() > rotations.size() ? scales.size()
                                                : rotations.size());
        const auto num_joints = joints.value.array->size();
        u32 translation_frames_number = 0;
        for (const auto& k : frame_inputs_translation) {
            if (k.size() > translation_frames_number) {
                translation_frames_number = k.size();
            }
        }
        u32 rotation_frames_number = 0;
        for (const auto& k : frame_inputs_rotation) {
            if (k.size() > rotation_frames_number) {
                rotation_frames_number = k.size();
            }
        }
        u32 scale_frames_number = 0;
        for (const auto& k : frame_inputs_scale) {
            if (k.size() > scale_frames_number) {
                scale_frames_number = k.size();
            }
        }
        const auto frames_max = translation_frames_number > scale_frames_number
            ? (translation_frames_number > rotation_frames_number
                   ? translation_frames_number
                   : rotation_frames_number)
            : (scale_frames_number > rotation_frames_number
                   ? scale_frames_number
                   : rotation_frames_number);
        for (const auto& k : frame_inputs_translation) {
            if (k.size() > frames.size()) {
                frames = k;
            }
        }
        for (const auto& k : frame_inputs_rotation) {
            if (k.size() > frames.size()) {
                frames = k;
            }
        }
        for (const auto& k : frame_inputs_scale) {
            if (k.size() > frames.size()) {
                frames = k;
            }
        }
        Vec<i32> joint_to_translation_ch;
        Vec<i32> joint_to_rotation_ch;
        Vec<i32> joint_to_scale_ch;
        for (u32 k = 0; k < num_joints; ++k) {
            joint_to_translation_ch.push_back(-1);
            joint_to_rotation_ch.push_back(-1);
            joint_to_scale_ch.push_back(-1);
        }
        for (u32 k = 0; k < nodes_map_translations.size(); ++k) {
            const u32 joint_idx =
                get_joint_index(joints, as<i32>(nodes_map_translations[k]));
            if (joint_idx != UINT32_MAX) {
                joint_to_translation_ch[joint_idx] = as<i32>(k);
            }
        }
        for (u32 k = 0; k < nodes_map_rotations.size(); ++k) {
            const u32 joint_idx =
                get_joint_index(joints, as<i32>(nodes_map_rotations[k]));
            if (joint_idx != UINT32_MAX) {
                joint_to_rotation_ch[joint_idx] = as<i32>(k);
            }
        }
        for (u32 k = 0; k < nodes_map_scales.size(); ++k) {
            const u32 joint_idx =
                get_joint_index(joints, as<i32>(nodes_map_scales[k]));
            if (joint_idx != UINT32_MAX) {
                joint_to_scale_ch[joint_idx] = as<i32>(k);
            }
        }
        // Build animated_nodes_matrices_accumulator indexed by joint-index
        // (0..numJoints - 1).
        Vec<Vec<Matrix<f32, 4>>> animated_nodes_matrices_accumulator;
        for (u32 j = 0; j < num_joints; ++j) {
            const i32 translation_idx = joint_to_translation_ch[j];
            const i32 rotation_idx = joint_to_rotation_ch[j];
            const i32 scale_idx = joint_to_scale_ch[j];
            // Local defaults fresh on every joint, so as not to make it
            // possible to collect data from previous iterations.
            Vec<f32> default_translations;
            Vec<f32> default_rotations;
            Vec<f32> default_scales;
            // Static TRS from node. Used if the channel does not exist.
            const i32 node_idx =
                as<i32>((*joints.value.array)[j].value.int_number);
            f32 static_tx = 0.f, static_ty = 0.f, static_tz = 0.f;
            f32 static_rx = 0.f, static_ry = 0.f, static_rz = 0.f,
                static_rw = 1.f;
            f32 static_sx = 1.f, static_sy = 1.f, static_sz = 1.f;
            if ((*gltf)["nodes"][node_idx].is_object() == JsonObject) {
                auto* nd = (*gltf)["nodes"][node_idx].value.object;
                if (nd->contains("translation")) {
                    static_tx =
                        as<f32>((*gltf)["nodes"][node_idx]["translation"][0]
                                    .value.float_number);
                    static_ty =
                        as<f32>((*gltf)["nodes"][node_idx]["translation"][1]
                                    .value.float_number);
                    static_tz =
                        as<f32>((*gltf)["nodes"][node_idx]["translation"][2]
                                    .value.float_number);
                }
                if (nd->contains("rotation")) {
                    static_rx =
                        as<f32>((*gltf)["nodes"][node_idx]["rotation"][0]
                                    .value.float_number);
                    static_ry =
                        as<f32>((*gltf)["nodes"][node_idx]["rotation"][1]
                                    .value.float_number);
                    static_rz =
                        as<f32>((*gltf)["nodes"][node_idx]["rotation"][2]
                                    .value.float_number);
                    static_rw =
                        as<f32>((*gltf)["nodes"][node_idx]["rotation"][3]
                                    .value.float_number);
                }
                if (nd->contains("scale")) {
                    static_sx = as<f32>((*gltf)["nodes"][node_idx]["scale"][0]
                                            .value.float_number);
                    static_sy = as<f32>((*gltf)["nodes"][node_idx]["scale"][1]
                                            .value.float_number);
                    static_sz = as<f32>((*gltf)["nodes"][node_idx]["scale"][2]
                                            .value.float_number);
                }
            }
            if (translation_idx < 0) {
                for (u32 f = 0; f < frames_max; ++f) {
                    default_translations.push_back(static_tx);
                    default_translations.push_back(static_ty);
                    default_translations.push_back(static_tz);
                }
            }
            if (rotation_idx < 0) {
                for (u32 f = 0; f < frames_max; ++f) {
                    default_rotations.push_back(static_rx);
                    default_rotations.push_back(static_ry);
                    default_rotations.push_back(static_rz);
                    default_rotations.push_back(static_rw);
                }
            }
            if (scale_idx < 0) {
                for (u32 f = 0; f < frames_max; ++f) {
                    default_scales.push_back(static_sx);
                    default_scales.push_back(static_sy);
                    default_scales.push_back(static_sz);
                }
            }
            Vec<f32>& bone_t = (translation_idx >= 0)
                ? translations[translation_idx]
                : default_translations;
            Vec<f32>& bone_r = (rotation_idx >= 0) ? rotations[rotation_idx]
                                                   : default_rotations;
            Vec<f32>& bone_s =
                (scale_idx >= 0) ? scales[scale_idx] : default_scales;
            // Channels can have various numbers of frames; frames_max - global
            // maximum. Clamp the index to the last valid channel frame, so as
            // not to run out of the vectors' bounds.
            const auto translation_frames = bone_t.size() / 3;
            const auto rotation_frames = bone_r.size() / 4;
            const auto scale_frames = bone_s.size() / 3;
            Vec<Matrix<f32, 4>> per_frame_matrices;
            for (u32 i = 0; i < frames_max; ++i) {
                if (translation_frames == 0 || rotation_frames == 0
                    || scale_frames == 0) {
                    // Malformed data; skip joint.
                    const Vec<Matrix<f32, 4>> empty;
                    animated_nodes_matrices_accumulator.push_back(empty);
                    continue;
                }
                const auto ti =
                    (i < translation_frames) ? i : translation_frames - 1;
                const auto ri = (i < rotation_frames) ? i : rotation_frames - 1;
                const auto si = (i < scale_frames) ? i : scale_frames - 1;
                Matrix<f32, 4> frame_translation(1.0f);
                Matrix<f32, 4> frame_scale(1.0f);
                for (u32 q = 0; q < 3; ++q) {
                    frame_translation[3][q] = bone_t[ti * 3 + q];
                    frame_scale[q][q] = bone_s[si * 3 + q];
                }
                Quaternion frame_rotation_quaternion;
                Matrix<f32, 4> frame_rotation(1.0f);
                frame_rotation_quaternion.x = bone_r[ri * 4];
                frame_rotation_quaternion.y = bone_r[ri * 4 + 1];
                frame_rotation_quaternion.z = bone_r[ri * 4 + 2];
                frame_rotation_quaternion.w = bone_r[ri * 4 + 3];
                frame_rotation =
                    rotate_quaternion<f32, 4>(frame_rotation_quaternion);
                frame_rotation.self_tensor_transpose();
                const Matrix<f32, 4> local_transform =
                    frame_scale * frame_rotation * frame_translation;
                per_frame_matrices.push_back(local_transform);
            }
            animated_nodes_matrices_accumulator.push_back(per_frame_matrices);
        }
        // Final construction of joint-matrices. Both arrays indexed by
        // joint-index now, that's why nodes_hierarchy[j][b] addresses the
        // accumulator correctly.
        for (u32 j = 0; j < num_joints; ++j) {
            Vec<Matrix<f32, 4>> global_all_frame_node_matrix;
            for (u32 i = 0; i < frames_max; ++i) {
                Matrix<f32, 4> root_transform(1.0f);
                for (u32 b = 0; b < nodes_hierarchy[j].size() - 1; ++b) {
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
    for (u32 i = 0; i < mesh_indices.size(); ++i) {
        indices.push_back(i);
        u32 index = mesh_indices[i] * 3;
        if (index + 2 < vertices_position.size()) {
            Vector<f32, 3> position = {
                vertices_position[index],
                vertices_position[index + 1],
                vertices_position[index + 2]
            };
            if (position[1] > top_y) {
                top_y = position[1];
            }
            vertices.push_back(position[0]);
            vertices.push_back(position[1]);
            vertices.push_back(position[2]);
        }
        if (index + 2 < normals.size()) {
            Vector<f32, 3> normal =
                {normals[index], normals[index + 1], normals[index + 2]};
            vertices.push_back(normal[0]);
            vertices.push_back(normal[1]);
            vertices.push_back(normal[2]);
        }
        index = mesh_indices[i] * 2;
        if (index + 1 < texture_coordinates.size()) {
            vertices.push_back(texture_coordinates[index]);
            vertices.push_back(texture_coordinates[index + 1]);
        }
        index = mesh_indices[i] * 4;
        if (index + 3 < joints_indices.size()) {
            vertices.push_back(joints_indices[index]);
            vertices.push_back(joints_indices[index + 1]);
            vertices.push_back(joints_indices[index + 2]);
            vertices.push_back(joints_indices[index + 3]);
        }
        if (index + 3 < weights_container.size()) {
            vertices.push_back(weights_container[index]);
            vertices.push_back(weights_container[index + 1]);
            vertices.push_back(weights_container[index + 2]);
            vertices.push_back(weights_container[index + 3]);
        }
    }
    delete[] buffer;
    buffer = nullptr;
}

auto JsonParser::traverse_bones(
    Vec<Vec<i32>> children,
    JsonValue joints,
    Vec<u32> node_stack,
    Vec<u32> depth_stack,
    Vec<Vec<u32>>& result
) -> void {
    u32 top_joint_index = 0;
    if (!node_stack.empty()) {
        // Pass array of all joints and root joint and return index of root
        // joint in array.
        top_joint_index = get_joint_index(joints, node_stack.back());
    }
    if (node_stack.size() > depth_stack.size()) {
        // First 0 level start from.
        const u32 first_child = 0;
        depth_stack.push_back(first_child);
    }
    // Main exit check.
    if (depth_stack.empty()) {
        return;
    }
    u32 next_node_index = 0;
    // Check current root joint has any children. Children maps linearly with
    // root joint array index.
    if (top_joint_index != UINT32_MAX && !children[top_joint_index].empty()) {
        // Check if on last child level.
        if (depth_stack.back() > 0
            && depth_stack.back() == children[top_joint_index].size()) {
            depth_stack.pop_back();
            node_stack.pop_back();
            traverse_bones(children, joints, node_stack, depth_stack, result);
            return;
        }
        // Check if not on last child level.
        if (depth_stack.back() > 0
            && depth_stack.back() < children[top_joint_index].size()) {
            next_node_index = children[top_joint_index][depth_stack.back()];
            node_stack.push_back(next_node_index);
            Vec<u32> current_node_indices;
            for (auto i : node_stack) {
                const u32 current_joint_index = get_joint_index(joints, i);
                current_node_indices.push_back(current_joint_index);
            }
            ++depth_stack.back();
            traverse_bones(children, joints, node_stack, depth_stack, result);
            return;
        } else {
            Vec<u32> current_node_indices;
            for (auto i : node_stack) {
                const u32 current_joint_index = get_joint_index(joints, i);
                current_node_indices.push_back(current_joint_index);
            }
            result.push_back(current_node_indices);
            next_node_index = children[top_joint_index][depth_stack.back()];
            node_stack.push_back(next_node_index);
            ++depth_stack.back();
            traverse_bones(children, joints, node_stack, depth_stack, result);
            return;
        }
    } else {
        Vec<u32> current_node_indices;
        if (top_joint_index == UINT32_MAX) {
            current_node_indices.push_back(node_stack.back());
            result.push_back(current_node_indices);
            return;
        }
        for (const auto i : node_stack) {
            const u32 current_joint_index = get_joint_index(joints, i);
            current_node_indices.push_back(current_joint_index);
        }
        result.push_back(current_node_indices);
        depth_stack.pop_back();
        node_stack.pop_back();
        traverse_bones(children, joints, node_stack, depth_stack, result);
        return;
    }
}

auto JsonParser::make_render_joints_indices(Vec<Vec<u32>>& input)
    -> Vec<Vec<u32>> {
    Vec<Vec<u32>> result;
    bool accumulator_flag = false;
    bool inner_flag = false;
    const u32 accumulator = input[0][0];
    for (auto& i : input) {
        for (u32 j = 0; j < i.size(); ++j) {
            Vec<u32> inner;
            for (u32 v = 0; v < j + 1; ++v) {
                if (i[j] == accumulator && accumulator_flag) {
                    inner_flag = false;
                    continue;
                } else {
                    inner_flag = true;
                    inner.push_back(i[v]);

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

auto JsonParser::contains_element(Vec<Vec<u32>> container, u32 element)
    -> bool {
    const bool flag = false;
    for (const auto& i : container) {
        for (const auto j : i) {
            if (j == element) {
                return true;
            }
        }
    }
    return flag;
}

auto JsonParser::get_joint_index(JsonValue joints, i32 searching_index) -> u32 {
    for (u32 i = 0; i < joints.value.array->size(); ++i) {
        const i32 current_joint_index =
            (*joints.value.array)[i].value.int_number;
        if (current_joint_index == searching_index) {
            return i;
        }
    }
    return -1;
}

JsonParser::~JsonParser() {
    delete root;
}
} // namespace glvm

namespace glvm {
MeshManager* MeshManager::instance = nullptr;
Mutex MeshManager::mutex;

MeshManager::MeshManager() {
}

MeshManager::~MeshManager() {
}

auto MeshManager::set_mesh(const char* mesh_path) -> void {
    paths_array.push_back(mesh_path);
}

auto MeshManager::set_mesh_gltf(const char* path_to_mesh) -> void {
    paths_gltf.push_back(path_to_mesh);
}

auto MeshManager::get_instance() -> MeshManager* {
    const MutexGuard<Mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new MeshManager();
    }
    return instance;
}
} // namespace glvm

#ifdef __linux__
#endif

#ifdef _WIN32
#endif

namespace glvm {
auto SoundEngineFactory::create_sound_engine() -> SoundEngine* {
#ifdef __linux__
    return new SoundEngineAlsa;
#endif

#ifdef _WIN32
    return new SoundEngineWaveform;
#endif
}
#ifdef __linux__
SoundEngineAlsa::~SoundEngineAlsa() {
    for (u32 i = 0; i < sound_container.size(); ++i) {
        delete sound_container[i];
        sound_container[i] = nullptr;
    }
}

auto SoundEngineAlsa::open_device(const char* device) -> void {
    snd_pcm_open(&pcm, device, SND_PCM_STREAM_PLAYBACK, 0);
}

auto SoundEngineAlsa::close_device() -> void {
    snd_pcm_drain(pcm);
    snd_pcm_close(pcm);
}

auto SoundEngineAlsa::sound_stream() -> void {
    auto pending = Vec<SoundSample*>();
    {
        MutexGuard<Mutex> lock(sound_mutex);
        pending = sound_container;
        sound_container.clear();
    }
    for (auto* sample : pending) {
        playback_sound_sample(*sample);
        delete sample;
    }
}

auto SoundEngineAlsa::playback_sound_sample(SoundSample& sample) -> void {
    const snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    const snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
    constexpr auto channels = 2;
    // 0.5 s.
    constexpr auto latency = 500000;
    constexpr auto frame_size = channels * 2;
    constexpr auto alsa_frames = 32;

    snd_pcm_set_params(pcm, format, access, channels, sample.ui_rate, 1, latency);

    FILE* file_descriptor = fopen(sample.path_to_file, "r");
    if (file_descriptor == nullptr) {
        return;
    }
    char* buffer = as<char*>(malloc(alsa_frames * frame_size));
    for (i32 i = 0; i < 300; ++i) {
        i32 frames = fread(buffer, frame_size, alsa_frames, file_descriptor);
        if (frames <= 0) {
            break;
        }
        i32 rest = frames;

        i16* samples = reinterpret_cast<i16*>(buffer);
        i32 sample_count = frames * channels;
        for (i32 j = 0; j < sample_count; ++j) {
            i32 scaled = as<i32>(samples[j] * sample.volume);
            samples[j] = as<i16>(std::clamp(scaled, -32768, 32767));
        }

        char* data = buffer;
        while (rest > 0) {
            frames = snd_pcm_writei(pcm, data, rest);
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

auto SoundEngineAlsa::set_master_volume(long volume_percent) -> void {
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

auto SoundEngineAlsa::get_sound_container() -> Vec<SoundSample*>& {
    return sound_container;
}

auto SoundEngineAlsa::create_sound_sample(
    const char* file_path,
    u32 duration,
    u32 rate,
    f32 volume
) -> void {
    auto sample = new SoundSample {file_path, duration, rate, volume};
    MutexGuard<Mutex> lock(sound_mutex);
    sound_container.push_back(sample);
}
#endif // __linux__

#ifdef _WIN32
auto SoundEngineWaveform::open_device(const char* /* device */) -> void {
}

auto SoundEngineWaveform::close_device() -> void {
    played_chunks.clear();
    if (wave_out != nullptr) {
        waveOutClose(wave_out);
        wave_out = nullptr;
    }
}

auto SoundEngineWaveform::create_sound_sample(
    const char* file_path,
    u32 duration,
    u32 rate,
    f32 volume
) -> void {
    auto sample = new SoundSample {file_path, duration, rate, volume};
    const MutexGuard<Mutex> lock(sound_mutex);
    sound_container.push_back(sample);
}
#endif // _WIN32
} // namespace glvm

namespace glvm {
SystemManager* SystemManager::instance = nullptr;
Mutex SystemManager::mutex;

SystemManager::SystemManager() {
}

SystemManager::~SystemManager() {
    delete instance;
    instance = nullptr;
}

auto SystemManager::get_instance() -> SystemManager* {
    const MutexGuard<Mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new SystemManager();
    }
    return instance;
}

auto SystemManager::activate_system(System* system) -> void {
    system_container.push_back(system);
    ++system_count;
}

auto SystemManager::deactivate_system(System* system) -> void {
    for (auto* deactivated : deactivated_systems) {
        if (deactivated == system) {
            return;
        }
    }
    deactivated_systems.push_back(system);
}

auto SystemManager::return_system_to_activated_state(System* system) -> void {
    for (u32 i = 0; i < deactivated_systems.size(); ++i) {
        if (system == deactivated_systems[i]) {
            deactivated_systems.erase(deactivated_systems.begin() + i);
            return;
        }
    }
}

auto SystemManager::update() -> void {
    for (u32 i = 0; i < system_count; ++i) {
        bool skipped = false;
        for (auto* deactivated : deactivated_systems) {
            if (deactivated == system_container[i]) {
                skipped = true;
                break;
            }
        }
        if (!skipped) {
            system_container[i]->update();
        }
    }
}
} // namespace glvm

namespace glvm {
auto SpatialGridSystem::update() -> void {
    SpatialGrid& spatial_grid = world.spatial_grid;
    assert(
        spatial_grid.width > 0 && spatial_grid.height > 0
        && spatial_grid.depth > 0
    );
    const auto chunk_size = spatial_grid.grid[0][0][0].SIZE;

    const auto half_width = spatial_grid.width * chunk_size * 0.5f;
    const auto half_height = spatial_grid.height * chunk_size * 0.5f;
    const auto half_depth = spatial_grid.depth * chunk_size * 0.5f;

    cached_archetypes_number = 0;
    world.search_cache_archetypes(
        required_mask,
        cached_archetypes.data(),
        cached_archetypes_number
    );

    for (u32 i0 = 0; i0 < cached_archetypes_number; ++i0) {
        Archetype* arch = cached_archetypes[i0];
        view.transforms = as<Transform*>(
            arch->components[ComponentsIndices::TransformComponent]
        );
        view.meshes =
            as<Mesh*>(arch->components[ComponentsIndices::MeshComponent]);

        for (u32 i1 = 0; i1 < arch->entity_count; ++i1) {
            const auto entity = arch->entities[i1];
            EntityLocation& entity_location =
                world.entity_locations[get_id(entity)];
            if (!entity_location.is_dirty && is_initialized) {
                continue;
            }

            if (entity_location.grid_cell_counter > 0) {
                for (u32 i2 = 0; i2 < entity_location.grid_cell_counter; ++i2) {
                    const u32 z = entity_location.grid_cell_indices[i2][0];
                    const u32 y = entity_location.grid_cell_indices[i2][1];
                    const u32 x = entity_location.grid_cell_indices[i2][2];
                    Vec<u32>& chunk_entities =
                        spatial_grid.grid[z][y][x].entities;
                    // Remove by value: the recorded index can be stale after
                    // other removals shifted the cell's vector.
                    for (u32 i3 = 0; i3 < chunk_entities.size(); ++i3) {
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

            const MeshHandle entity_mesh_handle = mesh.handle;
            const MeshAxisMaxAbsoluteValues entity_chunk_bounds =
                all_mesh_max_absolute_values[entity_mesh_handle.id];
            Vec<Vector<f32, 3>> entity_box_corner_bound_points =
                compute_box_corner_bound_points(
                    entity_chunk_bounds,
                    transform.position,
                    transform.scale
                );

            // Only the left-bottom-back corner point and the right-upper-front
            // corner point are needed to obtain all box bounds.
            const Vector<f32, 3> min_entity_position =
                entity_box_corner_bound_points[0];
            const Vector<f32, 3> max_entity_position =
                entity_box_corner_bound_points[1];

            i32 index_min_x =
                as<i32>((min_entity_position[0] + half_width) / chunk_size);
            i32 index_min_y =
                as<i32>((min_entity_position[1] + half_height) / chunk_size);
            i32 index_min_z =
                as<i32>((min_entity_position[2] + half_depth) / chunk_size);

            i32 index_max_x =
                as<i32>((max_entity_position[0] + half_width) / chunk_size);
            i32 index_max_y =
                as<i32>((max_entity_position[1] + half_height) / chunk_size);
            i32 index_max_z =
                as<i32>((max_entity_position[2] + half_depth) / chunk_size);

            // Entity can legitimately leave the fixed-size world grid (fell off
            // the world edge, projectile flew away) - clamp to nearest edge
            // cell instead of crashing.
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
                        Vec<u32>& chunk_entities =
                            spatial_grid.grid[i2][i3][i4].entities;
                        if (!is_exist<u32>(chunk_entities, entity)) {
                            chunk_entities.push_back(entity);
                            const auto current_grid_cell =
                                entity_location.grid_cell_counter;
                            assert(current_grid_cell < 32);
                            entity_location
                                .grid_cell_indices[current_grid_cell] =
                                Vector<f32, 3>(i2, i3, i4);
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
TextureManager* TextureManager::instance = nullptr;
Mutex TextureManager::mutex;

TextureManager::TextureManager() = default;

auto TextureManager::bind_texture(u32 entity_id, u32 texture_id) -> void {
    texture_vector[texture_id].entities_own_this_type_of_texture.push_back(
        entity_id
    );
}

auto TextureManager::get_instance() -> TextureManager* {
    const MutexGuard<Mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new TextureManager();
    }
    return instance;
}

auto TextureManager::set_texture_vector(Vec<Texture> textures) -> void {
    texture_vector = textures;
}

auto TextureManager::get_texture_vector() -> Vec<Texture>& {
    return texture_vector;
}
} // namespace glvm

ThreadPool::ThreadPool(usize num_threads) : stop(false) {
    for (usize i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] -> void {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<Mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] -> bool {
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
        const std::unique_lock<Mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) {
        worker.join();
    }
}

#ifdef __linux__

namespace glvm {
TimerX::TimerX() {
    init_frequency();
    reset();
}

auto TimerX::init_frequency() -> f64 {
    return frequency_ = 1e+9;
}

auto TimerX::reset() -> f64 {
    clock_gettime(CLOCK_MONOTONIC, &start_);
    return start_.tv_sec + start_.tv_nsec;
}

auto TimerX::get_elapsed() -> f64 {
    clock_gettime(CLOCK_MONOTONIC, &now_);
    seconds_ = now_.tv_sec - start_.tv_sec;
    nanoseconds_ = now_.tv_nsec - start_.tv_nsec;
    return seconds_ + nanoseconds_ / frequency_;
}
} // namespace glvm
#endif
namespace glvm {
auto TimerCreator::create() -> Chrono* {
#ifdef __linux__
    return new TimerX;
#endif

#ifdef _WIN32
    return new TimerWin;
#endif
}
} // namespace glvm

#ifdef _WIN32

namespace glvm {
TimerWin::TimerWin() {
    init_frequency();
    reset();
}

auto TimerWin::init_frequency() -> f64 {
    QueryPerformanceFrequency(reinterpret_cast<PLARGE_INTEGER>(&i64_freq));
    return as<f64>(i64_freq);
}

auto TimerWin::reset() -> f64 {
    QueryPerformanceCounter(reinterpret_cast<PLARGE_INTEGER>(&i64_start));
    return as<f64>(i64_start);
}

auto TimerWin::get_elapsed() -> f64 {
    QueryPerformanceCounter(reinterpret_cast<PLARGE_INTEGER>(&i64_now));
    return as<f64>((i64_now - i64_start)) / i64_freq;
}
} // namespace glvm
#endif // _WIN32

#ifdef _WIN32

namespace glvm {
auto SoundEngineWaveform::sound_stream() -> void {
    auto pending = Vec<SoundSample*>();
    {
        const MutexGuard<Mutex> lock(sound_mutex);
        pending = sound_container;
        sound_container.clear();
    }
    for (auto* sample : pending) {
        playback_sound_sample(*sample);
        delete sample;
    }
}

auto SoundEngineWaveform::playback_sound_sample(SoundSample& sample) -> void {
    played_chunks.clear();
    if (wave_out == nullptr || device_sample_rate != sample.ui_rate) {
        if (wave_out != nullptr) {
            waveOutClose(wave_out);
            wave_out = nullptr;
        }
        WAVEFORMATEX format;
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 2;
        format.nSamplesPerSec = sample.ui_rate;
        format.nAvgBytesPerSec = format.nSamplesPerSec * format.nChannels * 2;
        // Change this field first if there are any problems.
        format.nBlockAlign = 4;
        format.wBitsPerSample = 16;
        format.cbSize = 0;
        // Open a waveform device for output using a window callback.
        u32 rc = 0;
        rc = waveOutOpen(&wave_out, WAVE_MAPPER, &format, 0L, 0L, 0L);
        if (rc != MMSYSERR_NOERROR) {
            std::cerr << "waveOutOpen: " << "error code: " << rc << std::endl;
            std::exit(-1);
        }
        device_sample_rate = sample.ui_rate;
    }
    std::ifstream file(
        sample.path_to_file,
        std::ios_base::binary | std::ios_base::in
    );
    if (!file) {
        std::cerr << "Failed to open file." << std::endl;
        std::exit(-1);
    }
    auto header = make_box<WAVEHDR>();
    constexpr auto BYTES_PER_FRAME = 4u;
    constexpr auto CHUNK_SECONDS = 2u;
    auto data = Vec<char>(sample.ui_rate * BYTES_PER_FRAME * CHUNK_SECONDS);
    auto chunk_bytes = as<std::streamsize>(data.size());
    while (true) {
        file.read(data.data(), chunk_bytes);
        if (file.gcount() == 0) {
            break;
        }
        auto* samples = reinterpret_cast<i16*>(data.data());
        auto sample_count =
            as<i32>(file.gcount() / as<std::streamsize>(sizeof(i16)));
        for (i32 i = 0; i < sample_count; ++i) {
            const i32 scaled = as<i32>(samples[i] * sample.volume);
            samples[i] = as<i16>(std::clamp(scaled, -32768, 32767));
        }
        header->lpData = data.data();
        header->dwBufferLength = file.gcount();
        header->dwFlags = 0L;
        header->dwLoops = 0L;
        waveOutPrepareHeader(wave_out, header.get(), sizeof(WAVEHDR));
        waveOutWrite(wave_out, header.get(), sizeof(WAVEHDR));
        // Wait until the driver is done with the header. A Sleep estimate
        // assumes real-time playback, but under load (e.g. engine teardown)
        // audio lags behind: unpreparing early lets the wdmaud thread touch
        // freed memory and crash the process.
        auto waited_ms = 0u;
        constexpr auto MAX_WAIT_MS = 5000u;
        while ((header->dwFlags & WHDR_DONE) == 0u && waited_ms < MAX_WAIT_MS) {
            Sleep(1);
            ++waited_ms;
        }
        waveOutUnprepareHeader(wave_out, header.get(), sizeof(WAVEHDR));
    }
    played_chunks.push_back(
        PlayedChunk {.header = std::move(header), .data = std::move(data)}
    );
}

auto SoundEngineWaveform::set_master_volume(long /* volume */) -> void {
}

auto SoundEngineWaveform::get_sound_container() -> Vec<SoundSample*>& {
    return sound_container;
}
} // namespace glvm
#endif // _WIN32

#ifdef _WIN32

extern IMGUI_IMPL_API auto ImGui_ImplWin32_WndProcHandler(
    // NOLINT(readability-identifier-naming)
    HWND h_wnd,
    UINT msg,
    WPARAM w_param,
    LPARAM l_param
) -> LRESULT;

namespace glvm {
WindowWinVulkan* WindowWinVulkan::instance = nullptr;

WindowWinVulkan::WindowWinVulkan() {
    instance = this;
    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);
    const char* title = "Game";
    const i32 window_width = width / 2;
    const i32 window_height = height / 2;
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

    DWORD const style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
        | WS_MAXIMIZEBOX | WS_THICKFRAME;

    RECT rect;
    SetRect(&rect, 0, 0, window_width, window_height);
    AdjustWindowRect(&rect, style, FALSE);
    const i32 window_x = (width - (rect.right - rect.left)) / 2;
    const i32 window_y = (height - (rect.bottom - rect.top)) / 2;

    // Create the main window.
    modern_window = CreateWindowA(
        "Game",
        title,
        style,
        window_x,
        window_y,
        rect.right - rect.left,
        rect.bottom - rect.top,
        as<HWND>(nullptr),
        as<HMENU>(nullptr),
        NULL,
        as<LPVOID>(nullptr)
    );

    // Show the window and paint its contents.
    ShowWindow(modern_window, SW_SHOWDEFAULT);
    UpdateWindow(modern_window);
}

auto WindowWinVulkan::swap_buffers() -> void {
}

auto WindowWinVulkan::clear_display() -> void {
}

auto WindowWinVulkan::handle_event(Event& event) -> bool {
    // Create a message struct object.
    MSG msg;

    SetWindowLongPtrW(
        modern_window,
        GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(&event)
    );
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        input_stack->control_input(event);
        // DispatchMessage may not have set the event (e.g. a WM_CHAR left
        // over from TranslateMessage, or WM_KEYUP for an unhandled key). The
        // stale value would otherwise be re-pushed by ControlInput on the next
        // message/frame and toggle the cursor a second time.
        event.set_event(EventKind::Default);
    }
    return false;
}

auto WindowWinVulkan::close() -> void {
    DestroyWindow(modern_window);
    PostQuitMessage(0);
}

auto WindowWinVulkan::get_classic_window_hwnd() -> HWND {
    return classic_window;
}

auto WindowWinVulkan::get_modern_window_hwnd() -> HWND {
    return modern_window;
}

auto WindowWinVulkan::cursor_lock(
    i32 pointer_x,
    i32 pointer_y,
    i32* out_offset_x,
    i32* out_offset_y
) -> void {
    RECT client_rect;
    GetClientRect(modern_window, &client_rect);
    const auto center_x = client_rect.right / 2;
    const auto center_y = client_rect.bottom / 2;
    POINT point_position {center_x, center_y};
    ClientToScreen(modern_window, &point_position);
    // Solve a problem with endlessly growing numbers on the first game run.
    if (pointer_x > client_rect.right || pointer_x < 0
        || pointer_y > client_rect.bottom || pointer_y < 0) {
        return;
    }
    i32 offset_x = 0, offset_y = 0;
    offset_x = pointer_x - previous_x;
    offset_y = pointer_y - previous_y;
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
    if (offset_x > 250 || offset_x < -250 || offset_y > 250
        || offset_y < -250) {
    } else {
        *out_offset_x += offset_x;
        *out_offset_y -= offset_y;
    }
    // Pitch is limited by angle in Engine::SetViewMatrix(), so this offset may
    // accumulate freely; no pixel clamp here (resolution-independent).
    SetCursorPos(point_position.x, point_position.y);
    SetCursor(NULL);
    // Baseline for the next frame: where the cursor actually ended up after the
    // warp (matches what the next WM_MOUSEMOVE will report).
    POINT actual_position;
    GetCursorPos(&actual_position);
    ScreenToClient(modern_window, &actual_position);
    previous_x = actual_position.x;
    previous_y = actual_position.y;
}

// Callback method for handling events.
auto WindowWinVulkan::main_wnd_proc(
    HWND hwnd,
    UINT msg,
    WPARAM w_param,
    LPARAM l_param
) -> LRESULT {
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param);
    Event* event =
        reinterpret_cast<Event*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (msg == WM_KEYDOWN && w_param == VK_ESCAPE && event != nullptr
        && (l_param & (1 << 30)) == 0) {
        event->set_event(EventKind::CursorReleased);
        return 0;
    }

    if (ImGui::GetCurrentContext()) {
        const ImGuiIO& io = ImGui::GetIO();
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

    if (event == nullptr) {
        return DefWindowProcA(hwnd, msg, w_param, l_param);
    }
    i32 mouse_position_x, mouse_position_y;
    switch (msg) {
        case WM_CREATE:
            return 0;
        case WM_SIZE:
            return 0;
        case WM_LBUTTONDOWN:
            event->set_event(EventKind::MouseLeftButton);
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
            event->set_event(EventKind::MouseLeftButtonRelease);
            event->is_left_mouse_button_released = true;
            return 0;
        case WM_MOUSEMOVE:
            mouse_position_x = GET_X_LPARAM(l_param);
            mouse_position_y = GET_Y_LPARAM(l_param);
            event->set_event(EventKind::MouseMoved);
            event->mouse_pointer_position.position_x = mouse_position_x;
            event->mouse_pointer_position.position_y = mouse_position_y;
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
                    event->set_event(EventKind::MoveForward);
                    break;
                case VK_S:
                    event->set_event(EventKind::MoveBackward);
                    break;
                case VK_A:
                    event->set_event(EventKind::MoveLeft);
                    break;
                case VK_D:
                    event->set_event(EventKind::MoveRight);
                    break;
                case VK_SPACE:
                    event->set_event(EventKind::Jump);
                    break;
                case VK_I:
                    event->set_event(EventKind::InventoryToggle);
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
                    event->set_event(EventKind::KeyReleaseW);
                    break;
                case VK_S:
                    event->set_event(EventKind::KeyReleaseS);
                    break;
                case VK_A:
                    event->set_event(EventKind::KeyReleaseA);
                    break;
                case VK_D:
                    event->set_event(EventKind::KeyReleaseD);
                    break;
                case VK_SPACE:
                    event->set_event(EventKind::KeyReleaseJump);
                    break;
                case VK_I:
                    event->set_event(EventKind::InventoryRelease);
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
            event->set_event(EventKind::GameLoopKill);
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

constexpr auto WIDTH_OFFSET = 1.0f / 3;

Array<f32, 30> VERTICES = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, WIDTH_OFFSET, 0.75f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,         0.75f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,         1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET, 1.0f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.75f
};

Array<f32, 30> VERTICES2 = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET * 2, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, WIDTH_OFFSET * 2, 0.75f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, WIDTH_OFFSET,     0.75f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, WIDTH_OFFSET,     1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET * 2, 1.0f,
    -0.5f, -0.5f, 0.0f, WIDTH_OFFSET,     0.75f
};

Array<f32, 30> VERTICES3 = {
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
    WIDTH_OFFSET * 2,
    0.75f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    WIDTH_OFFSET * 2,
    1.0f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    1.0f,
    -0.5f,
    -0.5f,
    0.0f,
    WIDTH_OFFSET * 2,
    0.75f
};

Array<f32, 30> VERTICES4 = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET, 0.75f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, WIDTH_OFFSET, 0.5f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,         0.5f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,         0.75f, // Up left vertex.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET, 0.75f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f
};

Array<f32, 30> VERTICES5 = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET * 2, 0.75f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, WIDTH_OFFSET * 2, 0.5f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, WIDTH_OFFSET,     0.5f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, WIDTH_OFFSET,     0.75f, // Up left vertex.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET * 2, 0.75f,
    -0.5f, -0.5f, 0.0f, WIDTH_OFFSET,     0.5f
};

Array<f32, 30> VERTICES6 = {
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
    WIDTH_OFFSET * 2,
    0.5f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    WIDTH_OFFSET * 2,
    0.75f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.75f,
    -0.5f,
    -0.5f,
    0.0f,
    WIDTH_OFFSET * 2,
    0.5f
};

Array<f32, 30> VERTICES7 = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET, 0.5f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, WIDTH_OFFSET, 0.25f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,         0.25f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,         0.5f, // Up left vertex.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET, 0.5f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.25f
};

Array<f32, 30> VERTICES8 = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET * 2, 0.5f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, WIDTH_OFFSET * 2, 0.25f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, WIDTH_OFFSET,     0.25f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, WIDTH_OFFSET,     0.5f, // Up left vertex.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET * 2, 0.5f,
    -0.5f, -0.5f, 0.0f, WIDTH_OFFSET,     0.25f
};

Array<f32, 30> VERTICES9 = {
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
    WIDTH_OFFSET * 2,
    0.25f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    WIDTH_OFFSET * 2,
    0.5f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.5f,
    -0.5f,
    -0.5f,
    0.0f,
    WIDTH_OFFSET * 2,
    0.25f
};

Array<f32, 30> VERTICES10 = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET, 0.25f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, WIDTH_OFFSET, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,         0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,         0.25f, // Up left vertex.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET, 0.25f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f
};

Array<f32, 30> VERTICES11 = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET * 2, 0.25f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, WIDTH_OFFSET * 2, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, WIDTH_OFFSET,     0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, WIDTH_OFFSET,     0.25f, // Up left vertex.
    0.5f,  0.5f,  0.0f, WIDTH_OFFSET * 2, 0.25f,
    -0.5f, -0.5f, 0.0f, WIDTH_OFFSET,     0.0f
};

Array<f32, 30> VERTICES12 = {
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
    WIDTH_OFFSET * 2,
    0.0f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    WIDTH_OFFSET * 2,
    0.25f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.25f,
    -0.5f,
    -0.5f,
    0.0f,
    WIDTH_OFFSET * 2,
    0.0f
};

i32 VERTICES_SIZE = as<i32>(VERTICES.size() * sizeof(f32));

namespace glvm {
static auto equals_c_str(const Vec<char>& v, const char* s) -> bool {
    return strcmp(v.data(), s) == 0;
}

WavefrontObjParser::WavefrontObjParser() {
}

auto WavefrontObjParser::get_coordinate_vertices() const
    -> const Vec<Position>& {
    return coordinate_vertices;
}

auto WavefrontObjParser::get_texture_vertices() const -> const Vec<Position>& {
    return texture_vertices;
}

auto WavefrontObjParser::get_normals() const -> const Vec<Position>& {
    return normals;
}

auto WavefrontObjParser::get_faces() const -> const Vec<Face>& {
    return faces;
}

auto WavefrontObjParser::read_file(const char* file_path) -> void {
    std::ifstream wavefront_obj_file_input_stream;
    std::stringstream wavefront_obj_file_output_stream;

    wavefront_obj_file_input_stream.open(file_path);
    if (wavefront_obj_file_input_stream.good()) {
        wavefront_obj_file_output_stream
            << wavefront_obj_file_input_stream.rdbuf();
        wavefront_obj_file_input_stream.close();
        wavefront_obj_file_data = wavefront_obj_file_output_stream.str();
    } else {
        return;
    }

    wavefront_obj_file_data_ptr = wavefront_obj_file_data.c_str();
}

auto WavefrontObjParser::parse_file() -> void {
    while (wavefront_obj_file_data_ptr[cursor] != '\0') {
        Vec<Vec<char>> line =
            split(wavefront_obj_file_data_ptr, ' ', '\n', cursor);
        if (equals_c_str(line[0], "v")) {
            const Position vertex = parse_vertices(line);
            coordinate_vertices.push_back(vertex);
        }
        if (equals_c_str(line[0], "vt")) {
            const Position vertex = parse_vertices(line);
            texture_vertices.push_back(vertex);
        }
        if (equals_c_str(line[0], "vn")) {
            const Position vertex = parse_vertices(line);
            normals.push_back(vertex);
        }
        if (equals_c_str(line[0], "f")) {
            const Face face = parse_faces(line);
            faces.push_back(face);
        }
    }
}

auto WavefrontObjParser::split(
    const char* data,
    const char separator,
    const char exit_symbol,
    u32& position
) -> Vec<Vec<char>> {
    Vec<Vec<char>> words_container;
    u32 outer_index = 0;
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

auto WavefrontObjParser::parse_vertices(Vec<Vec<char>> words) -> Position {
    Position vertex;
    u32 ui_vertex_index = 0;

    const u32 ui_words_container_size = words.size();
    for (u32 i = 1; i < ui_words_container_size; ++i) {
        const f32 float_number = parse_float(words[i]);
        vertex[ui_vertex_index++] = float_number;
    }

    return vertex;
}

auto WavefrontObjParser::parse_faces(Vec<Vec<char>> words) -> Face {
    Face face;
    Vec<Vec<char>> words_inner_container;
    Vec<char> word;

    const u32 ui_words_container_size = words.size();

    for (u32 i = 1; i < ui_words_container_size; ++i) {
        u32 counter = 0;
        words_inner_container = split(words[i].data(), '/', '\0', counter);

        for (u32 j = 0; j < words_inner_container.size(); ++j) {
            word = words_inner_container[j];
            const i32 value = parse_integer(word);

            face[j].push_back(value);
        }
    }
    return face;
}

auto WavefrontObjParser::parse_integer(Vec<char> digits) -> i32 {
    Vec<i32> base_container;

    for (u32 i = 0; i < digits.size() - 1; ++i) {
        base_container.push_back(digits[i] - 48);
    }

    i32 result = 0;
    const bool negate_flag = false;

    const u32 base_container_size = base_container.size();
    for (u32 i = 0; i < base_container_size; ++i) {
        if (negate_flag && i == 0) {
            continue;
        } else if (base_container[i] == -5 && i == 0) {
            continue;
        }

        result +=
            base_container[i] * std::pow(10, (base_container_size - 1) - i);
    }

    return result;
}

auto WavefrontObjParser::parse_float(Vec<char> digits) -> f32 {
    Vec<i32> base_container;

    for (u32 i = 0; i < digits.size() - 1; ++i) {
        base_container.push_back(digits[i] - 48);
    }

    i32 integer_part = 0;
    f32 floating_part = 0;
    Vec<i32> integer_part_container;
    Vec<i32> floating_part_container;
    bool dot_flag = false;
    bool negate_flag = false;
    const u32 base_container_size = base_container.size();

    if (base_container[0] == -3) {
        negate_flag = true;
    }

    for (u32 i = 0; i < base_container_size; ++i) {
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

    const u32 integer_part_container_size = integer_part_container.size();
    for (u32 i = 0; i < integer_part_container_size; ++i) {
        integer_part += integer_part_container[i]
            * std::pow(10, (integer_part_container_size - 1) - i);
    }

    const u32 floating_part_container_size = floating_part_container.size();
    for (u32 i = 0; i < floating_part_container_size; ++i) {
        floating_part +=
            as<f32>(floating_part_container[i] / std::pow(10, i + 1));
    }

    f32 result = 0;
    result = as<f32>((integer_part + floating_part));

    if (negate_flag) {
        result *= -1.0f;
    }

    return result;
}
} // namespace glvm

#ifdef __linux__

namespace glvm {

static WindowWaylandVulkan wayland_window;
static i32 global_pointer_x = 0;
static i32 global_pointer_y = 0;

auto xdg_surface_configure(
    void* data,
    struct xdg_surface* xdg_surface,
    u32 serial
) -> void {
    // The compositor sends a configure event before the surface is shown;
    // it must be acknowledged, otherwise the window never appears.
    WindowWaylandVulkan* config_data = as<WindowWaylandVulkan*>(data);

    xdg_surface_ack_configure(xdg_surface, serial);
    if (!config_data->pixels) {
        resize(data);
    }
}

auto new_frame_callback(
    void* data,
    struct wl_callback* frame_callback,
    u32 callback_data
) -> void {
    wl_callback_destroy(frame_callback);
    frame_callback = wl_surface_frame(wayland_window.wl_surface);
    wl_callback_add_listener(
        frame_callback,
        &wayland_window.callback_listener,
        data
    );
}

auto shell_ping(void* data, struct xdg_wm_base* shell, u32 serial) -> void {
    xdg_wm_base_pong(shell, serial);
}

auto keyboard_keymap(
    void* data,
    struct wl_keyboard* keyboard,
    u32 format,
    i32 keymap_file_descriptor,
    u32 size
) -> void {
}

auto keyboard_enter(
    void* data,
    struct wl_keyboard* keyboard,
    u32 serial,
    struct wl_surface* surface,
    struct wl_array* keys
) -> void {
    WindowWaylandVulkan* wayland_window_data = as<WindowWaylandVulkan*>(data);
    wayland_window_data->is_focused = true;
}

auto keyboard_leave(
    void* data,
    struct wl_keyboard* keyboard,
    u32 serial,
    struct wl_surface* surface
) -> void {
    WindowWaylandVulkan* wayland_window_data = as<WindowWaylandVulkan*>(data);
    wayland_window_data->is_focused = false;
}

auto push_event(EventKind event_type) -> void {
    global_event.set_event(event_type);
    global_input_stack.control_input(global_event);
}

auto keyboard_key(
    void* data,
    struct wl_keyboard* keyboard,
    u32 serial,
    u32 time,
    u32 key,
    u32 state
) -> void {
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        if (key == 1) {
            push_event(EventKind::GameLoopKill);
        }
        if (key == 17) {
            push_event(EventKind::MoveForward);
        }
        if (key == 31) {
            push_event(EventKind::MoveBackward);
        }
        if (key == 30) {
            push_event(EventKind::MoveLeft);
        }
        if (key == 32) {
            push_event(EventKind::MoveRight);
        }
        if (key == 57) {
            push_event(EventKind::Jump);
        }
        if (key == 23) {
            push_event(EventKind::InventoryToggle);
        }
    }

    if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
        if (key == 17) {
            push_event(EventKind::KeyReleaseW);
        }
        if (key == 31) {
            push_event(EventKind::KeyReleaseS);
        }
        if (key == 30) {
            push_event(EventKind::KeyReleaseA);
        }
        if (key == 32) {
            push_event(EventKind::KeyReleaseD);
        }
        if (key == 57) {
            push_event(EventKind::KeyReleaseJump);
        }
        if (key == 23) {
            push_event(EventKind::InventoryRelease);
        }
    }
}

auto keyboard_modifiers(
    void* data,
    struct wl_keyboard* keyboard,
    u32 serial,
    u32 mods_depressed,
    u32 mods_latched,
    u32 mods_locked,
    u32 group
) -> void {
}

auto keyboard_repeat_info(
    void* data,
    struct wl_keyboard* keyboard,
    i32 rate,
    i32 delay
) -> void {
}

auto pointer_enter(
    void* data,
    struct wl_pointer* pointer,
    u32 serial,
    struct wl_surface* surface,
    wl_fixed_t sx,
    wl_fixed_t sy
) -> void {
}

auto pointer_leave(
    void* data,
    struct wl_pointer* pointer,
    u32 serial,
    struct wl_surface* surface
) -> void {
}

auto pointer_motion(
    void* data,
    struct wl_pointer* pointer,
    u32 time,
    wl_fixed_t sx,
    wl_fixed_t sy
) -> void {
}

auto pointer_axis(
    void* data,
    struct wl_pointer* pointer,
    u32 time,
    u32 axis,
    wl_fixed_t value
) -> void {
}

auto pointer_button(
    void* data,
    struct wl_pointer* pointer,
    u32 serial,
    u32 time,
    u32 button,
    u32 state
) -> void {
    if (state == WL_POINTER_BUTTON_STATE_PRESSED && button == 272) {
        push_event(EventKind::MouseLeftButton);
    }
    if (state == WL_POINTER_BUTTON_STATE_RELEASED && button == 272) {
        push_event(EventKind::MouseLeftButtonRelease);
        global_event.is_left_mouse_button_released = true;
    }
    // Hide the cursor on first opportunity.
    if (!wayland_window.hide_and_lock_pointer) {
        wayland_window.hide_and_lock_pointer = true;
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

auto handle_relative_motion(
    void* data,
    struct zwp_relative_pointer_v1* rel_pointer,
    u32 utime_hi,
    u32 utime_lo,
    wl_fixed_t dx,
    wl_fixed_t dy,
    wl_fixed_t dx_unaccel,
    wl_fixed_t dy_unaccel
) -> void {
    global_pointer_x = wl_fixed_to_int(dx);
    global_pointer_y = wl_fixed_to_int(dy);
}

auto seat_capabilities(void* data, struct wl_seat* seat, u32 capabilities)
    -> void {
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

auto seat_name(void* data, struct wl_seat* seat, const char* name) -> void {
}

auto output_geometry(
    void* data,
    struct wl_output* output,
    i32 x,
    i32 y,
    i32 physical_width,
    i32 physical_height,
    i32 subpixel,
    const char* make,
    const char* model,
    i32 transform
) -> void {
}

auto output_mode(
    void* data,
    struct wl_output* output,
    u32 flags,
    i32 width,
    i32 height,
    i32 refresh
) -> void {
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        WindowWaylandVulkan* wayland_window_data =
            as<WindowWaylandVulkan*>(data);
        wayland_window_data->width = width;
        wayland_window_data->height = height;
    }
}

auto output_done(void* data, struct wl_output* output) -> void {
}

auto registry_global(
    void* data,
    struct wl_registry* registry,
    u32 name,
    const char* interface,
    u32 version
) -> void {
    if (!strcmp(interface, wl_compositor_interface.name)) {
        wayland_window.compositor = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, 4)
        );
    } else if (!strcmp(interface, wl_shm_interface.name)) {
        wayland_window.shared_memory = static_cast<wl_shm*>(
            wl_registry_bind(registry, name, &wl_shm_interface, 1)
        );
        wayland_window.pointer_shared_memory = static_cast<wl_shm*>(
            wl_registry_bind(registry, name, &wl_shm_interface, 1)
        );
    } else if (!strcmp(interface, zwp_pointer_constraints_v1_interface.name)) {
        wayland_window.pointer_constraints =
            static_cast<zwp_pointer_constraints_v1*>(wl_registry_bind(
                registry,
                name,
                &zwp_pointer_constraints_v1_interface,
                1
            ));
    } else if (!strcmp(
                   interface,
                   zwp_relative_pointer_manager_v1_interface.name
               )) {
        wayland_window.relative_pointer_manager =
            static_cast<zwp_relative_pointer_manager_v1*>(wl_registry_bind(
                registry,
                name,
                &zwp_relative_pointer_manager_v1_interface,
                1
            ));
    } else if (!strcmp(interface, xdg_wm_base_interface.name)) {
        wayland_window.xdg_shell = static_cast<xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1)
        );
        xdg_wm_base_add_listener(
            wayland_window.xdg_shell,
            &wayland_window.shell_listener,
            0
        );
    } else if (!strcmp(interface, wl_seat_interface.name)) {
        wayland_window.seat = static_cast<wl_seat*>(
            wl_registry_bind(registry, name, &wl_seat_interface, 1)
        );
        wl_seat_add_listener(
            wayland_window.seat,
            &wayland_window.seat_listener,
            data
        );
    } else if (!strcmp(interface, wl_output_interface.name)) {
        struct wl_output* output = static_cast<wl_output*>(
            wl_registry_bind(registry, name, &wl_output_interface, 1)
        );
        wl_output_add_listener(output, &wayland_window.output_listener, data);
    }
}

auto registry_global_remove(void* data, struct wl_registry* registry, u32 name)
    -> void {
}

auto allocate_shared_memory(u64 size) -> i32 {
    Array<char, 8> name;
    name[0] = '/';
    name[7] = 0;
    for (i8 i = 1; i < 6; ++i) {
        name[i] = (rand() & 23) + 97;
    }
    i32 file_descriptor = shm_open(
        name.data(),
        O_RDWR | O_CREAT | O_EXCL,
        S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH
    );
    shm_unlink(name.data());
    i32 result = ftruncate(file_descriptor, size);

    return file_descriptor;
}

auto resize(void* data) -> void {
    // Rendering goes through the Vulkan swapchain; the CPU shm buffer path is
    // not used. ponytail: drop pixels/buffer members if nothing revives it.
}

auto xdg_toplevel_configure(
    void* data,
    struct xdg_toplevel* xdg_toplevel,
    i32 new_width,
    i32 new_height,
    struct wl_array* state
) -> void {
    if (!new_width && !new_height) {
        return;
    }

    WindowWaylandVulkan* toplevel_data = as<WindowWaylandVulkan*>(data);

    if (toplevel_data->width != new_width
        || toplevel_data->height != new_height) {
        toplevel_data->width = new_width;
        toplevel_data->height = new_height;
        resize(data);
    }
}

auto xdg_toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel) -> void {
    WindowWaylandVulkan* toplevel_data = as<WindowWaylandVulkan*>(data);

    toplevel_data->close_xdg_toplevel = 1;
}

WindowWaylandVulkan::WindowWaylandVulkan() {
    // Compositor may never report a size (WSLg sends 0,0); pick a default.
    width = 1280;
    height = 720;
}

auto WindowWaylandVulkan::init() -> void {
    display = wl_display_connect(0);
    registry = wl_display_get_registry(display);
    wl_registry_add_listener(
        registry,
        &registry_listener,
        as<void*>((&wayland_window))
    );
    wl_display_roundtrip(display);
    if (!compositor || !xdg_shell) {
        fprintf(stderr, "Error: compositor or xdg_shell is NULL!\n");
        exit(1);
    }
    wl_surface = wl_compositor_create_surface(compositor);
    pointer_surface = wl_compositor_create_surface(compositor);
    // No frame callback: the event pump in handle_event is non-blocking, so a
    // vsync-paced callback would only add latency (see the Wayland FPS fix).
    xdg_surface = xdg_wm_base_get_xdg_surface(xdg_shell, wl_surface);
    xdg_surface_add_listener(
        xdg_surface,
        &xdg_surface_listener,
        as<void*>((&wayland_window))
    );
    xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
    xdg_toplevel_add_listener(
        xdg_toplevel,
        &xdg_toplevel_listener,
        as<void*>((&wayland_window))
    );
    xdg_toplevel_set_title(xdg_toplevel, "wayland glvm client");
    xdg_toplevel_set_fullscreen(xdg_toplevel, nullptr);
    wl_surface_commit(wl_surface);
}

auto WindowWaylandVulkan::handle_event(Event& event) -> bool {
    event.mouse_pointer_position.position_x = global_pointer_x;
    event.mouse_pointer_position.position_y = global_pointer_y;
    global_pointer_x = 0;
    global_pointer_y = 0;
    while (wl_display_prepare_read(display) != 0) {
        wl_display_dispatch_pending(display);
    }
    wl_display_flush(display);
    pollfd wayland_descriptor {};
    wayland_descriptor.fd = wl_display_get_fd(display);
    wayland_descriptor.events = POLLIN;
    if (poll(&wayland_descriptor, 1, 0) > 0) {
        wl_display_read_events(display);
    } else {
        wl_display_cancel_read(display);
    }
    wl_display_dispatch_pending(display);
    wl_display_flush(display);
    return close_xdg_toplevel != 0;
}

// Create transparent cursor.
auto WindowWaylandVulkan::create_transparent_cursor(struct wl_shm* shm)
    -> struct wl_buffer* {
    i32 size = 4 * 64 * 64; // 64x64 RGBA cursor (common size).
    i32 file_descriptor = allocate_shared_memory(size);
    void* data =
        mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);

    // Fill with transparent pixels.
    for (i32 i = 0; i < 64 * 64; ++i) {
        (as<i32*>(data))[i] = 0x00000000;
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

auto WindowWaylandVulkan::swap_buffers() -> void {
}

auto WindowWaylandVulkan::clear_display() -> void {
}

auto WindowWaylandVulkan::cursor_lock(
    i32 pointer_x,
    i32 pointer_y,
    i32* out_offset_x,
    i32* out_offset_y
) -> void {
    static i32 flag = 0;
    if (flag == 0) {
        *out_offset_x = -(as<i32>(width) / 2);
        *out_offset_y = -(as<i32>(height) / 2);
        ++flag;
    } else {
        *out_offset_x = pointer_x;
        *out_offset_y = pointer_y;
    }
};

auto WindowWaylandVulkan::close() -> void {
    if (keyboard) {
        wl_keyboard_destroy(keyboard);
    }
    // Release the Wayland seat object responsible for managing input devices.
    wl_seat_release(seat);
    if (buffer) {
        wl_buffer_destroy(buffer);
    }
    xdg_toplevel_destroy(xdg_toplevel);
    xdg_surface_destroy(xdg_surface);
    wl_surface_destroy(wl_surface);
    wl_display_disconnect(display);
}

auto initialize_wayland_window() -> WindowWaylandVulkan* {
    wayland_window.xdg_toplevel_listener = {
        .configure = xdg_toplevel_configure,
        .close = xdg_toplevel_close,
        .configure_bounds = nullptr,
        .wm_capabilities = nullptr
    };
    wayland_window.xdg_surface_listener = {.configure = xdg_surface_configure};
    // No callback listener: frame callbacks are not armed anymore.
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
    wayland_window.seat_listener = {
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

namespace glvm {
WindowXVulkan::WindowXVulkan() {
    display = XOpenDisplay(nullptr);
    root_window = DefaultRootWindow(display);
    set_window_attributes.event_mask = KeyPressMask | KeyReleaseMask
        | PointerMotionMask | StructureNotifyMask | ButtonPressMask
        | ButtonReleaseMask | FocusChangeMask;

    const auto screen_number = XDefaultScreen(display);
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
        // Xlib's None/True/CopyFromParent are #undef'd in glvm.hpp; the
        // literals below are their values.
        0,
        InputOutput,
        nullptr,
        CWEventMask,
        &set_window_attributes
    );

    XMapWindow(display, win);

    XWarpPointer(display, 0, win, 0, 0, 0, 0, 0, 0);

    Cursor invisible_cursor;
    Pixmap bitmap_no_data;
    XColor black;
    static Array<char, 8> no_data = {0, 0, 0, 0, 0, 0, 0, 0};
    black.red = black.green = black.blue = 0;

    bitmap_no_data = XCreateBitmapFromData(display, win, no_data.data(), 8, 8);
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

auto WindowXVulkan::get_window() -> Window {
    return win;
}

auto WindowXVulkan::get_display() -> ::Display* {
    return display;
}

auto WindowXVulkan::cursor_lock(
    i32 pointer_x,
    i32 pointer_y,
    i32* out_offset_x,
    i32* out_offset_y
) -> void {
    *out_offset_x += pointer_x - as<i32>((width / 2));
    *out_offset_y -= pointer_y - as<i32>((height / 2));
    // Pitch is limited by angle in Engine::SetViewMatrix(), so this offset may
    // accumulate freely; no pixel clamp here (resolution-independent).
    XWarpPointer(
        display,
        0,
        win,
        0,
        0,
        0,
        0,
        as<i32>((width / 2)),
        as<i32>((height / 2))
    );
    XFlush(display);
}

auto WindowXVulkan::swap_buffers() -> void {
}

auto WindowXVulkan::clear_display() -> void {
}

auto WindowXVulkan::handle_event(Event& event) -> bool {
    XEvent x_event;

    while (XPending(display)) {
        XNextEvent(display, &x_event);
        KeySym key;
        u32 mouse_button;
        XMotionEvent motion;

        switch (x_event.type) {
            case MotionNotify:
                motion = x_event.xmotion;

                event.set_event(EventKind::MouseMoved);
                event.mouse_pointer_position.position_x = motion.x;
                event.mouse_pointer_position.position_y = motion.y;
                [[fallthrough]];
            case MapNotify:
                XGrabPointer(
                    display,
                    win,
                    1,
                    PointerMotionMask,
                    GrabModeAsync,
                    GrabModeAsync,
                    win,
                    0,
                    CurrentTime
                );
                break;
            case FocusIn:
                is_focused = true;
                XGrabPointer(
                    display,
                    win,
                    1,
                    PointerMotionMask,
                    GrabModeAsync,
                    GrabModeAsync,
                    win,
                    0,
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
                        event.set_event(EventKind::MouseLeftButton);
                        break;
                }
                break;

            case ButtonRelease:
                mouse_button = x_event.xbutton.button;
                switch (mouse_button) {
                    case 1:
                        event.set_event(EventKind::MouseLeftButtonRelease);
                        event.is_left_mouse_button_released = true;
                        break;
                }
                break;

            case KeyPress:
                key = XLookupKeysym(&x_event.xkey, 0);
                switch (key) {
                    case XKEY_I:
                        event.set_event(EventKind::InventoryToggle);
                        break;
                    case XKEY_ESCAPE:
                        event.set_event(EventKind::GameLoopKill);
                        break;
                    case XKEY_A:
                        event.set_event(EventKind::MoveLeft);
                        break;
                    case XKEY_D:
                        event.set_event(EventKind::MoveRight);
                        break;
                    case XKEY_S:
                        event.set_event(EventKind::MoveBackward);
                        break;
                    case XKEY_W:
                        event.set_event(EventKind::MoveForward);
                        break;
                    case XKEY_SPACE:
                        event.set_event(EventKind::Jump);
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
                        event.set_event(EventKind::InventoryRelease);
                        break;
                    case XKEY_A:
                        event.set_event(EventKind::KeyReleaseA);
                        break;
                    case XKEY_D:
                        event.set_event(EventKind::KeyReleaseD);
                        break;
                    case XKEY_S:
                        event.set_event(EventKind::KeyReleaseS);
                        break;
                    case XKEY_W:
                        event.set_event(EventKind::KeyReleaseW);
                        break;
                    case XKEY_SPACE:
                        event.set_event(EventKind::KeyReleaseJump);
                        break;
                }
                break;
        }

        global_input_stack.control_input(event);
    }
    return false;
}

auto WindowXVulkan::close() -> void {
    XDestroyWindow(display, win);
    XCloseDisplay(display);
}
} // namespace glvm

#include <xcb/xcb_cursor.h>
#include <xcb/xcb_keysyms.h>

namespace glvm {
WindowXCBVulkan::WindowXCBVulkan() {
    // Open the connection to the X server.
    connection = xcb_connect(nullptr, nullptr);
    i32 error = xcb_connection_has_error(connection);
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

    u32 event_mask = 0;
    event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    Array<u32, 2> event_flags;
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
        event_flags.data()
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

auto WindowXCBVulkan::configure_window() -> void {
    u16 mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
        | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
    const Array<u32, 4> values = {
        320, // x.
        180, // y.
        width,
        height
    };

    xcb_configure_window(connection, window, mask, values.data());
    xcb_flush(connection);
}

auto WindowXCBVulkan::hide_cursor() -> void {
    xcb_pixmap_t foreground_pixmap_id = xcb_generate_id(connection);
    xcb_create_pixmap(connection, 1, foreground_pixmap_id, window, 8, 8);

    // Create graphical context.
    xcb_gcontext_t graphical_context = xcb_generate_id(connection);

    u32 mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    Array<u32, 2> values_list;
    values_list[0] = screen->black_pixel;
    values_list[1] = screen->white_pixel;

    xcb_create_gc(
        connection,
        graphical_context,
        window,
        XCB_GC_FOREGROUND | XCB_GC_BACKGROUND,
        values_list.data()
    );

    const Array<u8, 32> pix_map_data = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

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
        pix_map_data.data()
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
    u32 value_list = cursor;
    xcb_change_window_attributes(connection, window, mask, &value_list);

    xcb_free_cursor(connection, cursor);
}

auto WindowXCBVulkan::get_connection() -> xcb_connection_t* {
    return connection;
}

auto WindowXCBVulkan::get_window() -> u32 {
    return window;
}

auto WindowXCBVulkan::disconnect() -> void {
    xcb_disconnect(connection);
}

auto WindowXCBVulkan::swap_buffers() -> void {
}

auto WindowXCBVulkan::clear_display() -> void {
}

auto WindowXCBVulkan::handle_event([[maybe_unused]] Event& event) -> bool {
    xcb_generic_event_t* generic_event;
    bool next_generic_event_flag = false;
    while (next_generic_event_flag
           || (generic_event = xcb_poll_for_event(connection))) {
        next_generic_event_flag = false;
        switch (generic_event->response_type & ~0x80) {
            case XCB_EXPOSE: {
                [[maybe_unused]] xcb_expose_event_t* expose_event =
                    reinterpret_cast<xcb_expose_event_t*>(generic_event);

                if (!is_window_resize_read) {
                    width = expose_event->width;
                    height = expose_event->height;
                    is_window_resize_read = true;
                }
                break;
            }
            case XCB_BUTTON_PRESS: {
                xcb_button_press_event_t* expose_event =
                    reinterpret_cast<xcb_button_press_event_t*>(generic_event);
                switch (expose_event->detail) {
                    case 1:
                        event.set_event(EventKind::MouseLeftButton);
                        break;
                    case 3:
                        event.set_event(EventKind::MouseRightButton);
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
                    reinterpret_cast<xcb_button_release_event_t*>(generic_event);
                switch (expose_event->detail) {
                    case 1:
                        event.set_event(EventKind::MouseLeftButtonRelease);
                        event.is_left_mouse_button_released = true;
                        break;
                    case 3:
                        event.set_event(EventKind::MouseRightButtonRelease);
                        break;
                }

                break;
            }
            case XCB_MOTION_NOTIFY: {
                xcb_motion_notify_event_t* expose_event =
                    reinterpret_cast<xcb_motion_notify_event_t*>(generic_event);

                event.set_event(EventKind::MouseMoved);
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
                    reinterpret_cast<xcb_enter_notify_event_t*>(generic_event);
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
                    reinterpret_cast<xcb_key_press_event_t*>(generic_event);
                xcb_keysym_t keysym =
                    xcb_key_press_lookup_keysym(key_symbols, expose_event, 0);
                switch (keysym) {
                    case 65307:
                        event.set_event(EventKind::GameLoopKill);
                        break;
                    case 105:
                        event.set_event(EventKind::InventoryToggle);
                        break;
                    case 97:
                        event.set_event(EventKind::MoveLeft);
                        break;
                    case 100:
                        event.set_event(EventKind::MoveRight);
                        break;
                    case 115:
                        event.set_event(EventKind::MoveBackward);
                        break;
                    case 119:
                        event.set_event(EventKind::MoveForward);
                        break;
                    case 32:
                        event.set_event(EventKind::Jump);
                        break;
                }

                break;
            }
            case XCB_KEY_RELEASE: {
                xcb_key_release_event_t* key_release_event =
                    reinterpret_cast<xcb_key_release_event_t*>(generic_event);
                next_generic_event = xcb_poll_for_event(connection);
                if (next_generic_event != nullptr) {
                    xcb_key_press_event_t* key_press_event =
                        reinterpret_cast<xcb_key_press_event_t*>(
                            next_generic_event
                        );
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
                        event.set_event(EventKind::InventoryRelease);
                        break;
                    case 97:
                        event.set_event(EventKind::KeyReleaseA);
                        break;
                    case 100:
                        event.set_event(EventKind::KeyReleaseD);
                        break;
                    case 115:
                        event.set_event(EventKind::KeyReleaseS);
                        break;
                    case 119:
                        event.set_event(EventKind::KeyReleaseW);
                        break;
                    case 32:
                        event.set_event(EventKind::KeyReleaseJump);
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
        global_input_stack.control_input(event);
    }
    is_window_resize_read = false;
    return false;
}

auto WindowXCBVulkan::close() -> void {
    xcb_key_symbols_free(key_symbols);
    xcb_disconnect(connection);
}

auto WindowXCBVulkan::cursor_lock(
    i32 pointer_x,
    i32 pointer_y,
    i32* out_offset_x,
    i32* out_offset_y
) -> void {
    *out_offset_x += pointer_x - as<i32>((width / 2));
    *out_offset_y -= pointer_y - as<i32>((height / 2));
    xcb_warp_pointer(
        connection,
        XCB_NONE,
        window,
        0,
        0,
        0,
        0,
        as<i32>((width / 2)),
        as<i32>((height / 2))
    );
    xcb_flush(connection);
}
} // namespace glvm
#endif // __linux__
