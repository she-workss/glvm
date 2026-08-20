#include "glvm/mesh_manager.hpp"

#include "glvm/components/vertex_component.hpp"

namespace glvm::core {
MeshManager* MeshManager::pInstance_ = nullptr;
std::mutex MeshManager::Mutex_;

MeshManager::MeshManager() {}

MeshManager::~MeshManager() {}

void MeshManager::SetMesh(const char* _pathToMesh) {
    pathsArray_.push_back(_pathToMesh);
}

void MeshManager::SetMeshGLTF(const char* pathToMesh) {
    pathsGLTF_.push_back(pathToMesh);
}

MeshManager* MeshManager::GetInstance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new MeshManager();
    }
    return pInstance_;
}
} // namespace glvm::core
