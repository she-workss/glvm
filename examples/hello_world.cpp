#include "assets/textures/chelik.h"
#include "assets/textures/container2.h"
#include "assets/textures/container2_specular.h"
#include "assets/textures/crosshair.h"
#include "assets/textures/fontAtlas.h"
#include "assets/textures/gray.h"
#include "assets/textures/inventorySlot.h"
#include "assets/textures/tileset.h"
#include "assets/textures/witch.h"
#include "glvm/glvm.hpp"

#include <cstdio>
#include <map>
#include <random>

using namespace glvm;

auto main() -> i32 {
    auto* entity_manager = EntityManager::get_instance();
    auto* component_manager = ComponentManager::get_instance();
    auto* arch_entity_manager = ArchetypeEntityManager::get_instance();
    auto* engine = Engine::get_instance();
    auto hyper_cube = engine->load_mesh_from_gltf(
        "../../../examples/assets/gltf/hyper_cube.gltf"
    );
    auto hyper_cube2 = engine->load_mesh_from_gltf(
        "../../../examples/assets/gltf/hyper_cube2.gltf"
    );
    auto mega_chel = engine->load_mesh_from_gltf(
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
    auto chelik_texture =
        engine->load_texture_from_address(128, 96, chelik_dat_len, chelik_dat);
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
        Crosshair_dat_len,
        Crosshair_dat
    );
    auto font_atlas_texture = engine->load_texture_from_address(
        84,
        132,
        fontAtlas_dat_len,
        fontAtlas_dat
    );
    auto inventory_texture = engine->load_texture_from_address(
        64,
        64,
        inventorySlot_dat_len,
        inventorySlot_dat
    );
    auto tileset_texture =
        engine
            ->load_texture_from_address(512, 512, tileset_dat_len, tileset_dat);
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
        WORLD.archetypes.push_back(level_chunk_arch);
        WORLD.archetypes.push_back(player_arch);
        WORLD.archetypes.push_back(enemy_arch);
        WORLD.archetypes.push_back(projectile_arch);
        WORLD.archetypes.push_back(static_mesh_arch);
        WORLD.archetypes.push_back(crosshair_arch);
        WORLD.archetypes.push_back(inventory_arch);
        WORLD.archetypes.push_back(item_arch);
        WORLD.archetypes.push_back(directional_light_arch);
        WORLD.archetypes.push_back(point_light_arch);
        WORLD.archetypes.push_back(spot_light_arch);
    }
    auto player = arch_entity_manager->create_entity();
    WORLD.add_entity_to_archetype(player, WORLD.archetypes[1]);
    auto player_location = WORLD.entity_locations[get_id(player)];
    auto* player_arch = dynamic_cast<PlayerArchetype*>(player_location.arch);
    const auto player_index = player_location.index;
    player_arch->transforms[player_index] = {
        .position = {15.0f, 15.0f, 15.0f},
        .scale = 1.0f
    };
    player_arch->rigid_bodies[player_index] = {.f_mass = 3.0f};
    player_arch->health[player_index] = {
        .max_health = 100,
        .current_health = 100
    };
    player_arch->beholders[player_index] = {
        .position = {0.0f, 2.0f, -3.0f},
        .forward = {0.0f, 0.0f, -1.0f}
    };
    player_arch->meshes[player_index] = {.handle = mega_chel, .gltf = true};
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
        WORLD.add_entity_to_archetype(enemy, WORLD.archetypes[2]);
        auto enemy_location = WORLD.entity_locations[get_id(enemy)];
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
        enemy_arch->rigid_bodies[enemy_index] = {.f_mass = 0.0f};
        enemy_arch->enemies[enemy_index] = {.detect_radius = 10.0f};
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
        enemy_font_component->life_time = 0.0f;
        enemy_font_component->removeble = false;
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
        WORLD.add_entity_to_archetype(cube, WORLD.archetypes[4]);
        auto cube_location = WORLD.entity_locations[get_id(cube)];
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
    WORLD.add_entity_to_archetype(crosshair, WORLD.archetypes[5]);
    auto crosshair_location = WORLD.entity_locations[get_id(crosshair)];
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
    WORLD.add_entity_to_archetype(inventory, WORLD.archetypes[6]);
    auto inventory_location = WORLD.entity_locations[get_id(inventory)];
    auto* inventory_arch =
        dynamic_cast<InventoryArchetype*>(inventory_location.arch);
    const auto inventory_index = inventory_location.index;
    auto* inventory_component = &inventory_arch->invetories[inventory_index];
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
        WORLD.add_entity_to_archetype(item, WORLD.archetypes[7]);
        EntityLocation item_location = WORLD.entity_locations[get_id(item)];
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
        item_arch->rigid_bodies[item_index] = {.f_mass = 0.0f};
        item_arch->meshes[item_index].handle = hyper_cube;
        item_arch->materials[item_index] = {
            .diffuse_texture_id = container2,
            .specular_texture_id = container2_specular_texture,
            .ambient = {0.05f, 0.05f, 0.05f},
            .shininess = 128.0f * 0.078125f
        };
    }
    auto directional_light = arch_entity_manager->create_entity();
    WORLD.add_entity_to_archetype(directional_light, WORLD.archetypes[8]);
    auto directional_light_location =
        WORLD.entity_locations[get_id(directional_light)];
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
    WORLD.add_entity_to_archetype(point_light, WORLD.archetypes[9]);
    auto point_light_location = WORLD.entity_locations[get_id(point_light)];
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
    WORLD.add_entity_to_archetype(spot_light, WORLD.archetypes[10]);
    auto spot_light_location = WORLD.entity_locations[get_id(spot_light)];
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
    engine->game_loop();
    engine->game_kill();
    delete entity_manager;
    delete component_manager;
    delete engine;
}
