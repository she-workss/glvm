// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef IRENDERER
#define IRENDERER

#include "Texture.hpp"
#include "Vector.hpp"
#include "components/MaterialComponent.hpp"
#include "components/TransformComponent.hpp"
#include <vector>

namespace GLVM::core {
class IRenderer {
public:
    virtual ~IRenderer() {
    }

    virtual void draw() = 0;
    virtual void loadWavefrontObj() = 0;
    virtual void EnlargeFrameAccumulator(float value) = 0;
    virtual void SetTextureData(std::vector<ecs::Texture> &_texture_data) = 0;
    virtual void SetMeshData(std::vector<const char *> _pathsArray,
                             core::vector<const char *> pathsGLTF_) = 0;
    virtual void SetViewMatrix(mat4 _viewMatrix) = 0;
    virtual void SetProjectionMatrix(mat4 _projectionMatrix) = 0;
    virtual void run() = 0;
};
} // namespace GLVM::core

#endif
