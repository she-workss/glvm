#pragma once

#include "glvm/components/material_component.hpp"
#include "glvm/constants.hpp"
#include "glvm/texture.hpp"

#include <mutex>
#include <vector>

namespace glvm::ecs {
class TextureManager {
    static TextureManager* pInstance_;
    static std::mutex Mutex_;

    std::vector<Texture> textureVector_;

public:
    TextureManager();

    void SetTextureVector(std::vector<Texture> _textureVector);
    // It possibly to get only one instance of this class with this method.
    static TextureManager* GetInstance();
    static TextureManager* GetHUDInstance();
    void BindTexture(unsigned int _entityID, unsigned int _textureID);
    void LoadTextureData(glvm::ecs::Texture& _Texture);
    std::vector<Texture>& GetTextureVector();
    void UnbindTexture(components::material _textureComponent, unsigned int _entity);
};
} // namespace glvm::ecs
