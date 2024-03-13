// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "systems/gui_system.hpp"

#include <GL/gl.h>

namespace GLVM::ecs {
CGUISystem::CGUISystem() {
    _Shader_Program = new Shader("assets/shaders/gl_shaders/GUI.vert",
                                 "assets/shaders/gl_shaders/GUI.frag");
    debugLines = new Shader("assets/shaders/gl_shaders/debugLines.vert",
                            "assets/shaders/gl_shaders/debugLines.frag");
}

void CGUISystem::Update() {
    Matrix<float, 4> tModel_Matrix(1.0);
    tModel_Matrix[0][0] = 0.05;
    tModel_Matrix[1][1] = 0.05;
    tModel_Matrix[2][2] = 0.05;
    Matrix<float, 4> tProjection_Matrix(1.0f);
    Matrix<float, 4> tView_Matrix(1.0f);

    _Shader_Program->Use();

    unsigned int uiTransformt_Loc =
            pGLGet_Uniform_Location(_Shader_Program->iID, "modelMatrix");
    pGLUniform_Matrix4fv(uiTransformt_Loc, NUMBER_OF_MATRICES, GL_FALSE,
                         &tModel_Matrix[0][0]);
    unsigned int transformLocationProjection =
            pGLGet_Uniform_Location(_Shader_Program->iID, "projectionMatrix");
    pGLUniform_Matrix4fv(transformLocationProjection, NUMBER_OF_MATRICES,
                         GL_FALSE, &tProjection_Matrix[0][0]);

    float aCrosshair_Vertices[] = {
            -0.1, 0.5, -0.3, 0.1,  -0.5, -0.3, 0.1, 0.5,  -0.3,
            -0.1, 0.5, -0.3, -0.1, -0.5, -0.3, 0.1, -0.5, -0.3,
            -0.5, 0.1, -0.3, 0.5,  -0.1, -0.3, 0.5, 0.1,  -0.3,
            -0.5, 0.1, -0.3, -0.5, -0.1, -0.3, 0.5, -0.1, -0.3};

    pGLGen_Vertex_Arrays(1, &iVao_Crosshair_);
    pGLGen_Buffers(1, &iVbo_Crosshair_);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    pGLBind_Vertex_Array(iVao_Crosshair_);

    pGLBind_Buffer(GL_ARRAY_BUFFER, iVbo_Crosshair_);
    pGLBuffer_Data(GL_ARRAY_BUFFER, sizeof(aCrosshair_Vertices),
                   aCrosshair_Vertices, GL_DYNAMIC_DRAW);

    pGLVertex_Attrib_Pointer(0, VERTEX_SIZE, GL_FLOAT, GL_FALSE,
                             3 * sizeof(float), (void *)VERTEX_OFFSET);
    pGLEnable_Vertex_Attrib_Array(LAYOUT_0);

    glClear(GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 12);

    // Note that this is allowed, the call to glVertexAttribPointer registered
    // VBO as the vertex attribute's bound vertex buffer object so afterwards we
    // can safely unbind
    pGLBind_Buffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally
    // modify this VAO, but this rarely happens. Modifying other VAOs requires a
    // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
    // VBOs) when it's not directly necessary.
    pGLBind_Vertex_Array(0);
}

void CGUISystem::RaycastringDebug() {
}

} // namespace GLVM::ecs
