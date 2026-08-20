#pragma once

#include "glvm/Components/VertexComponent.hpp"
#include "glvm/Vector.hpp"

#include <mutex>
#include <vector>

typedef unsigned int Mesh_ID;

namespace glvm::core {
class MeshManager {
    static MeshManager* pInstance_;
    static std::mutex Mutex_;

    MeshManager();
    ~MeshManager();

public:
    std::vector<const char*> pathsArray_;
    core::vector<const char*> pathsGLTF_;

    // It possibly to get only one instance of this class with this method.
    static MeshManager* GetInstance();
    void SetMesh(const char* _pathToMesh);
    void SetMeshGLTF(const char* pathToMesh);
};
} // namespace glvm::core
