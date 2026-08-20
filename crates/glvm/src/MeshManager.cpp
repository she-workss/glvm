#include "glvm/MeshManager.hpp"

#include "glvm/Components/VertexComponent.hpp"

namespace glvm::core {
MeshManager* MeshManager::pInstance_ = nullptr;
std::mutex MeshManager::Mutex_;

MeshManager::MeshManager() {}

MeshManager::~MeshManager() {}

void MeshManager::SetMesh(const char* _pathToMesh) {
    pathsArray_.push_back(_pathToMesh);
}

void MeshManager::SetMeshGLTF(const char* pathToMesh) {
    pathsGLTF_.Push(pathToMesh);
}

MeshManager* MeshManager::GetInstance() {
    std::lock_guard<std::mutex> lock(Mutex_);
    if (pInstance_ == nullptr) {
        pInstance_ = new MeshManager();
    }
    return pInstance_;
}
} // namespace glvm::core
