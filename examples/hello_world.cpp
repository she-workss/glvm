#include "glvm/glvm.hpp"
#include "glvm_log/prelude.hpp"

#include <cmath>

using namespace glvm;
using namespace glvm_log::prelude;

// Only base engine components: no registration needed, the engine
// pre-registers all of these for generic swap-remove.
struct DemoCameraArchetype: Archetype {
    Array<Transform, 4> transforms;
    Array<Beholder, 4> beholders;

    DemoCameraArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::ViewComponent] = beholders.data();
        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::ViewComponent);
        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::ViewComponent;
        component_count = 2;
    }
};

struct RectArchetype: Archetype {
    Array<Transform, 4> transforms;
    Array<Mesh, 4> meshes;
    Array<Material, 4> materials;

    RectArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms.data();
        components[ComponentsIndices::MeshComponent] = meshes.data();
        components[ComponentsIndices::MaterialComponent] = materials.data();
        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent);
        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::MaterialComponent;
        component_count = 3;
    }
};

struct DemoLightArchetype: Archetype {
    Array<Transform, 4> transforms;
    Array<Mesh, 4> meshes;
    Array<Material, 4> materials;
    Array<DirectionalLightComponent, 4> directional_lights;

    DemoLightArchetype() {
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

auto main() -> i32 {
    glvm_log::info("hello", "main start");
    auto* arch_entity_manager = ArchetypeEntityManager::get_instance();
    auto* engine = Engine::get_instance();
    Array<u8, 32 * 32 * 4> white_pixels;
    white_pixels.fill(255);
    auto white_texture = engine->load_texture_from_address(
        32,
        32,
        32 * 32 * 4,
        white_pixels.data()
    );
    auto rect_mesh = engine->load_mesh_from_gltf(
        "../../../examples/assets/gltf/hyper_plane.gltf"
    );
    auto* camera_arch = new DemoCameraArchetype;
    auto* rect_arch = new RectArchetype;
    auto* light_arch = new DemoLightArchetype;
    world.archetypes.push_back(camera_arch);
    world.archetypes.push_back(rect_arch);
    world.archetypes.push_back(light_arch);
    auto camera = arch_entity_manager->create_entity();
    world.add_entity_to_archetype(camera, camera_arch);
    auto rect = arch_entity_manager->create_entity();
    world.add_entity_to_archetype(rect, rect_arch);
    auto light = arch_entity_manager->create_entity();
    world.add_entity_to_archetype(light, light_arch);
    const auto rect_index = world.entity_locations[get_id(rect)].index;
    rect_arch->transforms[rect_index] = {
        .position = {0.0f, 0.8f, 0.0f},
        .scale = 0.05f
    };
    rect_arch->meshes[rect_index] = {.handle = rect_mesh, .gltf = true};
    rect_arch->materials[rect_index] = {
        .diffuse_texture_id = white_texture,
        .specular_texture_id = white_texture,
        .ambient = {0.25f, 0.25f, 0.25f},
        .shininess = 32.0f * 0.078125f
    };
    light_arch->directional_lights[0] = {
        .position = {0.0f, 25.0f, 15.0f},
        .direction = {1.0f, 10.0f, 0.0f},
        .ambient = {0.05f, 0.05f, 0.05f},
        .diffuse = {0.4f, 0.4f, 0.4f},
        .specular = {1.0f, 1.0f, 1.0f}
    };
    light_arch->transforms[0] = {
        .position = {0.0f, 10.0f, -15.0f},
        .scale = 0.1f
    };
    light_arch->meshes[0].handle = rect_mesh;
    light_arch->materials[0] = {
        .diffuse_texture_id = white_texture,
        .specular_texture_id = white_texture
    };
    const Vector<f32, 3> center = {0.0f, 0.8f, 0.0f};
    auto* camera_arch_ptr = dynamic_cast<DemoCameraArchetype*>(
        world.entity_locations[get_id(camera)].arch
    );
    const auto camera_index = world.entity_locations[get_id(camera)].index;
    f32 orbit_angle = 0.0f;
    engine->set_pre_update_hook([&]() -> void {
        orbit_angle += 0.8f * engine->get_delta_frame_time();
        camera_arch_ptr->beholders[camera_index].position = {
            center[0] + (5.0f * std::cos(orbit_angle)),
            center[1] + 1.2f,
            center[2] + (5.0f * std::sin(orbit_angle))
        };
        camera_arch_ptr->beholders[camera_index].forward = normalize(
            center - camera_arch_ptr->beholders[camera_index].position
        );
        engine->renderer()->forward =
            camera_arch_ptr->beholders[camera_index].forward;
        global_event.mouse_pointer_position.offset_x = 0;
        global_event.mouse_pointer_position.offset_y = 0;
    });
    engine->set_frame_data_hook([&](u32 actor_counter) -> u32 {
        auto* renderer = engine->renderer();
        renderer->actors.push_back({});
        renderer->actors[actor_counter].model_matrix =
            engine->compute_model_matrix(
                &rect_arch->transforms[rect_index],
                0.0f
            );
        renderer->actors[actor_counter].joint_matrices =
            Vec<Matrix<f32, 4>>(MAX_JOINTS_NUMBER, Matrix<f32, 4>(1.0f));
        renderer->actors[actor_counter].mesh_id =
            rect_arch->meshes[rect_index].handle.id;
        renderer->actors[actor_counter].diffuse_texture_index =
            rect_arch->materials[rect_index].diffuse_texture_id.id;
        renderer->actors[actor_counter].specular_texture_index =
            rect_arch->materials[rect_index].specular_texture_id.id;
        renderer->actors[actor_counter].ambient =
            rect_arch->materials[rect_index].ambient;
        renderer->actors[actor_counter].shininess =
            rect_arch->materials[rect_index].shininess;
        return actor_counter + 1;
    });
    engine->add_base_systems();
    glvm_log::info("hello", "scene built, entering game loop");
    engine->game_loop();
    engine->game_kill();
    delete engine;
    glvm_log::info("hello", "teardown done, exiting");
}
