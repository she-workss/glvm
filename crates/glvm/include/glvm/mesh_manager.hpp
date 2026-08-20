#pragma once

#include "glvm/components/vertex_component.hpp"

#include <mutex>
#include <vector>

namespace glvm::core {
class MeshManager {
    static MeshManager* pInstance_;
    static std::mutex Mutex_;

    MeshManager();
    ~MeshManager();

public:
    std::vector<const char*> pathsArray_;
    std::vector<const char*> pathsGLTF_;

    // It possibly to get only one instance of this class with this method.
    static MeshManager* GetInstance();
    void SetMesh(const char* _pathToMesh);
    void SetMeshGLTF(const char* pathToMesh);
};
} // namespace glvm::core
