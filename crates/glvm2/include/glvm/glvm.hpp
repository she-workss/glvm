#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cfloat>
#include <chrono>
#include <climits>
#include <cmath>
#include <compare>
#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <numbers>
#include <optional>
#include <ostream>
#include <print>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include <vulkan/vulkan_core.h>

#ifdef _WIN32
#include <cwchar>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif
// clang-format off
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
// clang-format on
#endif // _WIN32
#ifdef __linux__
#include "wayland-client.h"

#include <X11/Xlib.h>
#include <algorithm>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <bits/types/FILE.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>
#endif // __linux__

constexpr auto ARCHETYPE_CHUNK_SIZE = 16384;
constexpr auto BASE_ARRAY_COUNTER_VALUE = 0;
constexpr auto BASE_INDEX_VERTEX_ARRAY = 0;
constexpr auto CUBE_MAP_LAYER_NUMBER = 6;
constexpr auto DIRECTIONAL_LIGHTS_NUMBER = 4;
constexpr auto ENTITY_ID_BITS = 32;
constexpr auto FLAT_SHADOW_MAP_SIZE = 2048;
constexpr auto HOMOGENEOUS_COORDINATE = 1;
constexpr auto INDIRECT_TEXTURE_HEIGHT = 5;
constexpr auto INDIRECT_TEXTURE_WIDTH = 7;
constexpr auto LAYOUT_0 = 0;
constexpr auto LAYOUT_1 = 1;
constexpr auto LIMITER = 1;
constexpr auto MATRIX_RANGE = 16;
constexpr auto MAX_JOINTS_NUMBER = 128;
constexpr auto MIPMAP_LEVEL = 0;
constexpr auto NUMBER_OF_CREATING_TEXTURE_OBJECT_1 = 1;
constexpr auto NUMBER_OF_CREATING_VAO_OBJECT_1 = 1;
constexpr auto NUMBER_OF_CREATING_VBO_OBJECT_1 = 1;
constexpr auto NUMBER_OF_DROWING_VERTEXES = 36;
constexpr auto NUMBER_OF_MATRICES = 1;
constexpr auto PI = std::numbers::pi;
constexpr auto POINT_LIGHTS_NUMBER = 32;
constexpr auto SHADOW_MAP_SIZE = 1024;
constexpr auto SIZE_OF_VERTEX_DATA = 5;
constexpr auto SOME_OLD_STUFF = 0;
constexpr auto SOME_STRANGE_STUFF = 0;
constexpr auto SPOT_LIGHTS_NUMBER = 8;
constexpr auto TEXTURE_OFFSET = 3;
constexpr auto TEXTURE_SIZE = 2;
constexpr auto TILESET_COLUMN = 8;
constexpr auto TILESET_ROW = 8;
constexpr auto VERTEX_ARRAY_RANGE = 180;
constexpr auto VERTEX_OFFSET = 0;
constexpr auto VERTEX_SIZE = 3;
constexpr auto VK_DEBUG_DESCRIPTOR_SET_LAYOUT_RED =
    "\x1b[31mDEBUG DESCRIPTOR SET LAYOUT\x1b[0m";
constexpr auto VK_DEBUG_DESCRIPTOR_SET_RED =
    "\x1b[31mDEBUG DESCRIPTOR SET\x1b[0m";
constexpr auto VK_DEBUG_IMAGE_SET_RED = "\x1b[31mDEBUG IMAGE\x1b[0m";
constexpr auto VK_DEBUG_PIPELINE_LAYOUT_RED =
    "\x1b[31mDEBUG PIPELINE LAYOUT:\x1b[0m";
constexpr auto VK_DEBUG_PIPELINE_RED = "\x1b[31mDEBUG PIPELINE:\x1b[0m";

#ifdef __linux__
constexpr auto XKEY_A = 0x61;
constexpr auto XKEY_D = 0x64;
constexpr auto XKEY_ESCAPE = 0xff1b;
constexpr auto XKEY_I = 0x69;
constexpr auto XKEY_S = 0x73;
constexpr auto XKEY_SPACE = 0x20;
constexpr auto XKEY_W = 0x77;
#endif

#ifdef _WIN32
constexpr auto WGL_COLOR_BITS_ARB = 0x2014;
constexpr auto WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x00000001;
constexpr auto WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
constexpr auto WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
constexpr auto WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
constexpr auto WGL_DEPTH_BITS_ARB = 0x2022;
constexpr auto WGL_DOUBLE_BUFFER_ARB = 0x2011;
constexpr auto WGL_DRAW_TO_WINDOW_ARB = 0x2001;
constexpr auto WGL_PIXEL_TYPE_ARB = 0x2013;
constexpr auto WGL_SAMPLE_BUFFERS_ARB = 0x2041;
constexpr auto WGL_SAMPLES_ARB = 0x2042;
constexpr auto WGL_STENCIL_BITS_ARB = 0x2023;
constexpr auto WGL_TYPE_RGBA_ARB = 0x202B;
#endif

namespace glvm {
constexpr uint64_t entityBitsMask = (1ull << ENTITY_ID_BITS) - 1;

struct ComponentsIndices {
    enum Types : uint32_t {
        TransformComponent,
        RigidBodyComponent,
        MeshComponent,
        FontComponent,
        ColliderComponent,
        ColliderFlagsComponent,
        MaterialComponent,
        ViewComponent,
        HealthComponent,
        AnimationComponent,
        StateComponent,
        EnemyComponent,
        DamageComponent,
        AttackComponent,
        InventoryComponent,
        DirectionalLightComponent,
        SpotLightComponent,
        PointLightComponent,
        ItemComponent,
        MoveComponent,
        ProjectileBundleComponent,
        RotationComponent,

        LevelChunkTagComponent,
        PlayerTagComponent,
        CrosshairTagComponent,
        StaticMeshTagComponent,
        ProjectileTagComponent,

        ComponentsCount,
    };
};

constexpr uint64_t playerComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::ViewComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::RigidBodyComponent)
    | (1ull << ComponentsIndices::HealthComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::MoveComponent)
    | (1ull << ComponentsIndices::AttackComponent)
    | (1ull << ComponentsIndices::AnimationComponent)
    | (1ull << ComponentsIndices::FontComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::PlayerTagComponent);

constexpr uint64_t enemyComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::EnemyComponent)
    | (1ull << ComponentsIndices::StateComponent)
    | (1ull << ComponentsIndices::FontComponent)
    | (1ull << ComponentsIndices::AnimationComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::HealthComponent)
    | (1ull << ComponentsIndices::RigidBodyComponent)
    | (1ull << ComponentsIndices::AttackComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::MoveComponent);

constexpr uint64_t staticMeshComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::FontComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::StaticMeshTagComponent);

constexpr uint64_t crosshairComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::CrosshairTagComponent);

constexpr uint64_t itemComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::RigidBodyComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::MoveComponent)
    | (1ull << ComponentsIndices::ItemComponent);

constexpr uint64_t inventoryComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::InventoryComponent)
    | (1ull << ComponentsIndices::MaterialComponent);

constexpr uint64_t directionalLightComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::DirectionalLightComponent);

constexpr uint64_t spotLightComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::SpotLightComponent);

constexpr uint64_t pointLightComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::PointLightComponent);

constexpr uint64_t levelChunkComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::LevelChunkTagComponent);

constexpr uint64_t projectileComponentMask =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::ProjectileBundleComponent)
    | (1ull << ComponentsIndices::ProjectileTagComponent);
}; // namespace glvm

namespace glvm {
struct actor {};
} // namespace glvm

namespace glvm {
struct animation {
    uint32_t currentAnimationFrame = 0;
    float frameAccumulator = 0.0f;
};
} // namespace glvm

namespace glvm {}

namespace glvm {
struct attack {
    float damage;
};
} // namespace glvm

namespace glvm {
class collider {
public:
    std::vector<unsigned int> colliders;
};
} // namespace glvm

namespace glvm {
struct colliderFlags {
    // 0001 = wallCollision; 0010 = groundCollision; 0100 = roofCollision; 1000
    // = itemDrag.
    int flags : 4;
};
}; // namespace glvm

namespace glvm {
struct controller {};
} // namespace glvm

namespace glvm {
struct crosshair {};
}; // namespace glvm

namespace glvm {
struct damage {
    float maximumDamage;
    float minimumDamage;
    float criticalHitRate;
    float criticalModifier;
};
} // namespace glvm

namespace glvm {
struct enemy {
    float detectRadius;
};
} // namespace glvm

namespace glvm {
struct font {
    std::vector<char> font_string;
    float lifeTime;
    bool removeble;
};
} // namespace glvm

namespace glvm {
struct health {
    float maxHealth;
    float currentHealth;
};
} // namespace glvm

namespace glvm {
struct hud {
    bool hud = false;
    bool gltf = true;
};
}; // namespace glvm

namespace glvm {
struct interface_ui {};
}; // namespace glvm

namespace glvm {
struct inventorySlot {
    unsigned int itemEntity = UINT_MAX;
};
} // namespace glvm

namespace glvm {
struct ItemSlotType {
    unsigned int height;
    unsigned int width;
};

struct item {
    // Array that contain entities with inventorySlotComponent.
    std::vector<unsigned int> occupiedSlots;
    ItemSlotType itemSlotType;
    bool isActor;
};
} // namespace glvm

namespace glvm {
struct physics {
    float gravityAccumulator = 0.0f;
};
}; // namespace glvm

namespace glvm {
class projectile {
public:
    unsigned int owner;
    bool bCollision_Status_ = false;
    float fDamage_;
    float fSpeed_;
    float fFlying_Range_;
    float damage;
};
} // namespace glvm

namespace glvm {
struct rotation {
    float yaw = 0.0f;
    float pitch = 0.0f;
};
}; // namespace glvm

namespace glvm {
struct texture {
    unsigned int id;
};
} // namespace glvm

namespace glvm {
struct MeshHandle {
    uint32_t id;
};

struct mesh {
    MeshHandle handle;
    bool gltf = true;
};
} // namespace glvm

constexpr unsigned int k_iUint_Max = 4000000000;
constexpr int k_iNull = 0;
constexpr int boxIndicesForIndexBuffer[36] = {0, 1, 2, 3, 0, 2, 4, 0, 3,
                                              7, 4, 3, 4, 5, 1, 0, 4, 1,
                                              1, 5, 6, 2, 1, 6, 5, 4, 7,
                                              6, 5, 7, 3, 2, 6, 7, 3, 6};

namespace glvm {

class CStack;

enum EEvents {
    eDEFAULT,
    eKEYRELEASE_A,
    eKEYRELEASE_D,
    eKEYRELEASE_S,
    eKEYRELEASE_W,
    eKEYRELEASE_JUMP,
    eGRAVITY_COLLISION_FLAG,
    eRENDER,
    eATACK,
    eSPAWN,
    eJUMP,
    eINVENTORY,
    eINVENTORY_RELEASE,
    eMOVE_FORWARD,
    eMOVE_BACKWARD,
    eMOVE_LEFT,
    eMOVE_RIGHT,
    eMOVE_DIAGONAL_FB,
    eMOVE_DIAGONAL_FL,
    eMOVE_DIAGONAL_LB,
    eMOVE_DIAGONAL_BR,
    eMOUSE_POINTER_POSITION,
    eMOUSE_LEFT_BUTTON_RELEASE,
    eMOUSE_LEFT_BUTTON,
    eMOUSE_RIGHT_BUTTON_RELEASE,
    eMOUSE_RIGHT_BUTTON,
    eCURSOR_RELEASED,
    eGAME_LOOP_KILL,
    eEmpty,
};

struct SMousePointerPosition {
    int position_X;
    int position_Y;
    int offset_X = 0;
    int offset_Y = 0;
    float pitch;
    float yaw;
};

class CEvent {
    EEvents eEvent_;
    EEvents nextEvent;

public:
    SMousePointerPosition mousePointerPosition;
    bool nextEventFlag = false;

    CEvent();
    EEvents& GetEvent();
    void SetEvent(EEvents _eEvent);
    void SetNextEvent(EEvents _eEvent);
    EEvents GetNextEvent();
    void SetLastEvent(CStack _Stack);

    bool isLeftMouseButtonReleased = true;
};

} // namespace glvm

template<typename T>
struct Node {
    std::string key_;
    T value_;
    Node* next_ = nullptr;

    Node(const char* _key) : key_(_key) {}
};

template<typename S>
class HashMap {
    unsigned int capacity_ = 10;

public:
    Node<S>** hashMap_ = nullptr;

    HashMap() {
        hashMap_ = new Node<S>*[capacity_];

        for (unsigned int i = 0; i < capacity_; ++i) {
            hashMap_[i] = nullptr;
        }
    }

    HashMap(const HashMap<S>& _map) {
        capacity_ = _map.capacity_;
        hashMap_ = new Node<S>*[capacity_];

        for (unsigned int i = 0; i < capacity_; ++i) {
            hashMap_[i] = nullptr;
        }

        for (unsigned int i = 0; i < capacity_; ++i) {
            Node<S>* currentNode = _map.hashMap_[i];

            while (currentNode != nullptr) {
                unsigned int hash = HashFunction(currentNode->key_.c_str());
                Link(hashMap_[hash], currentNode->key_.c_str()) =
                    currentNode->value_;

                currentNode = currentNode->next_;
            }
        }
    }

    void operator=(const HashMap<S>& _map) {
        capacity_ = _map.capacity_;
        hashMap_ = new Node<S>*[capacity_];

        for (int i = 0; i < capacity_; ++i) {
            hashMap_[i] = nullptr;
        }

        for (int i = 0; i < capacity_; ++i) {
            Node<S>* currentNode = _map.hashMap_[i];
            while (currentNode != nullptr) {
                unsigned int hash = HashFunction(currentNode->key_.c_str());
                Link(hashMap_[hash], currentNode->key_.c_str()) =
                    currentNode->value_;

                currentNode = currentNode->next_;
            }
        }
    }

    S& operator[](const char* _key) {
        unsigned int hash = HashFunction(_key);

        if (hash >= capacity_) {
            Rehash(hash);
        }

        return Link(hashMap_[hash], _key);
    }

    bool Contain(const char* _key) {
        unsigned int hash = HashFunction(_key);
        Node<S>* node = hashMap_[hash];

        while (node != nullptr) {
            if (node->key_ == _key) {
                return true;
            } else {
                node = node->next_;
            }
        }

        return false;
    }

    ~HashMap() {
        for (unsigned int i = 0; i < capacity_; ++i) {
            Node<S>* node = hashMap_[i];
            while (node != nullptr) {
                Node<S>* nodeTemp = node;
                node = node->next_;
                delete nodeTemp;
            }
        }
        delete[] hashMap_;
        hashMap_ = nullptr;
    }

    bool SearchKey(const char* key_) {
        for (int i = 0; i < capacity_; ++i) {
            if (hashMap_[i] != nullptr && hashMap_[i]->key_ == key_) {
                std::cout << "key: " << key_ << std::endl;
                return true;
            }
        }

        return false;
    }

    unsigned int GetCapacity() {
        return capacity_;
    }

private:
    S& Link(Node<S>*& _node, const char* _key) {
        if (_node == nullptr) {
            _node = new Node<S>(_key);
            return _node->value_;
        } else {
            if (_node->key_ == _key) {
                return _node->value_;
            }

            return Link(_node->next_, _key);
        }
    }

    unsigned int HashFunction(const char* _key) {
        unsigned int sum = 0;
        unsigned int counter = 0;
        while (_key[counter] != '\0') {
            sum += _key[counter];
            ++counter;
        }

        unsigned int reminder = sum % capacity_;
        return reminder;
    }

    void Rehash(unsigned int _hash) {
        unsigned int reminder = _hash % 10;
        capacity_ = _hash + (10 - reminder);

        Node<S>** temp = new Node<S>*[capacity_];

        for (unsigned int i = 0; i < capacity_; ++i) {
            temp[i] = hashMap_[i];
        }

        delete[] hashMap_;
        hashMap_ = nullptr;
        hashMap_ = temp;
    }
};

namespace glvm {
class IChrono {
public:
    virtual ~IChrono() {}

    virtual double InitFrequency() = 0;
    virtual double Reset() = 0;
    virtual double GetElapsed() = 0;
};
} // namespace glvm

namespace glvm {
struct scalar {
    float value;
};

// Vector in 3D PGA.
struct plane {
    // e1 basis vector.
    float x;
    // e2 basis vector.
    float y;
    // e3 basis vector.
    float z;
    // e0 projective plane in infinity.
    float w;
};

// Bivector.
struct line {
    float rx;
    float ry;
    float rz;
    float ix;
    float iy;
    float iz;
};

struct rline {
    float rx;
    float ry;
    float rz;
};

struct iline {
    float ix;
    float iy;
    float iz;
};

// Trivector.
struct point {
    float x;
    float y;
    float z;
    float w;
};

struct pseudoScalar {
    float w;
};

struct motor {
    float rx;
    float ry;
    float rz;
    // Scalar.
    float rw;
    float ix;
    float iy;
    float iz;
    // Pseudoscalar.
    float iw;
};

struct rotor {
    float rx;
    float ry;
    float rz;
    float rw;
};

struct translator {
    float ix;
    float iy;
    float iz;
    float iw;
};

inline std::ostream& operator<<(std::ostream& os, const line& line) {
    os << "rx: " << line.rx << " ry: " << line.ry << " rz: " << line.rz
       << " ix: " << line.ix << " iy: " << line.iy << " iz: " << line.iz;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const point& point) {
    os << "x: " << point.x << " y: " << point.y << " z: " << point.z
       << " w: " << point.w;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const plane& plane) {
    os << "x: " << plane.x << " y: " << plane.y << " z: " << plane.z
       << " w: " << plane.w;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const scalar& scalar) {
    os << "value: " << scalar.value;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const motor& motor) {
    os << "rx: " << motor.rx << " ry: " << motor.ry << " rz: " << motor.rz
       << " rw: " << motor.rw << " ix: " << motor.ix << " iy: " << motor.iy
       << " iz: " << motor.iz << " iw: " << motor.iw;
    return os;
}

inline std::ostream& operator<<(
    std::ostream& os,
    const pseudoScalar& pseudoScalar
) {
    os << "w: " << pseudoScalar.w;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const rline& rline) {
    std::cout << "rx: " << rline.rx << " ry: " << rline.ry
              << " rz: " << rline.rz;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const rotor& rotor) {
    std::cout << "rx: " << rotor.rx << " ry: " << rotor.ry
              << " rz: " << rotor.rz << " rw: " << rotor.rw;
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const translator& translator) {
    std::cout << "ix: " << translator.ix << " iy: " << translator.iy
              << " iz: " << translator.iz << " iw: " << translator.iw;
    return os;
}

inline line operator-(line line) {
    return {
        .rx = -line.rx,
        .ry = -line.ry,
        .rz = -line.rz,
        .ix = -line.ix,
        .iy = -line.iy,
        .iz = -line.iz
    };
}

inline point operator-(point point) {
    return {.x = -point.x, .y = -point.y, .z = -point.z, .w = -point.w};
}

// Dual operator.

inline point operator!(const plane& plane) {
    return point {.x = plane.x, .y = plane.y, .z = plane.z, .w = plane.w};
}

inline plane operator!(const point& point) {
    return plane {.x = point.x, .y = point.y, .z = point.z, .w = point.w};
}

inline line operator!(const line& line) {
    return {
        .rx = line.ix,
        .ry = line.iy,
        .rz = line.iz,
        .ix = line.rx,
        .iy = line.ry,
        .iz = line.rz
    };
}

inline scalar operator!(const pseudoScalar& pseudoScalar) {
    return scalar {.value = pseudoScalar.w};
}

inline pseudoScalar operator!(const scalar& scalar) {
    return pseudoScalar {.w = scalar.value};
}

inline plane normalize(const plane& plane) {
    float length =
        std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
    assert(length != 0);
    return {
        .x = plane.x / length,
        .y = plane.y / length,
        .z = plane.z / length,
        .w = plane.w / length
    };
}

inline line normalize(const line& line) {
    float length =
        std::sqrt(line.ix * line.ix + line.iy * line.iy + line.iz * line.iz);
    assert(length != 0);
    return {
        .rx = line.rx / length,
        .ry = line.ry / length,
        .rz = line.rz / length,
        .ix = line.ix / length,
        .iy = line.iy / length,
        .iz = line.iz / length
    };
}

// Reverse.
inline plane operator~(const plane& plane) {
    return plane;
}

inline line operator~(const line& line) {
    return -line;
}

inline point operator~(const point& point) {
    return -point;
}

inline scalar operator~(const scalar& scalar) {
    return scalar;
}

inline rline operator~(const rline& rline) {
    return rline;
}

// Inner product.

// Scalar product of the plane normals.
inline float operator|(const plane& plane0, const plane& plane1) {
    return plane0.x * plane1.x + plane0.y * plane1.y + plane0.z * plane1.z;
}

// This gives the oriented distance from the point to the plane (if normalized).
inline line operator|(const plane& plane, const point& point) {
    return {
        // e2 ^ e3.
        .rx = plane.x * point.w,
        // e3 ^ e1.
        .ry = plane.y * point.w,
        // e1 ^ e2.
        .rz = plane.z * point.w,
        // e0 ^ e1.
        .ix = plane.z * point.y - plane.y * point.z,
        // e0 ^ e2.
        .iy = plane.x * point.z - plane.z * point.x,
        // e0 ^ e3.
        .iz = plane.y * point.x - plane.x * point.y
    };
}

// If the plane and the line intersect, w ≠ 0, otherwise the result is an
// infinite point.
inline point operator|(const plane& plane, const line& line) {
    return {
        // e1.
        .x = -plane.y * line.rz + plane.z * line.ry,
        // e2.
        .y = plane.x * line.rz - plane.z * line.rx,
        // e3.
        .z = -plane.x * line.ry + plane.y * line.rx,
        // e0.
        .w = -plane.x * line.ix - plane.y * line.iy - plane.z * line.iz
    };
}

// Angular measure between directions. These are dot directions; if the lines
// are normalized, this is the cos(θ) between them.
inline float operator|(const line& line0, const line& line1) {
    return -line0.rx * line1.rx - line0.ry * line1.ry - line0.rz * line1.rz;
}

// A line through a point defines a plane. the form is similar to plane ⋅ line,
// but semantically it is a plane containing l and pt
inline plane operator|(const line& line, const point& point) {
    return {
        .x = -line.rx * point.w,
        .y = -line.ry * point.w,
        .z = -line.rz * point.w,
        .w = line.rx * point.x + line.ry * point.y + line.rz * point.z
    };
}

// Points do not have an inner product: it is always zero (if strictly by
// definition).
inline scalar operator|(
    [[maybe_unused]] const point& point0,
    [[maybe_unused]] const point& point1
) {
    return {.value = -point0.w * point1.w};
}

// Outer product.

// plane ^ plane -> line (those intersection).
inline line operator^(const plane& plane0, const plane& plane1) {
    return {
        // Real part (moment): e23, e31, e12.
        .rx = plane0.y * plane1.z - plane0.z * plane1.y,
        .ry = plane0.z * plane1.x - plane0.x * plane1.z,
        .rz = plane0.x * plane1.y - plane0.y * plane1.x,
        // Ideal part (direction): e01, e02, e03.
        .ix = plane0.w * plane1.x - plane0.x * plane1.w,
        .iy = plane0.w * plane1.y - plane0.y * plane1.w,
        .iz = plane0.w * plane1.z - plane0.z * plane1.w
    };
}

// plane ∧ point -> line passing through a point on a plane.
inline pseudoScalar operator^(const plane& plane, const point& point) {
    return {
        .w = plane.x * point.x + plane.y * point.y + plane.z * point.z
            + plane.w * point.w
    };
}

// line ^ point -> plane.
inline float operator^(
    [[maybe_unused]] const line& line,
    [[maybe_unused]] const point& point
) {
    return 0.0;
}

// point ∧ point → line (through two points).
inline float operator^(
    [[maybe_unused]] const point& point0,
    [[maybe_unused]] const point& point1
) {
    return 0.0;
}

// line ∧ line → point (if intersecting). If w == 0, then the lines do not
// intersect (the result is a point at infinity).
inline pseudoScalar operator^(const line& line0, const line& line1) {
    /// e0 ^ (e1 ^ (e2 ^ e3)).
    return {
        .w = line0.rx * line1.ix + line0.ry * line1.iy + line0.rz * line1.iz
            + line0.ix * line1.rx + line0.iy * line1.ry + line0.iz * line1.rz
    };
}

// This is the wedge product between a plane and a line, the result is the
// intersection point if they are not parallel. If they are parallel, the point
// will be at infinity (w = 0). This is the same formula as inner(plane, line) -
// in PGA 3D the result of plane ∧ line and plane | line have the same component
// form, but semantically they are different operations:
// 1. inner - orthogonal projection.
// 2. outer - geometric "generating" subspace.
inline point operator^(const plane& plane, const line& line) {
    return {
        .x = plane.y * line.iz - plane.z * line.iy - plane.w * line.rx,
        .y = -plane.x * line.iz + plane.z * line.ix - plane.w * line.ry,
        .z = plane.x * line.iy - plane.y * line.ix - plane.w * line.rz,
        .w = plane.x * line.rx + plane.y * line.ry + plane.z * line.rz
    };
}

inline point operator^(const line& line, const plane& plane) {
    return {
        .x = -line.rx * plane.w - line.iy * plane.z + line.iz * plane.y,
        .y = -line.ry * plane.w + line.ix * plane.z - line.iz * plane.x,
        .z = -line.rz * plane.w - line.ix * plane.y + line.iy * plane.x,
        .w = line.rx * plane.x + line.ry * plane.y + line.rz * plane.z
    };
}

// Regressive product.
// Regressive product gives the intersection of objects. In Projective Geometric
// Algebra (PGA), the regressive product, denoted by ∨ (vee), is the dual
// operation to the exterior product (∧). That is: A ∨ B = (⟦A⟧ ∧ ⟦B⟧)*, where
// ⟦A⟧ is the dual of object A, and * is the dual of the result
inline float operator&(
    [[maybe_unused]] plane plane0,
    [[maybe_unused]] plane plane1
) {
    return 0.0f;
}

inline scalar operator&(plane plane, point point) {
    // Plane below link with dual point from outer product and point below link
    // with dual plane from outer product.
    return {
        .value = -plane.x * point.x + -plane.y * point.y + -plane.z * point.z
            + -plane.w * point.w
    };
}

inline scalar operator&(point point, plane plane) {
    // Plane below link with dual point from outer product and point below link
    // with dual plane from outer product.
    return {
        .value = plane.x * point.x + plane.y * point.y + plane.z * point.z
            + plane.w * point.w
    };
}

inline plane operator&([[maybe_unused]] point point, [[maybe_unused]] line line) {
    // Point below link with dual plane from outer product and line below link
    // with dual line from outer product.
    return {
        .x = point.y * line.rz - point.z * line.ry - point.w * line.ix,
        .y = -point.x * line.rz + point.z * line.rx - point.w * line.iy,
        .z = point.x * line.ry - point.y * line.rx - point.w * line.iz,
        .w = point.x * line.ix + point.y * line.iy + point.z * line.iz
    };
}

inline plane operator&([[maybe_unused]] line line, [[maybe_unused]] point point) {
    // Point below link with dual plane from outer product and line below link
    // with dual line from outer product.
    return {
        .x = -line.ix * point.w - line.ry * point.z + line.rz * point.y,
        .y = -line.iy * point.w + line.rx * point.z - line.rz * point.x,
        .z = -line.iz * point.w - line.rx * point.y + line.ry * point.x,
        .w = line.ix * point.x + line.iy * point.y + line.iz * point.z,
    };
}

inline line operator&(
    [[maybe_unused]] point point0,
    [[maybe_unused]] point point1
) {
    // point0 below link with dual plane0 from outer product and point1 below
    // link with dual plane1 from outer product.
    return {
        // Real part (moment): e23, e31, e12.
        .rx = point0.w * point1.x - point0.x * point1.w,
        .ry = point0.w * point1.y - point0.y * point1.w,
        .rz = point0.w * point1.z - point0.z * point1.w,
        // Ideal part (direction): e01, e02, e03.
        .ix = point0.y * point1.z - point0.z * point1.y,
        .iy = point0.z * point1.x - point0.x * point1.z,
        .iz = point0.x * point1.y - point0.y * point1.x
    };
}

inline scalar operator&(line line0, line line1) {
    return {
        .value = line0.rx * line1.ix + line0.ry * line1.iy + line0.rz * line1.iz
            + line0.ix * line1.rx + line0.iy * line1.ry + line0.iz * line1.rz
    };
}

inline float operator&([[maybe_unused]] plane plane, [[maybe_unused]] line line) {
    return 0.0f;
}

inline float operator&([[maybe_unused]] line line, [[maybe_unused]] plane plane) {
    return 0.0f;
}

// Geometric product.

inline motor operator*(plane plane0, plane plane1) {
    return {
        // Real part (moment): e23, e31, e12.
        .rx = plane0.y * plane1.z - plane0.z * plane1.y,
        .ry = plane0.z * plane1.x - plane0.x * plane1.z,
        .rz = plane0.x * plane1.y - plane0.y * plane1.x,
        // Scalar.
        .rw = plane0.x * plane1.x + plane0.y * plane1.y + plane0.z * plane1.z,
        // Ideal part (direction): e01, e02, e03.
        .ix = plane0.w * plane1.x - plane0.x * plane1.w,
        .iy = plane0.w * plane1.y - plane0.y * plane1.w,
        .iz = plane0.w * plane1.z - plane0.z * plane1.w,
        // Pseudoscalar.
        .iw = 0.0
    };
}

inline motor operator*(line line0, line line1) {
    return {
        // Real part (moment): e23, e31, e12.
        .rx = -line0.ry * line1.rz + line0.rz * line1.ry,
        .ry = line0.rx * line1.rz - line0.rz * line1.rx,
        .rz = -line0.rx * line1.ry + line0.ry * line1.rx,
        // Scalar.
        .rw = -line0.rx * line1.rx - line0.ry * line1.ry - line0.rz * line1.rz,
        // Ideal part (direction): e01, e02, e03.
        .ix = -line0.ry * line1.iz + line0.rz * line1.iy - line0.iy * line1.rz
            + line0.iz * line1.ry,
        .iy = line0.rx * line1.iz - line0.rz * line1.ix + line0.ix * line1.rz
            - line0.iz * line1.rx,
        .iz = -line0.rx * line1.iy + line0.ry * line1.ix - line0.ix * line1.ry
            + line0.iy * line1.rx,
        // Pseudoscalar.
        .iw = line0.rx * line1.ix + line0.ry * line1.iy + line0.rz * line1.iz
            + line0.ix * line1.rx + line0.iy * line1.ry + line0.iz * line1.rz
    };
}

inline rotor operator*(rline rline0, rline rline1) {
    return {
        // Real part (moment): e23, e31, e12.
        .rx = -rline0.ry * rline1.rz + rline0.rz * rline1.ry,
        .ry = rline0.rx * rline1.rz - rline0.rz * rline1.rx,
        .rz = -rline0.rx * rline1.ry + rline0.ry * rline1.rx,
        // Scalar.
        .rw = -rline0.rx * rline1.rx - rline0.ry * rline1.ry
            - rline0.rz * rline1.rz,
    };
}

inline translator operator*(point point0, point point1) {
    return {
        .ix = point0.x * point1.w - point0.w * point1.x,
        .iy = point0.y * point1.w - point0.w * point1.y,
        .iz = point0.z * point1.w - point0.w * point1.z,
        .iw = -point0.w * point1.w
    };
}

inline rotor exp(float theta, rline rline) {
    float sin = std::sin(theta / 2.0f);
    return {
        .rx = rline.rx * sin,
        .ry = rline.ry * sin,
        .rz = rline.rz * sin,
        .rw = std::cos(theta / 2.0f)
    };
}

inline translator exp(float distance, iline iline) {
    float half = distance / 2.0f;
    return {
        .ix = iline.ix * half,
        .iy = iline.iy * half,
        .iz = iline.iz * half,
        .iw = 1.0f
    };
}

inline point operator>>(const rotor& rotor, const point& point) {
    const float d0 =
        point.x * rotor.rw + point.y * rotor.rz - point.z * rotor.ry;
    const float d1 =
        point.x * rotor.ry - point.y * rotor.rx + point.z * rotor.rw;
    const float d2 =
        -point.x * rotor.rz + point.y * rotor.rw + point.z * rotor.rx;
    return {
        .x = point.x + 2.0f * (-rotor.ry * d1 + rotor.rz * d2),
        .y = point.y + 2.0f * (-rotor.rz * d0 + rotor.rx * d1),
        .z = point.z + 2.0f * (-rotor.rx * d2 + rotor.ry * d0),
        .w = point.w
    };
}

inline point operator>>(const translator& translator, const point& point) {
    const float pwrw = point.w * translator.iw;
    const float rww = translator.iw * translator.iw;
    return {
        .x = point.x * rww - 2.0f * pwrw * translator.ix,
        .y = point.y * rww - 2.0f * pwrw * translator.iy,
        .z = point.z * rww - 2.0f * pwrw * translator.iz,
        .w = point.w * rww
    };
}
}; // namespace glvm

namespace glvm {
enum States : uint8_t { IDLE, ATTACK, ROAMING };
} // namespace glvm

namespace glvm {
struct crossHairTagComponent {};
}; // namespace glvm

namespace glvm {
struct levelChunkTagComponent {};
}; // namespace glvm

namespace glvm {
struct playerTagComponent {};
}; // namespace glvm

namespace glvm {
struct projectileTagComponent {};
}; // namespace glvm

namespace glvm {
struct staticMeshTagComponent {};
}; // namespace glvm

namespace glvm {
struct TextureHandle {
    uint32_t id;
};

struct Texture {
    // This field using to choose specific instance of texture image in Vulkan.
    unsigned int vkAvailableInnerId_ = 0;
    unsigned int vkInnerIdLimit_ = 10;

    const char* path_to_image = "";
    std::vector<unsigned int> entitiesOwnsThisTypeOfTexture_ = {};
    unsigned int id_ = 0;
    unsigned int iWidth_ = 0;
    unsigned int iHeight_ = 0;
    unsigned int dat_length_ = 0;
    unsigned char* u_iData_ = 0;
};
} // namespace glvm

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>;

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>> {
    auto task = std::make_shared<
        std::packaged_task<typename std::invoke_result_t<F, Args...>()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<typename std::invoke_result_t<F, Args...>> res =
        task->get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex);

        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

#ifdef __linux__
#include "pointer-constraints-unstable-v1-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#endif // __linux__

template<class T>
struct point2D {
    T x;
    T y;
};

template<typename T>
std::ostream& operator<<(std::ostream& ostream, const point2D<T>& point) {
    ostream << "x: " << point.x << " y: " << point.y;
    return ostream;
}

struct point3D {
    float x;
    float y;
    float z;
};

template<typename T>
void clamp(T lowerThreshold, T& targetValue, T upperThreshold) {
    if (targetValue < lowerThreshold) {
        targetValue = lowerThreshold;
    } else if (targetValue > upperThreshold) {
        targetValue = upperThreshold;
    }
}

template<class T2, int var2>
class Vector;

template<class T, int var>
class Matrix {
    T m_matrix[var][var] {};

public:
    Matrix(T arg = 0) {
        for (int i = 0; i < var; ++i) {
            m_matrix[i][i] = arg;
        }
    }

    Matrix(
        Vector<T, var> _vector1,
        Vector<T, var> _vector2,
        Vector<T, var> _vector3,
        Vector<T, var> _vector4
    ) {
        m_matrix[0][0] = _vector1[0];
        m_matrix[0][1] = _vector1[1];
        m_matrix[0][2] = _vector1[2];
        m_matrix[0][3] = _vector1[3];

        m_matrix[1][0] = _vector2[0];
        m_matrix[1][1] = _vector2[1];
        m_matrix[1][2] = _vector2[2];
        m_matrix[1][3] = _vector2[3];

        m_matrix[2][0] = _vector3[0];
        m_matrix[2][1] = _vector3[1];
        m_matrix[2][2] = _vector3[2];
        m_matrix[2][3] = _vector3[3];

        m_matrix[3][0] = _vector4[0];
        m_matrix[3][1] = _vector4[1];
        m_matrix[3][2] = _vector4[2];
        m_matrix[3][3] = _vector4[3];
    }

    void SelfTensorTranspose() {
        T tempMatrix[var][var];
        for (int p = 0; p < var; ++p) {
            for (int u = 0; u < var; ++u) {
                tempMatrix[p][u] = m_matrix[u][p];
            }
        }
        for (int j = 0; j < var; ++j) {
            for (int z = 0; z < var; ++z) {
                this->m_matrix[j][z] = tempMatrix[j][z];
            }
        }
    }

    void SelfIdentity() {
        for (int i = 0; i < var; ++i) {
            for (int j = 0; j < var; ++j) {
                if (i == j) {
                    this->m_matrix[i][j] = 1.0f;
                } else {
                    this->m_matrix[i][j] = 0.0f;
                }
            }
        }
    }

    Matrix<T, var> operator+(const Matrix& matrix);
    Matrix<T, var> operator*(const T scalar);
    Matrix<T, var> operator*(const Matrix& matrix);
    T* operator[](const int index);
    const T* operator[](const int index) const;
    template<class T2, int var2>
    Vector<T2, var2> operator*(const Vector<T2, var2>& vector);
};

template<class T, int var>
Matrix<T, var> Matrix<T, var>::operator+(const Matrix& matrix) {
    Matrix<T, var> tempMatrix;
    for (int i = 0; i < var; ++i) {
        for (int j = 0; j < var; ++j) {
            tempMatrix[i][j] = this->m_matrix[i][j] + matrix.m_matrix[i][j];
        }
    }

    return tempMatrix;
}

template<class T, int var>
Matrix<T, var> Matrix<T, var>::operator*(const T scalar) {
    Matrix<T, var> tempMatrix;
    for (int i = 0; i < var; ++i) {
        for (int j = 0; j < var; ++j) {
            tempMatrix[i][j] = this->m_matrix[i][j] * scalar;
        }
    }

    return tempMatrix;
}

template<class T, int var>
Matrix<T, var> Matrix<T, var>::operator*(const Matrix& matrix) {
    Matrix<T, var> tempMatrix;
    for (int i = 0; i < var; ++i) {
        for (int j = 0; j < var; ++j) {
            for (int n = 0; n < var; ++n) {
                tempMatrix.m_matrix[i][j] +=
                    m_matrix[i][n] * matrix.m_matrix[n][j];
            }
        }
    }
    return tempMatrix;
}

template<class T, int var>
T* Matrix<T, var>::operator[](const int index) {
    return m_matrix[index];
}

template<class T, int var>
const T* Matrix<T, var>::operator[](const int index) const {
    return m_matrix[index];
}

template<int var2>
std::ostream& operator<<(
    std::ostream& ostream,
    const Matrix<float, var2>& matrix
) {
    for (int i = 0; i < var2; ++i) {
        ostream << std::endl;
        for (int j = 0; j < var2; ++j) {
            ostream << matrix[i][j] << " ";
        }
    }
    return ostream;
}

template<class T, int var>
template<class T2, int var2>
Vector<T2, var2> Matrix<T, var>::operator*(const Vector<T2, var2>& vector) {
    static_assert(var == var2, "Size error");
    Vector<T2, var2> tempVector;
    for (int i = 0; i < var2; ++i) {
        for (int j = 0; j < var; ++j) {
            tempVector[i] += m_matrix[i][j] * vector[j];
        }
    }
    return tempVector;
}

template<class T2, int dim>
class Vector {
public:
    T2 m_vector[dim] {};

public:
    Vector(T2 x = 0, T2 y = 0, T2 z = 0, T2 w = 0) {
        T2 array[4] = {x, y, z, w};
        for (int i = 0; i < dim; ++i) {
            m_vector[i] = array[i];
        }
    }

    T2& operator[](const int index);
    const T2& operator[](const int index) const;
    template<class T, int dim2>
    Vector<T2, dim> operator*(const Matrix<T, dim2>& matrix);
    Vector<T2, dim> operator*(const Vector<T2, dim>& _vector);
    Vector<T2, dim> operator*=(const Vector<T2, dim>& _vector);
    Vector<T2, dim> operator-(const Vector<T2, dim>& _vector) const;
    Vector<T2, dim> operator+(const Vector<T2, dim>& _vector) const;
    void operator-=(const Vector<T2, dim>& _vector);
    void operator+=(const Vector<T2, dim>& _vector);
    Vector<T2, dim> operator*(const T2& _scalar);
    Vector<T2, dim> operator-();
    T2 Length() const;
};

template<class T2, int var2>
T2 Vector<T2, var2>::Length() const {
    return std::sqrt(
        m_vector[0] * m_vector[0] + m_vector[1] * m_vector[1]
        + m_vector[2] * m_vector[2]
    );
}

template<class T2, int var2>
std::ostream& operator<<(std::ostream& ostream, const Vector<T2, var2>& vector) {
    if (var2 == 3) {
        ostream << "x: " << vector[0] << " y: " << vector[1]
                << " z: " << vector[2] << " length: " << vector.Length();
    } else if (var2 == 4) {
        ostream << "x: " << vector[0] << " y: " << vector[1]
                << " z: " << vector[2] << " w: " << vector[3]
                << " length: " << vector.Length();
    }

    return ostream;
}

template<class T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator-() {
    Vector<T2, var2> tempVector;
    for (int i = 0; i < var2; ++i) {
        tempVector[i] = -m_vector[i];
    }

    return tempVector;
}

template<class T2, int var2>
T2& Vector<T2, var2>::operator[](const int index) {
    return m_vector[index];
}

template<class T2, int var2>
const T2& Vector<T2, var2>::operator[](const int index) const {
    return m_vector[index];
}

template<class T2, int var2>
template<class T, int var>
Vector<T2, var2> Vector<T2, var2>::operator*(const Matrix<T, var>& matrix) {
    static_assert(var == var2, "Size error");
    Vector<T2, var2> tempVector;
    for (int i = 0; i < var2; ++i) {
        for (int j = 0; j < var; ++j) {
            tempVector[i] += m_vector[j] * matrix[j][i];
        }
    }
    return tempVector;
}

template<typename T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator*(const Vector<T2, var2>& _vector) {
    Vector<T2, var2> tempVector;
    for (int i = 0; i < 3; ++i) {
        tempVector[i] = m_vector[i] * _vector[i];
    }

    return tempVector;
}

template<typename T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator*=(const Vector<T2, var2>& _vector) {
    Vector<T2, var2> tempVector;
    for (int i = 0; i < 3; ++i) {
        tempVector[i] = m_vector[i] * _vector[i];
    }

    return tempVector;
}

template<typename T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator-(
    const Vector<T2, var2>& _vector
) const {
    Vector<T2, var2> temp_Vector(1);

    temp_Vector[0] = m_vector[0] - _vector[0];
    temp_Vector[1] = m_vector[1] - _vector[1];
    temp_Vector[2] = m_vector[2] - _vector[2];

    return temp_Vector;
}

template<typename T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator+(
    const Vector<T2, var2>& _vector
) const {
    Vector<T2, var2> temp_Vector(1);

    temp_Vector[0] = m_vector[0] + _vector[0];
    temp_Vector[1] = m_vector[1] + _vector[1];
    temp_Vector[2] = m_vector[2] + _vector[2];

    return temp_Vector;
}

template<typename T2, int var2>
void Vector<T2, var2>::operator-=(const Vector<T2, var2>& _vector) {
    m_vector[0] = m_vector[0] - _vector[0];
    m_vector[1] = m_vector[1] - _vector[1];
    m_vector[2] = m_vector[2] - _vector[2];
}

template<typename T2, int var2>
void Vector<T2, var2>::operator+=(const Vector<T2, var2>& _vector) {
    m_vector[0] = m_vector[0] + _vector[0];
    m_vector[1] = m_vector[1] + _vector[1];
    m_vector[2] = m_vector[2] + _vector[2];
}

template<typename T2, int var2>
Vector<T2, var2> Vector<T2, var2>::operator*(const T2& _scalar) {
    Vector<T2, var2> temp_vec(1.0f);

    for (int i = 0; i < var2; ++i) {
        temp_vec[i] = m_vector[i] * _scalar;
    }

    return temp_vec;
}

template<typename T2, int var2>
Vector<T2, var2> operator*(const Vector<T2, var2>& vector, const T2 _scalar) {
    Vector<T2, var2> temp;
    for (int i = 0; i < var2; ++i) {
        temp[i] = vector[i] * _scalar;
    }
    return temp;
}

template<typename T>
T determinant_2x2(Matrix<T, 2> matrix) {
    return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
}

template<typename T>
T determinant_3x3(Matrix<T, 3> matrix) {
    Matrix<T, 2> remove_1row_1col(0.0f);
    remove_1row_1col[0][0] = matrix[1][1];
    remove_1row_1col[0][1] = matrix[1][2];
    remove_1row_1col[1][0] = matrix[2][1];
    remove_1row_1col[1][1] = matrix[2][2];
    Matrix<T, 2> remove_1row_2col(0.0f);
    remove_1row_2col[0][0] = matrix[1][0];
    remove_1row_2col[0][1] = matrix[1][2];
    remove_1row_2col[1][0] = matrix[2][0];
    remove_1row_2col[1][1] = matrix[2][2];
    Matrix<T, 2> remove_1row_3col(0.0f);
    remove_1row_3col[0][0] = matrix[1][0];
    remove_1row_3col[0][1] = matrix[1][1];
    remove_1row_3col[1][0] = matrix[2][0];
    remove_1row_3col[1][1] = matrix[2][1];
    T determinant = matrix[0][0] * determinant_2x2<T>(remove_1row_1col)
        - matrix[0][1] * determinant_2x2<T>(remove_1row_2col)
        + matrix[0][2] * determinant_2x2<T>(remove_1row_3col);
    return determinant;
}

template<typename T>
T determinant_4x4(Matrix<T, 4> matrix) {
    Matrix<T, 3> remove_1row_1col(0.0f);
    remove_1row_1col[0][0] = matrix[1][1];
    remove_1row_1col[0][1] = matrix[1][2];
    remove_1row_1col[0][2] = matrix[1][3];
    remove_1row_1col[1][0] = matrix[2][1];
    remove_1row_1col[1][1] = matrix[2][2];
    remove_1row_1col[1][2] = matrix[2][3];
    remove_1row_1col[2][0] = matrix[3][1];
    remove_1row_1col[2][1] = matrix[3][2];
    remove_1row_1col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_1row_2col(0.0f);
    remove_1row_2col[0][0] = matrix[1][0];
    remove_1row_2col[0][1] = matrix[1][2];
    remove_1row_2col[0][2] = matrix[1][3];
    remove_1row_2col[1][0] = matrix[2][0];
    remove_1row_2col[1][1] = matrix[2][2];
    remove_1row_2col[1][2] = matrix[2][3];
    remove_1row_2col[2][0] = matrix[3][0];
    remove_1row_2col[2][1] = matrix[3][2];
    remove_1row_2col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_1row_3col(0.0f);
    remove_1row_3col[0][0] = matrix[1][0];
    remove_1row_3col[0][1] = matrix[1][1];
    remove_1row_3col[0][2] = matrix[1][3];
    remove_1row_3col[1][0] = matrix[2][0];
    remove_1row_3col[1][1] = matrix[2][1];
    remove_1row_3col[1][2] = matrix[2][3];
    remove_1row_3col[2][0] = matrix[3][0];
    remove_1row_3col[2][1] = matrix[3][1];
    remove_1row_3col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_1row_4col(0.0f);
    remove_1row_4col[0][0] = matrix[1][0];
    remove_1row_4col[0][1] = matrix[1][1];
    remove_1row_4col[0][2] = matrix[1][2];
    remove_1row_4col[1][0] = matrix[2][0];
    remove_1row_4col[1][1] = matrix[2][1];
    remove_1row_4col[1][2] = matrix[2][2];
    remove_1row_4col[2][0] = matrix[3][0];
    remove_1row_4col[2][1] = matrix[3][1];
    remove_1row_4col[2][2] = matrix[3][2];
    T determinant = matrix[0][0] * determinant_3x3<float>(remove_1row_1col)
        - matrix[0][1] * determinant_3x3<float>(remove_1row_2col)
        + matrix[0][2] * determinant_3x3<float>(remove_1row_3col)
        - matrix[0][3] * determinant_3x3<float>(remove_1row_4col);
    return determinant;
}

template<typename T>
Matrix<T, 4> inverse_matrix_4x4(Matrix<T, 4> matrix) {
    Matrix<T, 3> remove_1row_1col(0.0f);
    remove_1row_1col[0][0] = matrix[1][1];
    remove_1row_1col[0][1] = matrix[1][2];
    remove_1row_1col[0][2] = matrix[1][3];
    remove_1row_1col[1][0] = matrix[2][1];
    remove_1row_1col[1][1] = matrix[2][2];
    remove_1row_1col[1][2] = matrix[2][3];
    remove_1row_1col[2][0] = matrix[3][1];
    remove_1row_1col[2][1] = matrix[3][2];
    remove_1row_1col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_1row_2col(0.0f);
    remove_1row_2col[0][0] = matrix[1][0];
    remove_1row_2col[0][1] = matrix[1][2];
    remove_1row_2col[0][2] = matrix[1][3];
    remove_1row_2col[1][0] = matrix[2][0];
    remove_1row_2col[1][1] = matrix[2][2];
    remove_1row_2col[1][2] = matrix[2][3];
    remove_1row_2col[2][0] = matrix[3][0];
    remove_1row_2col[2][1] = matrix[3][2];
    remove_1row_2col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_1row_3col(0.0f);
    remove_1row_3col[0][0] = matrix[1][0];
    remove_1row_3col[0][1] = matrix[1][1];
    remove_1row_3col[0][2] = matrix[1][3];
    remove_1row_3col[1][0] = matrix[2][0];
    remove_1row_3col[1][1] = matrix[2][1];
    remove_1row_3col[1][2] = matrix[2][3];
    remove_1row_3col[2][0] = matrix[3][0];
    remove_1row_3col[2][1] = matrix[3][1];
    remove_1row_3col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_1row_4col(0.0f);
    remove_1row_4col[0][0] = matrix[1][0];
    remove_1row_4col[0][1] = matrix[1][1];
    remove_1row_4col[0][2] = matrix[1][2];
    remove_1row_4col[1][0] = matrix[2][0];
    remove_1row_4col[1][1] = matrix[2][1];
    remove_1row_4col[1][2] = matrix[2][2];
    remove_1row_4col[2][0] = matrix[3][0];
    remove_1row_4col[2][1] = matrix[3][1];
    remove_1row_4col[2][2] = matrix[3][2];
    Matrix<T, 3> remove_2row_1col(0.0f);
    remove_2row_1col[0][0] = matrix[0][1];
    remove_2row_1col[0][1] = matrix[0][2];
    remove_2row_1col[0][2] = matrix[0][3];
    remove_2row_1col[1][0] = matrix[2][1];
    remove_2row_1col[1][1] = matrix[2][2];
    remove_2row_1col[1][2] = matrix[2][3];
    remove_2row_1col[2][0] = matrix[3][1];
    remove_2row_1col[2][1] = matrix[3][2];
    remove_2row_1col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_2row_2col(0.0f);
    remove_2row_2col[0][0] = matrix[0][0];
    remove_2row_2col[0][1] = matrix[0][2];
    remove_2row_2col[0][2] = matrix[0][3];
    remove_2row_2col[1][0] = matrix[2][0];
    remove_2row_2col[1][1] = matrix[2][2];
    remove_2row_2col[1][2] = matrix[2][3];
    remove_2row_2col[2][0] = matrix[3][0];
    remove_2row_2col[2][1] = matrix[3][2];
    remove_2row_2col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_2row_3col(0.0f);
    remove_2row_3col[0][0] = matrix[0][0];
    remove_2row_3col[0][1] = matrix[0][1];
    remove_2row_3col[0][2] = matrix[0][3];
    remove_2row_3col[1][0] = matrix[2][0];
    remove_2row_3col[1][1] = matrix[2][1];
    remove_2row_3col[1][2] = matrix[2][3];
    remove_2row_3col[2][0] = matrix[3][0];
    remove_2row_3col[2][1] = matrix[3][1];
    remove_2row_3col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_2row_4col(0.0f);
    remove_2row_4col[0][0] = matrix[0][0];
    remove_2row_4col[0][1] = matrix[0][1];
    remove_2row_4col[0][2] = matrix[0][2];
    remove_2row_4col[1][0] = matrix[2][0];
    remove_2row_4col[1][1] = matrix[2][1];
    remove_2row_4col[1][2] = matrix[2][2];
    remove_2row_4col[2][0] = matrix[3][0];
    remove_2row_4col[2][1] = matrix[3][1];
    remove_2row_4col[2][2] = matrix[3][2];
    Matrix<T, 3> remove_3row_1col(0.0f);
    remove_3row_1col[0][0] = matrix[0][1];
    remove_3row_1col[0][1] = matrix[0][2];
    remove_3row_1col[0][2] = matrix[0][3];
    remove_3row_1col[1][0] = matrix[1][1];
    remove_3row_1col[1][1] = matrix[1][2];
    remove_3row_1col[1][2] = matrix[1][3];
    remove_3row_1col[2][0] = matrix[3][1];
    remove_3row_1col[2][1] = matrix[3][2];
    remove_3row_1col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_3row_2col(0.0f);
    remove_3row_2col[0][0] = matrix[0][0];
    remove_3row_2col[0][1] = matrix[0][2];
    remove_3row_2col[0][2] = matrix[0][3];
    remove_3row_2col[1][0] = matrix[1][0];
    remove_3row_2col[1][1] = matrix[1][2];
    remove_3row_2col[1][2] = matrix[1][3];
    remove_3row_2col[2][0] = matrix[3][0];
    remove_3row_2col[2][1] = matrix[3][2];
    remove_3row_2col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_3row_3col(0.0f);
    remove_3row_3col[0][0] = matrix[0][0];
    remove_3row_3col[0][1] = matrix[0][1];
    remove_3row_3col[0][2] = matrix[0][3];
    remove_3row_3col[1][0] = matrix[1][0];
    remove_3row_3col[1][1] = matrix[1][1];
    remove_3row_3col[1][2] = matrix[1][3];
    remove_3row_3col[2][0] = matrix[3][0];
    remove_3row_3col[2][1] = matrix[3][1];
    remove_3row_3col[2][2] = matrix[3][3];
    Matrix<T, 3> remove_3row_4col(0.0f);
    remove_3row_4col[0][0] = matrix[0][0];
    remove_3row_4col[0][1] = matrix[0][1];
    remove_3row_4col[0][2] = matrix[0][2];
    remove_3row_4col[1][0] = matrix[1][0];
    remove_3row_4col[1][1] = matrix[1][1];
    remove_3row_4col[1][2] = matrix[1][2];
    remove_3row_4col[2][0] = matrix[3][0];
    remove_3row_4col[2][1] = matrix[3][1];
    remove_3row_4col[2][2] = matrix[3][2];
    Matrix<T, 3> remove_4row_1col(0.0f);
    remove_4row_1col[0][0] = matrix[0][1];
    remove_4row_1col[0][1] = matrix[0][2];
    remove_4row_1col[0][2] = matrix[0][3];
    remove_4row_1col[1][0] = matrix[1][1];
    remove_4row_1col[1][1] = matrix[1][2];
    remove_4row_1col[1][2] = matrix[1][3];
    remove_4row_1col[2][0] = matrix[2][1];
    remove_4row_1col[2][1] = matrix[2][2];
    remove_4row_1col[2][2] = matrix[2][3];
    Matrix<T, 3> remove_4row_2col(0.0f);
    remove_4row_2col[0][0] = matrix[0][0];
    remove_4row_2col[0][1] = matrix[0][2];
    remove_4row_2col[0][2] = matrix[0][3];
    remove_4row_2col[1][0] = matrix[1][0];
    remove_4row_2col[1][1] = matrix[1][2];
    remove_4row_2col[1][2] = matrix[1][3];
    remove_4row_2col[2][0] = matrix[2][0];
    remove_4row_2col[2][1] = matrix[2][2];
    remove_4row_2col[2][2] = matrix[2][3];
    Matrix<T, 3> remove_4row_3col(0.0f);
    remove_4row_3col[0][0] = matrix[0][0];
    remove_4row_3col[0][1] = matrix[0][1];
    remove_4row_3col[0][2] = matrix[0][3];
    remove_4row_3col[1][0] = matrix[1][0];
    remove_4row_3col[1][1] = matrix[1][1];
    remove_4row_3col[1][2] = matrix[1][3];
    remove_4row_3col[2][0] = matrix[2][0];
    remove_4row_3col[2][1] = matrix[2][1];
    remove_4row_3col[2][2] = matrix[2][3];
    Matrix<T, 3> remove_4row_4col(0.0f);
    remove_4row_4col[0][0] = matrix[0][0];
    remove_4row_4col[0][1] = matrix[0][1];
    remove_4row_4col[0][2] = matrix[0][2];
    remove_4row_4col[1][0] = matrix[1][0];
    remove_4row_4col[1][1] = matrix[1][1];
    remove_4row_4col[1][2] = matrix[1][2];
    remove_4row_4col[2][0] = matrix[2][0];
    remove_4row_4col[2][1] = matrix[2][1];
    remove_4row_4col[2][2] = matrix[2][2];
    Matrix<T, 4> matrix_of_minors(0.0f);
    matrix_of_minors[0][0] = determinant_3x3<T>(remove_1row_1col);
    matrix_of_minors[0][1] = determinant_3x3<T>(remove_1row_2col);
    matrix_of_minors[0][2] = determinant_3x3<T>(remove_1row_3col);
    matrix_of_minors[0][3] = determinant_3x3<T>(remove_1row_4col);
    matrix_of_minors[1][0] = determinant_3x3<T>(remove_2row_1col);
    matrix_of_minors[1][1] = determinant_3x3<T>(remove_2row_2col);
    matrix_of_minors[1][2] = determinant_3x3<T>(remove_2row_3col);
    matrix_of_minors[1][3] = determinant_3x3<T>(remove_2row_4col);
    matrix_of_minors[2][0] = determinant_3x3<T>(remove_3row_1col);
    matrix_of_minors[2][1] = determinant_3x3<T>(remove_3row_2col);
    matrix_of_minors[2][2] = determinant_3x3<T>(remove_3row_3col);
    matrix_of_minors[2][3] = determinant_3x3<T>(remove_3row_4col);
    matrix_of_minors[3][0] = determinant_3x3<T>(remove_4row_1col);
    matrix_of_minors[3][1] = determinant_3x3<T>(remove_4row_2col);
    matrix_of_minors[3][2] = determinant_3x3<T>(remove_4row_3col);
    matrix_of_minors[3][3] = determinant_3x3<T>(remove_4row_4col);
    // Compute matrix of cofactors.
    Matrix<T, 4> matrix_of_cofactors(0.0f);
    for (unsigned int i = 0; i < 4; ++i) {
        for (unsigned int j = 0; j < 4; ++j) {
            if ((i + j) % 2 == 0) {
                matrix_of_cofactors[i][j] = matrix_of_minors[i][j];
            } else {
                matrix_of_cofactors[i][j] = -matrix_of_minors[i][j];
            }
        }
    }
    // Compute adjoint matrix.
    Matrix<T, 4> adjoint_matrix(0.0f);
    adjoint_matrix = matrix_of_cofactors;
    adjoint_matrix.SelfTensorTranspose();
    T determinant_of_basic_matrix = determinant_4x4<T>(matrix);
    Matrix<T, 4> inverse_matrix(0.0f);
    inverse_matrix = adjoint_matrix * (1 / determinant_of_basic_matrix);
    return inverse_matrix;
}

template<class T, class T2, int var, int var2>
Matrix<T, var> LookAt(Matrix<T, var> matrix, Vector<T2, var2> vector) {
    Matrix<T, var> tempMatrix(1.0f);
    tempMatrix = matrix;

    const unsigned int variable = 3;
    for (int i = 0; i < var2; ++i) {
        tempMatrix[i][variable] = -vector[i];
    }

    return tempMatrix;
}

template<class T, class T2, int var, int var2>
Matrix<T, var> Translate(Matrix<T, var> matrix, Vector<T2, var2> vector) {
    Matrix<T, var> tempMatrix(1.0f);
    tempMatrix = matrix;
    for (int i = 0; i < var; ++i) {
        tempMatrix[var - 1][i] += vector[i];
    }
    return tempMatrix;
}

template<class T, class T2, int var, int var2>
Matrix<T, var> Scale(Matrix<T, var> matrix, Vector<T2, var2> vector) {
    Matrix<T, var> tempMatrix;
    tempMatrix = matrix;
    for (int i = 0; i < var; ++i) {
        for (int j = 0; j < var; ++j) {
            tempMatrix[j][i] *= vector[i];
        }
    }
    return tempMatrix;
}

template<class T, int var>
Matrix<T, var> RotateZ(Matrix<T, var> matrix, float angle) {
    Matrix<T, var> tempMatrix(1.0f);
    tempMatrix[0][0] = std::cos(angle * PI / 180);
    tempMatrix[0][1] = -std::sin(angle * PI / 180);
    tempMatrix[1][0] = std::sin(angle * PI / 180);
    tempMatrix[1][1] = std::cos(angle * PI / 180);
    Matrix<T, var> tempMatrix2;
    tempMatrix2 = matrix * tempMatrix;
    return tempMatrix2;
}

template<typename T>
Vector<T, 3> Cross(const Vector<T, 3>& _vector1, const Vector<T, 3>& _vector2) {
    return Vector<T, 3>(
        _vector1[1] * _vector2[2] - _vector1[2] * _vector2[1],
        _vector1[2] * _vector2[0] - _vector1[0] * _vector2[2],
        _vector1[0] * _vector2[1] - _vector1[1] * _vector2[0]
    );
}

template<typename T>
T Dot(const Vector<T, 3>& _vector1, const Vector<T, 3>& _vector2) {
    return (
        _vector1[0] * _vector2[0] + _vector1[1] * _vector2[1]
        + _vector1[2] * _vector2[2]
    );
}

template<typename T>
T VectorLength(const Vector<T, 3>& _vector1, const Vector<T, 3>& _vector2) {
    T _x_axis = _vector2[0] - _vector1[0];
    T _y_axis = _vector2[1] - _vector1[1];
    T _z_axis = _vector2[2] - _vector1[2];
    return std::sqrt(_x_axis * _x_axis + _y_axis * _y_axis + _z_axis * _z_axis);
}

template<typename T>
T VecLength(const Vector<T, 3>& vector) {
    return std::sqrt(
        vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]
    );
}

template<typename T>
Vector<T, 3> Normalize(Vector<T, 3> _vector) {
    if (_vector[0] == 0 && _vector[1] == 0 && _vector[2] == 0) {
        return Vector<float, 3> {0.0f, 0.0f, 0.0f};
    }
    float range = std::sqrt(
        _vector[0] * _vector[0] + _vector[1] * _vector[1]
        + _vector[2] * _vector[2]
    );
    for (int l = 0; l < 3; ++l) {
        _vector[l] = _vector[l] / range;
    }
    return _vector;
}

template<typename T>
Matrix<T, 4> GLVM_perspectiveRH_NO(T fov, T aspect, T near_plane, T far_plane) {
    const T tanHalfFov = std::tan(fov / static_cast<T>(2));
    Matrix<float, 4> Result(static_cast<T>(0));
    Result[0][0] = static_cast<T>(1) / (aspect * tanHalfFov);
    Result[1][1] = static_cast<T>(1) / (tanHalfFov);
    Result[2][2] = -(far_plane - near_plane) / (far_plane - near_plane);
    Result[2][3] = -static_cast<T>(1);
    Result[3][2] = -(static_cast<T>(2) * far_plane * near_plane)
        / (far_plane - near_plane);
    return Result;
}

template<typename T>
Matrix<T, 4> Perspective(T fov, T aspect, T near_plane, T far_plane) {
    return GLVM_perspectiveRH_NO<T>(fov, aspect, near_plane, far_plane);
}

template<typename T>
Matrix<T, 4> lookAtRH(Vector<T, 3> _eye, Vector<T, 3> _center, Vector<T, 3> _up) {
    Vector<T, 3> f = (Normalize(_center - _eye));
    Vector<T, 3> s = (Normalize(Cross(f, _up)));
    Vector<T, 3> u = (Cross(s, f));
    Matrix<T, 4> Result(1.0f);
    Result[0][0] = s[0];
    Result[1][0] = s[1];
    Result[2][0] = s[2];
    Result[0][1] = u[0];
    Result[1][1] = u[1];
    Result[2][1] = u[2];
    Result[0][2] = -f[0];
    Result[1][2] = -f[1];
    Result[2][2] = -f[2];
    Result[3][0] = -Dot(s, _eye);
    Result[3][1] = -Dot(u, _eye);
    Result[3][2] = Dot(f, _eye);
    return Result;
}

template<typename T>
Matrix<T, 4> LookAtMain(
    Vector<T, 3> _eye,
    Vector<T, 3> _center,
    Vector<T, 3> _up
) {
    return lookAtRH<T>(_eye, _center, _up);
}

template<typename T>
Matrix<T, 4> FPSview(Vector<T, 3> _eye, Vector<T, 3> _center, Vector<T, 3> _up) {
    Vector<T, 3> f(Normalize(_center - _eye));
    Vector<T, 3> s(Normalize(Cross(f, _up)));
    Vector<T, 3> u(Cross(s, f));
    Matrix<T, 4> Result(1.0f);
    Result[0][0] = s[0];
    Result[1][0] = s[1];
    Result[2][0] = s[2];
    Result[0][1] = u[0];
    Result[1][1] = u[1];
    Result[2][1] = u[2];
    Result[0][2] = -f[0];
    Result[1][2] = -f[1];
    Result[2][2] = -f[2];
    Result[3][0] = -Dot(s, _eye);
    Result[3][1] = -Dot(u, _eye);
    Result[3][2] = Dot(f, _eye);
    return Result;
}

template<typename T3>
T3 Radians(T3 _angle) {
    _angle *= PI / static_cast<T3>(180);
    return _angle;
}

template<typename T>
Matrix<T, 4> FPS_View_RH(Vector<T, 3> _eye, float _pitch, float _yaw) {
    _pitch *= PI / 180;
    _yaw *= PI / 180;
    float fCos_Pitch = std::cos(_pitch);
    float fSin_Pitch = std::sin(_pitch);
    float fCos_Yaw = std::cos(_yaw);
    float fSin_Yaw = std::sin(_yaw);
    Vector<T, 3> x_axis(fCos_Yaw, 0, -fSin_Yaw);
    Vector<T, 3> y_axis(
        fSin_Yaw * fSin_Pitch,
        fCos_Pitch,
        fCos_Yaw * fSin_Pitch
    );
    Vector<T, 3> z_axis(
        fSin_Yaw * fCos_Pitch,
        -fSin_Pitch,
        fCos_Pitch * fCos_Yaw
    );
    Matrix<T, 4> tView(
        Vector<T, 4>(x_axis[0], y_axis[0], z_axis[0], 0),
        Vector<T, 4>(x_axis[1], y_axis[1], z_axis[1], 0),
        Vector<T, 4>(x_axis[2], y_axis[2], z_axis[2], 0),
        Vector<T, 4>(
            -Dot(x_axis, _eye),
            -Dot(y_axis, _eye),
            -Dot(z_axis, _eye),
            1
        )
    );
    return tView;
}

template<class T, int var, int vec_size>
Matrix<T, var> Rotate(Vector<T, vec_size> vector, float angle) {
    vector = (Normalize(vector));
    Matrix<T, var> tempMatrix(1.0f);
    // Transposed rotate matrix.
    tempMatrix[0][0] = std::cos(angle)
        + (vector[0] * vector[0]) * (static_cast<T>(1) - std::cos(angle));
    tempMatrix[1][0] =
        vector[0] * vector[1] * (static_cast<T>(1) - std::cos(angle))
        - vector[2] * std::sin(angle);
    tempMatrix[2][0] =
        vector[0] * vector[2] * (static_cast<T>(1) - std::cos(angle))
        + vector[1] * std::sin(angle);
    tempMatrix[3][0] = static_cast<T>(0);
    tempMatrix[0][1] =
        vector[1] * vector[0] * (static_cast<T>(1) - std::cos(angle))
        + vector[2] * std::sin(angle);
    tempMatrix[1][1] = std::cos(angle)
        + (vector[1] * vector[1]) * (static_cast<T>(1) - std::cos(angle));
    tempMatrix[2][1] =
        vector[1] * vector[2] * (static_cast<T>(1) - std::cos(angle))
        - vector[0] * std::sin(angle);
    tempMatrix[3][1] = static_cast<T>(0);
    tempMatrix[0][2] =
        vector[2] * vector[0] * (static_cast<T>(1) - std::cos(angle))
        - vector[1] * std::sin(angle);
    tempMatrix[1][2] =
        vector[2] * vector[1] * (static_cast<T>(1) - std::cos(angle))
        + vector[0] * std::sin(angle);
    tempMatrix[2][2] = std::cos(angle)
        + (vector[2] * vector[2]) * (static_cast<T>(1) - std::cos(angle));
    tempMatrix[3][2] = static_cast<T>(0);
    tempMatrix[0][3] = static_cast<T>(0);
    tempMatrix[1][3] = static_cast<T>(0);
    tempMatrix[2][3] = static_cast<T>(0);
    tempMatrix[3][3] = static_cast<T>(1);
    return tempMatrix;
}

template<class T, int var>
Matrix<T, var> Ortho(float w, float h, float zn, float zf) {
    Matrix<T, var> tempMatrix(1.0f);
    tempMatrix[0][0] = 2 / w;
    tempMatrix[1][1] = 2 / h;
    tempMatrix[2][2] = 1 / (zf - zn);
    return tempMatrix;
}

template<class T>
Matrix<T, 4> orthoRH_ZO(
    T left,
    T right,
    T bottom,
    T top,
    T near_plane,
    T far_plane
) {
    Matrix<float, 4> tempMatrix(1);
    tempMatrix[0][0] = static_cast<T>(2) / (right - left);
    tempMatrix[1][1] = static_cast<T>(2) / (top - bottom);
    tempMatrix[2][2] = -static_cast<T>(1) / (far_plane - near_plane);
    tempMatrix[3][0] = -(right + left) / (right - left);
    tempMatrix[3][1] = -(top + bottom) / (top - bottom);
    tempMatrix[3][2] = -near_plane / (far_plane - near_plane);

    return tempMatrix;
}

template<class T>
Matrix<T, 4> ortho(T left, T right, T bottom, T top, T near_plane, T far_plane) {
    return orthoRH_ZO<T>(left, right, bottom, top, near_plane, far_plane);
}

template<class T, int var>
Matrix<T, var> perspectiveRH_ZO(T fov, T aspect, T near_plane, T far_plane) {
    const float tanHalfFov = std::tan((fov * 0.5) * (PI / 360));
    Matrix<float, var> tempMatrix(static_cast<T>(0));
    tempMatrix[0][0] = static_cast<T>(1) / (aspect * tanHalfFov);
    tempMatrix[1][1] = static_cast<T>(1) / tanHalfFov;
    tempMatrix[2][2] = far_plane / (near_plane - far_plane);
    tempMatrix[2][3] = static_cast<T>(1);
    tempMatrix[3][2] = -(far_plane * near_plane) / (far_plane - near_plane);
    return tempMatrix;
}

template<class T, int var>
Matrix<T, var> Perspective(
    const T fov,
    const T aspect,
    const T near_plane,
    const T far_plane
) {
    return perspectiveRH_ZO(fov, aspect, near_plane, far_plane);
}

constexpr float Max(float var1, float var2) {
    return var1 > var2 ? var1 : var2;
}

constexpr float Min(float var1, float var2) {
    return var1 < var2 ? var1 : var2;
}

struct Quaternion {
    float w, x, y, z;
    Quaternion() = default;

    Quaternion(float _w, float _x, float _y, float _z) :
        w(_w),
        x(_x),
        y(_y),
        z(_z) {}

    Quaternion(float real, Vector<float, 3> imaginary) :
        w(real),
        x(imaginary[0]),
        y(imaginary[1]),
        z(imaginary[2]) {}
};

inline std::ostream& operator<<(
    std::ostream& ostream,
    const Quaternion& quaternion
) {
    ostream << "w: " << quaternion.w << " x: " << quaternion.x
            << " y: " << quaternion.y << " z: " << quaternion.z;
    return ostream;
}

inline Quaternion conjugate(Quaternion quaternion) {
    quaternion.w = quaternion.w;
    quaternion.x = -quaternion.x;
    quaternion.y = -quaternion.y;
    quaternion.z = -quaternion.z;
    return quaternion;
}

inline Quaternion multiplyQuaternion(const Quaternion& a, const Quaternion& b) {
    Quaternion result;
    result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    return result;
}

inline Quaternion operator*(const Quaternion& a, const Quaternion& b) {
    Quaternion result;
    result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    return result;
}

inline float normQuaternion(const Quaternion& quaternion) {
    return sqrt(
        quaternion.w * quaternion.w + quaternion.x * quaternion.x
        + quaternion.y * quaternion.y + quaternion.z * quaternion.z
    );
}

inline Quaternion normalizeQuaternion(Quaternion quaternion) {
    float norm = normQuaternion(quaternion);
    float inverseNorm = 1.0f / norm;
    quaternion.w *= inverseNorm;
    quaternion.x *= inverseNorm;
    quaternion.y *= inverseNorm;
    quaternion.z *= inverseNorm;
    return quaternion;
}

inline Quaternion inverseQuaternion(Quaternion quaternion) {
    Quaternion linkedValue = conjugate(quaternion);
    float norm = normQuaternion(quaternion);
    float inverseNorm = 1.0f / norm;
    quaternion.w = linkedValue.w * inverseNorm;
    quaternion.x = linkedValue.x * inverseNorm;
    quaternion.y = linkedValue.y * inverseNorm;
    quaternion.z = linkedValue.z * inverseNorm;
    return quaternion;
}

inline Quaternion eulerToQuaternion(
    const float roll,
    const float pitch,
    const float yaw
) {
    const float cr = cos(roll * 0.5);
    const float sr = sin(roll * 0.5);
    const float cp = cos(pitch * 0.5);
    const float sp = sin(pitch * 0.5);
    const float cy = cos(yaw * 0.5);
    const float sy = sin(yaw * 0.5);
    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    return q;
}

template<class T, int var>
Matrix<T, var> rotateQuaternion(Quaternion quaternion) {
    Matrix<float, 4> result(0.0f);

    quaternion = normalizeQuaternion(quaternion);

    result[0][0] =
        1 - 2 * (quaternion.y * quaternion.y + quaternion.z * quaternion.z);
    result[0][1] =
        2 * (quaternion.x * quaternion.y - quaternion.z * quaternion.w);
    result[0][2] =
        2 * (quaternion.x * quaternion.z + quaternion.y * quaternion.w);

    result[1][0] =
        2 * (quaternion.x * quaternion.y + quaternion.z * quaternion.w);
    result[1][1] =
        1 - 2 * (quaternion.x * quaternion.x + quaternion.z * quaternion.z);
    result[1][2] =
        2 * (quaternion.y * quaternion.z - quaternion.x * quaternion.w);

    result[2][0] =
        2 * (quaternion.x * quaternion.z - quaternion.y * quaternion.w);
    result[2][1] =
        2 * (quaternion.y * quaternion.z + quaternion.x * quaternion.w);
    result[2][2] =
        1 - 2 * (quaternion.x * quaternion.x + quaternion.y * quaternion.y);

    result[0][0] =
        2 * (quaternion.w * quaternion.w + quaternion.x * quaternion.x) - 1;
    result[0][1] =
        2 * (quaternion.x * quaternion.y - quaternion.w * quaternion.z);
    result[0][2] =
        2 * (quaternion.x * quaternion.z + quaternion.w * quaternion.y);

    result[3][3] = 1.0f;

    return result;
}

namespace glvm {
class SVertex {
    float x;
    float y;
    float z;

public:
    float& operator[](const unsigned int _iIndex) {
        assert(_iIndex < 3 && _iIndex >= 0 && "Wrong index");
        switch (_iIndex) {
            default:
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
        }
    }
};

class SFace {
    std::vector<int> vertexIndex;
    std::vector<int> textureIndex;
    std::vector<int> normalIndex;

public:
    std::vector<int>& operator[](const unsigned int _iIndex) {
        assert(_iIndex < 3 && _iIndex >= 0 && "Wrong index");
        switch (_iIndex) {
            default:
            case 0:
                return vertexIndex;
            case 1:
                return textureIndex;
            case 2:
                return normalIndex;
        }
    }

    const std::vector<int>& operator[](const unsigned int _iIndex) const {
        assert(_iIndex < 3 && _iIndex >= 0 && "Wrong index");
        switch (_iIndex) {
            default:
            case 0:
                return vertexIndex;
            case 1:
                return textureIndex;
            case 2:
                return normalIndex;
        }
    }
};

class CWaveFrontObjParser {
    std::vector<SVertex> coordinateVertices_;
    std::vector<SVertex> textureVertices_;
    std::vector<SVertex> normals_;
    std::vector<SFace> faces_;

    std::string sWavefrontObjFileData;
    const char* pWavefrontObjFileData;
    unsigned int uiCounter = 0;

public:
    CWaveFrontObjParser();

    [[nodiscard]] const std::vector<SVertex>& getCoordinateVertices() const;
    [[nodiscard]] const std::vector<SVertex>& getTextureVertices() const;
    [[nodiscard]] const std::vector<SVertex>& getNormals() const;
    [[nodiscard]] const std::vector<SFace>& getFaces() const;

    void ReadFile(const char* _filePath);
    void ParseFile();
    std::vector<std::vector<char>> Split(
        const char* _pWaveFrontObjFileData,
        const char _separator,
        const char _exitSymbol,
        unsigned int& _uiCounter
    );
    SVertex ParseVertices(std::vector<std::vector<char>> _wordsContainer);
    SFace ParseFaces(std::vector<std::vector<char>> _wordsContainer);
    int ParseInteger(std::vector<char> _word);
    float ParseFloating(std::vector<char> _word);
};
} // namespace glvm

namespace glvm {
class inventory {
public:
    inventory() {
        for (unsigned int i = 0; i < row; ++i) {
            slots[i] = new unsigned int[col];
        }

        for (unsigned int i = 0; i < row; ++i) {
            for (unsigned int j = 0; j < col; ++j) {
                slots[i][j] = -1;
            }
        }
    }

    inventory(const inventory& inv) {
        for (unsigned int i = 0; i < row; ++i) {
            this->slots[i] = new unsigned int[col];
        }

        for (unsigned int i = 0; i < row; ++i) {
            for (unsigned int j = 0; j < col; ++j) {
                this->slots[i][j] = inv.slots[i][j];
            }
        }

        this->entityOwner = inv.entityOwner;
        this->highlightedSlots = inv.highlightedSlots;
        this->isAvailableHighlightedSlots = inv.isAvailableHighlightedSlots;
    }

    ~inventory() {
        for (unsigned int i = 0; i < row; ++i) {
            delete[] slots[i];
        }

        delete[] slots;
    }

    unsigned int row = 8;
    unsigned int col = 8;
    // Array with entities contained inventorySlotComponents.
    unsigned int** slots = new unsigned int*[row];
    unsigned int entityOwner = UINT_MAX;
    std::vector<unsigned int> highlightedSlots;
    bool isAvailableHighlightedSlots = false;
    MeshHandle slotMeshID;
    float slotScale;
};
}; // namespace glvm

namespace glvm {
class MeshManager {
    static MeshManager* pInstance_;
    static std::mutex Mutex_;

    MeshManager();
    ~MeshManager();

public:
    std::vector<const char*> pathsArray_;
    std::vector<const char*> pathsGLTF_;

    // It possibly to get only one instance of this class with this method.
    static MeshManager* get_instance();
    void SetMesh(const char* _pathToMesh);
    void SetMeshGLTF(const char* pathToMesh);
};
} // namespace glvm

namespace glvm {
class CStack {
    int iHead_ = 0;
    static const int iStack_Range_ = 6;
    EEvents aStack_[iStack_Range_] = {};

public:
    void Push(const EEvents& _Event) {
        for (int i = 0; i < iHead_; ++i) {
            if (aStack_[i] == _Event) {
                return;
            }
        }

        if (iHead_ == iStack_Range_) {
            return;
        }

        aStack_[iHead_] = _Event;

        ++iHead_;
    }

    EEvents& Pop() {
        if (iHead_ == 0) {
            return aStack_[0];
        }
        return aStack_[iHead_ - 1];
    }

    void Remove(const EEvents& _Event) {
        EEvents aTemp_Stack[iStack_Range_] = {};
        bool removeFlag = false;
        int n = 0;

        for (int j = 0; j < iStack_Range_; ++j) {
            aTemp_Stack[j] = aStack_[j];
        }

        for (int i = 0; i < iHead_; ++i) {
            if (_Event == aTemp_Stack[i]) {
                removeFlag = true;
                continue;
            }

            aStack_[n] = aTemp_Stack[i];
            ++n;
        }

        if (removeFlag) {
            --iHead_;
            aStack_[iHead_] = EEvents::eDEFAULT;
        }
    }

    void ControlInput(CEvent& _eEvent) {
        if (!(SearchElement(_eEvent.GetEvent()) == eEmpty)) {
            return;
        }
        switch (_eEvent.GetEvent()) {
            case eGAME_LOOP_KILL:
                Push(eGAME_LOOP_KILL);
                break;
            case eKEYRELEASE_A:
                Remove(eMOVE_LEFT);
                break;
            case eKEYRELEASE_D:
                Remove(eMOVE_RIGHT);
                break;
            case eKEYRELEASE_S:
                Remove(eMOVE_BACKWARD);
                break;
            case eKEYRELEASE_W:
                Remove(eMOVE_FORWARD);
                break;
            case eINVENTORY_RELEASE:
                Remove(eINVENTORY);
                break;
            case eKEYRELEASE_JUMP:
                Remove(eJUMP);
                break;
            case eMOUSE_LEFT_BUTTON_RELEASE:
                Remove(eMOUSE_LEFT_BUTTON);
                break;
            case eMOVE_LEFT:
                Push(eMOVE_LEFT);
                break;
            case eMOVE_RIGHT:
                Push(eMOVE_RIGHT);
                break;
            case eMOVE_BACKWARD:
                Push(eMOVE_BACKWARD);
                break;
            case eMOVE_FORWARD:
                Push(eMOVE_FORWARD);
                break;
            case eJUMP:
                Push(eJUMP);
                break;
            case eINVENTORY:
                Push(eINVENTORY);
                break;
            case eCURSOR_RELEASED:
                Push(eCURSOR_RELEASED);
                break;
            case eMOUSE_LEFT_BUTTON:
                Push(eMOUSE_LEFT_BUTTON);
                break;
            default:
                break;
        }
    }

    EEvents SearchElement(EEvents _element) {
        for (int i = 0; i < iHead_; ++i) {
            if (aStack_[i] == _element) {
                return _element;
            }
        }

        return eEmpty;
    }

    EEvents& operator[](int _iIndex) {
        return aStack_[_iIndex];
    }

    void Clear() {
        for (int i = 0; i < iHead_; ++i) {
            aStack_[i] = EEvents::eDEFAULT;
        }
    }
};
} // namespace glvm

namespace glvm {

class IWindow {
public:
    // Window keyboard focus, updated by each backend.
    bool isFocused = true;

    virtual ~IWindow() = default;

    virtual void SwapBuffers() = 0;
    virtual void ClearDisplay() = 0;
    virtual bool HandleEvent(CEvent& _Event) = 0;
    virtual void Close() = 0;
    virtual void CursorLock(
        int _x_position,
        int _y_position,
        int* _x_offset,
        int* _y_offset
    ) = 0;
};

} // namespace glvm

namespace glvm {
class CTimerCreator {
public:
    ~CTimerCreator() {}

    IChrono* Create();
};
} // namespace glvm

#ifdef __linux__

namespace glvm {
class CTimerX: public IChrono {
    timespec start_;
    timespec now_;
    double lFrequency_;
    double lSeconds_;
    double lNanoseconds_;

public:
    CTimerX();

    double InitFrequency();
    double Reset();
    double GetElapsed();
};
} // namespace glvm
#endif // __linux__

#ifdef _WIN32

namespace glvm {
class CTimerWin: public IChrono {
    __int64 i64Freq_;
    __int64 i64Start_;
    __int64 i64Now_;

public:
    CTimerWin();

    double InitFrequency();
    double Reset();
    double GetElapsed();
};
} // namespace glvm
#endif // _WIN32

namespace glvm {
struct state {
    States state;
};
} // namespace glvm

namespace glvm {
struct CSoundSample {
    const char* kPath_to_File_;
    unsigned int uiDuration_;
    unsigned int uiRate_;
    float volume;
};

class ISoundEngine {
public:
    virtual ~ISoundEngine() {}

    virtual void OpenDevice(const char* device) = 0;
    virtual void CloseDevice() = 0;
    virtual std::vector<CSoundSample*>& GetSoundContainer() = 0;
    virtual void PlaybackSoundSample(CSoundSample& _sound_sample) = 0;
    virtual void SetMasterVolume(long _lVolume) = 0;
    virtual void SoundStream() = 0;
    virtual void CreateSoundSample(
        const char* filePath,
        uint32_t duration,
        uint32_t rate,
        float volume
    ) = 0;
};
} // namespace glvm

namespace glvm {
struct Archetype {
    virtual ~Archetype() = default;

    static constexpr uint32_t CAPACITY = 1024;

    uint64_t entities[CAPACITY];
    uint32_t entityCount = 0;
    uint32_t componentIds[ComponentsIndices::ComponentsCount] = {};
    uint32_t componentCount = 0;
    void* components[ComponentsIndices::ComponentsCount] = {};
    uint64_t mask = 0;

    uint32_t addEntity(uint64_t entity_);
    uint64_t removeEntity(uint32_t index);
};

struct EntityLocation {
    Archetype* arch;
    uint32_t index;
    static const uint8_t maxGridCellNumber = 8;
    uint8_t gridCellCounter = 0;
    Vector<float, 3> gridCellIndicies[maxGridCellNumber];
    uint32_t cellEntityIndices[maxGridCellNumber];
    // Is entity has been moved or removed.
    bool isDirty = false;
};
}; // namespace glvm

namespace glvm {
struct directionalLight {
    Vector<float, 3> position;
    Vector<float, 3> direction;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;
};
} // namespace glvm

namespace glvm {
struct material {
    TextureHandle diffuseTextureID_ = {};
    TextureHandle specularTextureID_ = {};
    Vector<float, 3> ambient = {0.0f, 0.0f, 0.0f};
    float shininess = 0.0f;
};
} // namespace glvm

namespace glvm {
struct move {
    EEvents eEvent_ = EEvents::eDEFAULT;
    Vector<float, 3> frameMovement {0.0f, 0.0f, 0.0f};
    Vector<float, 3> gravity {0.0f, 0.0f, 0.0f};
};
} // namespace glvm

namespace glvm {
struct pointLight {
    Vector<float, 3> position;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};
} // namespace glvm

namespace glvm {
class RigidBody {
public:
    float fMass_ = 0.0f;
    float jumpAccumulator = 0.0f;
};
} // namespace glvm

namespace glvm {
struct spotLight {
    Vector<float, 3> position;
    Vector<float, 3> direction;
    float cutOff;
    float outerCutOff;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};
} // namespace glvm

namespace glvm {
struct transform {
    Vector<float, 3> position {0.0f, 0.0f, 0.0f};
    Vector<float, 3> forward {0.0f, 0.0f, 0.0f};
    float scale = 1.0f;
    float gravityAccumulator = 0.0f;
};
} // namespace glvm

namespace glvm {
struct beholder {
    Vector<float, 3> Position {0.0f, 0.0f, 0.0f};
    Vector<float, 3> forward {0.0f, 0.0, 0.0f};
};
} // namespace glvm

namespace glvm {
enum JsonType {
    JsonInvalidValue,
    JsonObject,
    JsonFloatNumber,
    JsonIntegerNumber,
    JsonString,
    JsonBoolean,
    JsonNull,
    JsonArray,
};

struct JsonValue;

union JsonVariant {
    std::string* string;
    double fNumber;
    int iNumber;
    bool boolean;
    void* null;
    std::vector<JsonValue>* array;
    HashMap<JsonValue>* object;

    JsonVariant() {}

    JsonVariant(const JsonVariant& object) {
        memcpy((void*)this, &object, sizeof(JsonVariant));
    }

    ~JsonVariant() {}
};

struct JsonValue {
    JsonVariant value;
    JsonType type;

    JsonValue() {
        type = JsonInvalidValue;
    }

    JsonValue(std::string _string) {
        type = JsonString;
        value.string = new std::string(_string);
    }

    JsonValue(double _float) {
        type = JsonFloatNumber;
        value.fNumber = _float;
    }

    JsonValue(int _int) {
        type = JsonIntegerNumber;
        value.iNumber = _int;
    }

    JsonValue(bool _bool) {
        type = JsonBoolean;
        value.boolean = _bool;
    }

    JsonValue(const JsonValue& _value) {
        type = JsonInvalidValue;

        switch (_value.type) {
            case JsonObject:
                value.object = new HashMap<JsonValue>(*_value.value.object);
                break;
            case JsonIntegerNumber:
                value.iNumber = _value.value.iNumber;
                break;
            case JsonFloatNumber:
                value.fNumber = _value.value.fNumber;
                break;
            case JsonString:
                value.string = new std::string(*_value.value.string);
                break;
            case JsonBoolean:
                value.boolean = _value.value.boolean;
                break;
            case JsonNull:
                value.null = _value.value.null;
                break;
            case JsonArray:
                value.array = new std::vector<JsonValue>(*_value.value.array);
                break;
            default:
                break;
        }
        type = _value.type;
    }

    ~JsonValue() {
        switch (type) {
            case JsonInvalidValue:
                break;
            case JsonObject:
                delete value.object;
                break;
            case JsonIntegerNumber:
                break;
            case JsonFloatNumber:
                break;
            case JsonString:
                delete value.string;
                break;
            case JsonBoolean:
                break;
            case JsonNull:
                break;
            case JsonArray:
                delete value.array;
                break;
        }
    }

    void operator=(const JsonValue& _value) {
        switch (type) {
            case JsonInvalidValue:
                break;
            case JsonObject:
                delete value.object;
                break;
            case JsonIntegerNumber:
                break;
            case JsonFloatNumber:
                break;
            case JsonString:
                delete value.string;
                break;
            case JsonBoolean:
                break;
            case JsonNull:
                break;
            case JsonArray:
                delete value.array;
                break;
        }

        switch (_value.type) {
            case JsonObject:
                value.object = new HashMap<JsonValue>(*_value.value.object);
                break;
            case JsonIntegerNumber:
                value.iNumber = _value.value.iNumber;
                break;
            case JsonFloatNumber:
                value.fNumber = _value.value.fNumber;
                break;
            case JsonString:
                value.string = new std::string(*_value.value.string);
                break;
            case JsonBoolean:
                value.boolean = _value.value.boolean;
                break;
            case JsonNull:
                value.null = _value.value.null;
                break;
            case JsonArray:
                value.array = new std::vector<JsonValue>(*_value.value.array);
                break;
            default:
                break;
        }
        type = _value.type;
    }

    JsonValue& operator[](std::string key_) {
        const char* key = key_.c_str();
        switch (type) {
            case JsonObject:
                return (*value.object)[key];
                break;
            default:
                throw std::out_of_range("Type is not a json object");
                break;
        }
    }

    JsonValue& operator[](const unsigned int index_) {
        switch (type) {
            case JsonArray:
                return (*value.array)[index_];
                break;
            default:
                throw std::out_of_range("Type is not a json array");
                break;
        }
    }

    bool isInvalid() {
        return type == JsonInvalidValue;
    }

    bool isObject() {
        return type == JsonObject;
    }

    bool isFloat() {
        return type == JsonFloatNumber;
    }

    bool isInterger() {
        return type == JsonIntegerNumber;
    }

    bool isString() {
        return type == JsonString;
    }

    bool isBoolean() {
        return type == JsonBoolean;
    }

    bool isNull() {
        return type == JsonNull;
    }

    bool isArray() {
        return type == JsonArray;
    }
};

class CJsonParser {
    std::string sJsonFileData_;
    const char* pJsonFileData_;
    char currentChar_;
    unsigned int globalFileCounter_ = 0;

    std::vector<JsonValue*> stackOfJsonValues_;
    JsonValue* root_;
    bool keyFlag = true;
    std::string lastKey_ = "";
    std::string bufferString_ = "";

    void SearchInJsonArray(
        std::vector<JsonValue>* arrayValue,
        const char* key_,
        std::vector<JsonValue>& resultVector
    ) const;

public:
    void SearchInJsonObject(
        HashMap<JsonValue>* mapValue,
        const char* key_,
        std::vector<JsonValue>& resultVector
    ) const;

    ~CJsonParser();

    JsonValue* GetRoot() {
        return root_;
    }

    void ReadFile(const char* _filePath);
    void Parse();
    JsonValue CreateJsonHashMap();
    JsonValue CreateJsonArray();
    std::string BoolOrNullParse();
    bool IsContainChar(std::string _string, char _char);
    std::string NumberAsStringParse();
    std::string StringParse();
    std::vector<char> StringToVectorOfChars(std::string _string);
    int ParseInteger(std::vector<char> _word);
    double ParseFloating(std::vector<char> _word);
    std::vector<JsonValue> Search(const char* key_) const;
    void LoadGLTF(
        const char* pathsGLTF_,
        std::vector<float>& aVertexes_,
        std::vector<uint32_t>& aIndices_,
        std::vector<std::vector<Matrix<float, 4>>>& jointMatricesPerMesh,
        std::vector<float>& frames,
        bool& noAnimations,
        float& topY
    );
    void traversalBones(
        std::vector<std::vector<int>> children,
        JsonValue joints,
        std::vector<uint32_t> node_stack,
        std::vector<uint32_t> deepness_stack,
        std::vector<std::vector<uint32_t>>& result
    );
    std::vector<std::vector<unsigned int>> makeRenderJointsIndices(
        std::vector<std::vector<unsigned int>>& input
    );
    bool containsElement(
        std::vector<std::vector<unsigned int>> container,
        unsigned int element
    );
    unsigned int getJointIndex(JsonValue joints, int searchingIndex);
};
} // namespace glvm

namespace glvm {
struct LightSpaceMatrixUBO {
    alignas(16) Matrix<float, 4> spotSpaceMatrix[SPOT_LIGHTS_NUMBER];
    alignas(16) uint32_t spotLightsNumber;

    alignas(16) Matrix<float, 4> dirSpaceMatrix[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) uint32_t directionalLightsNumber;
};

struct alignas(64) ModelMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> view;
    Matrix<float, 4> proj;
    Matrix<float, 4> jointMatrices[MAX_JOINTS_NUMBER];

    Vector<float, 3> ambient;
    float shininess;

    alignas(16) Matrix<float, 4> spotSpaceMatrix[SPOT_LIGHTS_NUMBER];
    alignas(16) uint32_t spotLightsNumber;

    alignas(16) Matrix<float, 4> dirSpaceMatrix[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) uint32_t directionalLightsNumber;
};

struct alignas(16) ShadowMapMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> lightSpaceMatrix;
    Matrix<float, 4> jointMatrices[MAX_JOINTS_NUMBER];
};

struct alignas(16) SpotLightShadowMapMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> lightSpaceMatrix;
};

struct alignas(64) PointLightShadowMapMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> lightSpaceMatrix;
    Vector<float, 3> lightPosition;
    float farPlane;
    Matrix<float, 4> jointMatrices[MAX_JOINTS_NUMBER];
};

struct alignas(16) UniformBufferObjectLightUBO {
    Vector<float, 3> lightPosition;
    float farPlane;
};

struct alignas(16) DirectionalLight {
    Vector<float, 4> position;
    Vector<float, 4> direction;

    Vector<float, 4> ambient;
    Vector<float, 4> diffuse;
    Vector<float, 4> specular;
};

struct alignas(16) PointLight {
    Vector<float, 3> position;
    float padding0;

    Vector<float, 3> ambient;
    float padding1;
    Vector<float, 3> diffuse;
    float padding2;

    Vector<float, 3> specular;
    float constant;
    float linear;
    float quadratic;
};

struct alignas(16) SpotLight {
    alignas(16) Vector<float, 3> position;
    alignas(16) Vector<float, 3> direction;
    float cutOff;
    float outerCutOff;

    alignas(16) Vector<float, 3> ambient;
    alignas(16) Vector<float, 3> diffuse;
    alignas(16) Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};

struct alignas(64) LightData {
    Vector<float, 2> tilesetTilesCount;
    int tilesRaw;
    int tilesColumn;

    alignas(16) Vector<float, 3> viewPosition;

    PointLight pointLights[POINT_LIGHTS_NUMBER];
    int pointLightsArraySize;
    float farPlane;
    int padding0;
    int padding1;

    DirectionalLight directionalLights[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) int directionalLightsArraySize;

    SpotLight spotLights[SPOT_LIGHTS_NUMBER];
    int spotLightArraySize;
    int padding2;
    int padding3;
    int padding4;

    Vector<int, 4>
        indirectTexture[INDIRECT_TEXTURE_WIDTH * INDIRECT_TEXTURE_HEIGHT / 4 + 1];

    // Debug: 0 = off, 1 = directional, 2 = spot. When set, the main shader
    // renders the shadow map depth projected onto the scene instead of
    // lighting (visualized from the normal moving camera).
    int debugShadowMode;
    int debugShadowLight;
    int shadowsEnabled;
};

struct alignas(64) HUD_UBO {
    Matrix<float, 4> view;
    Matrix<float, 4> proj;
    Vector<float, 3> entityPosition;
    int isHudExists;
    float maxHP;
    float currentHP;
    float highestY;
};

struct alignas(64) HUD_SCREEN_UBO {
    Matrix<float, 4> model;
};

struct alignas(64) FONT_UBO {
    Matrix<float, 4> view;
    Matrix<float, 4> proj;
    Vector<float, 3> position;
    float scale;
};

struct alignas(64) UI_UBO {
    Matrix<float, 4> model;
    Vector<float, 3> color;
};

struct alignas(64) VIRTUAL_TEXTURE_UBO {};

struct alignas(64) SDF_UBO {
    Matrix<float, 4> model;
    float iTime;
};

} // namespace glvm

#ifdef _WIN32

namespace glvm {

class WindowWinVulkan: public IWindow {
    HWND pClassic_Window_;
    HDC pClassic_DC_;
    HGLRC pClassic_Context_;

    WNDCLASS window_Class_;
    HDC pModern_DC_;
    HGLRC pModern_Context_;
    HWND pModern_Window_;

    // Cursor-lock baseline: the cursor's actual position after the last warp
    // (not the computed center).
    int previous_X = 0;
    int previous_Y = 0;

public:
    static WindowWinVulkan* instance;
    CStack* Input_Stack_;
    uint32_t width = GetSystemMetrics(SM_CXSCREEN);
    uint32_t height = GetSystemMetrics(SM_CYSCREEN);
    WindowWinVulkan();

    void SwapBuffers() override;
    void ClearDisplay() override;
    bool HandleEvent(CEvent& _Event) override;
    HWND GetClassicWindowHWND();
    HWND GetModernWindowHWND();
    void Close() override;
    virtual void CursorLock(
        int _x_position,
        int _y_position,
        int* _x_offset,
        int* _y_offset
    ) override;
    // Callback method for events handling.
    static LRESULT MainWndProc(
        HWND _pHwnd,
        UINT _pMsg,
        WPARAM _pWParam,
        LPARAM _pLParam
    );
};
} // namespace glvm
#endif // _WIN32

#ifdef __linux__

namespace glvm {
class CSoundEngineAlsa: public ISoundEngine {
    snd_pcm_t* pPcm;
    std::vector<CSoundSample*> tSound_Contaier;

public:
    void OpenDevice(const char* device) override;
    void CloseDevice() override;
    void SoundStream() override;
    void PlaybackSoundSample(CSoundSample& _sound_sample) override;
    void SetMasterVolume(long _lVolume) override;
    std::vector<CSoundSample*>& GetSoundContainer() override;
    void CreateSoundSample(
        const char* filePath,
        uint32_t duration,
        uint32_t rate,
        float volume
    ) override;

    ~CSoundEngineAlsa();
};
} // namespace glvm
#endif // __linux__

namespace glvm {
class CSoundEngineFactory {
public:
    ISoundEngine* CreateSoundEngine();
};

} // namespace glvm

#ifdef _WIN32
namespace glvm {
class CSoundEngineWaveform: public ISoundEngine {
    HANDLE hData = NULL;
    HPSTR lpData = NULL;

    std::vector<CSoundSample*> tSound_Container;

public:
    void OpenDevice(const char* device) override;
    void CloseDevice() override;
    void SoundStream() override;
    void PlaybackSoundSample(CSoundSample& _sound_sample) override;
    void SetMasterVolume(long _lVolume) override;
    void CreateSoundSample(
        const char* filePath,
        uint32_t duration,
        uint32_t rate,
        float volume
    ) override;
    std::vector<CSoundSample*>& GetSoundContainer() override;
};
} // namespace glvm
#endif // _WIN32

namespace glvm {
struct ProjectileBundle {
    projectile projectile;
    damage damage;
    material material;
};
}; // namespace glvm

namespace glvm {
class TextureManager {
    static TextureManager* pInstance_;
    static std::mutex Mutex_;

    std::vector<Texture> textureVector_;

public:
    TextureManager();

    void SetTextureVector(std::vector<Texture> _textureVector);
    // It possibly to get only one instance of this class with this method.
    static TextureManager* get_instance();
    static TextureManager* GetHUDInstance();
    void BindTexture(unsigned int _entityID, unsigned int _textureID);
    void LoadTextureData(glvm::Texture& _Texture);
    std::vector<Texture>& GetTextureVector();
    void UnbindTexture(material _textureComponent, unsigned int _entity);
};
} // namespace glvm

namespace glvm {
class ComponentManager {
    static ComponentManager* pInstance_;
    static std::mutex Mutex_;
    unsigned int numberOfBaseComponents;

    ComponentManager();

    template<typename componentType>
    unsigned int CreateComponentContainer() {
        static unsigned int localContainerID = 0;
        static bool existComponentContainerFlag = false;
        if (existComponentContainerFlag) {
            return localContainerID;
        }
        // Give a value of global component container ID's counter to local
        // container ID of current component type.
        localContainerID = componentsContainerID;
        existComponentContainerFlag = true;
        // Create component container of current type.
        worldComponentsContainer.push_back(
            std::make_shared<std::vector<componentType>>()
        );
        // Create ID's component container.
        std::vector<unsigned int>* sparseEntitiesMapToComponents =
            new std::vector<unsigned int>;
        worldSparseEntitiesMapToComponents.push_back(
            sparseEntitiesMapToComponents
        );
        // Create ID's component container.
        std::vector<unsigned int>* denseEntitiesMapToComponents =
            new std::vector<unsigned int>;
        worldDenseComponentsMapToEntities.push_back(
            denseEntitiesMapToComponents
        );
        componentsTypes.push_back(typeid(componentType).name());
        ++componentsContainerID;
        return localContainerID;
    }

public:
    inline static unsigned int componentsContainerID = 0;
    // Contains all local containers for different types of components.
    std::vector<std::shared_ptr<void>> worldComponentsContainer;
    // Contains all local container with IDs for different types of components.
    std::vector<std::vector<unsigned int>*> worldSparseEntitiesMapToComponents;
    std::vector<std::vector<unsigned int>*> worldDenseComponentsMapToEntities;

    std::vector<const char*> componentsTypes;
    bool isComponentsCollectionChanged = true;

    ~ComponentManager();
    // Don't need to make cope because of singleton property.
    ComponentManager(ComponentManager& componentManager) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const ComponentManager& componentManager) = delete;
    // It possibly to get only one instance of this class with this method.
    static ComponentManager* get_instance();

    template<typename componentType>
    void CreateComponent(const unsigned int& entity) {
        // Index for world components and world ID's containers.
        unsigned int localContainerID = 0;
        componentType Component;
        localContainerID = CreateComponentContainer<componentType>();

        std::vector<unsigned int>& sparse =
            *static_cast<std::vector<unsigned int>*>(
                worldSparseEntitiesMapToComponents[localContainerID]
            );
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                worldDenseComponentsMapToEntities[localContainerID]
            );
        std::vector<componentType>& components =
            *std::static_pointer_cast<std::vector<componentType>>(
                worldComponentsContainer[localContainerID]
            );
        if (checkAvailability(sparse, dense, entity)) {
            return;
        }

        if (entity >= sparse.size()) {
            sparse.resize(entity + 1);
        }

        assert(dense.size() == components.size());

        sparse[entity] = dense.size();
        dense.push_back(entity);
        components.push_back(Component);
        isComponentsCollectionChanged = true;
    }

    bool checkAvailability(
        std::vector<unsigned int>& sparse,
        std::vector<unsigned int>& dense,
        unsigned int entity
    );

    // Allow to give a various components to chosen entity.
    template<typename componentType1, typename componentType2, typename... Args>
    void CreateComponent(unsigned int& entity) {
        CreateComponent<componentType2, Args...>(entity);
        CreateComponent<componentType1>(entity);
    }

    template<typename componentType, typename... Args>
    std::vector<unsigned int> collectLinkedEntities() {
        numberOfBaseComponents = 0;
        unsigned int firstComponentArrayIndex =
            CreateComponentContainer<componentType>();
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                worldDenseComponentsMapToEntities[firstComponentArrayIndex]
            );

        if (dense.size() > 0) {
            ++numberOfBaseComponents;
            numberOfBaseComponents += sizeof...(Args);
        }
        std::vector<unsigned int> returnVector;
        for (unsigned int i = 0; i < dense.size(); ++i) {
            if (multiCheckAvailability<Args...>(dense[i])) {
                returnVector.push_back(dense[i]);
            }
        }
        return returnVector;
    }

    template<typename componentType, typename... Args>
    std::vector<unsigned int> collectUniqueLinkedEntities() {
        std::vector<unsigned int> baseSubSetEntities;
        baseSubSetEntities = collectLinkedEntities<componentType, Args...>();
        unsigned int numberOfComponentArrays = 0;
        for (unsigned int j = 0; j < baseSubSetEntities.size(); ++j) {
            numberOfComponentArrays = 0;
            for (unsigned int i = 0;
                 i < worldDenseComponentsMapToEntities.size();
                 ++i) {
                std::vector<unsigned int>& sparse =
                    *static_cast<std::vector<unsigned int>*>(
                        worldSparseEntitiesMapToComponents[i]
                    );
                std::vector<unsigned int>& dense =
                    *static_cast<std::vector<unsigned int>*>(
                        worldDenseComponentsMapToEntities[i]
                    );

                if (checkAvailability(sparse, dense, baseSubSetEntities[j])) {
                    ++numberOfComponentArrays;
                }
            }
            if (numberOfComponentArrays > numberOfBaseComponents) {
                baseSubSetEntities.erase(
                    baseSubSetEntities.begin() + baseSubSetEntities[j]
                );
                --j;
            }
        }
        return baseSubSetEntities;
    }

    template<typename... Args>
    bool multiCheckAvailability(unsigned int entity) {
        return (multiCheckAvailabilityBase<Args>(entity) && ...);
    }

    template<typename componentType>
    bool multiCheckAvailabilityBase(unsigned int entity) {
        unsigned int componentArrayIndex =
            CreateComponentContainer<componentType>();
        std::vector<unsigned int>& sparse =
            *static_cast<std::vector<unsigned int>*>(
                worldSparseEntitiesMapToComponents[componentArrayIndex]
            );
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                worldDenseComponentsMapToEntities[componentArrayIndex]
            );
        return checkAvailability(sparse, dense, entity);
    }

    template<typename componentType>
    bool isComponentExists(const unsigned int& entity) {
        unsigned int localContainerID;
        localContainerID = CreateComponentContainer<componentType>();
        std::vector<unsigned int>& sparse =
            *static_cast<std::vector<unsigned int>*>(
                worldSparseEntitiesMapToComponents[localContainerID]
            );
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                worldDenseComponentsMapToEntities[localContainerID]
            );
        return checkAvailability(sparse, dense, entity);
    }

    template<typename componentType>
    componentType* GetComponent(const unsigned int& entity) {
        unsigned int localContainerID;
        localContainerID = CreateComponentContainer<componentType>();
        std::vector<unsigned int>& sparse =
            *static_cast<std::vector<unsigned int>*>(
                worldSparseEntitiesMapToComponents[localContainerID]
            );
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                worldDenseComponentsMapToEntities[localContainerID]
            );
        std::vector<componentType>& components =
            *std::static_pointer_cast<std::vector<componentType>>(
                worldComponentsContainer[localContainerID]
            );
        if (checkAvailability(sparse, dense, entity)) {
            unsigned int componentIndex = sparse[entity];
            return &components[componentIndex];
        } else {
            return nullptr;
        }
    }

    // Don't need to delete real component in this method. Because systems don't
    // work with component without indices for that component in ordered
    // container.
    template<typename componentType>
    void RemoveComponent(unsigned int& entity) {
        unsigned int localContainerID;
        localContainerID = CreateComponentContainer<componentType>();
        std::vector<unsigned int>& sparse =
            *static_cast<std::vector<unsigned int>*>(
                worldSparseEntitiesMapToComponents[localContainerID]
            );
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                worldDenseComponentsMapToEntities[localContainerID]
            );
        std::vector<componentType>& components =
            *std::static_pointer_cast<std::vector<componentType>>(
                worldComponentsContainer[localContainerID]
            );
        if (checkAvailability(sparse, dense, entity)) {
            assert(dense.size() == components.size());
            unsigned int indexInDenseOfRemovableEntity = sparse[entity];
            unsigned int indexInSparseOfSwapableEntity = dense.back();
            const componentType& componentFromLastIndex = components.back();
            dense[indexInDenseOfRemovableEntity] =
                indexInSparseOfSwapableEntity;
            dense.pop_back();
            components[indexInDenseOfRemovableEntity] = componentFromLastIndex;
            components.pop_back();
            sparse[indexInSparseOfSwapableEntity] =
                indexInDenseOfRemovableEntity;
            isComponentsCollectionChanged = true;
        }
    }

    void remove_all_components(unsigned int& entity) {
        for (unsigned int i = 0; i < worldComponentsContainer.size(); ++i) {
            if (componentsTypes[i] == typeid(transform).name()) {
                RemoveComponent<transform>(entity);
            } else if (componentsTypes[i] == typeid(beholder).name()) {
                RemoveComponent<beholder>(entity);
            } else if (componentsTypes[i] == typeid(RigidBody).name()) {
                RemoveComponent<RigidBody>(entity);
            } else if (componentsTypes[i] == typeid(collider).name()) {
                RemoveComponent<collider>(entity);
            } else if (componentsTypes[i] == typeid(directionalLight).name()) {
                RemoveComponent<directionalLight>(entity);
            } else if (componentsTypes[i] == typeid(pointLight).name()) {
                RemoveComponent<pointLight>(entity);
            } else if (componentsTypes[i] == typeid(spotLight).name()) {
                RemoveComponent<spotLight>(entity);
            } else if (componentsTypes[i] == typeid(material).name()) {
                RemoveComponent<material>(entity);
            } else if (componentsTypes[i] == typeid(move).name()) {
                RemoveComponent<move>(entity);
            } else if (componentsTypes[i] == typeid(mesh).name()) {
                RemoveComponent<mesh>(entity);
            } else if (componentsTypes[i] == typeid(glvm::controller).name()) {
                RemoveComponent<glvm::controller>(entity);
            } else if (componentsTypes[i] == typeid(projectile).name()) {
                RemoveComponent<projectile>(entity);
            } else if (componentsTypes[i] == typeid(enemy).name()) {
                RemoveComponent<enemy>(entity);
            } else if (componentsTypes[i] == typeid(font).name()) {
                RemoveComponent<font>(entity);
            } else if (componentsTypes[i] == typeid(health).name()) {
                RemoveComponent<health>(entity);
            } else if (componentsTypes[i] == typeid(state).name()) {
                RemoveComponent<state>(entity);
            } else if (componentsTypes[i] == typeid(actor).name()) {
                RemoveComponent<actor>(entity);
            } else {
                continue;
            }
        }
    }

    unsigned int GetContainerID();

    template<typename componentType>
    std::vector<componentType>* GetComponentContainer() {
        return std::static_pointer_cast<std::vector<componentType>>(
                   worldComponentsContainer
                       [CreateComponentContainer<componentType>()]
        )
            .get();
    }

    template<typename componentType>
    std::vector<unsigned int>* GetEntityContainer() {
        return static_cast<std::vector<unsigned int>*>(
            worldDenseComponentsMapToEntities
                [CreateComponentContainer<componentType>()]
        );
    }
};

} // namespace glvm

namespace glvm {
struct MeshAxisMaxAbsoluteValues {
    float absolute_x = 0.0f;
    float absolute_y = 0.0f;
    float absolute_z = 0.0f;

    float origin_offset_x = 0.0f;
    float origin_offset_y = 0.0f;
    float origin_offset_z = 0.0f;
};

struct MeshAxisLimitingValues {
    float lowest_x = FLT_MAX;
    float highest_x = -FLT_MAX;
    float lowest_y = FLT_MAX;
    float highest_y = -FLT_MAX;
    float lowest_z = FLT_MAX;
    float highest_z = -FLT_MAX;

    void setToDefaultValues() {
        highest_x = -FLT_MAX;
        lowest_x = FLT_MAX;
        highest_y = -FLT_MAX;
        lowest_y = FLT_MAX;
        highest_z = -FLT_MAX;
        lowest_z = FLT_MAX;
    }

    void comparePerDirectionAndSetToMaximumValueByModule(SVertex& vertex) {
        if (vertex[0] < lowest_x) {
            lowest_x = vertex[0];
        } else if (vertex[0] > highest_x) {
            highest_x = vertex[0];
        }

        if (vertex[1] < lowest_y) {
            lowest_y = vertex[1];
        } else if (vertex[1] > highest_y) {
            highest_y = vertex[1];
        }

        if (vertex[2] < lowest_z) {
            lowest_z = vertex[2];
        } else if (vertex[2] > highest_z) {
            highest_z = vertex[2];
        }
    }

    void comparePerDirectionAndSetToMaximumValueByModule(
        Vector<float, 3> position,
        float half_x,
        float half_y,
        float half_z
    ) {
        if (position[0] + half_x > highest_x) {
            highest_x = position[0] + half_x;
        }
        if (position[0] - half_x < lowest_x) {
            lowest_x = position[0] - half_x;
        }
        if (position[1] + half_y > highest_y) {
            highest_y = position[1] + half_y;
        }
        if (position[1] - half_y < lowest_y) {
            lowest_y = position[1] - half_y;
        }
        if (position[2] + half_z > highest_z) {
            highest_z = position[2] + half_z;
        }
        if (position[2] - half_z < lowest_z) {
            lowest_z = position[2] - half_z;
        }
    }
};

enum DescriptorSetDataLink {
    // Pipelines related values.
    SHADOW_MAP_DIRECTIONAL_LIGHT,
    SHADOW_MAP_SPOT_LIGHT,
    SHADOW_MAP_POINT_LIGHT,
    HUD,
    FONT_RENDER_UBO,
    FONT_RENDER_SAMPLER,
    HUD_SCREEN,
    UI,
    UI_SAMPLERS,
    UI_ICONS,
    UI_ICONS_SAMPLERS,
    VIRTUAL_TEXTURES_UBO,
    VIRTUAL_TEXTURES_TILESET,
    MAIN_RENDER_MATRIX_UBO,
    MAIN_RENDER_LIGHT_DATA_UBO,
    MAIN_RENDER_SPECULAR_SAMPLER,
    MAIN_RENDER_DIFFUSE_SAMPLER,
    SDF_DATA,
    // Not related to any pipeline values.
    RIDABLE_TEXTURES,
    DESCRIPTOR_CHUNKS_NUMBER
};

enum SpecificPipeline {
    DIRECTIONAL_LIGHT_PIPELINE,
    SPOT_LIGHT_PIPELINE,
    POINT_LIGHT_PIPELINE,
    HUD_PIPELINE,
    FONT_PIPELINE,
    HUD_SCREEN_PIPELINE,
    UI_PIPELINE,
    UI_ICONS_PIPELINE,
    VIRTUAL_TEXTURES_PIPELINE,
    MAIN_RENDER_PIPELINE,
    SDF_PIPELINE,
    PIPELINES_NUMBER
};

struct RenderPass {
    unsigned int actualAttachmentDescriptionNumber;
    VkAttachmentDescription attachmentDescriptions[16];
    unsigned int actualAttachmentReferenceNumber;
    VkAttachmentReference attachmentReferences[16];
    unsigned int actualSubpassDependencyNumber;
    VkSubpassDependency subpassDependencies[8];
};

struct VK_Image {
    VkImage image;
    VkDeviceMemory deviceMemory = {};
    std::vector<VkImageView> views = {};
    VkImageViewType viewType = {};
    VkImageCreateFlags createFlags = {};
    VkMemoryPropertyFlags memoryPropertyFlags = {};
    VkImageUsageFlags usageFlags = {};
    VkImageAspectFlags aspectFlags = {};
    VkFormat format = {};
    VkImageTiling tiling = {};
    VkSampler sampler = {};
    VkComponentSwizzle red = {};
    VkComponentSwizzle green = {};
    VkComponentSwizzle blue = {};
    VkComponentSwizzle alpha = {};
    uint32_t arrayLayers = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

// Metadata for descriptor bindings.
struct DescriptorBinding {
    VkDescriptorType vkType;
    VkShaderStageFlags shaderStageFlag;
    unsigned int binding;
    unsigned int shaderDescriptorsNumber;
    unsigned int globalDescriptorOffset;
    VkDeviceSize uboChunkSize;
};

// Metadata for descriptor sets.
struct DescriptorSet {
    unsigned int actualLinkedDescriptorBindingsNumber;
    unsigned int hostDescriptorNumber;
    VkDescriptorSetLayout setLayout;
    static constexpr unsigned int maximumLinkedDescriptorBindingsDS = 32;
    unsigned int descriptorsBindingsIDs[maximumLinkedDescriptorBindingsDS];
    unsigned int descriptorSetOffset;
    bool isTexture;
};

struct Pipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    const char* vertShader = nullptr;
    const char* fragShader = nullptr;
    VkVertexInputBindingDescription bindingDescription;
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions;
    unsigned int actualLinkedDescriptorSetsNumber;
    static constexpr unsigned int maximumLinkedDescriptorSetDS = 32;
    unsigned int linkedDescriptorSetIDs[maximumLinkedDescriptorSetDS];
};

struct GPUBuffer {
    VkBuffer buffer;
    VkDeviceMemory deviceMemory;
};

union Descriptor {
    Descriptor() {};
    ~Descriptor() {};

    GPUBuffer* GPUBuffer;
    VK_Image* GPUImage;
};

struct Vertex {
    Vector<float, 3> pos;
    Vector<float, 3> color;
    Vector<float, 2> texCoord;
    Vector<float, 4> joinIndices;
    Vector<float, 4> weights;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription {};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 5>
    getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 5>
            attributeDescriptions {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, joinIndices);

        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(Vertex, weights);

        return attributeDescriptions;
    }
};
} // namespace glvm

// Render objects.
struct RenderPlayer {
    Vector<float, 3> position;
    Vector<float, 3> forward;
};

struct RenderActor {
    Matrix<float, 4> modelMatrix;
    std::vector<Matrix<float, 4>> jointMatrices;
    unsigned int meshID;
    unsigned int diffuseTextureIndex;
    unsigned int specularTextureIndex;
    Vector<float, 3> ambient;
    float shininess;
};

struct RenderDirectionalLight {
    Matrix<float, 4> DirectionalLightSpaceMatrix;
    Vector<float, 4> position;
    Vector<float, 4> direction;

    Vector<float, 4> ambient;
    Vector<float, 4> diffuse;
    Vector<float, 4> specular;
};

struct RenderSpotLight {
    Matrix<float, 4> SpotLigthSpaceMatrix;
    Vector<float, 3> position;
    Vector<float, 3> direction;
    float cutOff;
    float outerCutOff;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};

struct RenderPointLight {
    Matrix<float, 4> pointLightSpaceMatrix[CUBE_MAP_LAYER_NUMBER];
    Vector<float, 3> position;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};

struct RenderHealth {
    Vector<float, 3> position;
    float maxHealth;
    float currentHealth;
    unsigned int meshID;
};

struct RenderFont {
    Vector<float, 3> position;
    std::vector<char> font_string;
    float lifeTime;
};

struct SlotData {
    Matrix<float, 4> model;
    Vector<float, 3> color;
};

struct RenderInventory {
    std::vector<SlotData> slotData;
    unsigned int inventoryTextureID;
    unsigned int meshID;
    unsigned int row;
    unsigned int col;
};

struct RenderItem {
    Matrix<float, 4> model;
    unsigned int meshID;
    unsigned int diffuseTextureID;
};

struct RenderCrosshair {
    Matrix<float, 4> model;
    unsigned int meshID;
};

namespace glvm {
struct EntityManager {
private:
    static EntityManager* instance;
    static std::mutex mutex;

    inline static unsigned int id = 0;
    std::vector<unsigned int> removed_entity_registry;
    std::vector<unsigned int> active_entity_registry;

    EntityManager();

public: // TODO: Delete this.
    ~EntityManager();
    // Don't need to make cope because of singleton property.
    EntityManager(EntityManager& entity_manager) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const EntityManager& entity_manager) = delete;
    // It possibly to get only one instance of this class with this method.
    static EntityManager* get_instance();
    [[nodiscard]] unsigned int create_entity();
    void remove_entity(
        unsigned int& entity_id,
        ComponentManager* component_manager
    );
    bool is_entities_collection_changed = true;
};
} // namespace glvm

namespace glvm {
struct ISystem {
    virtual ~ISystem() {}

    virtual void Update() = 0;
};
} // namespace glvm

extern glvm::CEvent g_eEvent;

// Contains all maximum absolute axis values.
extern std::vector<glvm::MeshAxisMaxAbsoluteValues> allMeshMaxAbsoluteValues;

extern glvm::CStack Input_Stack_;

extern int x_pointer;
extern int y_pointer;
extern int keys_pressed[6];

namespace glvm {
extern std::vector<VkDescriptorSet> descriptorSetsChunks;
extern std::vector<VkRenderPass> renderPasses;
extern std::vector<Descriptor> GPUDescriptors;
} // namespace glvm

namespace glvm {
enum DeactivatedSystems { DEACTIVATED_MOVEMENT_SYSTEM };

class CSystemManager: public ISystem {
    static CSystemManager* pInstance_;
    static std::mutex Mutex_;
    std::vector<DeactivatedSystems> deactivatedSystems;

    CSystemManager();

public:
    ~CSystemManager();
    // Don't need to make cope because of singleton property.
    CSystemManager(CSystemManager& _system_Manager) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const CSystemManager& _system_Manager) = delete;
    // It possibly to get only one instance of this class whith this method.
    static CSystemManager* get_instance();

    inline static unsigned int s_iSystem_ID = 0;
    std::vector<ISystem*> tSystemContainer;

    void ActivateSystem(ISystem* _System);
    void DeactivateSystem(DeactivatedSystems system);
    void ReturnSystemToActivatedState(DeactivatedSystems system);

    void Update() override;
};
} // namespace glvm

namespace glvm {
class DamageSystem: public ISystem {
public:
    void Update() override;

    float deltaTime;

    uint32_t cachedAttackableArchetypesNumber = 0;
    uint32_t cachedFontArchetypesNumber = 0;

    struct ArchView {
        Archetype* cachedAttackableArchetypes[32];
        Archetype* cachedFontArchetypes[32];
    } archView;

    struct ComponentsView {
        attack* attackableAttacks = nullptr;
        health* attackableHealth = nullptr;
        font* attackableFonts = nullptr;

        font* fonts = nullptr;
    } componentsView;

    uint64_t attackableRequiredMask =
        (1ul << ComponentsIndices::AttackComponent)
        | (1ul << ComponentsIndices::HealthComponent)
        | (1ul << ComponentsIndices::FontComponent);

    uint64_t fontRequiredMask = (1ull << ComponentsIndices::FontComponent);
};
} // namespace glvm

namespace glvm {
class CPhysicsSystem: public ISystem {
public:
    float fAcceleration_of_Gravity_;
    float fDelta_Time_;
    float& gravity;
    CStack& Input_Stack_;

    uint32_t cachedArchetypesNumber = 0;

    struct ArchView {
        Archetype* cachedArchetypes[32];
    } archView;

    struct ComponentsView {
        transform* transformsView = nullptr;
        move* movesView = nullptr;
        RigidBody* rigidBodiesView = nullptr;
        colliderFlags* colliderFlagsView = nullptr;
        collider* collidersView = nullptr;
        mesh* meshesView = nullptr;
    } componentsView;

    uint64_t requiredMask = (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MoveComponent)
        | (1ul << ComponentsIndices::RigidBodyComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::MeshComponent);

    CPhysicsSystem(float& gravity_, CStack& _input_Stack) :
        gravity(gravity_),
        Input_Stack_(_input_Stack) {}

    // Set Y-axis of transform component of backtracking entity to upper Y-axis
    // of ground entity.
    void Gravity();

    // This update searching for referring to colliders entities and check their
    // transform components for collision, and if collision detected check if
    // backtracking entity had gravity component for call Gravity function.
    void Update() override;
    void Repel(
        transform& _transform_Component,
        float& _fDelta_Time,
        beholder& _view_Component,
        CEvent& _event
    );
};
} // namespace glvm

namespace glvm {
constexpr uint32_t CROSSHAIR_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(mesh) + sizeof(material)
       + sizeof(crossHairTagComponent));

struct CrosshairArchetype: Archetype {
    transform transforms[CROSSHAIR_ARCH_CHUNK_SIZE];
    mesh meshes[CROSSHAIR_ARCH_CHUNK_SIZE];
    material materials[CROSSHAIR_ARCH_CHUNK_SIZE];
    crossHairTagComponent crosshairTagComponents[CROSSHAIR_ARCH_CHUNK_SIZE];

    CrosshairArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::CrosshairTagComponent] =
            crosshairTagComponents;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::CrosshairTagComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::MeshComponent;
        componentIds[2] = ComponentsIndices::MaterialComponent;
        componentIds[3] = ComponentsIndices::CrosshairTagComponent;
        componentCount = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(mesh) + sizeof(material)
       + sizeof(directionalLight));

struct DirectionalLightArchetype: Archetype {
    transform transforms[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];
    mesh meshes[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];
    material materials[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];
    directionalLight directionalLights[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];

    DirectionalLightArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::DirectionalLightComponent] =
            directionalLights;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::DirectionalLightComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::MeshComponent;
        componentIds[2] = ComponentsIndices::MaterialComponent;
        componentIds[3] = ComponentsIndices::DirectionalLightComponent;
        componentCount = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t ENEMY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(enemy) + sizeof(state) + sizeof(font)
       + sizeof(animation) + sizeof(material) + sizeof(mesh) + sizeof(collider)
       + sizeof(colliderFlags) + sizeof(health) + sizeof(RigidBody)
       + sizeof(attack) + sizeof(rotation) + sizeof(move));

struct EnemyArchetype: Archetype {
    transform transforms[ENEMY_ARCH_CHUNK_SIZE];
    enemy enemies[ENEMY_ARCH_CHUNK_SIZE];
    state states[ENEMY_ARCH_CHUNK_SIZE];
    font fonts[ENEMY_ARCH_CHUNK_SIZE];
    animation animations[ENEMY_ARCH_CHUNK_SIZE];
    material materials[ENEMY_ARCH_CHUNK_SIZE];
    mesh meshes[ENEMY_ARCH_CHUNK_SIZE];
    collider colliders[ENEMY_ARCH_CHUNK_SIZE];
    colliderFlags colliderFlags[ENEMY_ARCH_CHUNK_SIZE];
    health health[ENEMY_ARCH_CHUNK_SIZE];
    RigidBody rigidBodies[ENEMY_ARCH_CHUNK_SIZE];
    attack attacks[ENEMY_ARCH_CHUNK_SIZE];
    rotation rotations[ENEMY_ARCH_CHUNK_SIZE];
    move moves[ENEMY_ARCH_CHUNK_SIZE];

    EnemyArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::EnemyComponent] = enemies;
        components[ComponentsIndices::StateComponent] = states;
        components[ComponentsIndices::FontComponent] = fonts;
        components[ComponentsIndices::AnimationComponent] = animations;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = colliderFlags;
        components[ComponentsIndices::HealthComponent] = health;
        components[ComponentsIndices::RigidBodyComponent] = rigidBodies;
        components[ComponentsIndices::AttackComponent] = attacks;
        components[ComponentsIndices::RotationComponent] = rotations;
        components[ComponentsIndices::MoveComponent] = moves;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::EnemyComponent)
            | (1ull << ComponentsIndices::StateComponent)
            | (1ull << ComponentsIndices::FontComponent)
            | (1ull << ComponentsIndices::AnimationComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::HealthComponent)
            | (1ull << ComponentsIndices::RigidBodyComponent)
            | (1ull << ComponentsIndices::AttackComponent)
            | (1ull << ComponentsIndices::RotationComponent)
            | (1ull << ComponentsIndices::MoveComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::EnemyComponent;
        componentIds[2] = ComponentsIndices::StateComponent;
        componentIds[3] = ComponentsIndices::FontComponent;
        componentIds[4] = ComponentsIndices::AnimationComponent;
        componentIds[5] = ComponentsIndices::MaterialComponent;
        componentIds[6] = ComponentsIndices::MeshComponent;
        componentIds[7] = ComponentsIndices::ColliderComponent;
        componentIds[8] = ComponentsIndices::ColliderFlagsComponent;
        componentIds[9] = ComponentsIndices::HealthComponent;
        componentIds[10] = ComponentsIndices::RigidBodyComponent;
        componentIds[11] = ComponentsIndices::AttackComponent;
        componentIds[12] = ComponentsIndices::RotationComponent;
        componentIds[13] = ComponentsIndices::MoveComponent;
        componentCount = 14;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t INVENTORY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(mesh) + sizeof(inventory) + sizeof(material));

struct InventoryArchetype: Archetype {
    transform transforms[INVENTORY_ARCH_CHUNK_SIZE];
    mesh meshes[INVENTORY_ARCH_CHUNK_SIZE];
    inventory invetories[INVENTORY_ARCH_CHUNK_SIZE];
    material materials[INVENTORY_ARCH_CHUNK_SIZE];

    InventoryArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::InventoryComponent] = invetories;
        components[ComponentsIndices::MaterialComponent] = materials;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::InventoryComponent)
            | (1ull << ComponentsIndices::MaterialComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::MeshComponent;
        componentIds[2] = ComponentsIndices::InventoryComponent;
        componentIds[3] = ComponentsIndices::MaterialComponent;
        componentCount = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t ITEM_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(collider) + sizeof(colliderFlags)
       + sizeof(mesh) + sizeof(RigidBody) + sizeof(material) + sizeof(rotation)
       + sizeof(move) + sizeof(item));

struct ItemArchetype: Archetype {
    transform transforms[ITEM_ARCH_CHUNK_SIZE];
    collider colliders[ITEM_ARCH_CHUNK_SIZE];
    colliderFlags colliderFlags[ITEM_ARCH_CHUNK_SIZE];
    mesh meshes[ITEM_ARCH_CHUNK_SIZE];
    RigidBody rigidBodies[ITEM_ARCH_CHUNK_SIZE];
    material materials[ITEM_ARCH_CHUNK_SIZE];
    rotation rotations[ITEM_ARCH_CHUNK_SIZE];
    move moves[ITEM_ARCH_CHUNK_SIZE];
    item items[ITEM_ARCH_CHUNK_SIZE];

    ItemArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = colliderFlags;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::RigidBodyComponent] = rigidBodies;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::RotationComponent] = rotations;
        components[ComponentsIndices::MoveComponent] = moves;
        components[ComponentsIndices::ItemComponent] = items;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::RigidBodyComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::RotationComponent)
            | (1ull << ComponentsIndices::MoveComponent)
            | (1ull << ComponentsIndices::ItemComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::ColliderComponent;
        componentIds[2] = ComponentsIndices::ColliderFlagsComponent;
        componentIds[3] = ComponentsIndices::MeshComponent;
        componentIds[4] = ComponentsIndices::RigidBodyComponent;
        componentIds[5] = ComponentsIndices::MaterialComponent;
        componentIds[6] = ComponentsIndices::RotationComponent;
        componentIds[7] = ComponentsIndices::MoveComponent;
        componentIds[8] = ComponentsIndices::ItemComponent;
        componentCount = 9;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t LEVEL_CHUNK_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(material) + sizeof(mesh) + sizeof(collider)
       + sizeof(colliderFlags) + sizeof(rotation)
       + sizeof(levelChunkTagComponent));

struct LevelChunkArchetype: Archetype {
    transform transforms[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    material materials[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    mesh meshes[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    collider colliders[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    colliderFlags colliderFlags[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    rotation rotations[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    levelChunkTagComponent levelChunkTagComponents[LEVEL_CHUNK_ARCH_CHUNK_SIZE];

    LevelChunkArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = colliderFlags;
        components[ComponentsIndices::RotationComponent] = rotations;
        components[ComponentsIndices::LevelChunkTagComponent] =
            levelChunkTagComponents;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::RotationComponent)
            | (1ull << ComponentsIndices::LevelChunkTagComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::MaterialComponent;
        componentIds[2] = ComponentsIndices::MeshComponent;
        componentIds[3] = ComponentsIndices::ColliderComponent;
        componentIds[4] = ComponentsIndices::ColliderFlagsComponent;
        componentIds[5] = ComponentsIndices::RotationComponent;
        componentIds[6] = ComponentsIndices::LevelChunkTagComponent;
        componentCount = 7;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t PLAYER_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(beholder) + sizeof(collider)
       + sizeof(colliderFlags) + sizeof(mesh) + sizeof(RigidBody)
       + sizeof(health) + sizeof(material) + sizeof(move) + sizeof(attack)
       + sizeof(animation) + sizeof(font) + sizeof(rotation)
       + sizeof(playerTagComponent));

struct PlayerArchetype: Archetype {
    transform transforms[PLAYER_ARCH_CHUNK_SIZE];
    beholder beholders[PLAYER_ARCH_CHUNK_SIZE];
    collider colliders[PLAYER_ARCH_CHUNK_SIZE];
    colliderFlags colliderFlags[PLAYER_ARCH_CHUNK_SIZE];
    mesh meshes[PLAYER_ARCH_CHUNK_SIZE];
    RigidBody rigidBodies[PLAYER_ARCH_CHUNK_SIZE];
    health health[PLAYER_ARCH_CHUNK_SIZE];
    material materials[PLAYER_ARCH_CHUNK_SIZE];
    move moves[PLAYER_ARCH_CHUNK_SIZE];
    attack attacks[PLAYER_ARCH_CHUNK_SIZE];
    animation animations[PLAYER_ARCH_CHUNK_SIZE];
    font fonts[PLAYER_ARCH_CHUNK_SIZE];
    rotation rotations[PLAYER_ARCH_CHUNK_SIZE];
    playerTagComponent playerTagComponents[PLAYER_ARCH_CHUNK_SIZE];

    PlayerArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::ViewComponent] = beholders;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = colliderFlags;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::RigidBodyComponent] = rigidBodies;
        components[ComponentsIndices::HealthComponent] = health;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::MoveComponent] = moves;
        components[ComponentsIndices::AttackComponent] = attacks;
        components[ComponentsIndices::AnimationComponent] = animations;
        components[ComponentsIndices::FontComponent] = fonts;
        components[ComponentsIndices::RotationComponent] = rotations;
        components[ComponentsIndices::PlayerTagComponent] = playerTagComponents;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::ViewComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::RigidBodyComponent)
            | (1ull << ComponentsIndices::HealthComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::MoveComponent)
            | (1ull << ComponentsIndices::AttackComponent)
            | (1ull << ComponentsIndices::AnimationComponent)
            | (1ull << ComponentsIndices::FontComponent)
            | (1ull << ComponentsIndices::RotationComponent)
            | (1ull << ComponentsIndices::PlayerTagComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::ViewComponent;
        componentIds[2] = ComponentsIndices::ColliderComponent;
        componentIds[3] = ComponentsIndices::ColliderFlagsComponent;
        componentIds[4] = ComponentsIndices::MeshComponent;
        componentIds[5] = ComponentsIndices::RigidBodyComponent;
        componentIds[6] = ComponentsIndices::HealthComponent;
        componentIds[7] = ComponentsIndices::MaterialComponent;
        componentIds[8] = ComponentsIndices::MoveComponent;
        componentIds[9] = ComponentsIndices::AttackComponent;
        componentIds[10] = ComponentsIndices::AnimationComponent;
        componentIds[11] = ComponentsIndices::FontComponent;
        componentIds[12] = ComponentsIndices::RotationComponent;
        componentIds[13] = ComponentsIndices::PlayerTagComponent;
        componentCount = 14;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t POINT_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(mesh) + sizeof(material)
       + sizeof(pointLight));

struct PointLightArchetype: Archetype {
    transform transforms[POINT_LIGHT_ARCH_CHUNK_SIZE];
    mesh meshes[POINT_LIGHT_ARCH_CHUNK_SIZE];
    material materials[POINT_LIGHT_ARCH_CHUNK_SIZE];
    pointLight pointLights[POINT_LIGHT_ARCH_CHUNK_SIZE];

    PointLightArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::PointLightComponent] = pointLights;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::PointLightComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::MeshComponent;
        componentIds[2] = ComponentsIndices::MaterialComponent;
        componentIds[3] = ComponentsIndices::PointLightComponent;
        componentCount = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t PROJECTILE_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(mesh) + sizeof(collider)
       + sizeof(colliderFlags) + sizeof(rotation) + sizeof(ProjectileBundle)
       + sizeof(health) + sizeof(attack) + sizeof(font)
       + sizeof(projectileTagComponent));

struct ProjectileArchetype: Archetype {
    transform transforms[PROJECTILE_ARCH_CHUNK_SIZE];
    mesh meshes[PROJECTILE_ARCH_CHUNK_SIZE];
    collider colliders[PROJECTILE_ARCH_CHUNK_SIZE];
    colliderFlags colliderFlags[PROJECTILE_ARCH_CHUNK_SIZE];
    rotation rotations[PROJECTILE_ARCH_CHUNK_SIZE];
    ProjectileBundle projectileBundles[PROJECTILE_ARCH_CHUNK_SIZE];
    health heath[PROJECTILE_ARCH_CHUNK_SIZE];
    attack attacks[PROJECTILE_ARCH_CHUNK_SIZE];
    font fonts[PROJECTILE_ARCH_CHUNK_SIZE];
    projectileTagComponent projectileTagComponents[PROJECTILE_ARCH_CHUNK_SIZE];

    ProjectileArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = colliderFlags;
        components[ComponentsIndices::RotationComponent] = rotations;
        components[ComponentsIndices::ProjectileBundleComponent] =
            projectileBundles;
        components[ComponentsIndices::HealthComponent] = heath;
        components[ComponentsIndices::AttackComponent] = attacks;
        components[ComponentsIndices::FontComponent] = fonts;
        components[ComponentsIndices::ProjectileTagComponent] =
            projectileTagComponents;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::RotationComponent)
            | (1ull << ComponentsIndices::ProjectileBundleComponent)
            | (1ull << ComponentsIndices::HealthComponent)
            | (1ull << ComponentsIndices::AttackComponent)
            | (1ull << ComponentsIndices::FontComponent)
            | (1ull << ComponentsIndices::ProjectileTagComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::MeshComponent;
        componentIds[2] = ComponentsIndices::ColliderComponent;
        componentIds[3] = ComponentsIndices::ColliderFlagsComponent;
        componentIds[4] = ComponentsIndices::RotationComponent;
        componentIds[5] = ComponentsIndices::ProjectileBundleComponent;
        componentIds[6] = ComponentsIndices::HealthComponent;
        componentIds[7] = ComponentsIndices::AttackComponent;
        componentIds[8] = ComponentsIndices::FontComponent;
        componentIds[9] = ComponentsIndices::ProjectileTagComponent;
        componentCount = 10;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t RIGID_BODY_ARCH_CHUNK_SIZE =
    ARCHETYPE_CHUNK_SIZE / (sizeof(glvm::transform) + sizeof(glvm::RigidBody));

struct RigidBodyArch {
    transform transforms[RIGID_BODY_ARCH_CHUNK_SIZE];
    RigidBody rigidBodies[RIGID_BODY_ARCH_CHUNK_SIZE];
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t SPOT_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(mesh) + sizeof(material) + sizeof(spotLight));

struct SpotLightArchetype: Archetype {
    transform transforms[SPOT_LIGHT_ARCH_CHUNK_SIZE];
    mesh meshes[SPOT_LIGHT_ARCH_CHUNK_SIZE];
    material materials[SPOT_LIGHT_ARCH_CHUNK_SIZE];
    spotLight spotLights[SPOT_LIGHT_ARCH_CHUNK_SIZE];

    SpotLightArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::SpotLightComponent] = spotLights;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::SpotLightComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::MeshComponent;
        componentIds[2] = ComponentsIndices::MaterialComponent;
        componentIds[3] = ComponentsIndices::SpotLightComponent;
        componentCount = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t STATIC_MESH_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(collider) + sizeof(colliderFlags)
       + sizeof(mesh) + sizeof(material) + sizeof(font) + sizeof(rotation)
       + sizeof(staticMeshTagComponent));

struct StaticMeshArchetype: Archetype {
    transform transforms[STATIC_MESH_ARCH_CHUNK_SIZE];
    collider colliders[STATIC_MESH_ARCH_CHUNK_SIZE];
    colliderFlags colliderFlags[STATIC_MESH_ARCH_CHUNK_SIZE];
    mesh meshes[STATIC_MESH_ARCH_CHUNK_SIZE];
    material materials[STATIC_MESH_ARCH_CHUNK_SIZE];
    font fonts[STATIC_MESH_ARCH_CHUNK_SIZE];
    rotation rotations[STATIC_MESH_ARCH_CHUNK_SIZE];
    staticMeshTagComponent staticMeshTagComponents[STATIC_MESH_ARCH_CHUNK_SIZE];

    StaticMeshArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = colliderFlags;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::FontComponent] = fonts;
        components[ComponentsIndices::RotationComponent] = rotations;
        components[ComponentsIndices::StaticMeshTagComponent] =
            staticMeshTagComponents;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::FontComponent)
            | (1ull << ComponentsIndices::RotationComponent)
            | (1ull << ComponentsIndices::StaticMeshTagComponent);

        componentIds[0] = ComponentsIndices::TransformComponent;
        componentIds[1] = ComponentsIndices::ColliderComponent;
        componentIds[2] = ComponentsIndices::ColliderFlagsComponent;
        componentIds[3] = ComponentsIndices::MeshComponent;
        componentIds[4] = ComponentsIndices::MaterialComponent;
        componentIds[5] = ComponentsIndices::FontComponent;
        componentIds[6] = ComponentsIndices::RotationComponent;
        componentIds[7] = ComponentsIndices::StaticMeshTagComponent;
        componentCount = 8;
    }
};
}; // namespace glvm

#ifdef __linux__

namespace glvm {
struct WindowWaylandVulkan: IWindow {
    WindowWaylandVulkan();
    void init();
    void Close() override;
    bool HandleEvent(CEvent& _Event) override;
    static int create_anonymous_file(off_t size);
    struct wl_buffer* create_transparent_cursor(struct wl_shm* shm);
    void SwapBuffers() override;
    void ClearDisplay() override;
    void CursorLock(
        int _x_position,
        int _y_position,
        int* _x_offset,
        int* _y_offset
    ) override;
    bool hideAndLockPointer = false;
    struct xdg_toplevel_listener xdg_toplevel_listener;
    struct xdg_surface_listener xdg_surface_listener;
    struct wl_callback_listener callback_listener;
    struct xdg_wm_base_listener shell_listener;
    struct wl_keyboard_listener keyboard_listener;
    struct zwp_relative_pointer_v1_listener relative_pointer_listener;
    struct wl_pointer_listener pointer_listener;
    struct wl_seat_listener seat_lintener;
    struct wl_registry_listener registry_listener;
    struct wl_output_listener output_listener;

    struct wl_surface* wl_surface;
    struct wl_compositor* compositor;
    struct xdg_toplevel* xdg_topLevel;
    struct xdg_wm_base* xdg_shell;
    struct wl_buffer* buffer;
    struct wl_shm* shared_memory;
    struct wl_seat* seat;
    struct wl_keyboard* keyboard;
    struct wl_pointer* pointer;
    struct wl_shm* pointer_shared_memory;
    struct wl_surface* pointer_surface;
    struct zwp_pointer_constraints_v1* pointer_constraints;
    struct zwp_relative_pointer_manager_v1* relative_pointer_manager;
    struct zwp_relative_pointer_v1* relative_pointer;
    void* pixels;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t constant_byte = 0;
    uint8_t close_xdg_toplevel;
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_callback* frame_callback;
    struct xdg_surface* xdg_surface;
};

void xdg_toplevel_configure(
    void* data,
    struct xdg_toplevel* xdg_toplevel,
    int32_t new_width,
    int32_t new_height,
    struct wl_array* atate
);
void xdg_toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel);
int32_t alocate_shared_memory(uint64_t size);
void resize(void* data);
void draw(void* data);
void xdg_surface_configure(
    void* data,
    struct xdg_surface* xdg_surface,
    uint32_t serial
);
void new_frame(
    void* data,
    struct wl_callback* frame_call_back,
    uint32_t callback_data
);
void shell_ping(void* data, struct xdg_wm_base* shell, uint32_t serial);
void keyboard_keymap(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t format,
    int32_t keymap_file_descriptor,
    uint32_t size
);
void keyboard_enter(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    struct wl_surface* surface,
    struct wl_array* keys
);
void keyboard_leave(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    struct wl_surface* surface
);
void keyboard_key(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    uint32_t time,
    uint32_t key,
    uint32_t state
);
void keyboard_modifiers(
    void* data,
    struct wl_keyboard* keyboard,
    uint32_t serial,
    uint32_t mods_depressed,
    uint32_t mods_latched,
    uint32_t mods_locked,
    uint32_t group
);
void keyboard_repeat_info(
    void* data,
    struct wl_keyboard* keyboard,
    int32_t rate,
    int32_t delay
);
void pointer_enter(
    void* data,
    struct wl_pointer* pointer,
    uint32_t serial,
    struct wl_surface* surface,
    wl_fixed_t sx,
    wl_fixed_t sy
);
void pointer_leave(
    void* data,
    struct wl_pointer* pointer,
    uint32_t serial,
    struct wl_surface* surface
);
void pointer_motion(
    void* data,
    struct wl_pointer* pointer,
    uint32_t time,
    wl_fixed_t sx,
    wl_fixed_t sy
);
void pointer_axis(
    void* data,
    struct wl_pointer* pointer,
    uint32_t time,
    uint32_t axis,
    wl_fixed_t value
);
void pointer_button(
    void* data,
    struct wl_pointer* pointer,
    uint32_t serial,
    uint32_t time,
    uint32_t button,
    uint32_t state
);
void handle_relative_motion(
    void* data,
    struct zwp_relative_pointer_v1* rel_pointer,
    uint32_t utime_hi,
    uint32_t utime_lo,
    wl_fixed_t dx,
    wl_fixed_t dy,
    wl_fixed_t dx_unaccel,
    wl_fixed_t dy_unaccel
);
void seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities);
void seat_name(void* data, struct wl_seat* seat, const char* name);
void registry_global(
    void* data,
    struct wl_registry* registry,
    uint32_t name,
    const char* interface,
    uint32_t version
);
void registry_global_remove(
    void* data,
    struct wl_registry* registry,
    uint32_t name
);

[[nodiscard]] WindowWaylandVulkan* initializeWaylandWindow();
}; // namespace glvm
#endif // __linux__

#ifdef __linux__

namespace glvm {
class WindowXVulkan: public IWindow {
    XWindowAttributes GWindow_Attributes_;
    Window Root_Window_;
    XSetWindowAttributes Set_Window_Attributes_;

public:
    Display* pDisp_;
    Window Win_;
    uint32_t width;
    uint32_t height;

    WindowXVulkan();
    ~WindowXVulkan();

    Window GetWindow();
    Display* GetDisplay();
    void CursorLock(
        int _x_position,
        int _y_position,
        int* _x_offset,
        int* _y_offset
    ) override;
    void SwapBuffers() override;
    void ClearDisplay() override;
    bool HandleEvent(CEvent& _Event) override;
    void Close() override;
};
} // namespace glvm
#endif // __linux__

#ifdef __linux__

namespace glvm {
class WindowXCBVulkan: public IWindow {
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    uint32_t window;
    xcb_key_symbols_t* key_symbols;
    xcb_generic_event_t* next_generic_event = NULL;

    static void print_modifiers(uint32_t mask);

public:
    uint32_t width;
    uint32_t height;
    bool isWindowResizeRead = false;

    WindowXCBVulkan();

    void configureWindow();
    void HideCursor();
    xcb_connection_t* GetConnection();
    uint32_t GetWindow();
    void Disconnect();

    void SwapBuffers() override;
    void ClearDisplay() override;
    bool HandleEvent(CEvent& _Event) override;
    void Close() override;
    void CursorLock(
        int _x_position,
        int _y_position,
        int* _x_offset,
        int* _y_offset
    ) override;
};
} // namespace glvm
#endif // __linux__

namespace glvm {
class CVulkanRenderer;

struct DebugVertex {
    float x, y, z;
    float r, g, b;
};

class ImGuiOverlay {
public:
    explicit ImGuiOverlay(CVulkanRenderer& renderer);

    void init();
    void shutdown();
    void createSwapChainResources();
    void destroySwapChainResources();

    void newFrame();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    bool wantsMouse() const;

    bool isEnabled() const {
        return initialized_;
    }

    bool showPanel = true;
    bool showActorBounds = false;
    bool showLightFrustums = false;
    bool showShadowMaps = false;
    bool showSpatialGrid = false;
    bool shadowsEnabled = true;
    int shadowMapMode = 0; // 0 = directional, 1 = spot.
    int shadowMapLight = 0;

private:
    CVulkanRenderer& renderer_;
    bool initialized_ = false;

    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers_;

    VkPipelineLayout lineLayout_ = VK_NULL_HANDLE;
    VkPipeline linePipeline_ = VK_NULL_HANDLE;

    VkBuffer vertexBuffer_ = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory_ = VK_NULL_HANDLE;
    void* vertexBufferMapped_ = nullptr;
    uint32_t lineVertexCount_ = 0;

    void createRenderPass();
    void createLinePipeline();
    void createVertexBuffer();
    void buildPanel();
    void buildDebugVertices();
    void recordDebugDraws(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void recordImGuiDraws(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};

} // namespace glvm

namespace glvm {
inline DescriptorSet descriptorSetsConfig[32];
inline DescriptorBinding descriptorBindingsConfig[32];
inline Pipeline pipelineConfigs[32];
inline RenderPass renderPassConfigs[32];
constexpr uint32_t MAX_TEXTURES = 18;

inline void VkConfigInitializer() {
    // Pipelines and its render passes. Put all meta data related to pipeline
    // here. Also needed to add meta data of descriptor sets and it's bindings
    // that will be related to specific pipeline

    descriptorSetsConfig[SHADOW_MAP_DIRECTIONAL_LIGHT]
        .actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[SHADOW_MAP_DIRECTIONAL_LIGHT].hostDescriptorNumber =
        128;
    descriptorSetsConfig[SHADOW_MAP_DIRECTIONAL_LIGHT].isTexture = false;

    descriptorBindingsConfig[0].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[0].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[0].binding = 0;
    descriptorBindingsConfig[0].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[0].uboChunkSize = sizeof(ShadowMapMatrixUBO);

    pipelineConfigs[DIRECTIONAL_LIGHT_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/flat_shadow_map/vertFlatShadowMap.spv";
    pipelineConfigs[DIRECTIONAL_LIGHT_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[DIRECTIONAL_LIGHT_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .actualLinkedDescriptorSetsNumber = 1;

    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .actualAttachmentDescriptionNumber = 1;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .flags = 0;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .samples = VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .actualAttachmentReferenceNumber = 1;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .attachmentReferences[0]
        .attachment = 0;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE].actualSubpassDependencyNumber =
        2;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[0]
        .srcSubpass = VK_SUBPASS_EXTERNAL;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[0]
        .dstSubpass = 0;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[0]
        .srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[0]
        .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[0]
        .srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[0]
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[0]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[1]
        .srcSubpass = 0;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[1]
        .dstSubpass = VK_SUBPASS_EXTERNAL;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[1]
        .srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[1]
        .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[1]
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[1]
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    renderPassConfigs[DIRECTIONAL_LIGHT_PIPELINE]
        .subpassDependencies[1]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    descriptorSetsConfig[SHADOW_MAP_SPOT_LIGHT]
        .actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[SHADOW_MAP_SPOT_LIGHT].hostDescriptorNumber = 256;
    descriptorSetsConfig[SHADOW_MAP_SPOT_LIGHT].isTexture = false;

    descriptorBindingsConfig[1].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[1].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[1].binding = 0;
    descriptorBindingsConfig[1].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[1].uboChunkSize = sizeof(ShadowMapMatrixUBO);

    pipelineConfigs[SPOT_LIGHT_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/flat_shadow_map/vertFlatShadowMap.spv";
    pipelineConfigs[SPOT_LIGHT_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[SPOT_LIGHT_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[SPOT_LIGHT_PIPELINE].actualLinkedDescriptorSetsNumber = 1;

    renderPassConfigs[SPOT_LIGHT_PIPELINE].actualAttachmentDescriptionNumber =
        1;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].attachmentDescriptions[0].flags = 0;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].attachmentDescriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].attachmentDescriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].attachmentDescriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[SPOT_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[SPOT_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[SPOT_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].attachmentDescriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    renderPassConfigs[SPOT_LIGHT_PIPELINE].actualAttachmentReferenceNumber = 1;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].attachmentReferences[0].attachment =
        0;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[SPOT_LIGHT_PIPELINE].actualSubpassDependencyNumber = 2;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[0].srcSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[0].dstSubpass =
        0;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[0].srcAccessMask =
        VK_ACCESS_SHADER_READ_BIT;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[0].dstAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[SPOT_LIGHT_PIPELINE]
        .subpassDependencies[0]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[1].srcSubpass =
        0;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[1].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[1].dstStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[1].srcAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[SPOT_LIGHT_PIPELINE].subpassDependencies[1].dstAccessMask =
        VK_ACCESS_SHADER_READ_BIT;
    renderPassConfigs[SPOT_LIGHT_PIPELINE]
        .subpassDependencies[1]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    descriptorSetsConfig[SHADOW_MAP_POINT_LIGHT]
        .actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[SHADOW_MAP_POINT_LIGHT].hostDescriptorNumber = 512;
    descriptorSetsConfig[SHADOW_MAP_POINT_LIGHT].isTexture = false;

    descriptorBindingsConfig[2].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[2].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[2].binding = 0;
    descriptorBindingsConfig[2].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[2].uboChunkSize =
        sizeof(PointLightShadowMapMatrixUBO);

    pipelineConfigs[POINT_LIGHT_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/cube_shadow_map/vertCubeShadowMap.spv";
    pipelineConfigs[POINT_LIGHT_PIPELINE].fragShader =
        "../../../crates/glvm2/assets/shaders/cube_shadow_map/fragCubeShadowMap.spv";
    pipelineConfigs[POINT_LIGHT_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[POINT_LIGHT_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[POINT_LIGHT_PIPELINE].actualLinkedDescriptorSetsNumber = 1;

    renderPassConfigs[POINT_LIGHT_PIPELINE].actualAttachmentDescriptionNumber =
        1;
    renderPassConfigs[POINT_LIGHT_PIPELINE].attachmentDescriptions[0].flags = 0;
    renderPassConfigs[POINT_LIGHT_PIPELINE].attachmentDescriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[POINT_LIGHT_PIPELINE].attachmentDescriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[POINT_LIGHT_PIPELINE].attachmentDescriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[POINT_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[POINT_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[POINT_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[POINT_LIGHT_PIPELINE]
        .attachmentDescriptions[0]
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    renderPassConfigs[POINT_LIGHT_PIPELINE].actualAttachmentReferenceNumber = 1;
    renderPassConfigs[POINT_LIGHT_PIPELINE].attachmentReferences[0].attachment =
        0;
    renderPassConfigs[POINT_LIGHT_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[POINT_LIGHT_PIPELINE].actualSubpassDependencyNumber = 2;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[0].srcSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[0].dstSubpass =
        0;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[0].srcAccessMask =
        VK_ACCESS_SHADER_READ_BIT;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[0].dstAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[POINT_LIGHT_PIPELINE]
        .subpassDependencies[0]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[1].srcSubpass =
        0;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[1].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[1].dstStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[1].srcAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[POINT_LIGHT_PIPELINE].subpassDependencies[1].dstAccessMask =
        VK_ACCESS_SHADER_READ_BIT;
    renderPassConfigs[POINT_LIGHT_PIPELINE]
        .subpassDependencies[1]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    descriptorSetsConfig[HUD].actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[HUD].hostDescriptorNumber = 1024;
    descriptorSetsConfig[HUD].isTexture = false;

    descriptorBindingsConfig[3].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[3].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[3].binding = 0;
    descriptorBindingsConfig[3].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[3].uboChunkSize = sizeof(HUD_UBO);

    pipelineConfigs[HUD_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/hud/hud_vert.spv";
    pipelineConfigs[HUD_PIPELINE].fragShader =
        "../../../crates/glvm2/assets/shaders/hud/hud_frag.spv";
    pipelineConfigs[HUD_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[HUD_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[HUD_PIPELINE].actualLinkedDescriptorSetsNumber = 1;

    renderPassConfigs[HUD_PIPELINE].actualAttachmentDescriptionNumber = 2;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[0].flags = 0;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[0].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[0].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[0].initialLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[1].flags = 0;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[1].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[1].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[1].initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[HUD_PIPELINE].attachmentDescriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[HUD_PIPELINE].actualAttachmentReferenceNumber = 2;
    renderPassConfigs[HUD_PIPELINE].attachmentReferences[0].attachment = 0;
    renderPassConfigs[HUD_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    renderPassConfigs[HUD_PIPELINE].attachmentReferences[1].attachment = 1;
    renderPassConfigs[HUD_PIPELINE].attachmentReferences[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[HUD_PIPELINE].actualSubpassDependencyNumber = 1;
    renderPassConfigs[HUD_PIPELINE].subpassDependencies[0].srcSubpass = 0;
    renderPassConfigs[HUD_PIPELINE].subpassDependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[HUD_PIPELINE].subpassDependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[HUD_PIPELINE].subpassDependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[HUD_PIPELINE].subpassDependencies[0].srcAccessMask = {};
    renderPassConfigs[HUD_PIPELINE].subpassDependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[HUD_PIPELINE].subpassDependencies[0].dependencyFlags = {};

    descriptorSetsConfig[FONT_RENDER_UBO].actualLinkedDescriptorBindingsNumber =
        1;
    descriptorSetsConfig[FONT_RENDER_UBO].hostDescriptorNumber = 4096;
    descriptorSetsConfig[FONT_RENDER_UBO].isTexture = false;

    descriptorBindingsConfig[4].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[4].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[4].binding = 0;
    descriptorBindingsConfig[4].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[4].uboChunkSize = sizeof(FONT_UBO);

    descriptorSetsConfig[FONT_RENDER_SAMPLER]
        .actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[FONT_RENDER_SAMPLER].hostDescriptorNumber =
        MAX_TEXTURES;
    descriptorSetsConfig[FONT_RENDER_SAMPLER].isTexture = true;

    descriptorBindingsConfig[5].vkType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorBindingsConfig[5].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[5].binding = 0;
    descriptorBindingsConfig[5].shaderDescriptorsNumber = 1;

    pipelineConfigs[FONT_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/font/font_vert.spv";
    pipelineConfigs[FONT_PIPELINE].fragShader =
        "../../../crates/glvm2/assets/shaders/font/font_frag.spv";
    pipelineConfigs[FONT_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[FONT_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[FONT_PIPELINE].actualLinkedDescriptorSetsNumber = 2;

    renderPassConfigs[FONT_PIPELINE].actualAttachmentDescriptionNumber = 2;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[0].flags = 0;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[0].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[0].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[0].initialLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[1].flags = 0;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[1].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[1].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[1].initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[FONT_PIPELINE].attachmentDescriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[FONT_PIPELINE].actualAttachmentReferenceNumber = 2;
    renderPassConfigs[FONT_PIPELINE].attachmentReferences[0].attachment = 0;
    renderPassConfigs[FONT_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    renderPassConfigs[FONT_PIPELINE].attachmentReferences[1].attachment = 1;
    renderPassConfigs[FONT_PIPELINE].attachmentReferences[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[FONT_PIPELINE].actualSubpassDependencyNumber = 1;
    renderPassConfigs[FONT_PIPELINE].subpassDependencies[0].srcSubpass = 0;
    renderPassConfigs[FONT_PIPELINE].subpassDependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[FONT_PIPELINE].subpassDependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[FONT_PIPELINE].subpassDependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[FONT_PIPELINE].subpassDependencies[0].srcAccessMask = {};
    renderPassConfigs[FONT_PIPELINE].subpassDependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[FONT_PIPELINE].subpassDependencies[0].dependencyFlags = {};

    descriptorSetsConfig[HUD_SCREEN].actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[HUD_SCREEN].hostDescriptorNumber = 64;
    descriptorSetsConfig[HUD_SCREEN].isTexture = false;

    descriptorBindingsConfig[6].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[6].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[6].binding = 0;
    descriptorBindingsConfig[6].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[6].uboChunkSize = sizeof(HUD_SCREEN_UBO);

    pipelineConfigs[HUD_SCREEN_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/hud_screen/vert_hud_screen.spv";
    pipelineConfigs[HUD_SCREEN_PIPELINE].fragShader =
        "../../../crates/glvm2/assets/shaders/hud_screen/frag_hud_screen.spv";
    pipelineConfigs[HUD_SCREEN_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[HUD_SCREEN_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[HUD_SCREEN_PIPELINE].actualLinkedDescriptorSetsNumber = 1;

    renderPassConfigs[HUD_SCREEN_PIPELINE].actualAttachmentDescriptionNumber =
        2;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentDescriptions[0].flags = 0;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentDescriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentDescriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentDescriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[HUD_SCREEN_PIPELINE]
        .attachmentDescriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[HUD_SCREEN_PIPELINE]
        .attachmentDescriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[HUD_SCREEN_PIPELINE]
        .attachmentDescriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentDescriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentDescriptions[1].flags = 0;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentDescriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentDescriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentDescriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[HUD_SCREEN_PIPELINE]
        .attachmentDescriptions[1]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[HUD_SCREEN_PIPELINE]
        .attachmentDescriptions[1]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[HUD_SCREEN_PIPELINE]
        .attachmentDescriptions[1]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentDescriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[HUD_SCREEN_PIPELINE].actualAttachmentReferenceNumber = 2;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentReferences[0].attachment =
        0;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentReferences[1].attachment =
        1;
    renderPassConfigs[HUD_SCREEN_PIPELINE].attachmentReferences[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[HUD_SCREEN_PIPELINE].actualSubpassDependencyNumber = 1;
    renderPassConfigs[HUD_SCREEN_PIPELINE].subpassDependencies[0].srcSubpass =
        0;
    renderPassConfigs[HUD_SCREEN_PIPELINE].subpassDependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[HUD_SCREEN_PIPELINE].subpassDependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[HUD_SCREEN_PIPELINE].subpassDependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[HUD_SCREEN_PIPELINE]
        .subpassDependencies[0]
        .srcAccessMask = {};
    renderPassConfigs[HUD_SCREEN_PIPELINE].subpassDependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[HUD_SCREEN_PIPELINE]
        .subpassDependencies[0]
        .dependencyFlags = {};

    descriptorSetsConfig[UI].actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[UI].hostDescriptorNumber = 128;
    descriptorSetsConfig[UI].isTexture = false;

    descriptorBindingsConfig[7].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[7].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[7].binding = 0;
    descriptorBindingsConfig[7].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[7].uboChunkSize = sizeof(UI_UBO);

    descriptorSetsConfig[UI_SAMPLERS].actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[UI_SAMPLERS].hostDescriptorNumber = MAX_TEXTURES;
    descriptorSetsConfig[UI_SAMPLERS].isTexture = true;

    descriptorBindingsConfig[8].vkType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorBindingsConfig[8].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[8].binding = 0;
    descriptorBindingsConfig[8].shaderDescriptorsNumber = 1;

    pipelineConfigs[UI_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/ui/vert_ui.spv";
    pipelineConfigs[UI_PIPELINE].fragShader =
        "../../../crates/glvm2/assets/shaders/ui/frag_ui.spv";
    pipelineConfigs[UI_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[UI_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[UI_PIPELINE].actualLinkedDescriptorSetsNumber = 2;

    renderPassConfigs[UI_PIPELINE].actualAttachmentDescriptionNumber = 2;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[0].flags = 0;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[0].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[0].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[0].initialLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[1].flags = 0;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[1].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[1].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[1].initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[UI_PIPELINE].attachmentDescriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[UI_PIPELINE].actualAttachmentReferenceNumber = 2;
    renderPassConfigs[UI_PIPELINE].attachmentReferences[0].attachment = 0;
    renderPassConfigs[UI_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    renderPassConfigs[UI_PIPELINE].attachmentReferences[1].attachment = 1;
    renderPassConfigs[UI_PIPELINE].attachmentReferences[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[UI_PIPELINE].actualSubpassDependencyNumber = 1;
    renderPassConfigs[UI_PIPELINE].subpassDependencies[0].srcSubpass = 0;
    renderPassConfigs[UI_PIPELINE].subpassDependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[UI_PIPELINE].subpassDependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[UI_PIPELINE].subpassDependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[UI_PIPELINE].subpassDependencies[0].srcAccessMask = {};
    renderPassConfigs[UI_PIPELINE].subpassDependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[UI_PIPELINE].subpassDependencies[0].dependencyFlags = {};

    descriptorSetsConfig[UI_ICONS].actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[UI_ICONS].hostDescriptorNumber = 128;
    descriptorSetsConfig[UI_ICONS].isTexture = false;

    descriptorBindingsConfig[9].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[9].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[9].binding = 0;
    descriptorBindingsConfig[9].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[9].uboChunkSize = sizeof(UI_UBO);

    descriptorSetsConfig[UI_ICONS_SAMPLERS]
        .actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[UI_ICONS_SAMPLERS].hostDescriptorNumber = MAX_TEXTURES;
    descriptorSetsConfig[UI_ICONS_SAMPLERS].isTexture = true;

    descriptorBindingsConfig[10].vkType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorBindingsConfig[10].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[10].binding = 0;
    descriptorBindingsConfig[10].shaderDescriptorsNumber = 1;

    pipelineConfigs[UI_ICONS_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/ui_icons/vert_ui_icons.spv";
    pipelineConfigs[UI_ICONS_PIPELINE].fragShader =
        "../../../crates/glvm2/assets/shaders/ui_icons/frag_ui_icons.spv";
    pipelineConfigs[UI_ICONS_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[UI_ICONS_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[UI_ICONS_PIPELINE].actualLinkedDescriptorSetsNumber = 2;

    renderPassConfigs[UI_ICONS_PIPELINE].actualAttachmentDescriptionNumber = 2;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[0].flags = 0;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[0].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[UI_ICONS_PIPELINE]
        .attachmentDescriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[0].initialLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[1].flags = 0;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[1].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[UI_ICONS_PIPELINE]
        .attachmentDescriptions[1]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[1].initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentDescriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[UI_ICONS_PIPELINE].actualAttachmentReferenceNumber = 2;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentReferences[0].attachment = 0;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    renderPassConfigs[UI_ICONS_PIPELINE].attachmentReferences[1].attachment = 1;
    renderPassConfigs[UI_ICONS_PIPELINE].attachmentReferences[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[UI_ICONS_PIPELINE].actualSubpassDependencyNumber = 1;
    renderPassConfigs[UI_ICONS_PIPELINE].subpassDependencies[0].srcSubpass = 0;
    renderPassConfigs[UI_ICONS_PIPELINE].subpassDependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[UI_ICONS_PIPELINE].subpassDependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[UI_ICONS_PIPELINE].subpassDependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[UI_ICONS_PIPELINE].subpassDependencies[0].srcAccessMask =
        {};
    renderPassConfigs[UI_ICONS_PIPELINE].subpassDependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[UI_ICONS_PIPELINE]
        .subpassDependencies[0]
        .dependencyFlags = {};

    descriptorSetsConfig[VIRTUAL_TEXTURES_UBO]
        .actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[VIRTUAL_TEXTURES_UBO].hostDescriptorNumber = 128;
    descriptorSetsConfig[VIRTUAL_TEXTURES_UBO].isTexture = false;

    descriptorBindingsConfig[11].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[11].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[11].binding = 0;
    descriptorBindingsConfig[11].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[11].uboChunkSize = sizeof(VIRTUAL_TEXTURE_UBO);

    descriptorSetsConfig[VIRTUAL_TEXTURES_TILESET]
        .actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[VIRTUAL_TEXTURES_TILESET].hostDescriptorNumber =
        MAX_TEXTURES;
    descriptorSetsConfig[VIRTUAL_TEXTURES_TILESET].isTexture = true;

    descriptorBindingsConfig[12].vkType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorBindingsConfig[12].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[12].binding = 0;
    descriptorBindingsConfig[12].shaderDescriptorsNumber = 1;

    pipelineConfigs[VIRTUAL_TEXTURES_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/virtual_textures/virtualTexturesVert.spv";
    pipelineConfigs[VIRTUAL_TEXTURES_PIPELINE].fragShader =
        "../../../crates/glvm2/assets/shaders/virtual_textures/virtualTexturesFrag.spv";
    pipelineConfigs[VIRTUAL_TEXTURES_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[VIRTUAL_TEXTURES_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[VIRTUAL_TEXTURES_PIPELINE].actualLinkedDescriptorSetsNumber =
        2;

    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .actualAttachmentDescriptionNumber = 2;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE].attachmentDescriptions[0].flags =
        0;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[0]
        .samples = VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[0]
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[0]
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[0]
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE].attachmentDescriptions[1].flags =
        0;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[1]
        .samples = VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[1]
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[1]
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[1]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[1]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[1]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentDescriptions[1]
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .actualAttachmentReferenceNumber = 2;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentReferences[0]
        .attachment = 0;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .attachmentReferences[1]
        .attachment = 1;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE].attachmentReferences[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE].actualSubpassDependencyNumber =
        1;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .subpassDependencies[0]
        .srcSubpass = 0;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .subpassDependencies[0]
        .dstSubpass = VK_SUBPASS_EXTERNAL;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .subpassDependencies[0]
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .subpassDependencies[0]
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .subpassDependencies[0]
        .srcAccessMask = {};
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .subpassDependencies[0]
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[VIRTUAL_TEXTURES_PIPELINE]
        .subpassDependencies[0]
        .dependencyFlags = {};

    descriptorSetsConfig[MAIN_RENDER_MATRIX_UBO]
        .actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[MAIN_RENDER_MATRIX_UBO].hostDescriptorNumber = 1024;
    descriptorSetsConfig[MAIN_RENDER_MATRIX_UBO].isTexture = false;

    descriptorBindingsConfig[13].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[13].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[13].binding = 0;
    descriptorBindingsConfig[13].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[13].uboChunkSize = sizeof(ModelMatrixUBO);

    descriptorSetsConfig[MAIN_RENDER_LIGHT_DATA_UBO]
        .actualLinkedDescriptorBindingsNumber = 4;
    descriptorSetsConfig[MAIN_RENDER_LIGHT_DATA_UBO].hostDescriptorNumber = 2;
    descriptorSetsConfig[MAIN_RENDER_LIGHT_DATA_UBO].isTexture = false;

    descriptorBindingsConfig[14].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[14].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[14].binding = 0;
    descriptorBindingsConfig[14].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[14].uboChunkSize = sizeof(LightData);

    descriptorBindingsConfig[15].vkType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorBindingsConfig[15].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[15].binding = 1;
    descriptorBindingsConfig[15].shaderDescriptorsNumber = 4;

    descriptorBindingsConfig[16].vkType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorBindingsConfig[16].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[16].binding = 5;
    descriptorBindingsConfig[16].shaderDescriptorsNumber = 32;

    descriptorBindingsConfig[17].vkType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorBindingsConfig[17].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[17].binding = 37;
    descriptorBindingsConfig[17].shaderDescriptorsNumber = 8;

    descriptorSetsConfig[MAIN_RENDER_SPECULAR_SAMPLER]
        .actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[MAIN_RENDER_SPECULAR_SAMPLER].hostDescriptorNumber =
        MAX_TEXTURES;
    descriptorSetsConfig[MAIN_RENDER_SPECULAR_SAMPLER].isTexture = true;

    descriptorBindingsConfig[18].vkType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorBindingsConfig[18].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[18].binding = 0;
    descriptorBindingsConfig[18].shaderDescriptorsNumber = 1;

    descriptorSetsConfig[MAIN_RENDER_DIFFUSE_SAMPLER]
        .actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[MAIN_RENDER_DIFFUSE_SAMPLER].hostDescriptorNumber =
        MAX_TEXTURES;
    descriptorSetsConfig[MAIN_RENDER_DIFFUSE_SAMPLER].isTexture = true;

    descriptorBindingsConfig[19].vkType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorBindingsConfig[19].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[19].binding = 0;
    descriptorBindingsConfig[19].shaderDescriptorsNumber = 1;

    pipelineConfigs[MAIN_RENDER_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/main_renderer/vert.spv";
    pipelineConfigs[MAIN_RENDER_PIPELINE].fragShader =
        "../../../crates/glvm2/assets/shaders/main_renderer/frag.spv";
    pipelineConfigs[MAIN_RENDER_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[MAIN_RENDER_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[MAIN_RENDER_PIPELINE].actualLinkedDescriptorSetsNumber = 4;

    renderPassConfigs[MAIN_RENDER_PIPELINE].actualAttachmentDescriptionNumber =
        2;
    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentDescriptions[0].flags = 0;
    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentDescriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentDescriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentDescriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[MAIN_RENDER_PIPELINE]
        .attachmentDescriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[MAIN_RENDER_PIPELINE]
        .attachmentDescriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[MAIN_RENDER_PIPELINE]
        .attachmentDescriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[MAIN_RENDER_PIPELINE]
        .attachmentDescriptions[0]
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentDescriptions[1].flags = 0;
    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentDescriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentDescriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentDescriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[MAIN_RENDER_PIPELINE]
        .attachmentDescriptions[1]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[MAIN_RENDER_PIPELINE]
        .attachmentDescriptions[1]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[MAIN_RENDER_PIPELINE]
        .attachmentDescriptions[1]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[MAIN_RENDER_PIPELINE]
        .attachmentDescriptions[1]
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[MAIN_RENDER_PIPELINE].actualAttachmentReferenceNumber = 2;
    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentReferences[0].attachment =
        0;
    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentReferences[1].attachment =
        1;
    renderPassConfigs[MAIN_RENDER_PIPELINE].attachmentReferences[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[MAIN_RENDER_PIPELINE].actualSubpassDependencyNumber = 1;
    renderPassConfigs[MAIN_RENDER_PIPELINE].subpassDependencies[0].srcSubpass =
        0;
    renderPassConfigs[MAIN_RENDER_PIPELINE].subpassDependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[MAIN_RENDER_PIPELINE].subpassDependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[MAIN_RENDER_PIPELINE].subpassDependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[MAIN_RENDER_PIPELINE]
        .subpassDependencies[0]
        .srcAccessMask = {};
    renderPassConfigs[MAIN_RENDER_PIPELINE].subpassDependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[MAIN_RENDER_PIPELINE]
        .subpassDependencies[0]
        .dependencyFlags = {};

    descriptorSetsConfig[SDF_DATA].actualLinkedDescriptorBindingsNumber = 1;
    descriptorSetsConfig[SDF_DATA].hostDescriptorNumber = 64;
    descriptorSetsConfig[SDF_DATA].isTexture = false;

    descriptorBindingsConfig[20].vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorBindingsConfig[20].shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorBindingsConfig[20].binding = 0;
    descriptorBindingsConfig[20].shaderDescriptorsNumber = 1;
    descriptorBindingsConfig[20].uboChunkSize = sizeof(SDF_UBO);

    pipelineConfigs[SDF_PIPELINE].vertShader =
        "../../../crates/glvm2/assets/shaders/sdf/sdf_vert.spv";
    pipelineConfigs[SDF_PIPELINE].fragShader =
        "../../../crates/glvm2/assets/shaders/sdf/sdf_frag.spv";
    pipelineConfigs[SDF_PIPELINE].bindingDescription =
        Vertex::getBindingDescription();
    pipelineConfigs[SDF_PIPELINE].attributeDescriptions =
        Vertex::getAttributeDescriptions();
    pipelineConfigs[SDF_PIPELINE].actualLinkedDescriptorSetsNumber = 1;

    renderPassConfigs[SDF_PIPELINE].actualAttachmentDescriptionNumber = 2;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[0].flags = 0;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[0].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[0].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[0].initialLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[1].flags = 0;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[1].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[1].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[1].initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassConfigs[SDF_PIPELINE].attachmentDescriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[SDF_PIPELINE].actualAttachmentReferenceNumber = 2;
    renderPassConfigs[SDF_PIPELINE].attachmentReferences[0].attachment = 0;
    renderPassConfigs[SDF_PIPELINE].attachmentReferences[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    renderPassConfigs[SDF_PIPELINE].attachmentReferences[1].attachment = 1;
    renderPassConfigs[SDF_PIPELINE].attachmentReferences[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    renderPassConfigs[SDF_PIPELINE].actualSubpassDependencyNumber = 1;
    renderPassConfigs[SDF_PIPELINE].subpassDependencies[0].srcSubpass = 0;
    renderPassConfigs[SDF_PIPELINE].subpassDependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    renderPassConfigs[SDF_PIPELINE].subpassDependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[SDF_PIPELINE].subpassDependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    renderPassConfigs[SDF_PIPELINE].subpassDependencies[0].srcAccessMask = {};
    renderPassConfigs[SDF_PIPELINE].subpassDependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    renderPassConfigs[SDF_PIPELINE].subpassDependencies[0].dependencyFlags = {};

    // Not related to any pipeline descriptor sets and its bindings.
    descriptorSetsConfig[RIDABLE_TEXTURES].actualLinkedDescriptorBindingsNumber =
        1;
    descriptorSetsConfig[RIDABLE_TEXTURES].hostDescriptorNumber = MAX_TEXTURES;
    descriptorSetsConfig[RIDABLE_TEXTURES].isTexture = true;

    descriptorBindingsConfig[21].vkType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorBindingsConfig[21].shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorBindingsConfig[21].binding = 0;
    descriptorBindingsConfig[21].shaderDescriptorsNumber = MAX_TEXTURES;
}
}; // namespace glvm

namespace glvm {
void descriptorSetBuilder();
void pipelineBuilder();
void renderPassesBuilder();
}; // namespace glvm

namespace glvm {
uint64_t makeEntity(uint32_t id, uint32_t generation);
uint32_t getId(uint64_t entity);
uint32_t getGen(uint64_t entity);
auto matches_required_mask(
    const uint64_t archetype_mask,
    const uint64_t& system_mask
) -> bool;

template<typename T>
void unwrapArchetype(Archetype* arch, uint64_t mask, void (*func)(T*)) {
    switch (mask) {
        case playerComponentMask:
            func(static_cast<PlayerArchetype*>(arch));
            break;
        case enemyComponentMask:
            func(static_cast<EnemyArchetype*>(arch));
            break;
    }
}
}; // namespace glvm

namespace glvm {
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
);
void CreateBeginDebugUtilsLabelEXT(
    [[maybe_unused]] VkInstance instance,
    [[maybe_unused]] VkCommandBuffer commandBuffer,
    [[maybe_unused]] const VkDebugUtilsLabelEXT* labelInfo
);
void CreateEndDebugUtilsLabelEXT(
    [[maybe_unused]] VkInstance instance,
    [[maybe_unused]] VkCommandBuffer commandBuffer
);
void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
);
VkResult SetDebugObjectName(
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* objectNameInfo
);
void setImageDebugObjectName(
    VkDevice device,
    VK_Image image,
    std::string imageName
);
void setPipelineDebugObjectName(
    VkDevice device,
    VkPipeline pipeline,
    std::string pipelineName
);
void setDescriptorSetObjectName(
    VkDevice device,
    VkDescriptorSet descriptorSet,
    std::string descriptorSetName,
    unsigned int index
);
void setDebugObjectNames(
    VkDevice device,
    const std::vector<VkBuffer>& vertexBufferContainer,
    const std::vector<VkBuffer>& indexBufferContainer,
    const std::vector<Descriptor>& GPUDescriptors,
    const std::vector<unsigned int>& fontIndicesContainer,
    const std::vector<VkBuffer>& fontVertexBufferContainer,
    const std::vector<VkBuffer>& fontIndexBufferContainer
);
}; // namespace glvm

namespace glvm {
struct GridChunk {
    Vector<float, 3> position;
    static constexpr float size = 32;
    std::vector<uint32_t> entities;
};

struct SpatialGrid {
    static const uint32_t width = 64;
    static const uint32_t height = 64;
    static const uint32_t depth = 64;
    GridChunk grid[width][height][depth];
};

struct World {
    World();
    ~World();

    SpatialGrid spatialGrid;
    std::vector<Archetype*> archetypes;
    std::vector<EntityLocation> entityLocations;

    void addEntityToArchetype(uint64_t entity_, Archetype* arch);
    void removeEntity(uint64_t entity_);
    void searchCacheArchetypes(
        uint64_t requiredMask,
        Archetype* cachedArchetypes[],
        uint32_t& cachedArchetypesNumber
    );
};

extern World world;
}; // namespace glvm

namespace glvm {

struct ArchetypeEntityManager {
    inline static uint32_t nextId = 0;
    std::vector<uint32_t> generations;
    std::vector<uint32_t> freeList;

    ArchetypeEntityManager();
    static ArchetypeEntityManager* get_instance();

    uint64_t createEntity();
    void removeEntity(uint64_t entity_);
    bool isAlive(uint64_t entity_) const;

private:
    static ArchetypeEntityManager* pInstance_;
    static std::mutex Mutex_;

    ~ArchetypeEntityManager();
};
}; // namespace glvm

namespace glvm {
class InventorySystem: public ISystem {
public:
    uint32_t crosshairArchetypesNumber = 0;
    uint32_t inventoryArchetypesNumber = 0;

    struct ArchView {
        Archetype* crosshairCachedArchetype = nullptr;
        Archetype* inventoryCachedArchetype = nullptr;
    } archView;

    struct ComponentsView {
        transform* crosshairTransformsView = nullptr;

        transform* inventoryTransformsView = nullptr;
        inventory* inventoryView = nullptr;
        mesh* inventoryMeshesView = nullptr;
    } componentsView;

    uint64_t crosshairRequiredMask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::CrosshairTagComponent);

    uint64_t inventoryRequiredMask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::InventoryComponent)
        | (1ull << ComponentsIndices::MeshComponent);

    void Update() override;
    int determineSwappableStatusAndSlots(
        item* itemComponent,
        transform* inventoryTransformComponent,
        std::vector<unsigned int>& potentialOccupiedSlots,
        transform* crosshairTransformComponent,
        point2D<int> intersectionSlot,
        inventory* inventoryComponent,
        const float inventorySlotScale
    );
    void fillInventorySlots(
        item* itemComponent,
        const int itemWidth,
        const int itemHeight,
        inventory* inventoryComponent,
        const int fillValue
    );
    int determineSwappableField(
        item* itemComponent,
        const int itemWidth,
        const int itemHeight,
        int pivotRow,
        int pivotColumn,
        inventory* inventoryComponent,
        std::vector<unsigned int>& potentialOccupiedSlots
    );
    int calculateBasicOffset(
        const int itemAxisSize,
        const float axisValue,
        const float crosshairAxisPosition,
        const int axisSlotIndex,
        const float inventorySlotScale
    );
    bool checkCrosshairInventoryIntersection(
        transform* crosshairTransformComponent,
        transform* inventoryTransformComponent,
        inventory* inventoryComponent,
        const float inventorySlotScale,
        const float inventorySlotHalfScale
    );
    point2D<int> determineActualIntersectionSlot(
        transform* crosshairTransformComponent,
        transform* inventoryTransformComponent,
        const float inventorySlotScale,
        const float inventorySlotHalfScale
    );

    bool isInventoryOpened;
    int* isItemDraged;
    bool* isLeftMouseButtonReleased;
    bool isLeftMouseButtonPressed;
    float mouseOffsetX = 0;
    float mouseOffsetY = 0;
    // Window aspect ratio, set by engine each frame.
    float aspectRate = 0.0f;
    Archetype* cachedCrosshairArchetype;
    Archetype* cachedInventoryArchetype;
};
} // namespace glvm

#ifdef __linux__
// #define VK_USE_PLATFORM_XLIB_KHR
// #define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_wayland.h"
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_xcb.h"

#include <xcb/xcb.h>
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_xlib.h"

#include <X11/Xlib.h>
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#endif

namespace glvm {
const int MAX_FRAMES_IN_FLIGHT = 2;
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    "VK_KHR_shader_non_semantic_info"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool isComplete() {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class CVulkanRenderer {
public:
    bool print = true;
    Vector<int, 4>
        indirectTexture[INDIRECT_TEXTURE_WIDTH * INDIRECT_TEXTURE_HEIGHT / 4 + 1];
    std::vector<unsigned int> entitiesCollectionLinked__Trn_Mat_Mes_Act;
    std::vector<unsigned int> entitiesCollectionLinked__Trn_PoL_Mes_Act;

    char glyphs[128] = {'A',  'B',  'C', 'D', 'E', 'F', 'G',  'H',  'I', 'J',
                        'K',  'L',  'M', 'N', 'O', 'P', 'Q',  'R',  'S', 'T',
                        'U',  'V',  'W', 'X', 'Y', 'Z', 'a',  'b',  'c', 'd',
                        'e',  'f',  'g', 'h', 'i', 'j', 'k',  'l',  'm', 'n',
                        'o',  'p',  'q', 'r', 's', 't', 'u',  'v',  'y', 'x',
                        'y',  'z',  '0', '1', '2', '3', '4',  '5',  '6', '7',
                        '8',  '9',  '.', ',', '"', '"', '\'', '\'', '"', '"',
                        '\'', '\'', '?', '!', '_', '$', '(',  ')',  '+', '-',
                        '/',  ':',  ';', '<', '>', '=', '[',  ']',  '\\'};
    // FIXME: Some garbage here that needs maybe for align.
    const std::vector<Vertex> padding[128];
    const std::vector<uint32_t> symbol_g_indices = {0, 1, 2, 2, 1, 3};
    std::chrono::steady_clock::time_point startTime;

    std::vector<Texture> initializeTextureData_;
    std::vector<const char*> pathsArray_;
    std::vector<const char*> pathsGLTF_;
    std::vector<std::vector<Vertex>> levelGeneratedVertices;
    std::vector<std::vector<uint32_t>> levelGeneratedIndices;

    std::vector<std::vector<Vertex>> aVertices_;
    // Wavefront .obj indices.
    std::vector<std::vector<uint32_t>> aIndices_;
    // GLTF indices.
    std::vector<std::vector<float>> aVertexesTemp_;
    // highest GLTF y.
    std::vector<float> highest_gltf_Y;
    // Keep axis limiting values for every axis per mesh in current iteration
    // while initializing Wavefront .obj and GLTF.
    MeshAxisLimitingValues meshAxisLimitingValues;
    std::vector<std::vector<std::vector<Matrix<float, 4>>>> jointMatricesPerMesh;
    std::vector<std::vector<float>> frames;
    bool isInventoryOpened = false;
    bool isCursorReleased = false;
    Vector<float, 3> forward = {0.0f, 0.0f, -1.0f};
    float hud_screen_x = 0.0f;
    float hud_screen_y;

    unsigned int entities[32];
    std::vector<RenderActor> actors;
    std::vector<RenderDirectionalLight> directionalLights;
    std::vector<RenderSpotLight> spotLights;
    std::vector<RenderPointLight> pointLights;
    std::vector<RenderHealth> healthBars;
    std::vector<RenderFont> fonts;
    std::vector<RenderInventory> inventories;
    std::vector<RenderItem> items;
    std::vector<RenderCrosshair> crosshairs;
    std::vector<RenderPlayer> players;
    RenderPlayer player;

    float fYaw = -90.0f;
    float fPitch = 0.0f;
    float prev_Y = 0.0f;
    float current_Y = 0.0f;
    float prev_X = 0.0f;
    float current_X = 0.0f;
    // Window aspect ratio, updated on resize.
    float aspectRate = 0.0f;
    int draggedItemEntity;

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    glvm::WindowWaylandVulkan* Window;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    glvm::WindowXCBVulkan* Window = nullptr;
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
    glvm::WindowXVulkan* Window;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    glvm::WindowWinVulkan* Window;
#endif

    ImGuiOverlay* imguiOverlay = nullptr;

    CVulkanRenderer();
    ~CVulkanRenderer();

    void createTextureImage();
    void recreateSwapChain();
    void draw();
    void SetMeshData(
        std::vector<const char*> _pathsArray,
        std::vector<const char*> pathsGLTF
    );
    void SetProjectionMatrix(Matrix<float, 4> _projectionMatrix);
    void SetViewMatrix(Matrix<float, 4> _viewMatrix);
    void initializeGameLevelVertices();
    void run();

public:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    Matrix<float, 4> viewMatrix;
    Matrix<float, 4> projectionMatrix;
    ThreadPool* renderThreadPool;

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    VkWaylandSurfaceCreateInfoKHR createWaylandSurfaceInfo;
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
    VkXlibSurfaceCreateInfoKHR createXlibSurfaceInfo;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    VkXcbSurfaceCreateInfoKHR createXcbSurfaceInfo;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkWin32SurfaceCreateInfoKHR createWin32SurfaceInfo;
#endif

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkBuffer hudUniformBuffer;
    VkDeviceMemory hudUniformBuffersMemory;
    VkBuffer fontUniformBuffer;
    VkDeviceMemory fontUniformBuffersMemory;
    VkBuffer hudScreenUniformBuffer;
    VkDeviceMemory hudScreenUniformBuffersMemory;
    VkBuffer uiUniformBuffer;
    VkDeviceMemory uiUniformBuffersMemory;
    VkBuffer uiIconsUniformBuffer;
    VkDeviceMemory uiIconsUniformBuffersMemory;
    std::vector<VkDescriptorSet> virtualTexturesUBODesctiptorSets;
    std::vector<VkDescriptorSet> virtualTexturesSamplersDesctiptorSets;
    VkBuffer virtualTexturesUniformBuffer;
    VkDeviceMemory virtualTexturesUniformBufferMemory;

    VkCommandPool directionalLightCommandPool;
    VkCommandPool spotLightCommandPool;
    VkCommandPool pointLightCommandPool;
    VkCommandPool fontCommandPool;
    VkCommandPool hudCommandPool;
    VkCommandPool hudScreenCommandPool;
    VkCommandPool uiCommandPool;
    VkCommandPool uiIconsCommandPool;
    VkCommandPool mainRenderCommandPool;
    std::vector<VkCommandPool> secondaryBuffersCommandPools;
    VkCommandPool virtualTexturesCommandPool;

    // Main pipeline depth.
    VkImage mainDepthPipelineImage;
    VkDeviceMemory mainDepthPipelineImageMemory;
    VkImageView mainDepthImageView;

    // Depth variables for shadow map.
public:
    unsigned int directionalLightNumber = 0;
    std::vector<VkFramebuffer> directionalLightShadowMapFrameBuffers;
    VkBuffer shadowMapDirectionalLightModelMatrixUniformBuffer;
    VkDeviceMemory shadowMapDirectionalLightModelMatrixUniformBuffersMemory;
    std::vector<VK_Image> directionalLightTextureImages;

    Matrix<float, 4> dirLightSpaceMatrix[DIRECTIONAL_LIGHTS_NUMBER];
    Matrix<float, 4> spotLightSpaceMatrix[SPOT_LIGHTS_NUMBER];

    unsigned int pointLightNumber = 0;
    std::vector<std::vector<VkFramebuffer>> pointLightShadowMapFrameBuffers;
    VkBuffer shadowMapPointLightModelMatrixUniformBuffer;
    VkDeviceMemory shadowMapPointLightModelMatrixUniformBuffersMemory;
    std::vector<VK_Image> pointLightTextureImages;

    unsigned int spotLightNumber = 0;
    std::vector<VkFramebuffer> spotLightShadowMapFrameBuffers;
    VkBuffer shadowMapSpotLightModelMatrixUniformBuffer;
    VkDeviceMemory shadowMapSpotLightModelMatrixUniformBuffersMemory;
    std::vector<VK_Image> spotLightTextureImages;

    std::vector<VK_Image> textureImages;
    VkSampler textureSampler;
    VkSampler shadowMapSampler;
    uint32_t frameCounter = 0;

    std::vector<VkBuffer> vertexBufferContainer;
    std::vector<VkDeviceMemory> vertexBufferMemoryContainer;
    std::vector<VkBuffer> indexBufferContainer;
    std::vector<VkDeviceMemory> indexBufferMemoryContaner;
    uint32_t wavefrontObjCounter = 0;
    uint32_t gltfCounter = 0;

    std::vector<std::vector<Vertex>> symbolGVerticesContainer;
    std::vector<unsigned int> fontIndicesContainer;
    std::vector<VkBuffer> fontVertexBufferContainer;
    std::vector<VkDeviceMemory> fontVertexBufferMemoryContainer;
    std::vector<VkBuffer> fontIndexBufferContainer;
    std::vector<VkDeviceMemory> fontIndexBufferMemoryContaner;

    VkBuffer modelMatrixUniformBuffer;
    VkDeviceMemory modelMatrixUniformBuffersMemory;
    VkBuffer lightDataUniformBuffer;
    VkDeviceMemory lightDataUniformBuffersMemory;

    VkDescriptorPool descriptorPool;
    const unsigned int matrixUboDescriptorsNumber = 500;
    const unsigned int hudUboDescriptorNumber = 500;
    const unsigned int fontUboDescriptorNumber = 128;
    const unsigned int hudScreenUboDescriptorNumber = 32;
    const unsigned int uiUboDescriptorsNumber = 64;
    [[maybe_unused]] const unsigned int virtualTexturesDescriptorsNumber = 64;
    std::vector<VkCommandBuffer> directionalLightCommandBuffers;
    std::vector<VkCommandBuffer> spotLightCommandBuffers;
    std::vector<VkCommandBuffer> pointLightCommandBuffers;
    std::vector<VkCommandBuffer> fontCommandBuffers;
    std::vector<VkCommandBuffer> hudCommandBuffers;
    std::vector<VkCommandBuffer> mainRenderCommandBuffers;
    std::vector<VkCommandBuffer> directionalLightSecondaryCommandBuffers;
    std::vector<VkCommandBuffer> spotLightSecondaryCommandBuffers;
    std::vector<VkCommandBuffer> pointLightSecondaryCommandBuffers;
    std::vector<VkCommandBuffer> virtualTexturesCommandBuffers;

    // Main render pipeline sync objects.
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    // Hud render pipeline sync objects.
    std::vector<VkSemaphore> hudImageAvailableSemaphores;
    std::vector<VkSemaphore> hudRenderFinishedSemaphores;
    std::vector<VkFence> hudInFlightFences;

    // Font render pipeline sync objects.
    std::vector<VkSemaphore> fontImageAvailableSemaphores;
    std::vector<VkSemaphore> fontRenderFinishedSemaphores;
    std::vector<VkFence> fontInFlightFences;

    // Directional light shadow map sync objects.
    std::vector<VkSemaphore> directionalLightShadowMapImageAvailableSemaphores;
    std::vector<VkSemaphore> directionalLightShadowMapRenderFinishedSemaphores;
    std::vector<VkFence> directionalLightShadowMapInFlightFences;

    // Spotlight shadow map sync objects.
    std::vector<VkSemaphore> spotLightShadowMapImageAvailableSemaphores;
    std::vector<VkSemaphore> spotLightShadowMapRenderFinishedSemaphores;
    std::vector<VkFence> spotLightShadowMapInFlightFences;

    // Point light shadow map sync objects.
    std::vector<VkSemaphore> pointLightShadowMapImageAvailableSemaphores;
    std::vector<VkSemaphore> pointLightShadowMapRenderFinishedSemaphores;
    std::vector<VkFence> pointLightShadowMapInFlightFences;

    // Virtual textures pipeline sync objects.
    std::vector<VkSemaphore> virtualTexturesImageAvailableSemaphores;
    std::vector<VkSemaphore> virtualTexturesRenderFinishedSemaphores;
    std::vector<VkFence> virtualTexturesInFlightFences;

    uint32_t currentFrame = 0;
    uint32_t directionalLightCurrentFrame = 0;
    uint32_t spotLightCurrentFrame = 0;
    uint32_t pointLightCurrentFrame = 0;

    std::mutex mutex0;
    std::mutex mutex1;
    std::mutex mutex2;
    std::mutex shadowMapPassesMutex;

    bool framebufferResized = false;

    void initWindow();
    void initVulkan();
    void initializeVertexBuffersWithWavefrontData();
    void initializeVertexBuffersWithGLTFData();
    void initializeVertexBuffersWithFontData();
    void cleanupSwapChain();
    void cleanup();
    void createInstance();
    void populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo
    );
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createMainRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createRenderPassFramebuffers(
        std::vector<VkImageView>& attachments,
        VkRenderPass& renderPass_,
        VkFramebuffer& swapChainFramebuffer,
        uint32_t width,
        uint32_t height
    );
    void createFramebuffers();
    void createCommandPool(VkCommandPool& commandPool);
    void createDepthResources();
    void createDirectionalLightShadowMapDepthResources();
    void createSpotLightShadowMapDepthResources();
    void createPointLightShadowMapDepthResources();
    VkFormat findSupportedFormat(
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features
    );
    VkFormat findDepthFormat();
    bool hasStencilComponent(VkFormat format);
    void createTextureImageView();
    void createTextureSampler();
    void createShadowMapSampler();
    VkImageView createImageView(
        VK_Image image,
        uint32_t baseArrayLayers,
        uint32_t layerCount
    );
    void createImage(VK_Image& image);
    void transitionImageLayout(
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout
    );
    void transitionShadowMapImageLayout(
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout
    );
    void copyBufferToImage(
        VkBuffer& buffer,
        VkImage image,
        uint32_t width,
        uint32_t height
    );
    void createVertexBuffer(
        VkBuffer& _vertexBuffer,
        VkDeviceMemory& _vertexBufferMemory,
        std::vector<Vertex>& _vertices
    );
    void createIndexBuffer(
        VkBuffer& _indexBuffer,
        VkDeviceMemory& _indexBufferMemory,
        const std::vector<uint32_t>& _indices
    );
    void createMainRenderUniformBuffers();
    void createMainRenderDescriptorPool();
    void allocateDescriptorSets(
        std::vector<VkDescriptorSet>& descriptorSets,
        VkDescriptorSetLayout setLayout,
        const unsigned int descriptorSetsNumber,
        const unsigned int descriptorOffset
    );
    void updateDescriptorSetsUBO(
        VkBuffer ubo,
        const VkDeviceSize& uboStructSize,
        const unsigned int& uboDescriptorsNumber,
        int uboBinding,
        std::vector<VkDescriptorSet>& uboDescriptorSets,
        const unsigned int offset
    );
    void updateLightDataDescriptorSets(
        const DescriptorSet& currentDescriptorSet1
    );
    void updateDescriptorSetsCombinedImageSampler(
        const DescriptorSet& descriptorSet
    );
    void createDescriptorImageInfo(
        const unsigned int descriptorNumber,
        VkImageLayout imageLayout,
        std::vector<VK_Image>& textureImages,
        const unsigned int imageViewIndex,
        VkDescriptorImageInfo descriptorImageInfos[]
    );
    VkDescriptorBufferInfo createDescriptorBufferInfo(
        VkBuffer ubo,
        uint32_t offset,
        uint32_t range
    );
    void createMainRenderDescriptorSets();
    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory
    );
    VkCommandBuffer beginSingleTimeCommands(VkCommandPool& commandPool);
    void endSingleTimeCommands(
        VkCommandPool& commandPool,
        VkCommandBuffer& commandBuffer
    );
    void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);
    uint32_t findMemoryType(
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties
    );
    void createCommandBuffers(
        VkCommandPool& commandPool,
        std::vector<VkCommandBuffer>& commandBuffers,
        uint32_t commandBuffersNumber,
        VkCommandBufferLevel commandBufferLevelFlag
    );
    void executeSecondaryCommandBuffer(
        VkRenderPass renderPass,
        VkFramebuffer frameBuffer,
        VkExtent2D extent,
        VkCommandBuffer primaryCommandBuffer,
        VkCommandBuffer secondaryCommandBuffer
    );
    void updateHudUBO(
        uint32_t offset,
        bool isHudExists,
        float highestY,
        uint32_t healthCounter
    );
    void updateHudScreenUBO(uint32_t offset, uint32_t crosshair);
    void updateSdfUBO(uint32_t offset, uint32_t crosshair);
    void updateUBO_UI(
        const unsigned int currentInventoryRow,
        const unsigned int currentInventoryColumn,
        const unsigned int inventory,
        uint32_t offset
    );
    void updateUBO_IconsUI(uint32_t offset, uint32_t item);
    void hudRecordCommandBuffer(
        VkCommandBuffer& commandBuffer,
        uint32_t imageIndex
    );
    void uiRecordCommandBuffer(
        VkCommandBuffer& commandBuffer,
        uint32_t imageIndex
    );
    void uiIconsRecordCommandBuffer(
        VkCommandBuffer& commandBuffer,
        uint32_t imageIndex
    );
    void hudScreenRecordCommandBuffer(
        VkCommandBuffer& commandBuffer,
        uint32_t imageIndex
    );
    void sdfRecordCommandBuffer(
        VkCommandBuffer& commandBuffer,
        uint32_t imageIndex
    );
    void fontRecordCommandBuffer(
        VkCommandBuffer& commandBuffer,
        uint32_t imageIndex
    );
    void recordCommandBuffer(
        VkCommandBuffer& commandBuffer,
        uint32_t imageIndex
    );
    void createSyncObjects(
        std::vector<VkSemaphore>& imageAvailableSemaphores,
        std::vector<VkSemaphore>& renderFinishedSemaphores,
        std::vector<VkFence>& inFlightFences
    );
    void updateDirectionalLightShadowMapMatrixUBO(
        uint32_t currentImage,
        uint32_t currentLight,
        unsigned int actor
    );
    void updateSpotLightShadowMapMatrixUBO(
        uint32_t currentImage,
        uint32_t currentLight,
        unsigned int actor
    );
    void updatePointLightShadowMapMatrixUBO(
        uint32_t currentImage,
        uint32_t currentLight,
        uint32_t layer,
        unsigned int actor
    );
    void updateMatrixUniformBuffer(uint32_t offset, unsigned int actor);
    void updateViewPositionUniformBuffer(uint32_t currentImage, uint32_t player);
    void mainRenderDrawFrame();
    void directionalLightShadowMapDrawFrame();
    void spotLightShadowMapDrawFrame();
    void pointLightShadowMapDrawFrame();
    void directionalLightRecordCoomandBuffer(
        std::vector<VkCommandBuffer>& commandBuffer,
        uint32_t currentFrame
    );
    void spotLightRecordCommandBuffer(
        std::vector<VkCommandBuffer>& commandBuffer,
        uint32_t currentFrame
    );
    void pointLightRecordCommandBuffer(
        std::vector<VkCommandBuffer>& commandBuffers,
        uint32_t currentFrame
    );
    VkShaderModule createShaderModule(const std::vector<char>& code);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats
    );
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes
    );
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    VkDescriptorBufferInfo createDescriptorBufferInfo(
        VkBuffer ubo,
        const VkDeviceSize& uboStructSize,
        const VkDeviceSize& offsetStep
    );
    VkDescriptorImageInfo createDescriptorImageInfo(
        const VK_Image& textureImage,
        VkImageLayout layout,
        unsigned int textureIndex,
        VkSampler textureSampler
    );
    static std::vector<char> readFile(const std::string& filename);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );
    Matrix<float, 4> computeModelMatrix(transform* _transformComponent);
    void clearVK_Image(VK_Image* textureImages);
};

}; // namespace glvm

namespace glvm {
struct ItemSystem: public ISystem {
    uint32_t inventoryArchetypesNumber = 0;
    uint32_t itemArchetypesNumber = 0;
    uint32_t crosshairArchetypesNumber = 0;

    struct ArchView {
        Archetype* inventoryCachedArchetype = nullptr;
        Archetype* itemArchetype = nullptr;
        Archetype* crosshairArchetype = nullptr;
    } archView;

    struct ComponentsView {
        inventory* inventoriesView = nullptr;

        item* itemsView = nullptr;
        collider* itemCollidersView = nullptr;
        transform* itemTransformsView = nullptr;

        transform* crosshairTransforms = nullptr;
    } componentsView;

    uint64_t inventoryRequiredMask =
        (1ull << ComponentsIndices::InventoryComponent);

    uint64_t itemRequiredMask = (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ItemComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent);

    uint64_t crosshairRequiredMask =
        (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::CrosshairTagComponent);

    void Update();
    bool putItem2x2(inventory* inventoryComponent, unsigned int itemEntity);

    CStack* inputStack;
    bool isInventoryOpened;
    int* draggedItemEntity;
    bool* isLeftMouseButtonReleased;
    bool isLeftMouseButtonPressed;
    float mouseOffsetX = 0;
    float mouseOffsetY = 0;
};
} // namespace glvm

namespace glvm {
bool BoxCollider(
    const Vector<float, 3> backtrackingPosition,
    const Vector<float, 3> comparedPosition,
    const float backtrackingScale,
    const float comparedScale,
    const MeshAxisMaxAbsoluteValues& backtrackingMeshAxisMaxAbsoluteValues,
    const MeshAxisMaxAbsoluteValues& comparedMeshAxisMaxAbsoluteValues
);

std::vector<Vector<float, 3>> computeBoxCornerBoundPoints(
    const MeshAxisMaxAbsoluteValues entityChunkBounds,
    Vector<float, 3> entityPosition,
    const float scale
);

template<typename T>
bool isExist(const std::vector<T>& array, const T& element) {
    for (uint32_t i0 = 0; i0 < array.size(); ++i0) {
        if (element == array[i0]) {
            return true;
        }
    }

    return false;
}

void setMeshBounds(MeshAxisLimitingValues meshAxisLimitingValues);
void CreateProjectile(
    const Vector<float, 3>& projectilePosition,
    const Vector<float, 3>& projectileForward,
    const MeshHandle& meshHandle,
    const material& material,
    const damage& damage,
    const EntityLocation& projectileLocation
);
}; // namespace glvm

namespace glvm {
class CMovementSystem: public ISystem {
public:
    float deltaFrameTime;
    float gravity;
    CStack& inputStack;
    float prev_delta_x = 0.0f;
    float prev_X = 0.0f;
    float current_X = 0.0f;
    Vector<float, 3> prev_forward;

    uint32_t playerArchetypesNumber = 0;
    uint32_t rigidBodyContainedArchetypesNumber = 0;

    struct MovementArchView {
        Archetype* playerCachedArchetype = nullptr;
        Archetype* rigidBodyContainedArchetypesCache[32];
    } archView;

    struct MovementComponentsView {
        move* playerMoves = nullptr;
        beholder* playerViews = nullptr;
        colliderFlags* playerColliderFlags = nullptr;
        RigidBody* playerRigidBody = nullptr;

        // Components related to archetypes contains rigid.
        transform* transforms = nullptr;
        RigidBody* rigidBodies = nullptr;
        move* moves = nullptr;
        item* items = nullptr;
    } componentsView;

    uint64_t playerRequiredMask =
        (1ull << ComponentsIndices::PlayerTagComponent);
    uint64_t rigidBodyRequiredMask =
        (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RigidBodyComponent)
        | (1ul << ComponentsIndices::MoveComponent);

    CMovementSystem(CStack& inputStack);

    void Update();
    Vector<float, 3> CalculateVectorRL(beholder& beholder);
    Vector<float, 3> CalculateVectorFB(beholder& beholder, CEvent& event);
};
} // namespace glvm

namespace glvm {
class ProceduralLevelGeneratingSystem: public ISystem {
public:
    unsigned int levelNubmer = 0;
    bool bredoFlag = false;
    unsigned int previous_half_x_rand = 0;
    unsigned int previous_half_z_rand = 0;
    Vector<float, 3> currentLevelPosition = {5.0f, 0.0f, 15.0f};
    Vector<float, 3> transitionBridgePosition = {0.0f, 0.0f, 0.0f};
    unsigned int nextLevelTransitionDirection = 0;
    unsigned int previousIterationTransitionBridgeDirection = 0;

    uint32_t cachedLevelChunkArchNumber = 0;
    uint32_t cachedPlayerArchNumber = 0;

    struct ProceduralLevelArchView {
        Archetype* cachedLevelChunkArch = nullptr;
        Archetype* cachedPlayerArch = nullptr;
    } archView;

    struct ComponentsView {
        transform* playerTransforms = nullptr;
    } componentsView;

    uint64_t playerRequiredMask =
        (1ull << ComponentsIndices::PlayerTagComponent);

    uint64_t requiredMask = (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::MaterialComponent)
        | (1ull << ComponentsIndices::MeshComponent)
        | (1ull << ComponentsIndices::ColliderComponent)
        | (1ull << ComponentsIndices::ColliderFlagsComponent)
        | (1ull << ComponentsIndices::LevelChunkTagComponent);

    std::vector<MeshHandle> meshHandlers;
    std::vector<TextureHandle> textureHandlers;

    std::vector<std::vector<Vertex>> levelGeneratedVertices;
    // Wavefront .obj indices.
    std::vector<std::vector<uint32_t>> levelGeneratedIndices;
    // Keep axis limiting values for every axis per mesh in current iteration
    // while initializing Wavefront .obj and GLTF.
    MeshAxisLimitingValues meshAxisLimitingValues;
    // Contains maximum coordinate value in every direction for all generated
    // levels.
    MeshAxisLimitingValues coordinateMaximumValuePerDirection;

    void Update();
    void setHalfExtentsFromDirection(
        float& halfX,
        float& halfZ,
        const float& transitionBridgeHalfWidth,
        const float& transitionBridgeHalfHeight,
        const float& nextLevelTransitionDirection
    );
    void generateLevel(
        const unsigned int levelHalfX,
        const unsigned int levelHalfY,
        const unsigned int levelHalfZ,
        const float transitionBridgeHalfWidth,
        const float transitionBridgeHalfHeight
    );
    void generateTransitionBridge(
        const unsigned int levelHalfX,
        const unsigned int levelHalfY,
        const unsigned int levelHalfZ,
        const float transitionBridgeHalfWidth,
        const float transitionBridgeHalfHeight
    );
    void makeCubeObjectVertices(
        Vector<float, 4> joinIndices,
        Vector<float, 4> weights,
        float half_x,
        float half_y,
        float half_z,
        std::vector<Vertex>& destinationVerticesContainer
    );
    bool checkCollisionIntersectionWithMaximumCoordinates(
        Vector<float, 3> position,
        float half_x,
        float half_y,
        float half_z
    );
};
} // namespace glvm

namespace glvm {
class CCollisionSystem: public ISystem {
public:
    float fDelta_Time_;
    float gravity;
    bool isInventoryOpened;
    bool* isItemDraged;
    bool isLeftMouseButtonPressed;
    bool* isLeftMouseButtonReleased;
    CStack& Input_Stack_;
    Archetype* cachedArchetypes[32];
    uint32_t cachedArchetypesNumber = 0;

    struct CollisionComponentsView {
        transform* backtrackingTransforms = nullptr;
        collider* backtrackingColliders = nullptr;
        colliderFlags* backtrackingColliderFlags = nullptr;
        mesh* backtrackingMeshes = nullptr;
        move* backtrackingMove = nullptr;
        transform* comparedTransforms = nullptr;
        mesh* comparedMeshes = nullptr;
        move* comparedMove = nullptr;
    } view;

    uint64_t requiredMask = (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);

    CCollisionSystem(CStack& _input_Stack) : Input_Stack_(_input_Stack) {}

    void Update() override;
    bool UpperActorCheck(
        Vector<float, 3> backtrackingPosition,
        Vector<float, 3> comparedPosition,
        float backtrackingScale,
        float comparedScale,
        MeshHandle backtrackingMeshHandle,
        MeshHandle comparedMeshHandle
    );
};
} // namespace glvm

namespace glvm {
class EnemySystem: public ISystem {
public:
    uint32_t playerArchetypesNumber = 0;
    uint32_t enemyArchetypesNumber = 0;
    uint32_t projectileArchetypesNumber = 0;

    struct ArchView {
        Archetype* playerCachedArchetype = nullptr;
        Archetype* enemyCachedArchetype = nullptr;
        Archetype* projectileArchetype = nullptr;
    } archView;

    struct ComponentsView {
        transform* playerTransforms = nullptr;

        transform* enemyTransforms = nullptr;
        state* enemyStates = nullptr;
        enemy* enemies = nullptr;
    } componentsView;

    uint64_t playerRequiredMask =
        (1ull << ComponentsIndices::PlayerTagComponent);

    uint64_t enemyRequiredMask = (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::StateComponent)
        | (1ul << ComponentsIndices::EnemyComponent);

    uint64_t projectileRequiredMask =
        (1ull << ComponentsIndices::ProjectileTagComponent);

    void Update() override;
    ISoundEngine* soundEngine;
    std::vector<TextureHandle> textureHandlers;
    std::vector<MeshHandle> meshHandlers;
    float projectileCooldown = 5.0f;
    float deltaFrameTime;
};
} // namespace glvm

namespace glvm {
template<typename T>
concept UnitOrEnemy =
    std::is_same_v<T, PlayerArchetype> || std::is_same_v<T, EnemyArchetype>;

template<typename T>
concept HasAttack = requires(T* t) {
    { t->attacks };
};

class CProjectileSystem: public ISystem {
public:
    float fYaw = -90.0f;
    float fPitch = 0.0f;
    bool bFirst_Mouse = true;
    CStack& inputStack;
    std::vector<TextureHandle> textureHandlers;
    std::vector<MeshHandle> meshHandlers;
    ISoundEngine* soundEngine;
    float projectileCooldown = 2.0f;
    float deltaFrameTime;
    bool isInventoryOpened;

    uint32_t playerArchetypesNumber = 0;
    uint32_t projectileArchetypesNumber = 0;

    struct ArchView {
        Archetype* playerCachedArchetype = nullptr;
        Archetype* projectileArchetype = nullptr;
    } archView;

    struct ComponentsView {
        transform* playerTransforms = nullptr;
        beholder* playerViews = nullptr;

        transform* projectileTransforms = nullptr;
        colliderFlags* projectileColliderFlags = nullptr;
        collider* projectileColliders = nullptr;
        ProjectileBundle* projectileBundles = nullptr;
        health* projectileHealth = nullptr;
        attack* projectileAttacks = nullptr;
    } componentsView;

    uint64_t playerRequiredMask =
        (1ull << ComponentsIndices::PlayerTagComponent);

    uint64_t projectileRequiredMask =
        (1ull << ComponentsIndices::ProjectileTagComponent);

    CProjectileSystem(CStack& inputStack);
    void Update() override;
    template<typename T>
        requires UnitOrEnemy<T> && HasAttack<T>
    static void markAsAttacked(
        T* arch,
        damage* projectileDamage,
        uint32_t entityIndex
    );
};

template<typename T>
    requires UnitOrEnemy<T> && HasAttack<T>
void CProjectileSystem::markAsAttacked(
    T* arch,
    damage* projectileDamage,
    uint32_t enitityIndex
) {
    arch->attacks[enitityIndex].damage = projectileDamage->maximumDamage;
}

} // namespace glvm

namespace glvm {

class SpatialGridSystem: public ISystem {
    Archetype* cachedArchetypes[32];
    uint32_t cachedArchetypesNumber = 0;
    bool isInitialized = false;

    struct SpatialGridComponentsView {
        transform* transforms = nullptr;
        mesh* meshes = nullptr;
    } view;

    uint64_t requiredMask = (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);

    void Update() override;
};

}; // namespace glvm

namespace glvm {
enum RendererType { VULKAN_RENDERER };

class Engine {
    static Engine* pInstance_;
    static std::mutex Mutex_;

    IChrono* chrono;
    ISoundEngine* soundEngine;
    std::thread sound_thread;
    std::atomic<bool> runningSound {false};
    float deltaFrameTime;
    float gravity;
    bool isLeftMouseButtonPressed;
    std::vector<Texture> textureVector;
    std::vector<const char*> pathsArray_;
    std::vector<const char*> pathsGLTF_;
    uint32_t meshID = 0;
    bool isAlreadyCached;
    bool isInventoryKeyHeld = false;
    bool wasInventoryOpened = false;
    bool isCursorHidden = false;
    float hud_screen_x = 0.0f;
    float hud_screen_y;
    // If don't have any dragged item then this variable have value of -1.
    int draggedItemEntity = -1;
    float fYaw = -90.0f;
    float fPitch = 0.0f;
    float previousMouseOffsetX = 0.0f;
    float previousMouseOffsetY = 0.0f;
    CVulkanRenderer* vulkanRenderer;
    SpatialGridSystem* spatialGridSystem;
    CCollisionSystem* collisionSystem;
    CMovementSystem* movementSystem;
    CPhysicsSystem* physicsSystem;
    CProjectileSystem* projectileSystem;
    DamageSystem* damageSystem;
    EnemySystem* enemySytem;
    ItemSystem* itemSystem;
    ProceduralLevelGeneratingSystem* procuduralLevelGeneratingSystem;
    InventorySystem* inventorySystem;
    Archetype* cachedDirectionalLigthArchetypes[32];
    uint32_t directionalLightArchetypesNumber = 0;
    uint64_t directionalLightRequiredMask =
        (1ul << ComponentsIndices::DirectionalLightComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cachedSpotLigthArchetypes[32];
    uint32_t spotLightArchetypesNumber = 0;
    uint64_t spotLightRequiredMask =
        (1ul << ComponentsIndices::SpotLightComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cachedPointLigthArchetypes[32];
    uint32_t pointLightArchetypesNumber = 0;
    uint64_t pointLightRequiredMask =
        (1ul << ComponentsIndices::PointLightComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cachedAnimationActorsArchetypes[32];
    uint32_t animationActorsArchetypesNumber = 0;
    uint64_t animatedActorsRequiredMask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::AnimationComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cachedStaticActorsArchetypes[32];
    uint32_t staticActorsArchetypesNumber = 0;
    uint64_t staticActorsRequiredMask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::StaticMeshTagComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cachedPlayerArchetypes[32];
    uint32_t playerArchetypesNumber = 0;
    uint64_t playerRequiredMask = (1ul << ComponentsIndices::PlayerTagComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ViewComponent);
    Archetype* cachedAnimationArchetypes[32];
    uint32_t animationArchetypesNumber = 0;
    uint64_t animationRequiredMask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::AnimationComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cachedCrosshairActorsArchetypes[32];
    uint32_t crosshairActorsArchetypesNumber = 0;
    uint64_t crosshairRequiredMask =
        (1ul << ComponentsIndices::CrosshairTagComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cachedLevelChunkActorsArchetypes[32];
    uint32_t levelChunkActorsArchetypesNumber = 0;
    uint64_t levelChunkRequiredMask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::LevelChunkTagComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cachedProjectileActorsArchetypes[32];
    uint32_t projectileActorsArchetypesNumber = 0;
    uint64_t projectileRequiredMask =
        (1ul << ComponentsIndices::ProjectileBundleComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cachedItemActorsArchetypes[32];
    uint32_t itemActorsArchetypesNumber = 0;
    uint64_t rotationItemRequiredMask =
        (1ul << ComponentsIndices::ItemComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MaterialComponent);
    Archetype* cachedInventoryArchetypes[32];
    uint32_t inventoryArchetypesNumber = 0;
    uint64_t inventoryRequiredMask =
        (1ul << ComponentsIndices::InventoryComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MaterialComponent);
    Archetype* cachedItemArchetypes[32];
    uint32_t itemArchetypesNumber = 0;
    uint64_t itemRequiredMask = (1ul << ComponentsIndices::ItemComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::MaterialComponent);
    Archetype* cachedHealthBarsArchetypes[32];
    uint32_t healthBarsArchetypesNumber = 0;
    uint64_t healthBarsRequiredMask =
        (1ul << ComponentsIndices::HealthComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cachedFontsArchetypes[32];
    uint32_t fontsArchetypesNumber = 0;
    uint64_t fontRequiredMask = (1ul << ComponentsIndices::FontComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    double fpsAccumulator = 0;
    Engine();

public:
    std::vector<MeshHandle> meshHandlers;
    std::vector<TextureHandle> textureHandlers;
    uint32_t wavefrontObjCounter = 0;

    ~Engine();

    // Don't need to make copy because of singleton property.
    Engine(Engine& _engine) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const Engine& _engine) = delete;
    // It possibly to get only one instance of this class with this method.
    static Engine* get_instance();
    void GameLoop();
    void EventQueueFlush();
    void RenderVulkan();
    void EnlargeFrameAccumulator(float value);
    void SetViewMatrix();
    void SetProjectionMatrix();
    [[nodiscard]] std::vector<Matrix<float, 4>> updateAnimationFrames(
        animation* animationComponent,
        unsigned int meshID
    );
    Matrix<float, 4> updateDirectionalLightSpaceMatrixShadowMapUBO(
        directionalLight* directionalLightComponent
    );
    Matrix<float, 4> updateSpotLightSpaceMatrixShadowMapUBO(
        spotLight* spotLightComponent
    );
    Matrix<float, 4> updatePointLightSpaceMatrixShadowMapUBO(
        pointLight* pointLightComponent,
        uint32_t layer
    );
    SlotData updateDataUBO_UI(
        const unsigned int currentInventoryRow,
        const unsigned int currentInventoryColumn,
        inventory* inventoryComponent,
        transform* slotTransfromComponent,
        mesh* meshComponent
    );
    Matrix<float, 4> updateDataUBO_IconsUI(
        transform* itemTransfromComponent,
        [[maybe_unused]] collider* itemColliderComponent,
        item* itemComponent,
        const unsigned int rowInventory,
        const unsigned int columnInventory,
        transform* inventoryTransformComponent,
        mesh* itemMesh,
        int itemEntity
    );
    Matrix<float, 4> updateDataHudScreenUBO(transform* cursorTransform);
    void setFrameData();
    void loadWavefrontObj();
    void calculateMeshBounds(const Vector<float, 4>& animatedVertex);
    bool isModelCacheExists(const std::string& modelFilePath);
    void writeModelsCache(const std::string& modelFilePath);
    void initializeGLTF();
    void initializeFontData();
    Matrix<float, 4> computeModelMatrix(
        transform* _transformComponent,
        rotation* rotation
    );
    void computeHudScreeenCoordinates();
    TextureHandle LoadTextureFromFile(const char* path_to_texture);
    auto load_texture_from_address(
        unsigned int iWidth,
        unsigned int iHeight,
        unsigned int dat_length,
        unsigned char* u_iData
    ) -> TextureHandle;
    MeshHandle load_mesh_from_obj(const char* _pathToMesh);
    MeshHandle load_mesh_from_gltf(const char* pathToMesh);
    MeshHandle LoadMesh();
    void GameKill();
};
} // namespace glvm
