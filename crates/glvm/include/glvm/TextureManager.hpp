#pragma once

#include "glvm/Components/MaterialComponent.hpp"
#include "glvm/Constants.hpp"
#include "glvm/Texture.hpp"

#include <mutex>
#include <vector>

typedef unsigned int Entity_ID;
typedef unsigned int Texture_ID;

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
    void BindTexture(Entity_ID _entityID, Texture_ID _textureID);
    void LoadTextureData(glvm::ecs::Texture& _Texture);
    std::vector<Texture>& GetTextureVector();
    void UnbindTexture(components::material _textureComponent, Entity _entity);
};
} // namespace glvm::ecs
