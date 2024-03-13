// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "components/vertex_component.hpp"
#include "vector.hpp"
#include <mutex>
#include <vector>

typedef unsigned int Mesh_ID;

namespace GLVM::core {
class MeshManager {
    static MeshManager *pInstance_;
    static std::mutex Mutex_;

    MeshManager();
    ~MeshManager();

public:
    std::vector<const char *> pathsArray_;
    core::vector<const char *> pathsGLTF_;

    // It possibly to get only one instance of this class with this method.
    static MeshManager *GetInstance();
    void SetMesh(const char *_pathToMesh);
    void SetMeshGLTF(const char *pathToMesh);
};
} // namespace GLVM::core
