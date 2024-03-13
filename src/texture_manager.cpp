// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "texture_manager.hpp"
#include "components/material_component.hpp"
#include "texture.hpp"

#include <iostream>

namespace GLVM::ecs {
TextureManager *TextureManager::pInstance_ = nullptr;
std::mutex TextureManager::Mutex_;

TextureManager::TextureManager() = default;

void TextureManager::BindTexture(Entity_ID _entityID, Texture_ID _textureID) {
    textureVector_[_textureID].entitiesOwnsThisTypeOfTexture_.push_back(
            _entityID);
}

TextureManager *TextureManager::GetInstance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new TextureManager();
    }
    return pInstance_;
}

void TextureManager::SetTextureVector(std::vector<Texture> _textureVector) {
    textureVector_ = _textureVector;
}

std::vector<Texture> &TextureManager::GetTextureVector() {
    return textureVector_;
}
} // namespace GLVM::ecs
