#include "glvm/texture_manager.hpp"

#include "glvm/components/material_component.hpp"
#include "glvm/texture.hpp"

#include <iostream>

namespace glvm::ecs {
TextureManager* TextureManager::pInstance_ = nullptr;
std::mutex TextureManager::Mutex_;

TextureManager::TextureManager() = default;

void TextureManager::BindTexture(unsigned int _entityID, unsigned int _textureID) {
    textureVector_[_textureID].entitiesOwnsThisTypeOfTexture_.push_back(
        _entityID
    );
}

TextureManager* TextureManager::GetInstance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new TextureManager();
    }
    return pInstance_;
}

void TextureManager::SetTextureVector(std::vector<Texture> _textureVector) {
    textureVector_ = _textureVector;
}

std::vector<Texture>& TextureManager::GetTextureVector() {
    return textureVector_;
}
} // namespace glvm::ecs
