// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "glvm/components/transform_component.hpp"
#include "glvm/components/view_component.hpp"
#include "glvm/globals.hpp"
#include "glvm/i_system.hpp"
#include "glvm/vertex_math.hpp"

namespace GLVM::ecs {
class CCameraSystem: public ISystem {
public:
    Matrix<float, 4> tProjection_Matrix {1.0f};

    // Mouse parameters.
    float fYaw = -90.0f;
    float fPitch = 0.0f;
    float fLast_X = 1920.0f / 2.0f;
    float fLast_Y = 1080.0f / 2.0f;
    bool bFirst_Mouse = true;

    void Update() override;
    void SetViewMatrix(
        components::transform& _Player,
        components::beholder& _view_Component
    );
    void SetProjectionMatrix();
};
} // namespace GLVM::ecs
