#include "glvm/ArchetypeECS/ArchECS_Utils.hpp"
#include "glvm/ArchetypeECS/ArchECS_World.hpp"
#include "glvm/Archetypes/CrosshairArchetype.hpp"
#include "glvm/Archetypes/InventoryArchetype.hpp"
#include "glvm/Archetypes/LevelChunkArchetype.hpp"
#include "glvm/Archetypes/RigidBodyArchetype.hpp"
#include "glvm/Archetypes/SpotLightArchetype.hpp"
#include "glvm/Archetypes/StaticMeshArchetype.hpp"
#include "glvm/Components/ActorComponent.hpp"
#include "glvm/Components/AnimationMoveComponent.hpp"
#include "glvm/Components/ColliderComponent.hpp"
#include "glvm/Components/CrosshairComponent.hpp"
#include "glvm/Components/EnemyComponent.hpp"
#include "glvm/Components/FontComponent.hpp"
#include "glvm/Components/HealthComponent.hpp"
#include "glvm/Components/InterfaceComponent.hpp"
#include "glvm/Components/InventoryComponent.hpp"
#include "glvm/Components/InventorySlotComponent.hpp"
#include "glvm/Components/ItemComponent.hpp"
#include "glvm/Components/MaterialComponent.hpp"
#include "glvm/Components/RigidBodyComponent.hpp"
#include "glvm/Components/TransformComponent.hpp"
#include "glvm/Components/VertexComponent.hpp"
#include "glvm/Engine.hpp"
#include "glvm/PGA.hpp"
#include "glvm/SpritesData.hpp"
#include "glvm/Texture.hpp"
#include "glvm/VertexMath.hpp"

#include <cstdio>
#include <map>
#include <random>

using namespace glvm;
namespace pga = glvm::core::pga;
namespace arch = glvm::ecs::arch;

auto main() -> int {
    pga::plane plane0 = {.x = 2.1f, .y = 3.5f, .z = 4.2f, .w = 3.87f};
    pga::plane plane1 = {.x = 3.17f, .y = 10.20f, .z = 7.832f, .w = 3.87f};
    pga::point point0 = {.x = 1.5f, .y = 2.77f, .z = 6.55f, .w = 8.99f};
    pga::point point1 = {.x = 3.577f, .y = 0.787f, .z = 16.575f, .w = 888.99f};
    pga::line line0 = {
        .rx = 1.87,
        .ry = 2.053,
        .rz = 6.234,
        .ix = 10.34,
        .iy = 3234.32,
        .iz = 223.43
    };
    pga::line line1 = {
        .rx = 5.723,
        .ry = 10.234,
        .rz = 3.343,
        .ix = 0.344,
        .iy = 234.123,
        .iz = 77.345
    };
    pga::rline rline0 = {.rx = 21.87, .ry = 25.053, .rz = 63.234};
    pga::rline rline1 = {.rx = 15.723, .ry = 510.234, .rz = 73.343};
    auto* entity_manager = ecs::EntityManager::GetInstance();
    auto* component_manager = ecs::ComponentManager::GetInstance();
    auto* arch_entity_manager = arch::ArchetypeEntityManager::getInstance();
    auto* engine = core::Engine::GetInstance();
    auto hyper_cube =
        engine->LoadMeshFromFile_GLTF("../../../assets/gltf/hyper_cube.gltf");
    auto hyper_cube2 =
        engine->LoadMeshFromFile_GLTF("../../../assets/gltf/hyper_cube2.gltf");
    auto mega_chel =
        engine->LoadMeshFromFile_GLTF("../../../assets/gltf/mega_chel.gltf");
    auto simple_cube =
        engine->LoadMeshFromFile_GLTF("../../../assets/gltf/simpleCube2.gltf");
    auto crosshair_001_handle = engine->LoadMeshFromFile_GLTF(
        "../../../assets/gltf/crosshair_001.gltf"
    );
    auto inventory_handle =
        engine->LoadMeshFromFile_GLTF("../../../assets/gltf/inventory.gltf");
    auto cyborg_handle =
        engine->LoadMeshFromFile_GLTF("../../../assets/gltf/cyborg11.gltf");
    auto robot0_handle =
        engine->LoadMeshFromFile_GLTF("../../../assets/gltf/scene.gltf");

    auto chelik_texture =
        engine->LoadTextureFromAddress(128, 96, chelik_dat_len, chelik_dat);
    auto witch_texture =
        engine->LoadTextureFromAddress(32, 32, witch_dat_len, witch_dat);
    auto gray_texture =
        engine->LoadTextureFromAddress(32, 32, gray_dat_len, gray_dat);
    auto container2 = engine->LoadTextureFromAddress(
        500,
        500,
        container2_dat_len,
        container2_dat
    );
    auto container2_specular_texture = engine->LoadTextureFromAddress(
        500,
        500,
        container2_specular_dat_len,
        container2_specular_dat
    );
    auto crosshair_texture =
        engine->LoadTextureFromAddress(32, 32, Crosshair_dat_len, Crosshair_dat);
    auto font_atlas_texture =
        engine
            ->LoadTextureFromAddress(84, 132, fontAtlas_dat_len, fontAtlas_dat);
    auto inventory_texture = engine->LoadTextureFromAddress(
        64,
        64,
        inventorySlot_dat_len,
        inventorySlot_dat
    );
    auto tileset_texture =
        engine->LoadTextureFromAddress(512, 512, tileset_dat_len, tileset_dat);
    {
        auto* level_chunk_arch = new arch::LevelChunkArchetype;
        auto* player_arch = new arch::PlayerArchetype;
        auto* enemy_arch = new arch::EnemyArchetype;
        auto* projectile_arch = new arch::ProjectileArchetype;
        auto* static_mesh_arch = new arch::StaticMeshArchetype;
        auto* crosshair_arch = new arch::CrosshairArchetype;
        auto* inventory_arch = new arch::InventoryArchetype;
        auto* item_arch = new arch::ItemArchetype;
        auto* directional_light_arch = new arch::DirectionalLightArchetype;
        auto* point_light_arch = new arch::PointLightArchetype;
        auto* spot_light_arch = new arch::SpotLightArchetype;
        arch::world.archetypes.Push(level_chunk_arch);
        arch::world.archetypes.Push(player_arch);
        arch::world.archetypes.Push(enemy_arch);
        arch::world.archetypes.Push(projectile_arch);
        arch::world.archetypes.Push(static_mesh_arch);
        arch::world.archetypes.Push(crosshair_arch);
        arch::world.archetypes.Push(inventory_arch);
        arch::world.archetypes.Push(item_arch);
        arch::world.archetypes.Push(directional_light_arch);
        arch::world.archetypes.Push(point_light_arch);
        arch::world.archetypes.Push(spot_light_arch);
    }
    auto player = arch_entity_manager->createEntity();
    arch::world.addEntityToArchetype(player, arch::world.archetypes[1]);
    auto player_location = arch::world.entityLocations[arch::getId(player)];
    auto* player_arch =
        dynamic_cast<arch::PlayerArchetype*>(player_location.arch);
    const auto player_index = player_location.index;
    player_arch->transforms[player_index] = {
        .position = {15.0f, 15.0f, 15.0f},
        .scale = 1.0f
    };
    player_arch->rigidBodies[player_index] = {.fMass_ = 3.0f};
    player_arch->health[player_index] = {.maxHealth = 100, .currentHealth = 100};
    player_arch->beholders[player_index] = {
        .Position = {0.0f, 2.0f, -3.0f},
        .forward = {0.0f, 0.0f, -1.0f}
    };
    player_arch->meshes[player_index] = {.handle = mega_chel, .gltf = true};
    player_arch->materials[player_index] = {
        .diffuseTextureID_ = gray_texture,
        .specularTextureID_ = gray_texture,
        .ambient = {0.05f, 0.05f, 0.0f},
        .shininess = 128.0f * 0.078125f
    };
    std::random_device rd;
    std::map<int, int> hist;
    std::mt19937 mersenne(rd());
    std::uniform_int_distribution<int> dist(0, 3);
    for (auto i = 0; i < 5; ++i) {
        auto enemy = arch_entity_manager->createEntity();
        arch::world.addEntityToArchetype(enemy, arch::world.archetypes[2]);
        auto enemy_location = arch::world.entityLocations[arch::getId(enemy)];
        auto* enemy_arch =
            dynamic_cast<arch::EnemyArchetype*>(enemy_location.arch);
        const auto enemy_index = enemy_location.index;
        auto random = dist(mersenne);
        vec3 random_direction = {};
        switch (random) {
            case 0:
                random_direction = vec3(3.0f, 0.0f, 0.0f, 0.0);
                break;
            case 1:
                random_direction = vec3(-3.0f, 0.0f, 0.0f, 0.0);
                break;
            case 2:
                random_direction = vec3(0.0f, 0.0f, 3.0f, 0.0);
                break;
            case 3:
                random_direction = vec3(0.0f, 0.0f, -3.0f, 0.0);
                break;
            default:
                break;
        }
        enemy_arch->transforms[enemy_index] = {
            .position =
                {vec3(static_cast<float>(i) * 5, 3.0f, -3.0f)
                 + random_direction},
            .scale = 0.02f
        };
        enemy_arch->states[enemy_index] = {.state = core::States::ROAMING};
        enemy_arch->rigidBodies[enemy_index] = {.fMass_ = 0.0f};
        enemy_arch->enemies[enemy_index] = {.detectRadius = 10.0f};
        enemy_arch->health[enemy_index] = {
            .maxHealth = 100,
            .currentHealth = 100
        };
        auto* enemy_font_component = &enemy_arch->fonts[enemy_index];
        if (i < 3) {
            enemy_font_component->font_string.Push('1');
        } else if (i < 6) {
            enemy_font_component->font_string.Push('1');
            enemy_font_component->font_string.Push('0');
        } else if (i < 10) {
            enemy_font_component->font_string.Push('1');
            enemy_font_component->font_string.Push('0');
            enemy_font_component->font_string.Push('E');
        } else {
            enemy_font_component->font_string.Push('J');
            enemy_font_component->font_string.Push('r');
        }
        enemy_font_component->lifeTime = 0.0f;
        enemy_font_component->removeble = false;
        enemy_arch->meshes[enemy_index] = {
            .handle = robot0_handle,
            .gltf = true
        };
        enemy_arch->materials[enemy_index] = {
            .diffuseTextureID_ = gray_texture,
            .specularTextureID_ = gray_texture,
            .ambient = {0.05f, 0.05f, 0.05f},
            .shininess = 32.0f * 0.078125f
        };
    }
    for (auto i = 0; i < 5; ++i) {
        auto cube = arch_entity_manager->createEntity();
        arch::world.addEntityToArchetype(cube, arch::world.archetypes[4]);
        auto cube_location = arch::world.entityLocations[arch::getId(cube)];
        auto* cube_arch =
            dynamic_cast<arch::StaticMeshArchetype*>(cube_location.arch);
        const auto cube_index = cube_location.index;
        cube_arch->transforms[cube_index] = {
            .position = {7.0f, 2.0f, 10.0f + (static_cast<float>(i) * 2.0f)},
            .scale = 1.0f
        };
        cube_arch->meshes[cube_index] = {.handle = hyper_cube2, .gltf = true};
        cube_arch->materials[cube_index] = {
            .diffuseTextureID_ = tileset_texture,
            .specularTextureID_ = container2_specular_texture,
            .ambient = {0.05f, 0.05f, 0.05f},
            .shininess = 128.0f * 0.078125f
        };
        cube_arch->fonts[cube_index].font_string.Push('R');
    }
    auto crosshair = arch_entity_manager->createEntity();
    arch::world.addEntityToArchetype(crosshair, arch::world.archetypes[5]);
    auto crosshair_location =
        arch::world.entityLocations[arch::getId(crosshair)];
    auto* crosshair_arch =
        dynamic_cast<arch::CrosshairArchetype*>(crosshair_location.arch);
    const auto crosshair_index = crosshair_location.index;
    crosshair_arch->transforms[crosshair_index] = {.scale = 0.01f};
    crosshair_arch->meshes[crosshair_index].handle = crosshair_001_handle;
    crosshair_arch->materials[crosshair_index] = {
        .diffuseTextureID_ = container2,
        .specularTextureID_ = container2_specular_texture,
        .ambient = {0.05f, 0.05f, 0.05f},
        .shininess = 128.0f * 0.078125f
    };

    auto inventory = arch_entity_manager->createEntity();
    arch::world.addEntityToArchetype(inventory, arch::world.archetypes[6]);
    auto inventory_location =
        arch::world.entityLocations[arch::getId(inventory)];
    auto* inventory_arch =
        dynamic_cast<arch::InventoryArchetype*>(inventory_location.arch);
    const auto inventory_index = inventory_location.index;
    auto* inventory_component = &inventory_arch->invetories[inventory_index];
    inventory_component->entityOwner = player;
    inventory_component->slotMeshID = inventory_handle;
    inventory_component->slotScale = 0.05;
    auto* inventory_mesh = &inventory_arch->meshes[inventory_index];
    inventory_mesh->gltf = true;
    inventory_arch->transforms[inventory_index] = {
        .position = {0.0f, -0.5f, 0.0f},
        .scale = 1.0f
    };
    inventory_arch->materials[inventory_index] = {
        .diffuseTextureID_ = inventory_texture,
        .specularTextureID_ = inventory_texture,
        .ambient = {0.05f, 0.05f, 0.05f},
        .shininess = 128.0f * 0.078125f
    };
    for (auto i = 0; i < 5; ++i) {
        arch::entity item = arch_entity_manager->createEntity();
        arch::world.addEntityToArchetype(item, arch::world.archetypes[7]);
        arch::EntityLocation item_location =
            arch::world.entityLocations[arch::getId(item)];
        auto* item_arch =
            dynamic_cast<arch::ItemArchetype*>(item_location.arch);
        const auto item_index = item_location.index;
        auto row = i + 1;
        item_arch->items[item_index].itemSlotType = {
            .height = 2,
            .width = static_cast<unsigned int>(row)
        };
        item_arch->items[item_index].isActor = true;
        item_arch->transforms[item_index] = {
            .position = {3.0f, 5.0f, 10.0f + (static_cast<float>(i) * 2.0f)},
            .scale = 0.05f
        };
        item_arch->rigidBodies[item_index] = {.fMass_ = 0.0f};
        item_arch->meshes[item_index].handle = hyper_cube;
        item_arch->materials[item_index] = {
            .diffuseTextureID_ = container2,
            .specularTextureID_ = container2_specular_texture,
            .ambient = {0.05f, 0.05f, 0.05f},
            .shininess = 128.0f * 0.078125f
        };
    }
    auto directional_light = arch_entity_manager->createEntity();
    arch::world.addEntityToArchetype(
        directional_light,
        arch::world.archetypes[8]
    );
    auto directional_light_location =
        arch::world.entityLocations[arch::getId(directional_light)];
    auto* directional_light_arch =
        dynamic_cast<arch::DirectionalLightArchetype*>(
            directional_light_location.arch
        );
    const auto directional_light_index = directional_light_location.index;
    directional_light_arch->directionalLights[directional_light_index] = {
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
        .diffuseTextureID_ = container2,
        .specularTextureID_ = container2,
        .ambient = {0.05f, 0.05f, 0.0f},
        .shininess = 128.0f * 0.078125f
    };
    auto point_light = arch_entity_manager->createEntity();
    arch::world.addEntityToArchetype(point_light, arch::world.archetypes[9]);
    auto point_light_location =
        arch::world.entityLocations[arch::getId(point_light)];
    auto* point_light_arch =
        dynamic_cast<arch::PointLightArchetype*>(point_light_location.arch);
    const auto point_light_index = point_light_location.index;
    point_light_arch->pointLights[point_light_index] = {
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
        .diffuseTextureID_ = container2,
        .specularTextureID_ = container2,
        .ambient = {0.05f, 0.05f, 0.0f},
        .shininess = 128.0f * 0.078125f
    };
    auto spot_light = arch_entity_manager->createEntity();
    arch::world.addEntityToArchetype(spot_light, arch::world.archetypes[10]);
    auto spot_light_location =
        arch::world.entityLocations[arch::getId(spot_light)];
    auto* spot_light_arch =
        dynamic_cast<arch::SpotLightArchetype*>(spot_light_location.arch);
    const auto spot_light_index = spot_light_location.index;
    spot_light_arch->spotLights[spot_light_index] = {
        .position = {1.0f, 12.0f, 5.0f},
        .direction = {0.0f, -1.0f, 2.0f},
        .cutOff = 32.5f,
        .outerCutOff = 37.5f,
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
        .diffuseTextureID_ = gray_texture,
        .specularTextureID_ = gray_texture
    };
    engine->GameLoop();
    engine->GameKill();
    delete entity_manager;
    delete component_manager;
    delete engine;
}
