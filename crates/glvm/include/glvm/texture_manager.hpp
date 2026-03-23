// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "glvm/components/material_component.hpp"
#include "glvm/constants.hpp"
#include "glvm/texture.hpp"

#include <mutex>
#include <vector>

using EntityId = unsigned int;
using TextureId = unsigned int;

namespace GLVM::ecs {
class TextureManager {
    static TextureManager* p_instance;
    static std::mutex mutex;

    std::vector<Texture> texture_vector;

public:
    TextureManager();

    void SetTextureVector(std::vector<Texture> texture_vector);
    // It possibly to get only one instance of this class with this method.
    static auto GetInstance() -> TextureManager*;
    static auto GetHUDInstance() -> TextureManager*;
    void BindTexture(EntityId entity_id, TextureId texture_id);
    void LoadTextureData(GLVM::ecs::Texture& texture);
    auto GetTextureVector() -> std::vector<Texture>&;
    void unbind_texture(components::material texture_component, Entity entity);
};
} // namespace GLVM::ecs
