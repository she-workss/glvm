// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "texture_manager.hpp"

#include "components/material_component.hpp"
#include "texture.hpp"

#include <iostream>

namespace GLVM::ecs {
TextureManager* TextureManager::p_instance = nullptr;
std::mutex TextureManager::mutex;

TextureManager::TextureManager() = default;

void TextureManager::BindTexture(EntityId entity_id, TextureId texture_id) {
    texture_vector[texture_id].entitiesOwnsThisTypeOfTexture_.push_back(
        entity_id
    );
}

auto TextureManager::GetInstance() -> TextureManager* {
    std::lock_guard<std::mutex> lock(mutex);
    if (p_instance == nullptr) {
        p_instance = new TextureManager();
    }
    return p_instance;
}

void TextureManager::SetTextureVector(std::vector<Texture> _textureVector) {
    texture_vector = _textureVector;
}

std::vector<Texture>& TextureManager::GetTextureVector() {
    return texture_vector;
}
} // namespace GLVM::ecs
