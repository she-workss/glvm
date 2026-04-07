// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "glvm/components/rigid_body_component.hpp"
#include "glvm/engine.hpp"
#include "glvm/sprites_data.hpp"
#include "glvm/texture.hpp"

using namespace GLVM;
namespace cm = ecs::components;

auto main() -> int {
    auto* em = ecs::EntityManager::GetInstance();
    auto* cm_manager = ecs::ComponentManager::GetInstance();
    auto* instance = core::Engine::GetInstance();

    auto cube = instance->load_obj("../../../assets/obj/cube.obj");
    auto cone = instance->load_obj("../../../assets/obj/cone.obj");
    auto ico_sphere = instance->load_obj("../../../assets/obj/ico_sphere.obj");
    auto monkey = instance->load_obj("../../../assets/obj/monkey.obj");
    auto uv_sphere = instance->load_obj("../../../assets/obj/uv_sphere.obj");
    auto torus = instance->load_obj("../../../assets/obj/torus.obj");
    auto pipe = instance->load_obj("../../../assets/obj/pipe.obj");
    auto hyper_cube =
        instance->load_gltf("../../../assets/gltf/hyper_cube.gltf");
    auto mega_chel = instance->load_gltf("../../../assets/gltf/mega_chel.gltf");
    auto simple_cube =
        instance->load_gltf("../../../assets/gltf/simpleCube2.gltf");

    auto glvm_texture =
        instance->load_texture_from_address(128, 128, glvm_dat_len, glvm_dat);
    auto texture1 =
        instance
            ->load_texture_from_address(128, 128, sample1_dat_len, sample1_dat);
    auto texture2 =
        instance
            ->load_texture_from_address(128, 128, sample2_dat_len, sample2_dat);

    auto ui_player = em->CreateEntity();
    cm_manager->CreateComponent<
        cm::mesh,
        cm::controller,
        cm::collider,
        cm::animation,
        cm::beholder,
        cm::transform,
        cm::rigidBody,
        cm::event>(ui_player);
    *cm_manager->GetComponent<cm::transform>(
        ui_player
    ) = {.tPosition = {2.7f, 10.0f, 3.0f}, .fScale = 1.0f};
    *cm_manager->GetComponent<cm::rigidBody>(ui_player) = {.fMass_ = 6.0f};
    *cm_manager->GetComponent<cm::beholder>(
        ui_player
    ) = {.forward = {0.0f, 0.0f, -1.0f}, .up = {0.0f, 1.0f, 0.0f}};
    cm_manager->GetComponent<cm::mesh>(ui_player)->handle = simple_cube;

    auto plain = em->CreateEntity();
    cm_manager
        ->CreateComponent<cm::material, cm::mesh, cm::transform, cm::collider>(
            plain
        );
    *cm_manager->GetComponent<cm::transform>(plain) = {
        .tPosition = {0.0f, -20.5f, 0.0f},
        .yaw = 10.0f,
        .pitch = 0.0f,
        .fScale = 20.2f,
        .gltf = true
    };
    cm_manager->GetComponent<cm::mesh>(plain)->handle = hyper_cube;
    auto* material_plain0 = cm_manager->GetComponent<cm::material>(plain);
    *material_plain0 = {
        .diffuseTextureID_ = glvm_texture,
        .specularTextureID_ = glvm_texture,
        .ambient = {0.05f, 0.05f, 0.0f},
        .shininess = 128.0f * 0.078125f
    };

    for (u32 i = 0; i < 40; ++i) {
        auto ui_witch = em->CreateEntity();
        cm_manager
            ->CreateComponent<cm::material, cm::mesh, cm::collider, cm::transform>(
                ui_witch
            );
        *cm_manager->GetComponent<cm::transform>(ui_witch) = {
            .tPosition = {static_cast<float>(i), 10.0f, 0.0f},
            .yaw = 0.0f,
            .pitch = 0.0f,
            .fScale = 1.2f
        };
        cm_manager->GetComponent<cm::mesh>(ui_witch)->handle = mega_chel;
        auto* material_witch = cm_manager->GetComponent<cm::material>(ui_witch);
        *material_witch = {
            .diffuseTextureID_ = texture1,
            .specularTextureID_ = texture1,
            .ambient = {0.05f, 0.05f, 0.05f},
            .shininess = 128.0f * 0.078125f
        };
    }

    auto cube0 = em->CreateEntity();
    cm_manager
        ->CreateComponent<cm::material, cm::mesh, cm::collider, cm::transform>(
            cube0
        );
    *cm_manager->GetComponent<cm::transform>(cube0) = {
        .tPosition = {7.0f, 3.0f, 0.0f},
        .yaw = 0.0f,
        .pitch = 0.0f,
        .fScale = 1.0f
    };
    cm_manager->GetComponent<cm::mesh>(cube0)->handle = monkey;
    auto* material_cube0 = cm_manager->GetComponent<cm::material>(cube0);
    *material_cube0 = {
        .diffuseTextureID_ = texture2,
        .specularTextureID_ = texture2,
        .ambient = {0.05f, 0.05f, 0.05f},
        .shininess = 128.0f * 0.078125f
    };

    Entity point_light0 = em->CreateEntity();
    cm_manager
        ->CreateComponent<cm::mesh, cm::material, cm::pointLight, cm::transform>(
            point_light0
        );
    *cm_manager->GetComponent<cm::pointLight>(point_light0) = {
        .position = {0.0f, 15.0f, 2.0f},
        .ambient = {0.1f, 0.1f, 0.1f},
        .diffuse = {0.8f, 0.8f, 0.8f},
        .specular = {2.0f, 2.0f, 2.0f},
        .constant = 1.0f,
        .linear = 0.09f,
        .quadratic = 0.032f
    };
    *cm_manager->GetComponent<cm::transform>(
        point_light0
    ) = {.tPosition = {0.0f, 15.0f, 2.0f}, .fScale = 0.2f};
    cm_manager->GetComponent<cm::mesh>(point_light0)->handle = hyper_cube;
    auto* material_point_light0 =
        cm_manager->GetComponent<cm::material>(point_light0);
    *material_point_light0 = {
        .diffuseTextureID_ = glvm_texture,
        .specularTextureID_ = glvm_texture
    };
    instance->GameLoop(core::VULKAN_RENDERER);
    instance->GameKill();
}
