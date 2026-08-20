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
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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

namespace glvm {
#define ENTITY_ID_BITS 32

constexpr uint64_t entityBitsMask = (1ull << ENTITY_ID_BITS) - 1;

struct ComponentsIndices {
    enum Types : uint32_t {
        TRANSFORM_COMPONENT,
        RIGID_BODY_COMPONENT,
        MESH_COMPONENT,
        FONT_COMPONENT,
        COLLIDER_COMPONENT,
        COLLIDER_FLAGS_COMPONENT,
        MATERIAL_COMPONENT,
        VIEW_COMPONENT,
        HEALTH_COMPONENT,
        ANIMATION_COMPONENT,
        STATE_COMPONENT,
        ENEMY_COMPONENT,
        DAMAGE_COMPONENT,
        ATTACK_COMPONENT,
        INVENTORY_COMPONENT,
        DIRECTIONAL_LIGHT_COMPONENT,
        SPOT_LIGHT_COMPONENT,
        POINT_LIGHT_COMPONENT,
        ITEM_COMPONENT,
        MOVE_COMPONENT,
        PROJECTILE_BUNDLE_COMPONENT,
        ROTATION_COMPONENT,

        LEVEL_CHUNK_TAG_COMPONENT,
        PLAYER_TAG_COMPONENT,
        CROSSHAIR_TAG_COMPONENT,
        STATIC_MESH_TAG_COMPONENT,
        PROJECTILE_TAG_COMPONENT,

        COMPONENTS_COUNT
    };
};

constexpr uint64_t playerComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::VIEW_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::RIGID_BODY_COMPONENT)
    | (1ull << ComponentsIndices::HEALTH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::MOVE_COMPONENT)
    | (1ull << ComponentsIndices::ATTACK_COMPONENT)
    | (1ull << ComponentsIndices::ANIMATION_COMPONENT)
    | (1ull << ComponentsIndices::FONT_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::PLAYER_TAG_COMPONENT);

constexpr uint64_t enemyComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::ENEMY_COMPONENT)
    | (1ull << ComponentsIndices::STATE_COMPONENT)
    | (1ull << ComponentsIndices::FONT_COMPONENT)
    | (1ull << ComponentsIndices::ANIMATION_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::HEALTH_COMPONENT)
    | (1ull << ComponentsIndices::RIGID_BODY_COMPONENT)
    | (1ull << ComponentsIndices::ATTACK_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::MOVE_COMPONENT);

constexpr uint64_t staticMeshComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::FONT_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::STATIC_MESH_TAG_COMPONENT);

constexpr uint64_t crosshairComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::CROSSHAIR_TAG_COMPONENT);

constexpr uint64_t itemComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::RIGID_BODY_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::MOVE_COMPONENT)
    | (1ull << ComponentsIndices::ITEM_COMPONENT);

constexpr uint64_t inventoryComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::INVENTORY_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT);

constexpr uint64_t directionalLightComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT);

constexpr uint64_t spotLightComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::SPOT_LIGHT_COMPONENT);

constexpr uint64_t pointLightComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::POINT_LIGHT_COMPONENT);

constexpr uint64_t levelChunkComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT);

constexpr uint64_t projectileComponentMask =
    (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
    | (1ull << ComponentsIndices::MESH_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
    | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
    | (1ull << ComponentsIndices::ROTATION_COMPONENT)
    | (1ull << ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT)
    | (1ull << ComponentsIndices::PROJECTILE_TAG_COMPONENT);
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

#define MATRIX_RANGE 16
#define VERTEX_ARRAY_RANGE 180
#define SIZE_OF_VERTEX_DATA 5
#define LAYOUT_0 0
#define LAYOUT_1 1
#define VERTEX_SIZE 3
#define TEXTURE_SIZE 2
#define VERTEX_OFFSET 0
#define TEXTURE_OFFSET 3
#define NUMBER_OF_CREATING_VBO_OBJECT_1 1
#define NUMBER_OF_CREATING_VAO_OBJECT_1 1
#define BASE_ARRAY_COUNTER_VALUE 0
#define BASE_INDEX_VERTEX_ARRAY 0
#define NUMBER_OF_DROWING_VERTEXES 36
#define HOMOGENEOUS_COORDINATE 1
#define NUMBER_OF_MATRICES 1
#define LIMITER 1
#define PI 3.14159265

#define NUMBER_OF_CREATING_TEXTURE_OBJECT_1 1
#define SOME_STRANGE_STUFF 0
#define MIPMAP_LEVEL 0
#define SOME_OLD_STUFF 0

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
/* Generated by wayland-scanner 1.23.1 */

#ifndef POINTER_CONSTRAINTS_UNSTABLE_V1_CLIENT_PROTOCOL_H
#define POINTER_CONSTRAINTS_UNSTABLE_V1_CLIENT_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page page_pointer_constraints_unstable_v1 The pointer_constraints_unstable_v1
 * protocol protocol for constraining pointer motions
 *
 * @section page_desc_pointer_constraints_unstable_v1 Description
 *
 * This protocol specifies a set of interfaces used for adding constraints to
 * the motion of a pointer. Possible constraints include confining pointer
 * motions to a given region, or locking it to its current position.
 *
 * In order to constrain the pointer, a client must first bind the global
 * interface "wp_pointer_constraints" which, if a compositor supports pointer
 * constraints, is exposed by the registry. Using the bound global object, the
 * client uses the request that corresponds to the type of constraint it wants
 * to make. See wp_pointer_constraints for more details.
 *
 * Warning! The protocol described in this file is experimental and backward
 * incompatible changes may be made. Backward compatible changes may be added
 * together with the corresponding interface version bump. Backward
 * incompatible changes are done by bumping the version number in the protocol
 * and interface names and resetting the interface version. Once the protocol
 * is to be declared stable, the 'z' prefix and the version number in the
 * protocol and interface names are removed and the interface version number is
 * reset.
 *
 * @section page_ifaces_pointer_constraints_unstable_v1 Interfaces
 * - @subpage page_iface_zwp_pointer_constraints_v1 - constrain the movement of
 * a pointer
 * - @subpage page_iface_zwp_locked_pointer_v1 - receive relative pointer motion
 * events
 * - @subpage page_iface_zwp_confined_pointer_v1 - confined pointer object
 * @section page_copyright_pointer_constraints_unstable_v1 Copyright
 * <pre>
 *
 * Copyright © 2014      Jonas Ådahl
 * Copyright © 2015      Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * </pre>
 */
struct wl_pointer;
struct wl_region;
struct wl_surface;
struct zwp_confined_pointer_v1;
struct zwp_locked_pointer_v1;
struct zwp_pointer_constraints_v1;

#ifndef ZWP_POINTER_CONSTRAINTS_V1_INTERFACE
#define ZWP_POINTER_CONSTRAINTS_V1_INTERFACE
/**
 * @page page_iface_zwp_pointer_constraints_v1 zwp_pointer_constraints_v1
 * @section page_iface_zwp_pointer_constraints_v1_desc Description
 *
 * The global interface exposing pointer constraining functionality. It
 * exposes two requests: lock_pointer for locking the pointer to its
 * position, and confine_pointer for locking the pointer to a region.
 *
 * The lock_pointer and confine_pointer requests create the objects
 * wp_locked_pointer and wp_confined_pointer respectively, and the client can
 * use these objects to interact with the lock.
 *
 * For any surface, only one lock or confinement may be active across all
 * wl_pointer objects of the same seat. If a lock or confinement is requested
 * when another lock or confinement is active or requested on the same surface
 * and with any of the wl_pointer objects of the same seat, an
 * 'already_constrained' error will be raised.
 * @section page_iface_zwp_pointer_constraints_v1_api API
 * See @ref iface_zwp_pointer_constraints_v1.
 */
/**
 * @defgroup iface_zwp_pointer_constraints_v1 The zwp_pointer_constraints_v1
 * interface
 *
 * The global interface exposing pointer constraining functionality. It
 * exposes two requests: lock_pointer for locking the pointer to its
 * position, and confine_pointer for locking the pointer to a region.
 *
 * The lock_pointer and confine_pointer requests create the objects
 * wp_locked_pointer and wp_confined_pointer respectively, and the client can
 * use these objects to interact with the lock.
 *
 * For any surface, only one lock or confinement may be active across all
 * wl_pointer objects of the same seat. If a lock or confinement is requested
 * when another lock or confinement is active or requested on the same surface
 * and with any of the wl_pointer objects of the same seat, an
 * 'already_constrained' error will be raised.
 */
extern const struct wl_interface zwp_pointer_constraints_v1_interface;
#endif
#ifndef ZWP_LOCKED_POINTER_V1_INTERFACE
#define ZWP_LOCKED_POINTER_V1_INTERFACE
/**
 * @page page_iface_zwp_locked_pointer_v1 zwp_locked_pointer_v1
 * @section page_iface_zwp_locked_pointer_v1_desc Description
 *
 * The wp_locked_pointer interface represents a locked pointer state.
 *
 * While the lock of this object is active, the wl_pointer objects of the
 * associated seat will not emit any wl_pointer.motion events.
 *
 * This object will send the event 'locked' when the lock is activated.
 * Whenever the lock is activated, it is guaranteed that the locked surface
 * will already have received pointer focus and that the pointer will be
 * within the region passed to the request creating this object.
 *
 * To unlock the pointer, send the destroy request. This will also destroy
 * the wp_locked_pointer object.
 *
 * If the compositor decides to unlock the pointer the unlocked event is
 * sent. See wp_locked_pointer.unlock for details.
 *
 * When unlocking, the compositor may warp the cursor position to the set
 * cursor position hint. If it does, it will not result in any relative
 * motion events emitted via wp_relative_pointer.
 *
 * If the surface the lock was requested on is destroyed and the lock is not
 * yet activated, the wp_locked_pointer object is now defunct and must be
 * destroyed.
 * @section page_iface_zwp_locked_pointer_v1_api API
 * See @ref iface_zwp_locked_pointer_v1.
 */
/**
 * @defgroup iface_zwp_locked_pointer_v1 The zwp_locked_pointer_v1 interface
 *
 * The wp_locked_pointer interface represents a locked pointer state.
 *
 * While the lock of this object is active, the wl_pointer objects of the
 * associated seat will not emit any wl_pointer.motion events.
 *
 * This object will send the event 'locked' when the lock is activated.
 * Whenever the lock is activated, it is guaranteed that the locked surface
 * will already have received pointer focus and that the pointer will be
 * within the region passed to the request creating this object.
 *
 * To unlock the pointer, send the destroy request. This will also destroy
 * the wp_locked_pointer object.
 *
 * If the compositor decides to unlock the pointer the unlocked event is
 * sent. See wp_locked_pointer.unlock for details.
 *
 * When unlocking, the compositor may warp the cursor position to the set
 * cursor position hint. If it does, it will not result in any relative
 * motion events emitted via wp_relative_pointer.
 *
 * If the surface the lock was requested on is destroyed and the lock is not
 * yet activated, the wp_locked_pointer object is now defunct and must be
 * destroyed.
 */
extern const struct wl_interface zwp_locked_pointer_v1_interface;
#endif
#ifndef ZWP_CONFINED_POINTER_V1_INTERFACE
#define ZWP_CONFINED_POINTER_V1_INTERFACE
/**
 * @page page_iface_zwp_confined_pointer_v1 zwp_confined_pointer_v1
 * @section page_iface_zwp_confined_pointer_v1_desc Description
 *
 * The wp_confined_pointer interface represents a confined pointer state.
 *
 * This object will send the event 'confined' when the confinement is
 * activated. Whenever the confinement is activated, it is guaranteed that
 * the surface the pointer is confined to will already have received pointer
 * focus and that the pointer will be within the region passed to the request
 * creating this object. It is up to the compositor to decide whether this
 * requires some user interaction and if the pointer will warp to within the
 * passed region if outside.
 *
 * To unconfine the pointer, send the destroy request. This will also destroy
 * the wp_confined_pointer object.
 *
 * If the compositor decides to unconfine the pointer the unconfined event is
 * sent. The wp_confined_pointer object is at this point defunct and should
 * be destroyed.
 * @section page_iface_zwp_confined_pointer_v1_api API
 * See @ref iface_zwp_confined_pointer_v1.
 */
/**
 * @defgroup iface_zwp_confined_pointer_v1 The zwp_confined_pointer_v1 interface
 *
 * The wp_confined_pointer interface represents a confined pointer state.
 *
 * This object will send the event 'confined' when the confinement is
 * activated. Whenever the confinement is activated, it is guaranteed that
 * the surface the pointer is confined to will already have received pointer
 * focus and that the pointer will be within the region passed to the request
 * creating this object. It is up to the compositor to decide whether this
 * requires some user interaction and if the pointer will warp to within the
 * passed region if outside.
 *
 * To unconfine the pointer, send the destroy request. This will also destroy
 * the wp_confined_pointer object.
 *
 * If the compositor decides to unconfine the pointer the unconfined event is
 * sent. The wp_confined_pointer object is at this point defunct and should
 * be destroyed.
 */
extern const struct wl_interface zwp_confined_pointer_v1_interface;
#endif

#ifndef ZWP_POINTER_CONSTRAINTS_V1_ERROR_ENUM
#define ZWP_POINTER_CONSTRAINTS_V1_ERROR_ENUM

/**
 * @ingroup iface_zwp_pointer_constraints_v1
 * wp_pointer_constraints error values
 *
 * These errors can be emitted in response to wp_pointer_constraints
 * requests.
 */
enum zwp_pointer_constraints_v1_error {
    /**
     * pointer constraint already requested on that surface
     */
    ZWP_POINTER_CONSTRAINTS_V1_ERROR_ALREADY_CONSTRAINED = 1,
};
#endif /* ZWP_POINTER_CONSTRAINTS_V1_ERROR_ENUM */

#ifndef ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ENUM
#define ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ENUM

/**
 * @ingroup iface_zwp_pointer_constraints_v1
 * constraint lifetime
 *
 * These values represent different lifetime semantics. They are passed
 * as arguments to the factory requests to specify how the constraint
 * lifetimes should be managed.
 */
enum zwp_pointer_constraints_v1_lifetime {
    /**
     * the pointer constraint is defunct once deactivated
     *
     * A oneshot pointer constraint will never reactivate once it has
     * been deactivated. See the corresponding deactivation event
     * (wp_locked_pointer.unlocked and wp_confined_pointer.unconfined)
     * for details.
     */
    ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT = 1,
    /**
     * the pointer constraint may reactivate
     *
     * A persistent pointer constraint may again reactivate once it
     * has been deactivated. See the corresponding deactivation event
     * (wp_locked_pointer.unlocked and wp_confined_pointer.unconfined)
     * for details.
     */
    ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT = 2,
};
#endif /* ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ENUM */

#define ZWP_POINTER_CONSTRAINTS_V1_DESTROY 0
#define ZWP_POINTER_CONSTRAINTS_V1_LOCK_POINTER 1
#define ZWP_POINTER_CONSTRAINTS_V1_CONFINE_POINTER 2

/**
 * @ingroup iface_zwp_pointer_constraints_v1
 */
#define ZWP_POINTER_CONSTRAINTS_V1_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_zwp_pointer_constraints_v1
 */
#define ZWP_POINTER_CONSTRAINTS_V1_LOCK_POINTER_SINCE_VERSION 1
/**
 * @ingroup iface_zwp_pointer_constraints_v1
 */
#define ZWP_POINTER_CONSTRAINTS_V1_CONFINE_POINTER_SINCE_VERSION 1

/** @ingroup iface_zwp_pointer_constraints_v1 */
inline static void zwp_pointer_constraints_v1_set_user_data(
    struct zwp_pointer_constraints_v1* zwp_pointer_constraints_v1,
    void* user_data
) {
    wl_proxy_set_user_data(
        (struct wl_proxy*)zwp_pointer_constraints_v1,
        user_data
    );
}

/** @ingroup iface_zwp_pointer_constraints_v1 */
inline static void* zwp_pointer_constraints_v1_get_user_data(
    struct zwp_pointer_constraints_v1* zwp_pointer_constraints_v1
) {
    return wl_proxy_get_user_data((struct wl_proxy*)zwp_pointer_constraints_v1);
}

inline static uint32_t zwp_pointer_constraints_v1_get_version(
    struct zwp_pointer_constraints_v1* zwp_pointer_constraints_v1
) {
    return wl_proxy_get_version((struct wl_proxy*)zwp_pointer_constraints_v1);
}

/**
 * @ingroup iface_zwp_pointer_constraints_v1
 *
 * Used by the client to notify the server that it will no longer use this
 * pointer constraints object.
 */
inline static void zwp_pointer_constraints_v1_destroy(
    struct zwp_pointer_constraints_v1* zwp_pointer_constraints_v1
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_pointer_constraints_v1,
        ZWP_POINTER_CONSTRAINTS_V1_DESTROY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)zwp_pointer_constraints_v1),
        WL_MARSHAL_FLAG_DESTROY
    );
}

/**
 * @ingroup iface_zwp_pointer_constraints_v1
 *
 * The lock_pointer request lets the client request to disable movements of
 * the virtual pointer (i.e. the cursor), effectively locking the pointer
 * to a position. This request may not take effect immediately; in the
 * future, when the compositor deems implementation-specific constraints
 * are satisfied, the pointer lock will be activated and the compositor
 * sends a locked event.
 *
 * The protocol provides no guarantee that the constraints are ever
 * satisfied, and does not require the compositor to send an error if the
 * constraints cannot ever be satisfied. It is thus possible to request a
 * lock that will never activate.
 *
 * There may not be another pointer constraint of any kind requested or
 * active on the surface for any of the wl_pointer objects of the seat of
 * the passed pointer when requesting a lock. If there is, an error will be
 * raised. See general pointer lock documentation for more details.
 *
 * The intersection of the region passed with this request and the input
 * region of the surface is used to determine where the pointer must be
 * in order for the lock to activate. It is up to the compositor whether to
 * warp the pointer or require some kind of user interaction for the lock
 * to activate. If the region is null the surface input region is used.
 *
 * A surface may receive pointer focus without the lock being activated.
 *
 * The request creates a new object wp_locked_pointer which is used to
 * interact with the lock as well as receive updates about its state. See
 * the the description of wp_locked_pointer for further information.
 *
 * Note that while a pointer is locked, the wl_pointer objects of the
 * corresponding seat will not emit any wl_pointer.motion events, but
 * relative motion events will still be emitted via wp_relative_pointer
 * objects of the same seat. wl_pointer.axis and wl_pointer.button events
 * are unaffected.
 */
inline static struct zwp_locked_pointer_v1* zwp_pointer_constraints_v1_lock_pointer(
    struct zwp_pointer_constraints_v1* zwp_pointer_constraints_v1,
    struct wl_surface* surface,
    struct wl_pointer* pointer,
    struct wl_region* region,
    uint32_t lifetime
) {
    struct wl_proxy* id;

    id = wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_pointer_constraints_v1,
        ZWP_POINTER_CONSTRAINTS_V1_LOCK_POINTER,
        &zwp_locked_pointer_v1_interface,
        wl_proxy_get_version((struct wl_proxy*)zwp_pointer_constraints_v1),
        0,
        NULL,
        surface,
        pointer,
        region,
        lifetime
    );

    return (struct zwp_locked_pointer_v1*)id;
}

/**
 * @ingroup iface_zwp_pointer_constraints_v1
 *
 * The confine_pointer request lets the client request to confine the
 * pointer cursor to a given region. This request may not take effect
 * immediately; in the future, when the compositor deems implementation-
 * specific constraints are satisfied, the pointer confinement will be
 * activated and the compositor sends a confined event.
 *
 * The intersection of the region passed with this request and the input
 * region of the surface is used to determine where the pointer must be
 * in order for the confinement to activate. It is up to the compositor
 * whether to warp the pointer or require some kind of user interaction for
 * the confinement to activate. If the region is null the surface input
 * region is used.
 *
 * The request will create a new object wp_confined_pointer which is used
 * to interact with the confinement as well as receive updates about its
 * state. See the the description of wp_confined_pointer for further
 * information.
 */
inline static struct zwp_confined_pointer_v1*
zwp_pointer_constraints_v1_confine_pointer(
    struct zwp_pointer_constraints_v1* zwp_pointer_constraints_v1,
    struct wl_surface* surface,
    struct wl_pointer* pointer,
    struct wl_region* region,
    uint32_t lifetime
) {
    struct wl_proxy* id;

    id = wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_pointer_constraints_v1,
        ZWP_POINTER_CONSTRAINTS_V1_CONFINE_POINTER,
        &zwp_confined_pointer_v1_interface,
        wl_proxy_get_version((struct wl_proxy*)zwp_pointer_constraints_v1),
        0,
        NULL,
        surface,
        pointer,
        region,
        lifetime
    );

    return (struct zwp_confined_pointer_v1*)id;
}

/**
 * @ingroup iface_zwp_locked_pointer_v1
 * @struct zwp_locked_pointer_v1_listener
 */
struct zwp_locked_pointer_v1_listener {
    /**
     * lock activation event
     *
     * Notification that the pointer lock of the seat's pointer is
     * activated.
     */
    void (*locked)(
        void* data,
        struct zwp_locked_pointer_v1* zwp_locked_pointer_v1
    );
    /**
     * lock deactivation event
     *
     * Notification that the pointer lock of the seat's pointer is no
     * longer active. If this is a oneshot pointer lock (see
     * wp_pointer_constraints.lifetime) this object is now defunct and
     * should be destroyed. If this is a persistent pointer lock (see
     * wp_pointer_constraints.lifetime) this pointer lock may again
     * reactivate in the future.
     */
    void (*unlocked)(
        void* data,
        struct zwp_locked_pointer_v1* zwp_locked_pointer_v1
    );
};

/**
 * @ingroup iface_zwp_locked_pointer_v1
 */
inline static int zwp_locked_pointer_v1_add_listener(
    struct zwp_locked_pointer_v1* zwp_locked_pointer_v1,
    const struct zwp_locked_pointer_v1_listener* listener,
    void* data
) {
    return wl_proxy_add_listener(
        (struct wl_proxy*)zwp_locked_pointer_v1,
        (void (**)(void))listener,
        data
    );
}

#define ZWP_LOCKED_POINTER_V1_DESTROY 0
#define ZWP_LOCKED_POINTER_V1_SET_CURSOR_POSITION_HINT 1
#define ZWP_LOCKED_POINTER_V1_SET_REGION 2

/**
 * @ingroup iface_zwp_locked_pointer_v1
 */
#define ZWP_LOCKED_POINTER_V1_LOCKED_SINCE_VERSION 1
/**
 * @ingroup iface_zwp_locked_pointer_v1
 */
#define ZWP_LOCKED_POINTER_V1_UNLOCKED_SINCE_VERSION 1

/**
 * @ingroup iface_zwp_locked_pointer_v1
 */
#define ZWP_LOCKED_POINTER_V1_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_zwp_locked_pointer_v1
 */
#define ZWP_LOCKED_POINTER_V1_SET_CURSOR_POSITION_HINT_SINCE_VERSION 1
/**
 * @ingroup iface_zwp_locked_pointer_v1
 */
#define ZWP_LOCKED_POINTER_V1_SET_REGION_SINCE_VERSION 1

/** @ingroup iface_zwp_locked_pointer_v1 */
inline static void zwp_locked_pointer_v1_set_user_data(
    struct zwp_locked_pointer_v1* zwp_locked_pointer_v1,
    void* user_data
) {
    wl_proxy_set_user_data((struct wl_proxy*)zwp_locked_pointer_v1, user_data);
}

/** @ingroup iface_zwp_locked_pointer_v1 */
inline static void* zwp_locked_pointer_v1_get_user_data(
    struct zwp_locked_pointer_v1* zwp_locked_pointer_v1
) {
    return wl_proxy_get_user_data((struct wl_proxy*)zwp_locked_pointer_v1);
}

inline static uint32_t zwp_locked_pointer_v1_get_version(
    struct zwp_locked_pointer_v1* zwp_locked_pointer_v1
) {
    return wl_proxy_get_version((struct wl_proxy*)zwp_locked_pointer_v1);
}

/**
 * @ingroup iface_zwp_locked_pointer_v1
 *
 * Destroy the locked pointer object. If applicable, the compositor will
 * unlock the pointer.
 */
inline static void zwp_locked_pointer_v1_destroy(
    struct zwp_locked_pointer_v1* zwp_locked_pointer_v1
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_locked_pointer_v1,
        ZWP_LOCKED_POINTER_V1_DESTROY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)zwp_locked_pointer_v1),
        WL_MARSHAL_FLAG_DESTROY
    );
}

/**
 * @ingroup iface_zwp_locked_pointer_v1
 *
 * Set the cursor position hint relative to the top left corner of the
 * surface.
 *
 * If the client is drawing its own cursor, it should update the position
 * hint to the position of its own cursor. A compositor may use this
 * information to warp the pointer upon unlock in order to avoid pointer
 * jumps.
 *
 * The cursor position hint is double-buffered state, see
 * wl_surface.commit.
 */
inline static void zwp_locked_pointer_v1_set_cursor_position_hint(
    struct zwp_locked_pointer_v1* zwp_locked_pointer_v1,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_locked_pointer_v1,
        ZWP_LOCKED_POINTER_V1_SET_CURSOR_POSITION_HINT,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)zwp_locked_pointer_v1),
        0,
        surface_x,
        surface_y
    );
}

/**
 * @ingroup iface_zwp_locked_pointer_v1
 *
 * Set a new region used to lock the pointer.
 *
 * The new lock region is double-buffered, see wl_surface.commit.
 *
 * For details about the lock region, see wp_locked_pointer.
 */
inline static void zwp_locked_pointer_v1_set_region(
    struct zwp_locked_pointer_v1* zwp_locked_pointer_v1,
    struct wl_region* region
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_locked_pointer_v1,
        ZWP_LOCKED_POINTER_V1_SET_REGION,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)zwp_locked_pointer_v1),
        0,
        region
    );
}

/**
 * @ingroup iface_zwp_confined_pointer_v1
 * @struct zwp_confined_pointer_v1_listener
 */
struct zwp_confined_pointer_v1_listener {
    /**
     * pointer confined
     *
     * Notification that the pointer confinement of the seat's
     * pointer is activated.
     */
    void (*confined)(
        void* data,
        struct zwp_confined_pointer_v1* zwp_confined_pointer_v1
    );
    /**
     * pointer unconfined
     *
     * Notification that the pointer confinement of the seat's
     * pointer is no longer active. If this is a oneshot pointer
     * confinement (see wp_pointer_constraints.lifetime) this object is
     * now defunct and should be destroyed. If this is a persistent
     * pointer confinement (see wp_pointer_constraints.lifetime) this
     * pointer confinement may again reactivate in the future.
     */
    void (*unconfined)(
        void* data,
        struct zwp_confined_pointer_v1* zwp_confined_pointer_v1
    );
};

/**
 * @ingroup iface_zwp_confined_pointer_v1
 */
inline static int zwp_confined_pointer_v1_add_listener(
    struct zwp_confined_pointer_v1* zwp_confined_pointer_v1,
    const struct zwp_confined_pointer_v1_listener* listener,
    void* data
) {
    return wl_proxy_add_listener(
        (struct wl_proxy*)zwp_confined_pointer_v1,
        (void (**)(void))listener,
        data
    );
}

#define ZWP_CONFINED_POINTER_V1_DESTROY 0
#define ZWP_CONFINED_POINTER_V1_SET_REGION 1

/**
 * @ingroup iface_zwp_confined_pointer_v1
 */
#define ZWP_CONFINED_POINTER_V1_CONFINED_SINCE_VERSION 1
/**
 * @ingroup iface_zwp_confined_pointer_v1
 */
#define ZWP_CONFINED_POINTER_V1_UNCONFINED_SINCE_VERSION 1

/**
 * @ingroup iface_zwp_confined_pointer_v1
 */
#define ZWP_CONFINED_POINTER_V1_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_zwp_confined_pointer_v1
 */
#define ZWP_CONFINED_POINTER_V1_SET_REGION_SINCE_VERSION 1

/** @ingroup iface_zwp_confined_pointer_v1 */
inline static void zwp_confined_pointer_v1_set_user_data(
    struct zwp_confined_pointer_v1* zwp_confined_pointer_v1,
    void* user_data
) {
    wl_proxy_set_user_data((struct wl_proxy*)zwp_confined_pointer_v1, user_data);
}

/** @ingroup iface_zwp_confined_pointer_v1 */
inline static void* zwp_confined_pointer_v1_get_user_data(
    struct zwp_confined_pointer_v1* zwp_confined_pointer_v1
) {
    return wl_proxy_get_user_data((struct wl_proxy*)zwp_confined_pointer_v1);
}

inline static uint32_t zwp_confined_pointer_v1_get_version(
    struct zwp_confined_pointer_v1* zwp_confined_pointer_v1
) {
    return wl_proxy_get_version((struct wl_proxy*)zwp_confined_pointer_v1);
}

/**
 * @ingroup iface_zwp_confined_pointer_v1
 *
 * Destroy the confined pointer object. If applicable, the compositor will
 * unconfine the pointer.
 */
inline static void zwp_confined_pointer_v1_destroy(
    struct zwp_confined_pointer_v1* zwp_confined_pointer_v1
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_confined_pointer_v1,
        ZWP_CONFINED_POINTER_V1_DESTROY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)zwp_confined_pointer_v1),
        WL_MARSHAL_FLAG_DESTROY
    );
}

/**
 * @ingroup iface_zwp_confined_pointer_v1
 *
 * Set a new region used to confine the pointer.
 *
 * The new confine region is double-buffered, see wl_surface.commit.
 *
 * If the confinement is active when the new confinement region is applied
 * and the pointer ends up outside of newly applied region, the pointer may
 * warped to a position within the new confinement region. If warped, a
 * wl_pointer.motion event will be emitted, but no
 * wp_relative_pointer.relative_motion event.
 *
 * The compositor may also, instead of using the new region, unconfine the
 * pointer.
 *
 * For details about the confine region, see wp_confined_pointer.
 */
inline static void zwp_confined_pointer_v1_set_region(
    struct zwp_confined_pointer_v1* zwp_confined_pointer_v1,
    struct wl_region* region
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_confined_pointer_v1,
        ZWP_CONFINED_POINTER_V1_SET_REGION,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)zwp_confined_pointer_v1),
        0,
        region
    );
}

#ifdef __cplusplus
}
#endif

#endif
#endif // __linux__

#ifdef __linux__
/* Generated by wayland-scanner 1.23.1 */

#ifndef RELATIVE_POINTER_UNSTABLE_V1_CLIENT_PROTOCOL_H
#define RELATIVE_POINTER_UNSTABLE_V1_CLIENT_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page page_relative_pointer_unstable_v1 The relative_pointer_unstable_v1
 * protocol protocol for relative pointer motion events
 *
 * @section page_desc_relative_pointer_unstable_v1 Description
 *
 * This protocol specifies a set of interfaces used for making clients able to
 * receive relative pointer events not obstructed by barriers (such as the
 * monitor edge or other pointer barriers).
 *
 * To start receiving relative pointer events, a client must first bind the
 * global interface "wp_relative_pointer_manager" which, if a compositor
 * supports relative pointer motion events, is exposed by the registry. After
 * having created the relative pointer manager proxy object, the client uses
 * it to create the actual relative pointer object using the
 * "get_relative_pointer" request given a wl_pointer. The relative pointer
 * motion events will then, when applicable, be transmitted via the proxy of
 * the newly created relative pointer object. See the documentation of the
 * relative pointer interface for more details.
 *
 * Warning! The protocol described in this file is experimental and backward
 * incompatible changes may be made. Backward compatible changes may be added
 * together with the corresponding interface version bump. Backward
 * incompatible changes are done by bumping the version number in the protocol
 * and interface names and resetting the interface version. Once the protocol
 * is to be declared stable, the 'z' prefix and the version number in the
 * protocol and interface names are removed and the interface version number is
 * reset.
 *
 * @section page_ifaces_relative_pointer_unstable_v1 Interfaces
 * - @subpage page_iface_zwp_relative_pointer_manager_v1 - get relative pointer
 * objects
 * - @subpage page_iface_zwp_relative_pointer_v1 - relative pointer object
 * @section page_copyright_relative_pointer_unstable_v1 Copyright
 * <pre>
 *
 * Copyright © 2014      Jonas Ådahl
 * Copyright © 2015      Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * </pre>
 */
struct wl_pointer;
struct zwp_relative_pointer_manager_v1;
struct zwp_relative_pointer_v1;

#ifndef ZWP_RELATIVE_POINTER_MANAGER_V1_INTERFACE
#define ZWP_RELATIVE_POINTER_MANAGER_V1_INTERFACE
/**
 * @page page_iface_zwp_relative_pointer_manager_v1
 * zwp_relative_pointer_manager_v1
 * @section page_iface_zwp_relative_pointer_manager_v1_desc Description
 *
 * A global interface used for getting the relative pointer object for a
 * given pointer.
 * @section page_iface_zwp_relative_pointer_manager_v1_api API
 * See @ref iface_zwp_relative_pointer_manager_v1.
 */
/**
 * @defgroup iface_zwp_relative_pointer_manager_v1 The
 * zwp_relative_pointer_manager_v1 interface
 *
 * A global interface used for getting the relative pointer object for a
 * given pointer.
 */
extern const struct wl_interface zwp_relative_pointer_manager_v1_interface;
#endif
#ifndef ZWP_RELATIVE_POINTER_V1_INTERFACE
#define ZWP_RELATIVE_POINTER_V1_INTERFACE
/**
 * @page page_iface_zwp_relative_pointer_v1 zwp_relative_pointer_v1
 * @section page_iface_zwp_relative_pointer_v1_desc Description
 *
 * A wp_relative_pointer object is an extension to the wl_pointer interface
 * used for emitting relative pointer events. It shares the same focus as
 * wl_pointer objects of the same seat and will only emit events when it has
 * focus.
 * @section page_iface_zwp_relative_pointer_v1_api API
 * See @ref iface_zwp_relative_pointer_v1.
 */
/**
 * @defgroup iface_zwp_relative_pointer_v1 The zwp_relative_pointer_v1 interface
 *
 * A wp_relative_pointer object is an extension to the wl_pointer interface
 * used for emitting relative pointer events. It shares the same focus as
 * wl_pointer objects of the same seat and will only emit events when it has
 * focus.
 */
extern const struct wl_interface zwp_relative_pointer_v1_interface;
#endif

#define ZWP_RELATIVE_POINTER_MANAGER_V1_DESTROY 0
#define ZWP_RELATIVE_POINTER_MANAGER_V1_GET_RELATIVE_POINTER 1

/**
 * @ingroup iface_zwp_relative_pointer_manager_v1
 */
#define ZWP_RELATIVE_POINTER_MANAGER_V1_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_zwp_relative_pointer_manager_v1
 */
#define ZWP_RELATIVE_POINTER_MANAGER_V1_GET_RELATIVE_POINTER_SINCE_VERSION 1

/** @ingroup iface_zwp_relative_pointer_manager_v1 */
inline static void zwp_relative_pointer_manager_v1_set_user_data(
    struct zwp_relative_pointer_manager_v1* zwp_relative_pointer_manager_v1,
    void* user_data
) {
    wl_proxy_set_user_data(
        (struct wl_proxy*)zwp_relative_pointer_manager_v1,
        user_data
    );
}

/** @ingroup iface_zwp_relative_pointer_manager_v1 */
inline static void* zwp_relative_pointer_manager_v1_get_user_data(
    struct zwp_relative_pointer_manager_v1* zwp_relative_pointer_manager_v1
) {
    return wl_proxy_get_user_data(
        (struct wl_proxy*)zwp_relative_pointer_manager_v1
    );
}

inline static uint32_t zwp_relative_pointer_manager_v1_get_version(
    struct zwp_relative_pointer_manager_v1* zwp_relative_pointer_manager_v1
) {
    return wl_proxy_get_version(
        (struct wl_proxy*)zwp_relative_pointer_manager_v1
    );
}

/**
 * @ingroup iface_zwp_relative_pointer_manager_v1
 *
 * Used by the client to notify the server that it will no longer use this
 * relative pointer manager object.
 */
inline static void zwp_relative_pointer_manager_v1_destroy(
    struct zwp_relative_pointer_manager_v1* zwp_relative_pointer_manager_v1
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_relative_pointer_manager_v1,
        ZWP_RELATIVE_POINTER_MANAGER_V1_DESTROY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)zwp_relative_pointer_manager_v1),
        WL_MARSHAL_FLAG_DESTROY
    );
}

/**
 * @ingroup iface_zwp_relative_pointer_manager_v1
 *
 * Create a relative pointer interface given a wl_pointer object. See the
 * wp_relative_pointer interface for more details.
 */
inline static struct zwp_relative_pointer_v1*
zwp_relative_pointer_manager_v1_get_relative_pointer(
    struct zwp_relative_pointer_manager_v1* zwp_relative_pointer_manager_v1,
    struct wl_pointer* pointer
) {
    struct wl_proxy* id;

    id = wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_relative_pointer_manager_v1,
        ZWP_RELATIVE_POINTER_MANAGER_V1_GET_RELATIVE_POINTER,
        &zwp_relative_pointer_v1_interface,
        wl_proxy_get_version((struct wl_proxy*)zwp_relative_pointer_manager_v1),
        0,
        NULL,
        pointer
    );

    return (struct zwp_relative_pointer_v1*)id;
}

/**
 * @ingroup iface_zwp_relative_pointer_v1
 * @struct zwp_relative_pointer_v1_listener
 */
struct zwp_relative_pointer_v1_listener {
    /**
     * relative pointer motion
     *
     * Relative x/y pointer motion from the pointer of the seat
     * associated with this object.
     *
     * A relative motion is in the same dimension as regular wl_pointer
     * motion events, except they do not represent an absolute
     * position. For example, moving a pointer from (x, y) to (x', y')
     * would have the equivalent relative motion (x' - x, y' - y). If a
     * pointer motion caused the absolute pointer position to be
     * clipped by for example the edge of the monitor, the relative
     * motion is unaffected by the clipping and will represent the
     * unclipped motion.
     *
     * This event also contains non-accelerated motion deltas. The
     * non-accelerated delta is, when applicable, the regular pointer
     * motion delta as it was before having applied motion acceleration
     * and other transformations such as normalization.
     *
     * Note that the non-accelerated delta does not represent 'raw'
     * events as they were read from some device. Pointer motion
     * acceleration is device- and configuration-specific and
     * non-accelerated deltas and accelerated deltas may have the same
     * value on some devices.
     *
     * Relative motions are not coupled to wl_pointer.motion events,
     * and can be sent in combination with such events, but also
     * independently. There may also be scenarios where
     * wl_pointer.motion is sent, but there is no relative motion. The
     * order of an absolute and relative motion event originating from
     * the same physical motion is not guaranteed.
     *
     * If the client needs button events or focus state, it can receive
     * them from a wl_pointer object of the same seat that the
     * wp_relative_pointer object is associated with.
     * @param utime_hi high 32 bits of a 64 bit timestamp with microsecond
     * granularity
     * @param utime_lo low 32 bits of a 64 bit timestamp with microsecond
     * granularity
     * @param dx the x component of the motion vector
     * @param dy the y component of the motion vector
     * @param dx_unaccel the x component of the unaccelerated motion vector
     * @param dy_unaccel the y component of the unaccelerated motion vector
     */
    void (*relative_motion)(
        void* data,
        struct zwp_relative_pointer_v1* zwp_relative_pointer_v1,
        uint32_t utime_hi,
        uint32_t utime_lo,
        wl_fixed_t dx,
        wl_fixed_t dy,
        wl_fixed_t dx_unaccel,
        wl_fixed_t dy_unaccel
    );
};

/**
 * @ingroup iface_zwp_relative_pointer_v1
 */
inline static int zwp_relative_pointer_v1_add_listener(
    struct zwp_relative_pointer_v1* zwp_relative_pointer_v1,
    const struct zwp_relative_pointer_v1_listener* listener,
    void* data
) {
    return wl_proxy_add_listener(
        (struct wl_proxy*)zwp_relative_pointer_v1,
        (void (**)(void))listener,
        data
    );
}

#define ZWP_RELATIVE_POINTER_V1_DESTROY 0

/**
 * @ingroup iface_zwp_relative_pointer_v1
 */
#define ZWP_RELATIVE_POINTER_V1_RELATIVE_MOTION_SINCE_VERSION 1

/**
 * @ingroup iface_zwp_relative_pointer_v1
 */
#define ZWP_RELATIVE_POINTER_V1_DESTROY_SINCE_VERSION 1

/** @ingroup iface_zwp_relative_pointer_v1 */
inline static void zwp_relative_pointer_v1_set_user_data(
    struct zwp_relative_pointer_v1* zwp_relative_pointer_v1,
    void* user_data
) {
    wl_proxy_set_user_data((struct wl_proxy*)zwp_relative_pointer_v1, user_data);
}

/** @ingroup iface_zwp_relative_pointer_v1 */
inline static void* zwp_relative_pointer_v1_get_user_data(
    struct zwp_relative_pointer_v1* zwp_relative_pointer_v1
) {
    return wl_proxy_get_user_data((struct wl_proxy*)zwp_relative_pointer_v1);
}

inline static uint32_t zwp_relative_pointer_v1_get_version(
    struct zwp_relative_pointer_v1* zwp_relative_pointer_v1
) {
    return wl_proxy_get_version((struct wl_proxy*)zwp_relative_pointer_v1);
}

/**
 * @ingroup iface_zwp_relative_pointer_v1
 */
inline static void zwp_relative_pointer_v1_destroy(
    struct zwp_relative_pointer_v1* zwp_relative_pointer_v1
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)zwp_relative_pointer_v1,
        ZWP_RELATIVE_POINTER_V1_DESTROY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)zwp_relative_pointer_v1),
        WL_MARSHAL_FLAG_DESTROY
    );
}

#ifdef __cplusplus
}
#endif

#endif
#endif // __linux__

#ifdef __linux__
/* Generated by wayland-scanner 1.23.1 */

#ifndef XDG_SHELL_CLIENT_PROTOCOL_H
#define XDG_SHELL_CLIENT_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page page_xdg_shell The xdg_shell protocol
 * @section page_ifaces_xdg_shell Interfaces
 * - @subpage page_iface_xdg_wm_base - create desktop-style surfaces
 * - @subpage page_iface_xdg_positioner - child surface positioner
 * - @subpage page_iface_xdg_surface - desktop user interface surface base
 * interface
 * - @subpage page_iface_xdg_toplevel - toplevel surface
 * - @subpage page_iface_xdg_popup - short-lived, popup surfaces for menus
 * @section page_copyright_xdg_shell Copyright
 * <pre>
 *
 * Copyright © 2008-2013 Kristian Høgsberg
 * Copyright © 2013      Rafael Antognolli
 * Copyright © 2013      Jasper St. Pierre
 * Copyright © 2010-2013 Intel Corporation
 * Copyright © 2015-2017 Samsung Electronics Co., Ltd
 * Copyright © 2015-2017 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * </pre>
 */
struct wl_output;
struct wl_seat;
struct wl_surface;
struct xdg_popup;
struct xdg_positioner;
struct xdg_surface;
struct xdg_toplevel;
struct xdg_wm_base;

#ifndef XDG_WM_BASE_INTERFACE
#define XDG_WM_BASE_INTERFACE
/**
 * @page page_iface_xdg_wm_base xdg_wm_base
 * @section page_iface_xdg_wm_base_desc Description
 *
 * The xdg_wm_base interface is exposed as a global object enabling clients
 * to turn their wl_surfaces into windows in a desktop environment. It
 * defines the basic functionality needed for clients and the compositor to
 * create windows that can be dragged, resized, maximized, etc, as well as
 * creating transient windows such as popup menus.
 * @section page_iface_xdg_wm_base_api API
 * See @ref iface_xdg_wm_base.
 */
/**
 * @defgroup iface_xdg_wm_base The xdg_wm_base interface
 *
 * The xdg_wm_base interface is exposed as a global object enabling clients
 * to turn their wl_surfaces into windows in a desktop environment. It
 * defines the basic functionality needed for clients and the compositor to
 * create windows that can be dragged, resized, maximized, etc, as well as
 * creating transient windows such as popup menus.
 */
extern const struct wl_interface xdg_wm_base_interface;
#endif
#ifndef XDG_POSITIONER_INTERFACE
#define XDG_POSITIONER_INTERFACE
/**
 * @page page_iface_xdg_positioner xdg_positioner
 * @section page_iface_xdg_positioner_desc Description
 *
 * The xdg_positioner provides a collection of rules for the placement of a
 * child surface relative to a parent surface. Rules can be defined to ensure
 * the child surface remains within the visible area's borders, and to
 * specify how the child surface changes its position, such as sliding along
 * an axis, or flipping around a rectangle. These positioner-created rules are
 * constrained by the requirement that a child surface must intersect with or
 * be at least partially adjacent to its parent surface.
 *
 * See the various requests for details about possible rules.
 *
 * At the time of the request, the compositor makes a copy of the rules
 * specified by the xdg_positioner. Thus, after the request is complete the
 * xdg_positioner object can be destroyed or reused; further changes to the
 * object will have no effect on previous usages.
 *
 * For an xdg_positioner object to be considered complete, it must have a
 * non-zero size set by set_size, and a non-zero anchor rectangle set by
 * set_anchor_rect. Passing an incomplete xdg_positioner object when
 * positioning a surface raises an invalid_positioner error.
 * @section page_iface_xdg_positioner_api API
 * See @ref iface_xdg_positioner.
 */
/**
 * @defgroup iface_xdg_positioner The xdg_positioner interface
 *
 * The xdg_positioner provides a collection of rules for the placement of a
 * child surface relative to a parent surface. Rules can be defined to ensure
 * the child surface remains within the visible area's borders, and to
 * specify how the child surface changes its position, such as sliding along
 * an axis, or flipping around a rectangle. These positioner-created rules are
 * constrained by the requirement that a child surface must intersect with or
 * be at least partially adjacent to its parent surface.
 *
 * See the various requests for details about possible rules.
 *
 * At the time of the request, the compositor makes a copy of the rules
 * specified by the xdg_positioner. Thus, after the request is complete the
 * xdg_positioner object can be destroyed or reused; further changes to the
 * object will have no effect on previous usages.
 *
 * For an xdg_positioner object to be considered complete, it must have a
 * non-zero size set by set_size, and a non-zero anchor rectangle set by
 * set_anchor_rect. Passing an incomplete xdg_positioner object when
 * positioning a surface raises an invalid_positioner error.
 */
extern const struct wl_interface xdg_positioner_interface;
#endif
#ifndef XDG_SURFACE_INTERFACE
#define XDG_SURFACE_INTERFACE
/**
 * @page page_iface_xdg_surface xdg_surface
 * @section page_iface_xdg_surface_desc Description
 *
 * An interface that may be implemented by a wl_surface, for
 * implementations that provide a desktop-style user interface.
 *
 * It provides a base set of functionality required to construct user
 * interface elements requiring management by the compositor, such as
 * toplevel windows, menus, etc. The types of functionality are split into
 * xdg_surface roles.
 *
 * Creating an xdg_surface does not set the role for a wl_surface. In order
 * to map an xdg_surface, the client must create a role-specific object
 * using, e.g., get_toplevel, get_popup. The wl_surface for any given
 * xdg_surface can have at most one role, and may not be assigned any role
 * not based on xdg_surface.
 *
 * A role must be assigned before any other requests are made to the
 * xdg_surface object.
 *
 * The client must call wl_surface.commit on the corresponding wl_surface
 * for the xdg_surface state to take effect.
 *
 * Creating an xdg_surface from a wl_surface which has a buffer attached or
 * committed is a client error, and any attempts by a client to attach or
 * manipulate a buffer prior to the first xdg_surface.configure call must
 * also be treated as errors.
 *
 * After creating a role-specific object and setting it up (e.g. by sending
 * the title, app ID, size constraints, parent, etc), the client must
 * perform an initial commit without any buffer attached. The compositor
 * will reply with initial wl_surface state such as
 * wl_surface.preferred_buffer_scale followed by an xdg_surface.configure
 * event. The client must acknowledge it and is then allowed to attach a
 * buffer to map the surface.
 *
 * Mapping an xdg_surface-based role surface is defined as making it
 * possible for the surface to be shown by the compositor. Note that
 * a mapped surface is not guaranteed to be visible once it is mapped.
 *
 * For an xdg_surface to be mapped by the compositor, the following
 * conditions must be met:
 * (1) the client has assigned an xdg_surface-based role to the surface
 * (2) the client has set and committed the xdg_surface state and the
 * role-dependent state to the surface
 * (3) the client has committed a buffer to the surface
 *
 * A newly-unmapped surface is considered to have met condition (1) out
 * of the 3 required conditions for mapping a surface if its role surface
 * has not been destroyed, i.e. the client must perform the initial commit
 * again before attaching a buffer.
 * @section page_iface_xdg_surface_api API
 * See @ref iface_xdg_surface.
 */
/**
 * @defgroup iface_xdg_surface The xdg_surface interface
 *
 * An interface that may be implemented by a wl_surface, for
 * implementations that provide a desktop-style user interface.
 *
 * It provides a base set of functionality required to construct user
 * interface elements requiring management by the compositor, such as
 * toplevel windows, menus, etc. The types of functionality are split into
 * xdg_surface roles.
 *
 * Creating an xdg_surface does not set the role for a wl_surface. In order
 * to map an xdg_surface, the client must create a role-specific object
 * using, e.g., get_toplevel, get_popup. The wl_surface for any given
 * xdg_surface can have at most one role, and may not be assigned any role
 * not based on xdg_surface.
 *
 * A role must be assigned before any other requests are made to the
 * xdg_surface object.
 *
 * The client must call wl_surface.commit on the corresponding wl_surface
 * for the xdg_surface state to take effect.
 *
 * Creating an xdg_surface from a wl_surface which has a buffer attached or
 * committed is a client error, and any attempts by a client to attach or
 * manipulate a buffer prior to the first xdg_surface.configure call must
 * also be treated as errors.
 *
 * After creating a role-specific object and setting it up (e.g. by sending
 * the title, app ID, size constraints, parent, etc), the client must
 * perform an initial commit without any buffer attached. The compositor
 * will reply with initial wl_surface state such as
 * wl_surface.preferred_buffer_scale followed by an xdg_surface.configure
 * event. The client must acknowledge it and is then allowed to attach a
 * buffer to map the surface.
 *
 * Mapping an xdg_surface-based role surface is defined as making it
 * possible for the surface to be shown by the compositor. Note that
 * a mapped surface is not guaranteed to be visible once it is mapped.
 *
 * For an xdg_surface to be mapped by the compositor, the following
 * conditions must be met:
 * (1) the client has assigned an xdg_surface-based role to the surface
 * (2) the client has set and committed the xdg_surface state and the
 * role-dependent state to the surface
 * (3) the client has committed a buffer to the surface
 *
 * A newly-unmapped surface is considered to have met condition (1) out
 * of the 3 required conditions for mapping a surface if its role surface
 * has not been destroyed, i.e. the client must perform the initial commit
 * again before attaching a buffer.
 */
extern const struct wl_interface xdg_surface_interface;
#endif
#ifndef XDG_TOPLEVEL_INTERFACE
#define XDG_TOPLEVEL_INTERFACE
/**
 * @page page_iface_xdg_toplevel xdg_toplevel
 * @section page_iface_xdg_toplevel_desc Description
 *
 * This interface defines an xdg_surface role which allows a surface to,
 * among other things, set window-like properties such as maximize,
 * fullscreen, and minimize, set application-specific metadata like title and
 * id, and well as trigger user interactive operations such as interactive
 * resize and move.
 *
 * A xdg_toplevel by default is responsible for providing the full intended
 * visual representation of the toplevel, which depending on the window
 * state, may mean things like a title bar, window controls and drop shadow.
 *
 * Unmapping an xdg_toplevel means that the surface cannot be shown
 * by the compositor until it is explicitly mapped again.
 * All active operations (e.g., move, resize) are canceled and all
 * attributes (e.g. title, state, stacking, ...) are discarded for
 * an xdg_toplevel surface when it is unmapped. The xdg_toplevel returns to
 * the state it had right after xdg_surface.get_toplevel. The client
 * can re-map the toplevel by performing a commit without any buffer
 * attached, waiting for a configure event and handling it as usual (see
 * xdg_surface description).
 *
 * Attaching a null buffer to a toplevel unmaps the surface.
 * @section page_iface_xdg_toplevel_api API
 * See @ref iface_xdg_toplevel.
 */
/**
 * @defgroup iface_xdg_toplevel The xdg_toplevel interface
 *
 * This interface defines an xdg_surface role which allows a surface to,
 * among other things, set window-like properties such as maximize,
 * fullscreen, and minimize, set application-specific metadata like title and
 * id, and well as trigger user interactive operations such as interactive
 * resize and move.
 *
 * A xdg_toplevel by default is responsible for providing the full intended
 * visual representation of the toplevel, which depending on the window
 * state, may mean things like a title bar, window controls and drop shadow.
 *
 * Unmapping an xdg_toplevel means that the surface cannot be shown
 * by the compositor until it is explicitly mapped again.
 * All active operations (e.g., move, resize) are canceled and all
 * attributes (e.g. title, state, stacking, ...) are discarded for
 * an xdg_toplevel surface when it is unmapped. The xdg_toplevel returns to
 * the state it had right after xdg_surface.get_toplevel. The client
 * can re-map the toplevel by performing a commit without any buffer
 * attached, waiting for a configure event and handling it as usual (see
 * xdg_surface description).
 *
 * Attaching a null buffer to a toplevel unmaps the surface.
 */
extern const struct wl_interface xdg_toplevel_interface;
#endif
#ifndef XDG_POPUP_INTERFACE
#define XDG_POPUP_INTERFACE
/**
 * @page page_iface_xdg_popup xdg_popup
 * @section page_iface_xdg_popup_desc Description
 *
 * A popup surface is a short-lived, temporary surface. It can be used to
 * implement for example menus, popovers, tooltips and other similar user
 * interface concepts.
 *
 * A popup can be made to take an explicit grab. See xdg_popup.grab for
 * details.
 *
 * When the popup is dismissed, a popup_done event will be sent out, and at
 * the same time the surface will be unmapped. See the xdg_popup.popup_done
 * event for details.
 *
 * Explicitly destroying the xdg_popup object will also dismiss the popup and
 * unmap the surface. Clients that want to dismiss the popup when another
 * surface of their own is clicked should dismiss the popup using the destroy
 * request.
 *
 * A newly created xdg_popup will be stacked on top of all previously created
 * xdg_popup surfaces associated with the same xdg_toplevel.
 *
 * The parent of an xdg_popup must be mapped (see the xdg_surface
 * description) before the xdg_popup itself.
 *
 * The client must call wl_surface.commit on the corresponding wl_surface
 * for the xdg_popup state to take effect.
 * @section page_iface_xdg_popup_api API
 * See @ref iface_xdg_popup.
 */
/**
 * @defgroup iface_xdg_popup The xdg_popup interface
 *
 * A popup surface is a short-lived, temporary surface. It can be used to
 * implement for example menus, popovers, tooltips and other similar user
 * interface concepts.
 *
 * A popup can be made to take an explicit grab. See xdg_popup.grab for
 * details.
 *
 * When the popup is dismissed, a popup_done event will be sent out, and at
 * the same time the surface will be unmapped. See the xdg_popup.popup_done
 * event for details.
 *
 * Explicitly destroying the xdg_popup object will also dismiss the popup and
 * unmap the surface. Clients that want to dismiss the popup when another
 * surface of their own is clicked should dismiss the popup using the destroy
 * request.
 *
 * A newly created xdg_popup will be stacked on top of all previously created
 * xdg_popup surfaces associated with the same xdg_toplevel.
 *
 * The parent of an xdg_popup must be mapped (see the xdg_surface
 * description) before the xdg_popup itself.
 *
 * The client must call wl_surface.commit on the corresponding wl_surface
 * for the xdg_popup state to take effect.
 */
extern const struct wl_interface xdg_popup_interface;
#endif

#ifndef XDG_WM_BASE_ERROR_ENUM
#define XDG_WM_BASE_ERROR_ENUM

enum xdg_wm_base_error {
    /**
     * given wl_surface has another role
     */
    XDG_WM_BASE_ERROR_ROLE = 0,
    /**
     * xdg_wm_base was destroyed before children
     */
    XDG_WM_BASE_ERROR_DEFUNCT_SURFACES = 1,
    /**
     * the client tried to map or destroy a non-topmost popup
     */
    XDG_WM_BASE_ERROR_NOT_THE_TOPMOST_POPUP = 2,
    /**
     * the client specified an invalid popup parent surface
     */
    XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT = 3,
    /**
     * the client provided an invalid surface state
     */
    XDG_WM_BASE_ERROR_INVALID_SURFACE_STATE = 4,
    /**
     * the client provided an invalid positioner
     */
    XDG_WM_BASE_ERROR_INVALID_POSITIONER = 5,
    /**
     * the client didn't respond to a ping event in time
     */
    XDG_WM_BASE_ERROR_UNRESPONSIVE = 6,
};
#endif /* XDG_WM_BASE_ERROR_ENUM */

/**
 * @ingroup iface_xdg_wm_base
 * @struct xdg_wm_base_listener
 */
struct xdg_wm_base_listener {
    /**
     * check if the client is alive
     *
     * The ping event asks the client if it's still alive. Pass the
     * serial specified in the event back to the compositor by sending
     * a "pong" request back with the specified serial. See
     * xdg_wm_base.pong.
     *
     * Compositors can use this to determine if the client is still
     * alive. It's unspecified what will happen if the client doesn't
     * respond to the ping request, or in what timeframe. Clients
     * should try to respond in a reasonable amount of time. The
     * "unresponsive" error is provided for compositors that wish
     * to disconnect unresponsive clients.
     *
     * A compositor is free to ping in any way it wants, but a client
     * must always respond to any xdg_wm_base object it created.
     * @param serial pass this to the pong request
     */
    void (*ping)(void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial);
};

/**
 * @ingroup iface_xdg_wm_base
 */
inline static int xdg_wm_base_add_listener(
    struct xdg_wm_base* xdg_wm_base,
    const struct xdg_wm_base_listener* listener,
    void* data
) {
    return wl_proxy_add_listener(
        (struct wl_proxy*)xdg_wm_base,
        (void (**)(void))listener,
        data
    );
}

#define XDG_WM_BASE_DESTROY 0
#define XDG_WM_BASE_CREATE_POSITIONER 1
#define XDG_WM_BASE_GET_XDG_SURFACE 2
#define XDG_WM_BASE_PONG 3

/**
 * @ingroup iface_xdg_wm_base
 */
#define XDG_WM_BASE_PING_SINCE_VERSION 1

/**
 * @ingroup iface_xdg_wm_base
 */
#define XDG_WM_BASE_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_wm_base
 */
#define XDG_WM_BASE_CREATE_POSITIONER_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_wm_base
 */
#define XDG_WM_BASE_GET_XDG_SURFACE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_wm_base
 */
#define XDG_WM_BASE_PONG_SINCE_VERSION 1

/** @ingroup iface_xdg_wm_base */
inline static void xdg_wm_base_set_user_data(
    struct xdg_wm_base* xdg_wm_base,
    void* user_data
) {
    wl_proxy_set_user_data((struct wl_proxy*)xdg_wm_base, user_data);
}

/** @ingroup iface_xdg_wm_base */
inline static void* xdg_wm_base_get_user_data(struct xdg_wm_base* xdg_wm_base) {
    return wl_proxy_get_user_data((struct wl_proxy*)xdg_wm_base);
}

inline static uint32_t xdg_wm_base_get_version(struct xdg_wm_base* xdg_wm_base) {
    return wl_proxy_get_version((struct wl_proxy*)xdg_wm_base);
}

/**
 * @ingroup iface_xdg_wm_base
 *
 * Destroy this xdg_wm_base object.
 *
 * Destroying a bound xdg_wm_base object while there are surfaces
 * still alive created by this xdg_wm_base object instance is illegal
 * and will result in a defunct_surfaces error.
 */
inline static void xdg_wm_base_destroy(struct xdg_wm_base* xdg_wm_base) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_wm_base,
        XDG_WM_BASE_DESTROY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_wm_base),
        WL_MARSHAL_FLAG_DESTROY
    );
}

/**
 * @ingroup iface_xdg_wm_base
 *
 * Create a positioner object. A positioner object is used to position
 * surfaces relative to some parent surface. See the interface description
 * and xdg_surface.get_popup for details.
 */
inline static struct xdg_positioner* xdg_wm_base_create_positioner(
    struct xdg_wm_base* xdg_wm_base
) {
    struct wl_proxy* id;

    id = wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_wm_base,
        XDG_WM_BASE_CREATE_POSITIONER,
        &xdg_positioner_interface,
        wl_proxy_get_version((struct wl_proxy*)xdg_wm_base),
        0,
        NULL
    );

    return (struct xdg_positioner*)id;
}

/**
 * @ingroup iface_xdg_wm_base
 *
 * This creates an xdg_surface for the given surface. While xdg_surface
 * itself is not a role, the corresponding surface may only be assigned
 * a role extending xdg_surface, such as xdg_toplevel or xdg_popup. It is
 * illegal to create an xdg_surface for a wl_surface which already has an
 * assigned role and this will result in a role error.
 *
 * This creates an xdg_surface for the given surface. An xdg_surface is
 * used as basis to define a role to a given surface, such as xdg_toplevel
 * or xdg_popup. It also manages functionality shared between xdg_surface
 * based surface roles.
 *
 * See the documentation of xdg_surface for more details about what an
 * xdg_surface is and how it is used.
 */
inline static struct xdg_surface* xdg_wm_base_get_xdg_surface(
    struct xdg_wm_base* xdg_wm_base,
    struct wl_surface* surface
) {
    struct wl_proxy* id;

    id = wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_wm_base,
        XDG_WM_BASE_GET_XDG_SURFACE,
        &xdg_surface_interface,
        wl_proxy_get_version((struct wl_proxy*)xdg_wm_base),
        0,
        NULL,
        surface
    );

    return (struct xdg_surface*)id;
}

/**
 * @ingroup iface_xdg_wm_base
 *
 * A client must respond to a ping event with a pong request or
 * the client may be deemed unresponsive. See xdg_wm_base.ping
 * and xdg_wm_base.error.unresponsive.
 */
inline static void xdg_wm_base_pong(
    struct xdg_wm_base* xdg_wm_base,
    uint32_t serial
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_wm_base,
        XDG_WM_BASE_PONG,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_wm_base),
        0,
        serial
    );
}

#ifndef XDG_POSITIONER_ERROR_ENUM
#define XDG_POSITIONER_ERROR_ENUM

enum xdg_positioner_error {
    /**
     * invalid input provided
     */
    XDG_POSITIONER_ERROR_INVALID_INPUT = 0,
};
#endif /* XDG_POSITIONER_ERROR_ENUM */

#ifndef XDG_POSITIONER_ANCHOR_ENUM
#define XDG_POSITIONER_ANCHOR_ENUM

enum xdg_positioner_anchor {
    XDG_POSITIONER_ANCHOR_NONE = 0,
    XDG_POSITIONER_ANCHOR_TOP = 1,
    XDG_POSITIONER_ANCHOR_BOTTOM = 2,
    XDG_POSITIONER_ANCHOR_LEFT = 3,
    XDG_POSITIONER_ANCHOR_RIGHT = 4,
    XDG_POSITIONER_ANCHOR_TOP_LEFT = 5,
    XDG_POSITIONER_ANCHOR_BOTTOM_LEFT = 6,
    XDG_POSITIONER_ANCHOR_TOP_RIGHT = 7,
    XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT = 8,
};
#endif /* XDG_POSITIONER_ANCHOR_ENUM */

#ifndef XDG_POSITIONER_GRAVITY_ENUM
#define XDG_POSITIONER_GRAVITY_ENUM

enum xdg_positioner_gravity {
    XDG_POSITIONER_GRAVITY_NONE = 0,
    XDG_POSITIONER_GRAVITY_TOP = 1,
    XDG_POSITIONER_GRAVITY_BOTTOM = 2,
    XDG_POSITIONER_GRAVITY_LEFT = 3,
    XDG_POSITIONER_GRAVITY_RIGHT = 4,
    XDG_POSITIONER_GRAVITY_TOP_LEFT = 5,
    XDG_POSITIONER_GRAVITY_BOTTOM_LEFT = 6,
    XDG_POSITIONER_GRAVITY_TOP_RIGHT = 7,
    XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT = 8,
};
#endif /* XDG_POSITIONER_GRAVITY_ENUM */

#ifndef XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_ENUM
#define XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_ENUM

/**
 * @ingroup iface_xdg_positioner
 * constraint adjustments
 *
 * The constraint adjustment value define ways the compositor will adjust
 * the position of the surface, if the unadjusted position would result
 * in the surface being partly constrained.
 *
 * Whether a surface is considered 'constrained' is left to the compositor
 * to determine. For example, the surface may be partly outside the
 * compositor's defined 'work area', thus necessitating the child surface's
 * position be adjusted until it is entirely inside the work area.
 *
 * The adjustments can be combined, according to a defined precedence: 1)
 * Flip, 2) Slide, 3) Resize.
 */
enum xdg_positioner_constraint_adjustment {
    /**
     * don't move the child surface when constrained
     *
     * Don't alter the surface position even if it is constrained on
     * some axis, for example partially outside the edge of an output.
     */
    XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE = 0,
    /**
     * move along the x axis until unconstrained
     *
     * Slide the surface along the x axis until it is no longer
     * constrained.
     *
     * First try to slide towards the direction of the gravity on the x
     * axis until either the edge in the opposite direction of the
     * gravity is unconstrained or the edge in the direction of the
     * gravity is constrained.
     *
     * Then try to slide towards the opposite direction of the gravity
     * on the x axis until either the edge in the direction of the
     * gravity is unconstrained or the edge in the opposite direction
     * of the gravity is constrained.
     */
    XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X = 1,
    /**
     * move along the y axis until unconstrained
     *
     * Slide the surface along the y axis until it is no longer
     * constrained.
     *
     * First try to slide towards the direction of the gravity on the y
     * axis until either the edge in the opposite direction of the
     * gravity is unconstrained or the edge in the direction of the
     * gravity is constrained.
     *
     * Then try to slide towards the opposite direction of the gravity
     * on the y axis until either the edge in the direction of the
     * gravity is unconstrained or the edge in the opposite direction
     * of the gravity is constrained.
     */
    XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y = 2,
    /**
     * invert the anchor and gravity on the x axis
     *
     * Invert the anchor and gravity on the x axis if the surface is
     * constrained on the x axis. For example, if the left edge of the
     * surface is constrained, the gravity is 'left' and the anchor is
     * 'left', change the gravity to 'right' and the anchor to 'right'.
     *
     * If the adjusted position also ends up being constrained, the
     * resulting position of the flip_x adjustment will be the one
     * before the adjustment.
     */
    XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X = 4,
    /**
     * invert the anchor and gravity on the y axis
     *
     * Invert the anchor and gravity on the y axis if the surface is
     * constrained on the y axis. For example, if the bottom edge of
     * the surface is constrained, the gravity is 'bottom' and the
     * anchor is 'bottom', change the gravity to 'top' and the anchor
     * to 'top'.
     *
     * The adjusted position is calculated given the original anchor
     * rectangle and offset, but with the new flipped anchor and
     * gravity values.
     *
     * If the adjusted position also ends up being constrained, the
     * resulting position of the flip_y adjustment will be the one
     * before the adjustment.
     */
    XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y = 8,
    /**
     * horizontally resize the surface
     *
     * Resize the surface horizontally so that it is completely
     * unconstrained.
     */
    XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X = 16,
    /**
     * vertically resize the surface
     *
     * Resize the surface vertically so that it is completely
     * unconstrained.
     */
    XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y = 32,
};
#endif /* XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_ENUM */

#define XDG_POSITIONER_DESTROY 0
#define XDG_POSITIONER_SET_SIZE 1
#define XDG_POSITIONER_SET_ANCHOR_RECT 2
#define XDG_POSITIONER_SET_ANCHOR 3
#define XDG_POSITIONER_SET_GRAVITY 4
#define XDG_POSITIONER_SET_CONSTRAINT_ADJUSTMENT 5
#define XDG_POSITIONER_SET_OFFSET 6
#define XDG_POSITIONER_SET_REACTIVE 7
#define XDG_POSITIONER_SET_PARENT_SIZE 8
#define XDG_POSITIONER_SET_PARENT_CONFIGURE 9

/**
 * @ingroup iface_xdg_positioner
 */
#define XDG_POSITIONER_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_positioner
 */
#define XDG_POSITIONER_SET_SIZE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_positioner
 */
#define XDG_POSITIONER_SET_ANCHOR_RECT_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_positioner
 */
#define XDG_POSITIONER_SET_ANCHOR_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_positioner
 */
#define XDG_POSITIONER_SET_GRAVITY_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_positioner
 */
#define XDG_POSITIONER_SET_CONSTRAINT_ADJUSTMENT_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_positioner
 */
#define XDG_POSITIONER_SET_OFFSET_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_positioner
 */
#define XDG_POSITIONER_SET_REACTIVE_SINCE_VERSION 3
/**
 * @ingroup iface_xdg_positioner
 */
#define XDG_POSITIONER_SET_PARENT_SIZE_SINCE_VERSION 3
/**
 * @ingroup iface_xdg_positioner
 */
#define XDG_POSITIONER_SET_PARENT_CONFIGURE_SINCE_VERSION 3

/** @ingroup iface_xdg_positioner */
inline static void xdg_positioner_set_user_data(
    struct xdg_positioner* xdg_positioner,
    void* user_data
) {
    wl_proxy_set_user_data((struct wl_proxy*)xdg_positioner, user_data);
}

/** @ingroup iface_xdg_positioner */
inline static void* xdg_positioner_get_user_data(
    struct xdg_positioner* xdg_positioner
) {
    return wl_proxy_get_user_data((struct wl_proxy*)xdg_positioner);
}

inline static uint32_t xdg_positioner_get_version(
    struct xdg_positioner* xdg_positioner
) {
    return wl_proxy_get_version((struct wl_proxy*)xdg_positioner);
}

/**
 * @ingroup iface_xdg_positioner
 *
 * Notify the compositor that the xdg_positioner will no longer be used.
 */
inline static void xdg_positioner_destroy(
    struct xdg_positioner* xdg_positioner
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_positioner,
        XDG_POSITIONER_DESTROY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_positioner),
        WL_MARSHAL_FLAG_DESTROY
    );
}

/**
 * @ingroup iface_xdg_positioner
 *
 * Set the size of the surface that is to be positioned with the positioner
 * object. The size is in surface-local coordinates and corresponds to the
 * window geometry. See xdg_surface.set_window_geometry.
 *
 * If a zero or negative size is set the invalid_input error is raised.
 */
inline static void xdg_positioner_set_size(
    struct xdg_positioner* xdg_positioner,
    int32_t width,
    int32_t height
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_positioner,
        XDG_POSITIONER_SET_SIZE,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_positioner),
        0,
        width,
        height
    );
}

/**
 * @ingroup iface_xdg_positioner
 *
 * Specify the anchor rectangle within the parent surface that the child
 * surface will be placed relative to. The rectangle is relative to the
 * window geometry as defined by xdg_surface.set_window_geometry of the
 * parent surface.
 *
 * When the xdg_positioner object is used to position a child surface, the
 * anchor rectangle may not extend outside the window geometry of the
 * positioned child's parent surface.
 *
 * If a negative size is set the invalid_input error is raised.
 */
inline static void xdg_positioner_set_anchor_rect(
    struct xdg_positioner* xdg_positioner,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_positioner,
        XDG_POSITIONER_SET_ANCHOR_RECT,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_positioner),
        0,
        x,
        y,
        width,
        height
    );
}

/**
 * @ingroup iface_xdg_positioner
 *
 * Defines the anchor point for the anchor rectangle. The specified anchor
 * is used derive an anchor point that the child surface will be
 * positioned relative to. If a corner anchor is set (e.g. 'top_left' or
 * 'bottom_right'), the anchor point will be at the specified corner;
 * otherwise, the derived anchor point will be centered on the specified
 * edge, or in the center of the anchor rectangle if no edge is specified.
 */
inline static void xdg_positioner_set_anchor(
    struct xdg_positioner* xdg_positioner,
    uint32_t anchor
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_positioner,
        XDG_POSITIONER_SET_ANCHOR,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_positioner),
        0,
        anchor
    );
}

/**
 * @ingroup iface_xdg_positioner
 *
 * Defines in what direction a surface should be positioned, relative to
 * the anchor point of the parent surface. If a corner gravity is
 * specified (e.g. 'bottom_right' or 'top_left'), then the child surface
 * will be placed towards the specified gravity; otherwise, the child
 * surface will be centered over the anchor point on any axis that had no
 * gravity specified. If the gravity is not in the "gravity" enum, an
 * invalid_input error is raised.
 */
inline static void xdg_positioner_set_gravity(
    struct xdg_positioner* xdg_positioner,
    uint32_t gravity
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_positioner,
        XDG_POSITIONER_SET_GRAVITY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_positioner),
        0,
        gravity
    );
}

/**
 * @ingroup iface_xdg_positioner
 *
 * Specify how the window should be positioned if the originally intended
 * position caused the surface to be constrained, meaning at least
 * partially outside positioning boundaries set by the compositor. The
 * adjustment is set by constructing a bitmask describing the adjustment to
 * be made when the surface is constrained on that axis.
 *
 * If no bit for one axis is set, the compositor will assume that the child
 * surface should not change its position on that axis when constrained.
 *
 * If more than one bit for one axis is set, the order of how adjustments
 * are applied is specified in the corresponding adjustment descriptions.
 *
 * The default adjustment is none.
 */
inline static void xdg_positioner_set_constraint_adjustment(
    struct xdg_positioner* xdg_positioner,
    uint32_t constraint_adjustment
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_positioner,
        XDG_POSITIONER_SET_CONSTRAINT_ADJUSTMENT,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_positioner),
        0,
        constraint_adjustment
    );
}

/**
 * @ingroup iface_xdg_positioner
 *
 * Specify the surface position offset relative to the position of the
 * anchor on the anchor rectangle and the anchor on the surface. For
 * example if the anchor of the anchor rectangle is at (x, y), the surface
 * has the gravity bottom|right, and the offset is (ox, oy), the calculated
 * surface position will be (x + ox, y + oy). The offset position of the
 * surface is the one used for constraint testing. See
 * set_constraint_adjustment.
 *
 * An example use case is placing a popup menu on top of a user interface
 * element, while aligning the user interface element of the parent surface
 * with some user interface element placed somewhere in the popup surface.
 */
inline static void xdg_positioner_set_offset(
    struct xdg_positioner* xdg_positioner,
    int32_t x,
    int32_t y
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_positioner,
        XDG_POSITIONER_SET_OFFSET,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_positioner),
        0,
        x,
        y
    );
}

/**
 * @ingroup iface_xdg_positioner
 *
 * When set reactive, the surface is reconstrained if the conditions used
 * for constraining changed, e.g. the parent window moved.
 *
 * If the conditions changed and the popup was reconstrained, an
 * xdg_popup.configure event is sent with updated geometry, followed by an
 * xdg_surface.configure event.
 */
inline static void xdg_positioner_set_reactive(
    struct xdg_positioner* xdg_positioner
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_positioner,
        XDG_POSITIONER_SET_REACTIVE,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_positioner),
        0
    );
}

/**
 * @ingroup iface_xdg_positioner
 *
 * Set the parent window geometry the compositor should use when
 * positioning the popup. The compositor may use this information to
 * determine the future state the popup should be constrained using. If
 * this doesn't match the dimension of the parent the popup is eventually
 * positioned against, the behavior is undefined.
 *
 * The arguments are given in the surface-local coordinate space.
 */
inline static void xdg_positioner_set_parent_size(
    struct xdg_positioner* xdg_positioner,
    int32_t parent_width,
    int32_t parent_height
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_positioner,
        XDG_POSITIONER_SET_PARENT_SIZE,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_positioner),
        0,
        parent_width,
        parent_height
    );
}

/**
 * @ingroup iface_xdg_positioner
 *
 * Set the serial of an xdg_surface.configure event this positioner will be
 * used in response to. The compositor may use this information together
 * with set_parent_size to determine what future state the popup should be
 * constrained using.
 */
inline static void xdg_positioner_set_parent_configure(
    struct xdg_positioner* xdg_positioner,
    uint32_t serial
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_positioner,
        XDG_POSITIONER_SET_PARENT_CONFIGURE,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_positioner),
        0,
        serial
    );
}

#ifndef XDG_SURFACE_ERROR_ENUM
#define XDG_SURFACE_ERROR_ENUM

enum xdg_surface_error {
    /**
     * Surface was not fully constructed
     */
    XDG_SURFACE_ERROR_NOT_CONSTRUCTED = 1,
    /**
     * Surface was already constructed
     */
    XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED = 2,
    /**
     * Attaching a buffer to an unconfigured surface
     */
    XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER = 3,
    /**
     * Invalid serial number when acking a configure event
     */
    XDG_SURFACE_ERROR_INVALID_SERIAL = 4,
    /**
     * Width or height was zero or negative
     */
    XDG_SURFACE_ERROR_INVALID_SIZE = 5,
    /**
     * Surface was destroyed before its role object
     */
    XDG_SURFACE_ERROR_DEFUNCT_ROLE_OBJECT = 6,
};
#endif /* XDG_SURFACE_ERROR_ENUM */

/**
 * @ingroup iface_xdg_surface
 * @struct xdg_surface_listener
 */
struct xdg_surface_listener {
    /**
     * suggest a surface change
     *
     * The configure event marks the end of a configure sequence. A
     * configure sequence is a set of one or more events configuring
     * the state of the xdg_surface, including the final
     * xdg_surface.configure event.
     *
     * Where applicable, xdg_surface surface roles will during a
     * configure sequence extend this event as a latched state sent as
     * events before the xdg_surface.configure event. Such events
     * should be considered to make up a set of atomically applied
     * configuration states, where the xdg_surface.configure commits
     * the accumulated state.
     *
     * Clients should arrange their surface for the new states, and
     * then send an ack_configure request with the serial sent in this
     * configure event at some point before committing the new surface.
     *
     * If the client receives multiple configure events before it can
     * respond to one, it is free to discard all but the last event it
     * received.
     * @param serial serial of the configure event
     */
    void (*configure)(
        void* data,
        struct xdg_surface* xdg_surface,
        uint32_t serial
    );
};

/**
 * @ingroup iface_xdg_surface
 */
inline static int xdg_surface_add_listener(
    struct xdg_surface* xdg_surface,
    const struct xdg_surface_listener* listener,
    void* data
) {
    return wl_proxy_add_listener(
        (struct wl_proxy*)xdg_surface,
        (void (**)(void))listener,
        data
    );
}

#define XDG_SURFACE_DESTROY 0
#define XDG_SURFACE_GET_TOPLEVEL 1
#define XDG_SURFACE_GET_POPUP 2
#define XDG_SURFACE_SET_WINDOW_GEOMETRY 3
#define XDG_SURFACE_ACK_CONFIGURE 4

/**
 * @ingroup iface_xdg_surface
 */
#define XDG_SURFACE_CONFIGURE_SINCE_VERSION 1

/**
 * @ingroup iface_xdg_surface
 */
#define XDG_SURFACE_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_surface
 */
#define XDG_SURFACE_GET_TOPLEVEL_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_surface
 */
#define XDG_SURFACE_GET_POPUP_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_surface
 */
#define XDG_SURFACE_SET_WINDOW_GEOMETRY_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_surface
 */
#define XDG_SURFACE_ACK_CONFIGURE_SINCE_VERSION 1

/** @ingroup iface_xdg_surface */
inline static void xdg_surface_set_user_data(
    struct xdg_surface* xdg_surface,
    void* user_data
) {
    wl_proxy_set_user_data((struct wl_proxy*)xdg_surface, user_data);
}

/** @ingroup iface_xdg_surface */
inline static void* xdg_surface_get_user_data(struct xdg_surface* xdg_surface) {
    return wl_proxy_get_user_data((struct wl_proxy*)xdg_surface);
}

inline static uint32_t xdg_surface_get_version(struct xdg_surface* xdg_surface) {
    return wl_proxy_get_version((struct wl_proxy*)xdg_surface);
}

/**
 * @ingroup iface_xdg_surface
 *
 * Destroy the xdg_surface object. An xdg_surface must only be destroyed
 * after its role object has been destroyed, otherwise
 * a defunct_role_object error is raised.
 */
inline static void xdg_surface_destroy(struct xdg_surface* xdg_surface) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_surface,
        XDG_SURFACE_DESTROY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_surface),
        WL_MARSHAL_FLAG_DESTROY
    );
}

/**
 * @ingroup iface_xdg_surface
 *
 * This creates an xdg_toplevel object for the given xdg_surface and gives
 * the associated wl_surface the xdg_toplevel role.
 *
 * See the documentation of xdg_toplevel for more details about what an
 * xdg_toplevel is and how it is used.
 */
inline static struct xdg_toplevel* xdg_surface_get_toplevel(
    struct xdg_surface* xdg_surface
) {
    struct wl_proxy* id;

    id = wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_surface,
        XDG_SURFACE_GET_TOPLEVEL,
        &xdg_toplevel_interface,
        wl_proxy_get_version((struct wl_proxy*)xdg_surface),
        0,
        NULL
    );

    return (struct xdg_toplevel*)id;
}

/**
 * @ingroup iface_xdg_surface
 *
 * This creates an xdg_popup object for the given xdg_surface and gives
 * the associated wl_surface the xdg_popup role.
 *
 * If null is passed as a parent, a parent surface must be specified using
 * some other protocol, before committing the initial state.
 *
 * See the documentation of xdg_popup for more details about what an
 * xdg_popup is and how it is used.
 */
inline static struct xdg_popup* xdg_surface_get_popup(
    struct xdg_surface* xdg_surface,
    struct xdg_surface* parent,
    struct xdg_positioner* positioner
) {
    struct wl_proxy* id;

    id = wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_surface,
        XDG_SURFACE_GET_POPUP,
        &xdg_popup_interface,
        wl_proxy_get_version((struct wl_proxy*)xdg_surface),
        0,
        NULL,
        parent,
        positioner
    );

    return (struct xdg_popup*)id;
}

/**
 * @ingroup iface_xdg_surface
 *
 * The window geometry of a surface is its "visible bounds" from the
 * user's perspective. Client-side decorations often have invisible
 * portions like drop-shadows which should be ignored for the
 * purposes of aligning, placing and constraining windows.
 *
 * The window geometry is double-buffered state, see wl_surface.commit.
 *
 * When maintaining a position, the compositor should treat the (x, y)
 * coordinate of the window geometry as the top left corner of the window.
 * A client changing the (x, y) window geometry coordinate should in
 * general not alter the position of the window.
 *
 * Once the window geometry of the surface is set, it is not possible to
 * unset it, and it will remain the same until set_window_geometry is
 * called again, even if a new subsurface or buffer is attached.
 *
 * If never set, the value is the full bounds of the surface,
 * including any subsurfaces. This updates dynamically on every
 * commit. This unset is meant for extremely simple clients.
 *
 * The arguments are given in the surface-local coordinate space of
 * the wl_surface associated with this xdg_surface, and may extend outside
 * of the wl_surface itself to mark parts of the subsurface tree as part of
 * the window geometry.
 *
 * When applied, the effective window geometry will be the set window
 * geometry clamped to the bounding rectangle of the combined
 * geometry of the surface of the xdg_surface and the associated
 * subsurfaces.
 *
 * The effective geometry will not be recalculated unless a new call to
 * set_window_geometry is done and the new pending surface state is
 * subsequently applied.
 *
 * The width and height of the effective window geometry must be
 * greater than zero. Setting an invalid size will raise an
 * invalid_size error.
 */
inline static void xdg_surface_set_window_geometry(
    struct xdg_surface* xdg_surface,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_surface,
        XDG_SURFACE_SET_WINDOW_GEOMETRY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_surface),
        0,
        x,
        y,
        width,
        height
    );
}

/**
 * @ingroup iface_xdg_surface
 *
 * When a configure event is received, if a client commits the
 * surface in response to the configure event, then the client
 * must make an ack_configure request sometime before the commit
 * request, passing along the serial of the configure event.
 *
 * For instance, for toplevel surfaces the compositor might use this
 * information to move a surface to the top left only when the client has
 * drawn itself for the maximized or fullscreen state.
 *
 * If the client receives multiple configure events before it
 * can respond to one, it only has to ack the last configure event.
 * Acking a configure event that was never sent raises an invalid_serial
 * error.
 *
 * A client is not required to commit immediately after sending
 * an ack_configure request - it may even ack_configure several times
 * before its next surface commit.
 *
 * A client may send multiple ack_configure requests before committing, but
 * only the last request sent before a commit indicates which configure
 * event the client really is responding to.
 *
 * Sending an ack_configure request consumes the serial number sent with
 * the request, as well as serial numbers sent by all configure events
 * sent on this xdg_surface prior to the configure event referenced by
 * the committed serial.
 *
 * It is an error to issue multiple ack_configure requests referencing a
 * serial from the same configure event, or to issue an ack_configure
 * request referencing a serial from a configure event issued before the
 * event identified by the last ack_configure request for the same
 * xdg_surface. Doing so will raise an invalid_serial error.
 */
inline static void xdg_surface_ack_configure(
    struct xdg_surface* xdg_surface,
    uint32_t serial
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_surface,
        XDG_SURFACE_ACK_CONFIGURE,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_surface),
        0,
        serial
    );
}

#ifndef XDG_TOPLEVEL_ERROR_ENUM
#define XDG_TOPLEVEL_ERROR_ENUM

enum xdg_toplevel_error {
    /**
     * provided value is         not a valid variant of the resize_edge enum
     */
    XDG_TOPLEVEL_ERROR_INVALID_RESIZE_EDGE = 0,
    /**
     * invalid parent toplevel
     */
    XDG_TOPLEVEL_ERROR_INVALID_PARENT = 1,
    /**
     * client provided an invalid min or max size
     */
    XDG_TOPLEVEL_ERROR_INVALID_SIZE = 2,
};
#endif /* XDG_TOPLEVEL_ERROR_ENUM */

#ifndef XDG_TOPLEVEL_RESIZE_EDGE_ENUM
#define XDG_TOPLEVEL_RESIZE_EDGE_ENUM

/**
 * @ingroup iface_xdg_toplevel
 * edge values for resizing
 *
 * These values are used to indicate which edge of a surface
 * is being dragged in a resize operation.
 */
enum xdg_toplevel_resize_edge {
    XDG_TOPLEVEL_RESIZE_EDGE_NONE = 0,
    XDG_TOPLEVEL_RESIZE_EDGE_TOP = 1,
    XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM = 2,
    XDG_TOPLEVEL_RESIZE_EDGE_LEFT = 4,
    XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT = 5,
    XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT = 6,
    XDG_TOPLEVEL_RESIZE_EDGE_RIGHT = 8,
    XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT = 9,
    XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT = 10,
};
#endif /* XDG_TOPLEVEL_RESIZE_EDGE_ENUM */

#ifndef XDG_TOPLEVEL_STATE_ENUM
#define XDG_TOPLEVEL_STATE_ENUM

/**
 * @ingroup iface_xdg_toplevel
 * types of state on the surface
 *
 * The different state values used on the surface. This is designed for
 * state values like maximized, fullscreen. It is paired with the
 * configure event to ensure that both the client and the compositor
 * setting the state can be synchronized.
 *
 * States set in this way are double-buffered, see wl_surface.commit.
 */
enum xdg_toplevel_state {
    /**
     * the surface is maximized
     * the surface is maximized
     *
     * The surface is maximized. The window geometry specified in the
     * configure event must be obeyed by the client, or the
     * xdg_wm_base.invalid_surface_state error is raised.
     *
     * The client should draw without shadow or other decoration
     * outside of the window geometry.
     */
    XDG_TOPLEVEL_STATE_MAXIMIZED = 1,
    /**
     * the surface is fullscreen
     * the surface is fullscreen
     *
     * The surface is fullscreen. The window geometry specified in
     * the configure event is a maximum; the client cannot resize
     * beyond it. For a surface to cover the whole fullscreened area,
     * the geometry dimensions must be obeyed by the client. For more
     * details, see xdg_toplevel.set_fullscreen.
     */
    XDG_TOPLEVEL_STATE_FULLSCREEN = 2,
    /**
     * the surface is being resized
     * the surface is being resized
     *
     * The surface is being resized. The window geometry specified in
     * the configure event is a maximum; the client cannot resize
     * beyond it. Clients that have aspect ratio or cell sizing
     * configuration can use a smaller size, however.
     */
    XDG_TOPLEVEL_STATE_RESIZING = 3,
    /**
     * the surface is now activated
     * the surface is now activated
     *
     * Client window decorations should be painted as if the window
     * is active. Do not assume this means that the window actually has
     * keyboard or pointer focus.
     */
    XDG_TOPLEVEL_STATE_ACTIVATED = 4,
    /**
     * the surface's left edge is tiled
     *
     * The window is currently in a tiled layout and the left edge is
     * considered to be adjacent to another part of the tiling grid.
     *
     * The client should draw without shadow or other decoration
     * outside of the window geometry on the left edge.
     * @since 2
     */
    XDG_TOPLEVEL_STATE_TILED_LEFT = 5,
    /**
     * the surface's right edge is tiled
     *
     * The window is currently in a tiled layout and the right edge
     * is considered to be adjacent to another part of the tiling grid.
     *
     * The client should draw without shadow or other decoration
     * outside of the window geometry on the right edge.
     * @since 2
     */
    XDG_TOPLEVEL_STATE_TILED_RIGHT = 6,
    /**
     * the surface's top edge is tiled
     *
     * The window is currently in a tiled layout and the top edge is
     * considered to be adjacent to another part of the tiling grid.
     *
     * The client should draw without shadow or other decoration
     * outside of the window geometry on the top edge.
     * @since 2
     */
    XDG_TOPLEVEL_STATE_TILED_TOP = 7,
    /**
     * the surface's bottom edge is tiled
     *
     * The window is currently in a tiled layout and the bottom edge
     * is considered to be adjacent to another part of the tiling grid.
     *
     * The client should draw without shadow or other decoration
     * outside of the window geometry on the bottom edge.
     * @since 2
     */
    XDG_TOPLEVEL_STATE_TILED_BOTTOM = 8,
    /**
     * surface repaint is suspended
     *
     * The surface is currently not ordinarily being repainted; for
     * example because its content is occluded by another window, or
     * its outputs are switched off due to screen locking.
     * @since 6
     */
    XDG_TOPLEVEL_STATE_SUSPENDED = 9,
};

/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_STATE_TILED_LEFT_SINCE_VERSION 2
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_STATE_TILED_RIGHT_SINCE_VERSION 2
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_STATE_TILED_TOP_SINCE_VERSION 2
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_STATE_TILED_BOTTOM_SINCE_VERSION 2
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_STATE_SUSPENDED_SINCE_VERSION 6
#endif /* XDG_TOPLEVEL_STATE_ENUM */

#ifndef XDG_TOPLEVEL_WM_CAPABILITIES_ENUM
#define XDG_TOPLEVEL_WM_CAPABILITIES_ENUM

enum xdg_toplevel_wm_capabilities {
    /**
     * show_window_menu is available
     */
    XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU = 1,
    /**
     * set_maximized and unset_maximized are available
     */
    XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE = 2,
    /**
     * set_fullscreen and unset_fullscreen are available
     */
    XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN = 3,
    /**
     * set_minimized is available
     */
    XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE = 4,
};
#endif /* XDG_TOPLEVEL_WM_CAPABILITIES_ENUM */

/**
 * @ingroup iface_xdg_toplevel
 * @struct xdg_toplevel_listener
 */
struct xdg_toplevel_listener {
    /**
     * suggest a surface change
     *
     * This configure event asks the client to resize its toplevel
     * surface or to change its state. The configured state should not
     * be applied immediately. See xdg_surface.configure for details.
     *
     * The width and height arguments specify a hint to the window
     * about how its surface should be resized in window geometry
     * coordinates. See set_window_geometry.
     *
     * If the width or height arguments are zero, it means the client
     * should decide its own window dimension. This may happen when the
     * compositor needs to configure the state of the surface but
     * doesn't have any information about any previous or expected
     * dimension.
     *
     * The states listed in the event specify how the width/height
     * arguments should be interpreted, and possibly how it should be
     * drawn.
     *
     * Clients must send an ack_configure in response to this event.
     * See xdg_surface.configure and xdg_surface.ack_configure for
     * details.
     */
    void (*configure)(
        void* data,
        struct xdg_toplevel* xdg_toplevel,
        int32_t width,
        int32_t height,
        struct wl_array* states
    );
    /**
     * surface wants to be closed
     *
     * The close event is sent by the compositor when the user wants
     * the surface to be closed. This should be equivalent to the user
     * clicking the close button in client-side decorations, if your
     * application has any.
     *
     * This is only a request that the user intends to close the
     * window. The client may choose to ignore this request, or show a
     * dialog to ask the user to save their data, etc.
     */
    void (*close)(void* data, struct xdg_toplevel* xdg_toplevel);
    /**
     * recommended window geometry bounds
     *
     * The configure_bounds event may be sent prior to a
     * xdg_toplevel.configure event to communicate the bounds a window
     * geometry size is recommended to constrain to.
     *
     * The passed width and height are in surface coordinate space. If
     * width and height are 0, it means bounds is unknown and
     * equivalent to as if no configure_bounds event was ever sent for
     * this surface.
     *
     * The bounds can for example correspond to the size of a monitor
     * excluding any panels or other shell components, so that a
     * surface isn't created in a way that it cannot fit.
     *
     * The bounds may change at any point, and in such a case, a new
     * xdg_toplevel.configure_bounds will be sent, followed by
     * xdg_toplevel.configure and xdg_surface.configure.
     * @since 4
     */
    void (*configure_bounds)(
        void* data,
        struct xdg_toplevel* xdg_toplevel,
        int32_t width,
        int32_t height
    );
    /**
     * compositor capabilities
     *
     * This event advertises the capabilities supported by the
     * compositor. If a capability isn't supported, clients should hide
     * or disable the UI elements that expose this functionality. For
     * instance, if the compositor doesn't advertise support for
     * minimized toplevels, a button triggering the set_minimized
     * request should not be displayed.
     *
     * The compositor will ignore requests it doesn't support. For
     * instance, a compositor which doesn't advertise support for
     * minimized will ignore set_minimized requests.
     *
     * Compositors must send this event once before the first
     * xdg_surface.configure event. When the capabilities change,
     * compositors must send this event again and then send an
     * xdg_surface.configure event.
     *
     * The configured state should not be applied immediately. See
     * xdg_surface.configure for details.
     *
     * The capabilities are sent as an array of 32-bit unsigned
     * integers in native endianness.
     * @param capabilities array of 32-bit capabilities
     * @since 5
     */
    void (*wm_capabilities)(
        void* data,
        struct xdg_toplevel* xdg_toplevel,
        struct wl_array* capabilities
    );
};

/**
 * @ingroup iface_xdg_toplevel
 */
inline static int xdg_toplevel_add_listener(
    struct xdg_toplevel* xdg_toplevel,
    const struct xdg_toplevel_listener* listener,
    void* data
) {
    return wl_proxy_add_listener(
        (struct wl_proxy*)xdg_toplevel,
        (void (**)(void))listener,
        data
    );
}

#define XDG_TOPLEVEL_DESTROY 0
#define XDG_TOPLEVEL_SET_PARENT 1
#define XDG_TOPLEVEL_SET_TITLE 2
#define XDG_TOPLEVEL_SET_APP_ID 3
#define XDG_TOPLEVEL_SHOW_WINDOW_MENU 4
#define XDG_TOPLEVEL_MOVE 5
#define XDG_TOPLEVEL_RESIZE 6
#define XDG_TOPLEVEL_SET_MAX_SIZE 7
#define XDG_TOPLEVEL_SET_MIN_SIZE 8
#define XDG_TOPLEVEL_SET_MAXIMIZED 9
#define XDG_TOPLEVEL_UNSET_MAXIMIZED 10
#define XDG_TOPLEVEL_SET_FULLSCREEN 11
#define XDG_TOPLEVEL_UNSET_FULLSCREEN 12
#define XDG_TOPLEVEL_SET_MINIMIZED 13

/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_CONFIGURE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_CLOSE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION 4
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION 5

/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_SET_PARENT_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_SET_TITLE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_SET_APP_ID_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_SHOW_WINDOW_MENU_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_MOVE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_RESIZE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_SET_MAX_SIZE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_SET_MIN_SIZE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_SET_MAXIMIZED_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_UNSET_MAXIMIZED_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_SET_FULLSCREEN_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_UNSET_FULLSCREEN_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_toplevel
 */
#define XDG_TOPLEVEL_SET_MINIMIZED_SINCE_VERSION 1

/** @ingroup iface_xdg_toplevel */
inline static void xdg_toplevel_set_user_data(
    struct xdg_toplevel* xdg_toplevel,
    void* user_data
) {
    wl_proxy_set_user_data((struct wl_proxy*)xdg_toplevel, user_data);
}

/** @ingroup iface_xdg_toplevel */
inline static void* xdg_toplevel_get_user_data(
    struct xdg_toplevel* xdg_toplevel
) {
    return wl_proxy_get_user_data((struct wl_proxy*)xdg_toplevel);
}

inline static uint32_t xdg_toplevel_get_version(
    struct xdg_toplevel* xdg_toplevel
) {
    return wl_proxy_get_version((struct wl_proxy*)xdg_toplevel);
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * This request destroys the role surface and unmaps the surface;
 * see "Unmapping" behavior in interface section for details.
 */
inline static void xdg_toplevel_destroy(struct xdg_toplevel* xdg_toplevel) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_DESTROY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        WL_MARSHAL_FLAG_DESTROY
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Set the "parent" of this surface. This surface should be stacked
 * above the parent surface and all other ancestor surfaces.
 *
 * Parent surfaces should be set on dialogs, toolboxes, or other
 * "auxiliary" surfaces, so that the parent is raised when the dialog
 * is raised.
 *
 * Setting a null parent for a child surface unsets its parent. Setting
 * a null parent for a surface which currently has no parent is a no-op.
 *
 * Only mapped surfaces can have child surfaces. Setting a parent which
 * is not mapped is equivalent to setting a null parent. If a surface
 * becomes unmapped, its children's parent is set to the parent of
 * the now-unmapped surface. If the now-unmapped surface has no parent,
 * its children's parent is unset. If the now-unmapped surface becomes
 * mapped again, its parent-child relationship is not restored.
 *
 * The parent toplevel must not be one of the child toplevel's
 * descendants, and the parent must be different from the child toplevel,
 * otherwise the invalid_parent protocol error is raised.
 */
inline static void xdg_toplevel_set_parent(
    struct xdg_toplevel* xdg_toplevel,
    struct xdg_toplevel* parent
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_SET_PARENT,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0,
        parent
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Set a short title for the surface.
 *
 * This string may be used to identify the surface in a task bar,
 * window list, or other user interface elements provided by the
 * compositor.
 *
 * The string must be encoded in UTF-8.
 */
inline static void xdg_toplevel_set_title(
    struct xdg_toplevel* xdg_toplevel,
    const char* title
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_SET_TITLE,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0,
        title
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Set an application identifier for the surface.
 *
 * The app ID identifies the general class of applications to which
 * the surface belongs. The compositor can use this to group multiple
 * surfaces together, or to determine how to launch a new application.
 *
 * For D-Bus activatable applications, the app ID is used as the D-Bus
 * service name.
 *
 * The compositor shell will try to group application surfaces together
 * by their app ID. As a best practice, it is suggested to select app
 * ID's that match the basename of the application's .desktop file.
 * For example, "org.freedesktop.FooViewer" where the .desktop file is
 * "org.freedesktop.FooViewer.desktop".
 *
 * Like other properties, a set_app_id request can be sent after the
 * xdg_toplevel has been mapped to update the property.
 *
 * See the desktop-entry specification [0] for more details on
 * application identifiers and how they relate to well-known D-Bus
 * names and .desktop files.
 *
 * [0] https://standards.freedesktop.org/desktop-entry-spec/
 */
inline static void xdg_toplevel_set_app_id(
    struct xdg_toplevel* xdg_toplevel,
    const char* app_id
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_SET_APP_ID,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0,
        app_id
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Clients implementing client-side decorations might want to show
 * a context menu when right-clicking on the decorations, giving the
 * user a menu that they can use to maximize or minimize the window.
 *
 * This request asks the compositor to pop up such a window menu at
 * the given position, relative to the local surface coordinates of
 * the parent surface. There are no guarantees as to what menu items
 * the window menu contains, or even if a window menu will be drawn
 * at all.
 *
 * This request must be used in response to some sort of user action
 * like a button press, key press, or touch down event.
 */
inline static void xdg_toplevel_show_window_menu(
    struct xdg_toplevel* xdg_toplevel,
    struct wl_seat* seat,
    uint32_t serial,
    int32_t x,
    int32_t y
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_SHOW_WINDOW_MENU,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0,
        seat,
        serial,
        x,
        y
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Start an interactive, user-driven move of the surface.
 *
 * This request must be used in response to some sort of user action
 * like a button press, key press, or touch down event. The passed
 * serial is used to determine the type of interactive move (touch,
 * pointer, etc).
 *
 * The server may ignore move requests depending on the state of
 * the surface (e.g. fullscreen or maximized), or if the passed serial
 * is no longer valid.
 *
 * If triggered, the surface will lose the focus of the device
 * (wl_pointer, wl_touch, etc) used for the move. It is up to the
 * compositor to visually indicate that the move is taking place, such as
 * updating a pointer cursor, during the move. There is no guarantee
 * that the device focus will return when the move is completed.
 */
inline static void xdg_toplevel_move(
    struct xdg_toplevel* xdg_toplevel,
    struct wl_seat* seat,
    uint32_t serial
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_MOVE,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0,
        seat,
        serial
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Start a user-driven, interactive resize of the surface.
 *
 * This request must be used in response to some sort of user action
 * like a button press, key press, or touch down event. The passed
 * serial is used to determine the type of interactive resize (touch,
 * pointer, etc).
 *
 * The server may ignore resize requests depending on the state of
 * the surface (e.g. fullscreen or maximized).
 *
 * If triggered, the client will receive configure events with the
 * "resize" state enum value and the expected sizes. See the "resize"
 * enum value for more details about what is required. The client
 * must also acknowledge configure events using "ack_configure". After
 * the resize is completed, the client will receive another "configure"
 * event without the resize state.
 *
 * If triggered, the surface also will lose the focus of the device
 * (wl_pointer, wl_touch, etc) used for the resize. It is up to the
 * compositor to visually indicate that the resize is taking place,
 * such as updating a pointer cursor, during the resize. There is no
 * guarantee that the device focus will return when the resize is
 * completed.
 *
 * The edges parameter specifies how the surface should be resized, and
 * is one of the values of the resize_edge enum. Values not matching
 * a variant of the enum will cause the invalid_resize_edge protocol error.
 * The compositor may use this information to update the surface position
 * for example when dragging the top left corner. The compositor may also
 * use this information to adapt its behavior, e.g. choose an appropriate
 * cursor image.
 */
inline static void xdg_toplevel_resize(
    struct xdg_toplevel* xdg_toplevel,
    struct wl_seat* seat,
    uint32_t serial,
    uint32_t edges
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_RESIZE,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0,
        seat,
        serial,
        edges
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Set a maximum size for the window.
 *
 * The client can specify a maximum size so that the compositor does
 * not try to configure the window beyond this size.
 *
 * The width and height arguments are in window geometry coordinates.
 * See xdg_surface.set_window_geometry.
 *
 * Values set in this way are double-buffered, see wl_surface.commit.
 *
 * The compositor can use this information to allow or disallow
 * different states like maximize or fullscreen and draw accurate
 * animations.
 *
 * Similarly, a tiling window manager may use this information to
 * place and resize client windows in a more effective way.
 *
 * The client should not rely on the compositor to obey the maximum
 * size. The compositor may decide to ignore the values set by the
 * client and request a larger size.
 *
 * If never set, or a value of zero in the request, means that the
 * client has no expected maximum size in the given dimension.
 * As a result, a client wishing to reset the maximum size
 * to an unspecified state can use zero for width and height in the
 * request.
 *
 * Requesting a maximum size to be smaller than the minimum size of
 * a surface is illegal and will result in an invalid_size error.
 *
 * The width and height must be greater than or equal to zero. Using
 * strictly negative values for width or height will result in a
 * invalid_size error.
 */
inline static void xdg_toplevel_set_max_size(
    struct xdg_toplevel* xdg_toplevel,
    int32_t width,
    int32_t height
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_SET_MAX_SIZE,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0,
        width,
        height
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Set a minimum size for the window.
 *
 * The client can specify a minimum size so that the compositor does
 * not try to configure the window below this size.
 *
 * The width and height arguments are in window geometry coordinates.
 * See xdg_surface.set_window_geometry.
 *
 * Values set in this way are double-buffered, see wl_surface.commit.
 *
 * The compositor can use this information to allow or disallow
 * different states like maximize or fullscreen and draw accurate
 * animations.
 *
 * Similarly, a tiling window manager may use this information to
 * place and resize client windows in a more effective way.
 *
 * The client should not rely on the compositor to obey the minimum
 * size. The compositor may decide to ignore the values set by the
 * client and request a smaller size.
 *
 * If never set, or a value of zero in the request, means that the
 * client has no expected minimum size in the given dimension.
 * As a result, a client wishing to reset the minimum size
 * to an unspecified state can use zero for width and height in the
 * request.
 *
 * Requesting a minimum size to be larger than the maximum size of
 * a surface is illegal and will result in an invalid_size error.
 *
 * The width and height must be greater than or equal to zero. Using
 * strictly negative values for width and height will result in a
 * invalid_size error.
 */
inline static void xdg_toplevel_set_min_size(
    struct xdg_toplevel* xdg_toplevel,
    int32_t width,
    int32_t height
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_SET_MIN_SIZE,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0,
        width,
        height
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Maximize the surface.
 *
 * After requesting that the surface should be maximized, the compositor
 * will respond by emitting a configure event. Whether this configure
 * actually sets the window maximized is subject to compositor policies.
 * The client must then update its content, drawing in the configured
 * state. The client must also acknowledge the configure when committing
 * the new content (see ack_configure).
 *
 * It is up to the compositor to decide how and where to maximize the
 * surface, for example which output and what region of the screen should
 * be used.
 *
 * If the surface was already maximized, the compositor will still emit
 * a configure event with the "maximized" state.
 *
 * If the surface is in a fullscreen state, this request has no direct
 * effect. It may alter the state the surface is returned to when
 * unmaximized unless overridden by the compositor.
 */
inline static void xdg_toplevel_set_maximized(
    struct xdg_toplevel* xdg_toplevel
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_SET_MAXIMIZED,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Unmaximize the surface.
 *
 * After requesting that the surface should be unmaximized, the compositor
 * will respond by emitting a configure event. Whether this actually
 * un-maximizes the window is subject to compositor policies.
 * If available and applicable, the compositor will include the window
 * geometry dimensions the window had prior to being maximized in the
 * configure event. The client must then update its content, drawing it in
 * the configured state. The client must also acknowledge the configure
 * when committing the new content (see ack_configure).
 *
 * It is up to the compositor to position the surface after it was
 * unmaximized; usually the position the surface had before maximizing, if
 * applicable.
 *
 * If the surface was already not maximized, the compositor will still
 * emit a configure event without the "maximized" state.
 *
 * If the surface is in a fullscreen state, this request has no direct
 * effect. It may alter the state the surface is returned to when
 * unmaximized unless overridden by the compositor.
 */
inline static void xdg_toplevel_unset_maximized(
    struct xdg_toplevel* xdg_toplevel
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_UNSET_MAXIMIZED,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Make the surface fullscreen.
 *
 * After requesting that the surface should be fullscreened, the
 * compositor will respond by emitting a configure event. Whether the
 * client is actually put into a fullscreen state is subject to compositor
 * policies. The client must also acknowledge the configure when
 * committing the new content (see ack_configure).
 *
 * The output passed by the request indicates the client's preference as
 * to which display it should be set fullscreen on. If this value is NULL,
 * it's up to the compositor to choose which display will be used to map
 * this surface.
 *
 * If the surface doesn't cover the whole output, the compositor will
 * position the surface in the center of the output and compensate with
 * with border fill covering the rest of the output. The content of the
 * border fill is undefined, but should be assumed to be in some way that
 * attempts to blend into the surrounding area (e.g. solid black).
 *
 * If the fullscreened surface is not opaque, the compositor must make
 * sure that other screen content not part of the same surface tree (made
 * up of subsurfaces, popups or similarly coupled surfaces) are not
 * visible below the fullscreened surface.
 */
inline static void xdg_toplevel_set_fullscreen(
    struct xdg_toplevel* xdg_toplevel,
    struct wl_output* output
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_SET_FULLSCREEN,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0,
        output
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Make the surface no longer fullscreen.
 *
 * After requesting that the surface should be unfullscreened, the
 * compositor will respond by emitting a configure event.
 * Whether this actually removes the fullscreen state of the client is
 * subject to compositor policies.
 *
 * Making a surface unfullscreen sets states for the surface based on the
 * following:
 * * the state(s) it may have had before becoming fullscreen
 * * any state(s) decided by the compositor
 * * any state(s) requested by the client while the surface was fullscreen
 *
 * The compositor may include the previous window geometry dimensions in
 * the configure event, if applicable.
 *
 * The client must also acknowledge the configure when committing the new
 * content (see ack_configure).
 */
inline static void xdg_toplevel_unset_fullscreen(
    struct xdg_toplevel* xdg_toplevel
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_UNSET_FULLSCREEN,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0
    );
}

/**
 * @ingroup iface_xdg_toplevel
 *
 * Request that the compositor minimize your surface. There is no
 * way to know if the surface is currently minimized, nor is there
 * any way to unset minimization on this surface.
 *
 * If you are looking to throttle redrawing when minimized, please
 * instead use the wl_surface.frame event for this, as this will
 * also work with live previews on windows in Alt-Tab, Expose or
 * similar compositor features.
 */
inline static void xdg_toplevel_set_minimized(
    struct xdg_toplevel* xdg_toplevel
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_toplevel,
        XDG_TOPLEVEL_SET_MINIMIZED,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_toplevel),
        0
    );
}

#ifndef XDG_POPUP_ERROR_ENUM
#define XDG_POPUP_ERROR_ENUM

enum xdg_popup_error {
    /**
     * tried to grab after being mapped
     */
    XDG_POPUP_ERROR_INVALID_GRAB = 0,
};
#endif /* XDG_POPUP_ERROR_ENUM */

/**
 * @ingroup iface_xdg_popup
 * @struct xdg_popup_listener
 */
struct xdg_popup_listener {
    /**
     * configure the popup surface
     *
     * This event asks the popup surface to configure itself given
     * the configuration. The configured state should not be applied
     * immediately. See xdg_surface.configure for details.
     *
     * The x and y arguments represent the position the popup was
     * placed at given the xdg_positioner rule, relative to the upper
     * left corner of the window geometry of the parent surface.
     *
     * For version 2 or older, the configure event for an xdg_popup is
     * only ever sent once for the initial configuration. Starting with
     * version 3, it may be sent again if the popup is setup with an
     * xdg_positioner with set_reactive requested, or in response to
     * xdg_popup.reposition requests.
     * @param x x position relative to parent surface window geometry
     * @param y y position relative to parent surface window geometry
     * @param width window geometry width
     * @param height window geometry height
     */
    void (*configure)(
        void* data,
        struct xdg_popup* xdg_popup,
        int32_t x,
        int32_t y,
        int32_t width,
        int32_t height
    );
    /**
     * popup interaction is done
     *
     * The popup_done event is sent out when a popup is dismissed by
     * the compositor. The client should destroy the xdg_popup object
     * at this point.
     */
    void (*popup_done)(void* data, struct xdg_popup* xdg_popup);
    /**
     * signal the completion of a repositioned request
     *
     * The repositioned event is sent as part of a popup
     * configuration sequence, together with xdg_popup.configure and
     * lastly xdg_surface.configure to notify the completion of a
     * reposition request.
     *
     * The repositioned event is to notify about the completion of a
     * xdg_popup.reposition request. The token argument is the token
     * passed in the xdg_popup.reposition request.
     *
     * Immediately after this event is emitted, xdg_popup.configure and
     * xdg_surface.configure will be sent with the updated size and
     * position, as well as a new configure serial.
     *
     * The client should optionally update the content of the popup,
     * but must acknowledge the new popup configuration for the new
     * position to take effect. See xdg_surface.ack_configure for
     * details.
     * @param token reposition request token
     * @since 3
     */
    void (*repositioned)(
        void* data,
        struct xdg_popup* xdg_popup,
        uint32_t token
    );
};

/**
 * @ingroup iface_xdg_popup
 */
inline static int xdg_popup_add_listener(
    struct xdg_popup* xdg_popup,
    const struct xdg_popup_listener* listener,
    void* data
) {
    return wl_proxy_add_listener(
        (struct wl_proxy*)xdg_popup,
        (void (**)(void))listener,
        data
    );
}

#define XDG_POPUP_DESTROY 0
#define XDG_POPUP_GRAB 1
#define XDG_POPUP_REPOSITION 2

/**
 * @ingroup iface_xdg_popup
 */
#define XDG_POPUP_CONFIGURE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_popup
 */
#define XDG_POPUP_POPUP_DONE_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_popup
 */
#define XDG_POPUP_REPOSITIONED_SINCE_VERSION 3

/**
 * @ingroup iface_xdg_popup
 */
#define XDG_POPUP_DESTROY_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_popup
 */
#define XDG_POPUP_GRAB_SINCE_VERSION 1
/**
 * @ingroup iface_xdg_popup
 */
#define XDG_POPUP_REPOSITION_SINCE_VERSION 3

/** @ingroup iface_xdg_popup */
inline static void xdg_popup_set_user_data(
    struct xdg_popup* xdg_popup,
    void* user_data
) {
    wl_proxy_set_user_data((struct wl_proxy*)xdg_popup, user_data);
}

/** @ingroup iface_xdg_popup */
inline static void* xdg_popup_get_user_data(struct xdg_popup* xdg_popup) {
    return wl_proxy_get_user_data((struct wl_proxy*)xdg_popup);
}

inline static uint32_t xdg_popup_get_version(struct xdg_popup* xdg_popup) {
    return wl_proxy_get_version((struct wl_proxy*)xdg_popup);
}

/**
 * @ingroup iface_xdg_popup
 *
 * This destroys the popup. Explicitly destroying the xdg_popup
 * object will also dismiss the popup, and unmap the surface.
 *
 * If this xdg_popup is not the "topmost" popup, the
 * xdg_wm_base.not_the_topmost_popup protocol error will be sent.
 */
inline static void xdg_popup_destroy(struct xdg_popup* xdg_popup) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_popup,
        XDG_POPUP_DESTROY,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_popup),
        WL_MARSHAL_FLAG_DESTROY
    );
}

/**
 * @ingroup iface_xdg_popup
 *
 * This request makes the created popup take an explicit grab. An explicit
 * grab will be dismissed when the user dismisses the popup, or when the
 * client destroys the xdg_popup. This can be done by the user clicking
 * outside the surface, using the keyboard, or even locking the screen
 * through closing the lid or a timeout.
 *
 * If the compositor denies the grab, the popup will be immediately
 * dismissed.
 *
 * This request must be used in response to some sort of user action like a
 * button press, key press, or touch down event. The serial number of the
 * event should be passed as 'serial'.
 *
 * The parent of a grabbing popup must either be an xdg_toplevel surface or
 * another xdg_popup with an explicit grab. If the parent is another
 * xdg_popup it means that the popups are nested, with this popup now being
 * the topmost popup.
 *
 * Nested popups must be destroyed in the reverse order they were created
 * in, e.g. the only popup you are allowed to destroy at all times is the
 * topmost one.
 *
 * When compositors choose to dismiss a popup, they may dismiss every
 * nested grabbing popup as well. When a compositor dismisses popups, it
 * will follow the same dismissing order as required from the client.
 *
 * If the topmost grabbing popup is destroyed, the grab will be returned to
 * the parent of the popup, if that parent previously had an explicit grab.
 *
 * If the parent is a grabbing popup which has already been dismissed, this
 * popup will be immediately dismissed. If the parent is a popup that did
 * not take an explicit grab, an error will be raised.
 *
 * During a popup grab, the client owning the grab will receive pointer
 * and touch events for all their surfaces as normal (similar to an
 * "owner-events" grab in X11 parlance), while the top most grabbing popup
 * will always have keyboard focus.
 */
inline static void xdg_popup_grab(
    struct xdg_popup* xdg_popup,
    struct wl_seat* seat,
    uint32_t serial
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_popup,
        XDG_POPUP_GRAB,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_popup),
        0,
        seat,
        serial
    );
}

/**
 * @ingroup iface_xdg_popup
 *
 * Reposition an already-mapped popup. The popup will be placed given the
 * details in the passed xdg_positioner object, and a
 * xdg_popup.repositioned followed by xdg_popup.configure and
 * xdg_surface.configure will be emitted in response. Any parameters set
 * by the previous positioner will be discarded.
 *
 * The passed token will be sent in the corresponding
 * xdg_popup.repositioned event. The new popup position will not take
 * effect until the corresponding configure event is acknowledged by the
 * client. See xdg_popup.repositioned for details. The token itself is
 * opaque, and has no other special meaning.
 *
 * If multiple reposition requests are sent, the compositor may skip all
 * but the last one.
 *
 * If the popup is repositioned in response to a configure event for its
 * parent, the client should send an xdg_positioner.set_parent_configure
 * and possibly an xdg_positioner.set_parent_size request to allow the
 * compositor to properly constrain the popup.
 *
 * If the popup is repositioned together with a parent that is being
 * resized, but not in response to a configure event, the client should
 * send an xdg_positioner.set_parent_size request.
 */
inline static void xdg_popup_reposition(
    struct xdg_popup* xdg_popup,
    struct xdg_positioner* positioner,
    uint32_t token
) {
    wl_proxy_marshal_flags(
        (struct wl_proxy*)xdg_popup,
        XDG_POPUP_REPOSITION,
        NULL,
        wl_proxy_get_version((struct wl_proxy*)xdg_popup),
        0,
        positioner,
        token
    );
}

#ifdef __cplusplus
}
#endif

#endif
#endif // __linux__

#define PI 3.14159265

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

    void PrintStack() {
        for (int i = 0; i < 5; ++i) {
            std::cout << "Stack: " << aStack_[i] << std::endl;
        }
    }

    bool CheckEvent(EEvents _element) {
        for (int i = 0; i < iHead_; ++i) {
            if (aStack_[i] == _element) {
                return true;
            }
        }
        return false;
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
    uint32_t componentIds[ComponentsIndices::COMPONENTS_COUNT] = {};
    uint32_t componentCount = 0;
    void* components[ComponentsIndices::COMPONENTS_COUNT] = {};
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
class rigidBody {
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
class beholder {
public:
    Vector<float, 3> Position {0.0f, 0.0f, 0.0f};
    Vector<float, 3> forward {0.0f, 0.0, 0.0f};
};
} // namespace glvm

namespace glvm {
enum JsonType {
    JSON_INVALID_VALUE,
    JSON_OBJECT,
    JSON_FLOAT_NUMBER,
    JSON_INTEGER_NUMBER,
    JSON_STRING,
    JSON_BOOLEAN,
    JSON_NULL,
    JSON_ARRAY
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
        type = JSON_INVALID_VALUE;
    }

    JsonValue(std::string _string) {
        type = JSON_STRING;
        value.string = new std::string(_string);
    }

    JsonValue(double _float) {
        type = JSON_FLOAT_NUMBER;
        value.fNumber = _float;
    }

    JsonValue(int _int) {
        type = JSON_INTEGER_NUMBER;
        value.iNumber = _int;
    }

    JsonValue(bool _bool) {
        type = JSON_BOOLEAN;
        value.boolean = _bool;
    }

    JsonValue(const JsonValue& _value) {
        type = JSON_INVALID_VALUE;

        switch (_value.type) {
            case JSON_OBJECT:
                value.object = new HashMap<JsonValue>(*_value.value.object);
                break;
            case JSON_INTEGER_NUMBER:
                value.iNumber = _value.value.iNumber;
                break;
            case JSON_FLOAT_NUMBER:
                value.fNumber = _value.value.fNumber;
                break;
            case JSON_STRING:
                value.string = new std::string(*_value.value.string);
                break;
            case JSON_BOOLEAN:
                value.boolean = _value.value.boolean;
                break;
            case JSON_NULL:
                value.null = _value.value.null;
                break;
            case JSON_ARRAY:
                value.array = new std::vector<JsonValue>(*_value.value.array);
                break;
            default:
                break;
        }
        type = _value.type;
    }

    ~JsonValue() {
        switch (type) {
            case JSON_INVALID_VALUE:
                break;
            case JSON_OBJECT:
                delete value.object;
                break;
            case JSON_INTEGER_NUMBER:
                break;
            case JSON_FLOAT_NUMBER:
                break;
            case JSON_STRING:
                delete value.string;
                break;
            case JSON_BOOLEAN:
                break;
            case JSON_NULL:
                break;
            case JSON_ARRAY:
                delete value.array;
                break;
        }
    }

    void operator=(const JsonValue& _value) {
        switch (type) {
            case JSON_INVALID_VALUE:
                break;
            case JSON_OBJECT:
                delete value.object;
                break;
            case JSON_INTEGER_NUMBER:
                break;
            case JSON_FLOAT_NUMBER:
                break;
            case JSON_STRING:
                delete value.string;
                break;
            case JSON_BOOLEAN:
                break;
            case JSON_NULL:
                break;
            case JSON_ARRAY:
                delete value.array;
                break;
        }

        switch (_value.type) {
            case JSON_OBJECT:
                value.object = new HashMap<JsonValue>(*_value.value.object);
                break;
            case JSON_INTEGER_NUMBER:
                value.iNumber = _value.value.iNumber;
                break;
            case JSON_FLOAT_NUMBER:
                value.fNumber = _value.value.fNumber;
                break;
            case JSON_STRING:
                value.string = new std::string(*_value.value.string);
                break;
            case JSON_BOOLEAN:
                value.boolean = _value.value.boolean;
                break;
            case JSON_NULL:
                value.null = _value.value.null;
                break;
            case JSON_ARRAY:
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
            case JSON_OBJECT:
                return (*value.object)[key];
                break;
            default:
                throw std::out_of_range("Type is not a json object");
                break;
        }
    }

    JsonValue& operator[](const unsigned int index_) {
        switch (type) {
            case JSON_ARRAY:
                return (*value.array)[index_];
                break;
            default:
                throw std::out_of_range("Type is not a json array");
                break;
        }
    }

    bool isInvalid() {
        return type == JSON_INVALID_VALUE;
    }

    bool isObject() {
        return type == JSON_OBJECT;
    }

    bool isFloat() {
        return type == JSON_FLOAT_NUMBER;
    }

    bool isInterger() {
        return type == JSON_INTEGER_NUMBER;
    }

    bool isString() {
        return type == JSON_STRING;
    }

    bool isBoolean() {
        return type == JSON_BOOLEAN;
    }

    bool isNull() {
        return type == JSON_NULL;
    }

    bool isArray() {
        return type == JSON_ARRAY;
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
    bool containsElemnt(
        std::vector<std::vector<unsigned int>> container,
        unsigned int element
    );
    unsigned int getJointIndex(JsonValue joints, int searchingIndex);
};
} // namespace glvm

namespace glvm {
#define SHADOW_MAP_SIZE 1024
#define FLAT_SHADOW_MAP_SIZE 2048

#define VK_DEBUG_IMAGE_SET_RED "\x1b[31mVULKAN DEBUG IMAGE\x1b[0m"
#define VK_DEBUG_DESCRIPTOR_SET_RED "\x1b[31mVULKAN DEBUG DESCRIPTOR SET\x1b[0m"
#define VK_DEBUG_DESCRIPTOR_SET_LAYOUT_RED                                     \
    "\x1b[31mVULKAN DEBUG DESCRIPTOR SET LAYOUT\x1b[0m"
#define VK_DEBUG_PIPELINE_RED "\x1b[31mVULKAN DEBUG PIPELINE:\x1b[0m"
#define VK_DEBUG_PIPELINE_LAYOUT_RED                                           \
    "\x1b[31mVULKAN DEBUG PIPELINE LAYOUT:\x1b[0m"

#define DIRECTIONAL_LIGHTS_NUMBER 4
#define POINT_LIGHTS_NUMBER 32
#define SPOT_LIGHTS_NUMBER 8

#define MAX_JOINTS_NUMBER 128

#define INDIRECT_TEXTURE_WIDTH 7
#define INDIRECT_TEXTURE_HEIGHT 5
#define TILESET_ROW 8
#define TILESET_COLUMN 8

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

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB 0x2042
#define WGL_TYPE_RGBA_ARB 0x202B

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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// clang-format off
// clang-format on

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
            } else if (componentsTypes[i] == typeid(rigidBody).name()) {
                RemoveComponent<rigidBody>(entity);
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

#define CUBE_MAP_LAYER_NUMBER 6

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
    unsigned int diffuseTexureID;
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

#define ARCHETYPE_CHUNK_SIZE 16384

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
        (1ul << ComponentsIndices::ATTACK_COMPONENT)
        | (1ul << ComponentsIndices::HEALTH_COMPONENT)
        | (1ul << ComponentsIndices::FONT_COMPONENT);

    uint64_t fontRequiredMask = (1ull << ComponentsIndices::FONT_COMPONENT);
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
        rigidBody* rigidBodiesView = nullptr;
        colliderFlags* colliderFlagsView = nullptr;
        collider* collidersView = nullptr;
        mesh* meshesView = nullptr;
    } componentsView;

    uint64_t requiredMask = (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::MOVE_COMPONENT)
        | (1ul << ComponentsIndices::RIGID_BODY_COMPONENT)
        | (1ul << ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT);

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
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::CROSSHAIR_TAG_COMPONENT] =
            crosshairTagComponents;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::CROSSHAIR_TAG_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MESH_COMPONENT;
        componentIds[2] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[3] = ComponentsIndices::CROSSHAIR_TAG_COMPONENT;
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
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT] =
            directionalLights;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MESH_COMPONENT;
        componentIds[2] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[3] = ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT;
        componentCount = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t ENEMY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(enemy) + sizeof(state) + sizeof(font)
       + sizeof(animation) + sizeof(material) + sizeof(mesh) + sizeof(collider)
       + sizeof(colliderFlags) + sizeof(health) + sizeof(rigidBody)
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
    rigidBody rigidBodies[ENEMY_ARCH_CHUNK_SIZE];
    attack attacks[ENEMY_ARCH_CHUNK_SIZE];
    rotation rotations[ENEMY_ARCH_CHUNK_SIZE];
    move moves[ENEMY_ARCH_CHUNK_SIZE];

    EnemyArchetype() {
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::ENEMY_COMPONENT] = enemies;
        components[ComponentsIndices::STATE_COMPONENT] = states;
        components[ComponentsIndices::FONT_COMPONENT] = fonts;
        components[ComponentsIndices::ANIMATION_COMPONENT] = animations;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::COLLIDER_COMPONENT] = colliders;
        components[ComponentsIndices::COLLIDER_FLAGS_COMPONENT] = colliderFlags;
        components[ComponentsIndices::HEALTH_COMPONENT] = health;
        components[ComponentsIndices::RIGID_BODY_COMPONENT] = rigidBodies;
        components[ComponentsIndices::ATTACK_COMPONENT] = attacks;
        components[ComponentsIndices::ROTATION_COMPONENT] = rotations;
        components[ComponentsIndices::MOVE_COMPONENT] = moves;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::ENEMY_COMPONENT)
            | (1ull << ComponentsIndices::STATE_COMPONENT)
            | (1ull << ComponentsIndices::FONT_COMPONENT)
            | (1ull << ComponentsIndices::ANIMATION_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
            | (1ull << ComponentsIndices::HEALTH_COMPONENT)
            | (1ull << ComponentsIndices::RIGID_BODY_COMPONENT)
            | (1ull << ComponentsIndices::ATTACK_COMPONENT)
            | (1ull << ComponentsIndices::ROTATION_COMPONENT)
            | (1ull << ComponentsIndices::MOVE_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::ENEMY_COMPONENT;
        componentIds[2] = ComponentsIndices::STATE_COMPONENT;
        componentIds[3] = ComponentsIndices::FONT_COMPONENT;
        componentIds[4] = ComponentsIndices::ANIMATION_COMPONENT;
        componentIds[5] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[6] = ComponentsIndices::MESH_COMPONENT;
        componentIds[7] = ComponentsIndices::COLLIDER_COMPONENT;
        componentIds[8] = ComponentsIndices::COLLIDER_FLAGS_COMPONENT;
        componentIds[9] = ComponentsIndices::HEALTH_COMPONENT;
        componentIds[10] = ComponentsIndices::RIGID_BODY_COMPONENT;
        componentIds[11] = ComponentsIndices::ATTACK_COMPONENT;
        componentIds[12] = ComponentsIndices::ROTATION_COMPONENT;
        componentIds[13] = ComponentsIndices::MOVE_COMPONENT;
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
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::INVENTORY_COMPONENT] = invetories;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::INVENTORY_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MESH_COMPONENT;
        componentIds[2] = ComponentsIndices::INVENTORY_COMPONENT;
        componentIds[3] = ComponentsIndices::MATERIAL_COMPONENT;
        componentCount = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t ITEM_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(collider) + sizeof(colliderFlags)
       + sizeof(mesh) + sizeof(rigidBody) + sizeof(material) + sizeof(rotation)
       + sizeof(move) + sizeof(item));

struct ItemArchetype: Archetype {
    transform transforms[ITEM_ARCH_CHUNK_SIZE];
    collider colliders[ITEM_ARCH_CHUNK_SIZE];
    colliderFlags colliderFlags[ITEM_ARCH_CHUNK_SIZE];
    mesh meshes[ITEM_ARCH_CHUNK_SIZE];
    rigidBody rigidBodies[ITEM_ARCH_CHUNK_SIZE];
    material materials[ITEM_ARCH_CHUNK_SIZE];
    rotation rotations[ITEM_ARCH_CHUNK_SIZE];
    move moves[ITEM_ARCH_CHUNK_SIZE];
    item items[ITEM_ARCH_CHUNK_SIZE];

    ItemArchetype() {
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::COLLIDER_COMPONENT] = colliders;
        components[ComponentsIndices::COLLIDER_FLAGS_COMPONENT] = colliderFlags;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::RIGID_BODY_COMPONENT] = rigidBodies;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::ROTATION_COMPONENT] = rotations;
        components[ComponentsIndices::MOVE_COMPONENT] = moves;
        components[ComponentsIndices::ITEM_COMPONENT] = items;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::RIGID_BODY_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::ROTATION_COMPONENT)
            | (1ull << ComponentsIndices::MOVE_COMPONENT)
            | (1ull << ComponentsIndices::ITEM_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::COLLIDER_COMPONENT;
        componentIds[2] = ComponentsIndices::COLLIDER_FLAGS_COMPONENT;
        componentIds[3] = ComponentsIndices::MESH_COMPONENT;
        componentIds[4] = ComponentsIndices::RIGID_BODY_COMPONENT;
        componentIds[5] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[6] = ComponentsIndices::ROTATION_COMPONENT;
        componentIds[7] = ComponentsIndices::MOVE_COMPONENT;
        componentIds[8] = ComponentsIndices::ITEM_COMPONENT;
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
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::COLLIDER_COMPONENT] = colliders;
        components[ComponentsIndices::COLLIDER_FLAGS_COMPONENT] = colliderFlags;
        components[ComponentsIndices::ROTATION_COMPONENT] = rotations;
        components[ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT] =
            levelChunkTagComponents;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
            | (1ull << ComponentsIndices::ROTATION_COMPONENT)
            | (1ull << ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[2] = ComponentsIndices::MESH_COMPONENT;
        componentIds[3] = ComponentsIndices::COLLIDER_COMPONENT;
        componentIds[4] = ComponentsIndices::COLLIDER_FLAGS_COMPONENT;
        componentIds[5] = ComponentsIndices::ROTATION_COMPONENT;
        componentIds[6] = ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT;
        componentCount = 7;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t PLAYER_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(transform) + sizeof(beholder) + sizeof(collider)
       + sizeof(colliderFlags) + sizeof(mesh) + sizeof(rigidBody)
       + sizeof(health) + sizeof(material) + sizeof(move) + sizeof(attack)
       + sizeof(animation) + sizeof(font) + sizeof(rotation)
       + sizeof(playerTagComponent));

struct PlayerArchetype: Archetype {
    transform transforms[PLAYER_ARCH_CHUNK_SIZE];
    beholder beholders[PLAYER_ARCH_CHUNK_SIZE];
    collider colliders[PLAYER_ARCH_CHUNK_SIZE];
    colliderFlags colliderFlags[PLAYER_ARCH_CHUNK_SIZE];
    mesh meshes[PLAYER_ARCH_CHUNK_SIZE];
    rigidBody rigidBodies[PLAYER_ARCH_CHUNK_SIZE];
    health health[PLAYER_ARCH_CHUNK_SIZE];
    material materials[PLAYER_ARCH_CHUNK_SIZE];
    move moves[PLAYER_ARCH_CHUNK_SIZE];
    attack attacks[PLAYER_ARCH_CHUNK_SIZE];
    animation animations[PLAYER_ARCH_CHUNK_SIZE];
    font fonts[PLAYER_ARCH_CHUNK_SIZE];
    rotation rotations[PLAYER_ARCH_CHUNK_SIZE];
    playerTagComponent playerTagComponents[PLAYER_ARCH_CHUNK_SIZE];

    PlayerArchetype() {
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::VIEW_COMPONENT] = beholders;
        components[ComponentsIndices::COLLIDER_COMPONENT] = colliders;
        components[ComponentsIndices::COLLIDER_FLAGS_COMPONENT] = colliderFlags;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::RIGID_BODY_COMPONENT] = rigidBodies;
        components[ComponentsIndices::HEALTH_COMPONENT] = health;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::MOVE_COMPONENT] = moves;
        components[ComponentsIndices::ATTACK_COMPONENT] = attacks;
        components[ComponentsIndices::ANIMATION_COMPONENT] = animations;
        components[ComponentsIndices::FONT_COMPONENT] = fonts;
        components[ComponentsIndices::ROTATION_COMPONENT] = rotations;
        components[ComponentsIndices::PLAYER_TAG_COMPONENT] =
            playerTagComponents;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::VIEW_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::RIGID_BODY_COMPONENT)
            | (1ull << ComponentsIndices::HEALTH_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::MOVE_COMPONENT)
            | (1ull << ComponentsIndices::ATTACK_COMPONENT)
            | (1ull << ComponentsIndices::ANIMATION_COMPONENT)
            | (1ull << ComponentsIndices::FONT_COMPONENT)
            | (1ull << ComponentsIndices::ROTATION_COMPONENT)
            | (1ull << ComponentsIndices::PLAYER_TAG_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::VIEW_COMPONENT;
        componentIds[2] = ComponentsIndices::COLLIDER_COMPONENT;
        componentIds[3] = ComponentsIndices::COLLIDER_FLAGS_COMPONENT;
        componentIds[4] = ComponentsIndices::MESH_COMPONENT;
        componentIds[5] = ComponentsIndices::RIGID_BODY_COMPONENT;
        componentIds[6] = ComponentsIndices::HEALTH_COMPONENT;
        componentIds[7] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[8] = ComponentsIndices::MOVE_COMPONENT;
        componentIds[9] = ComponentsIndices::ATTACK_COMPONENT;
        componentIds[10] = ComponentsIndices::ANIMATION_COMPONENT;
        componentIds[11] = ComponentsIndices::FONT_COMPONENT;
        componentIds[12] = ComponentsIndices::ROTATION_COMPONENT;
        componentIds[13] = ComponentsIndices::PLAYER_TAG_COMPONENT;
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
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::POINT_LIGHT_COMPONENT] = pointLights;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::POINT_LIGHT_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MESH_COMPONENT;
        componentIds[2] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[3] = ComponentsIndices::POINT_LIGHT_COMPONENT;
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
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::COLLIDER_COMPONENT] = colliders;
        components[ComponentsIndices::COLLIDER_FLAGS_COMPONENT] = colliderFlags;
        components[ComponentsIndices::ROTATION_COMPONENT] = rotations;
        components[ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT] =
            projectileBundles;
        components[ComponentsIndices::HEALTH_COMPONENT] = heath;
        components[ComponentsIndices::ATTACK_COMPONENT] = attacks;
        components[ComponentsIndices::FONT_COMPONENT] = fonts;
        components[ComponentsIndices::PROJECTILE_TAG_COMPONENT] =
            projectileTagComponents;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
            | (1ull << ComponentsIndices::ROTATION_COMPONENT)
            | (1ull << ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT)
            | (1ull << ComponentsIndices::HEALTH_COMPONENT)
            | (1ull << ComponentsIndices::ATTACK_COMPONENT)
            | (1ull << ComponentsIndices::FONT_COMPONENT)
            | (1ull << ComponentsIndices::PROJECTILE_TAG_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MESH_COMPONENT;
        componentIds[2] = ComponentsIndices::COLLIDER_COMPONENT;
        componentIds[3] = ComponentsIndices::COLLIDER_FLAGS_COMPONENT;
        componentIds[4] = ComponentsIndices::ROTATION_COMPONENT;
        componentIds[5] = ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT;
        componentIds[6] = ComponentsIndices::HEALTH_COMPONENT;
        componentIds[7] = ComponentsIndices::ATTACK_COMPONENT;
        componentIds[8] = ComponentsIndices::FONT_COMPONENT;
        componentIds[9] = ComponentsIndices::PROJECTILE_TAG_COMPONENT;
        componentCount = 10;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t RIGID_BODY_ARCH_CHUNK_SIZE =
    ARCHETYPE_CHUNK_SIZE / (sizeof(glvm::transform) + sizeof(glvm::rigidBody));

struct RigidBodyArch {
    transform transforms[RIGID_BODY_ARCH_CHUNK_SIZE];
    rigidBody rigidBodies[RIGID_BODY_ARCH_CHUNK_SIZE];
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
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::SPOT_LIGHT_COMPONENT] = spotLights;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::SPOT_LIGHT_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::MESH_COMPONENT;
        componentIds[2] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[3] = ComponentsIndices::SPOT_LIGHT_COMPONENT;
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
        components[ComponentsIndices::TRANSFORM_COMPONENT] = transforms;
        components[ComponentsIndices::COLLIDER_COMPONENT] = colliders;
        components[ComponentsIndices::COLLIDER_FLAGS_COMPONENT] = colliderFlags;
        components[ComponentsIndices::MESH_COMPONENT] = meshes;
        components[ComponentsIndices::MATERIAL_COMPONENT] = materials;
        components[ComponentsIndices::FONT_COMPONENT] = fonts;
        components[ComponentsIndices::ROTATION_COMPONENT] = rotations;
        components[ComponentsIndices::STATIC_MESH_TAG_COMPONENT] =
            staticMeshTagComponents;

        mask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
            | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
            | (1ull << ComponentsIndices::MESH_COMPONENT)
            | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
            | (1ull << ComponentsIndices::FONT_COMPONENT)
            | (1ull << ComponentsIndices::ROTATION_COMPONENT)
            | (1ull << ComponentsIndices::STATIC_MESH_TAG_COMPONENT);

        componentIds[0] = ComponentsIndices::TRANSFORM_COMPONENT;
        componentIds[1] = ComponentsIndices::COLLIDER_COMPONENT;
        componentIds[2] = ComponentsIndices::COLLIDER_FLAGS_COMPONENT;
        componentIds[3] = ComponentsIndices::MESH_COMPONENT;
        componentIds[4] = ComponentsIndices::MATERIAL_COMPONENT;
        componentIds[5] = ComponentsIndices::FONT_COMPONENT;
        componentIds[6] = ComponentsIndices::ROTATION_COMPONENT;
        componentIds[7] = ComponentsIndices::STATIC_MESH_TAG_COMPONENT;
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

#define XKEY_I 0x69
#define XKEY_ESCAPE 0xff1b
#define XKEY_A 0x61
#define XKEY_D 0x64
#define XKEY_S 0x73
#define XKEY_W 0x77
#define XKEY_SPACE 0x20

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
        "../../../assets/shaders/flatShadowMapShaders/vertFlatShadowMap.spv";
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
        "../../../assets/shaders/flatShadowMapShaders/vertFlatShadowMap.spv";
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
        "../../../assets/shaders/cubeShadowMapShaders/vertCubeShadowMap.spv";
    pipelineConfigs[POINT_LIGHT_PIPELINE].fragShader =
        "../../../assets/shaders/cubeShadowMapShaders/fragCubeShadowMap.spv";
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
        "../../../assets/shaders/hudShaders/hud_vert.spv";
    pipelineConfigs[HUD_PIPELINE].fragShader =
        "../../../assets/shaders/hudShaders/hud_frag.spv";
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
        "../../../assets/shaders/fontShaders/font_vert.spv";
    pipelineConfigs[FONT_PIPELINE].fragShader =
        "../../../assets/shaders/fontShaders/font_frag.spv";
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
        "../../../assets/shaders/hud_screen_shaders/vert_hud_screen.spv";
    pipelineConfigs[HUD_SCREEN_PIPELINE].fragShader =
        "../../../assets/shaders/hud_screen_shaders/frag_hud_screen.spv";
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
        "../../../assets/shaders/ui_shaders/vert_ui.spv";
    pipelineConfigs[UI_PIPELINE].fragShader =
        "../../../assets/shaders/ui_shaders/frag_ui.spv";
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
        "../../../assets/shaders/ui_icons_shaders/vert_ui_icons.spv";
    pipelineConfigs[UI_ICONS_PIPELINE].fragShader =
        "../../../assets/shaders/ui_icons_shaders/frag_ui_icons.spv";
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
        "../../../assets/shaders/virtualTextures/virtualTexturesVert.spv";
    pipelineConfigs[VIRTUAL_TEXTURES_PIPELINE].fragShader =
        "../../../assets/shaders/virtualTextures/virtualTexturesFrag.spv";
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
        "../../../assets/shaders/mainRendererShaders/vert.spv";
    pipelineConfigs[MAIN_RENDER_PIPELINE].fragShader =
        "../../../assets/shaders/mainRendererShaders/frag.spv";
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
        "../../../assets/shaders/sdf/sdf_vert.spv";
    pipelineConfigs[SDF_PIPELINE].fragShader =
        "../../../assets/shaders/sdf/sdf_frag.spv";
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
        (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ull << ComponentsIndices::CROSSHAIR_TAG_COMPONENT);

    uint64_t inventoryRequiredMask =
        (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ull << ComponentsIndices::INVENTORY_COMPONENT)
        | (1ull << ComponentsIndices::MESH_COMPONENT);

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
        (1ull << ComponentsIndices::INVENTORY_COMPONENT);

    uint64_t itemRequiredMask = (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::ITEM_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT)
        | (1ul << ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << ComponentsIndices::COLLIDER_FLAGS_COMPONENT);

    uint64_t crosshairRequiredMask =
        (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::CROSSHAIR_TAG_COMPONENT);

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
        rigidBody* playerRigidBody = nullptr;

        // Components related to archetypes contains rigid.
        transform* transforms = nullptr;
        rigidBody* rigidBodies = nullptr;
        move* moves = nullptr;
        item* items = nullptr;
    } componentsView;

    uint64_t playerRequiredMask =
        (1ull << ComponentsIndices::PLAYER_TAG_COMPONENT);
    uint64_t rigidBodyRequiredMask =
        (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::RIGID_BODY_COMPONENT)
        | (1ul << ComponentsIndices::MOVE_COMPONENT);

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
        (1ull << ComponentsIndices::PLAYER_TAG_COMPONENT);

    uint64_t requiredMask = (1ull << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ull << ComponentsIndices::MATERIAL_COMPONENT)
        | (1ull << ComponentsIndices::MESH_COMPONENT)
        | (1ull << ComponentsIndices::COLLIDER_COMPONENT)
        | (1ull << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ull << ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT);

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

    uint64_t requiredMask = (1ul << ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT);

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
        (1ull << ComponentsIndices::PLAYER_TAG_COMPONENT);

    uint64_t enemyRequiredMask = (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::STATE_COMPONENT)
        | (1ul << ComponentsIndices::ENEMY_COMPONENT);

    uint64_t projectileRequiredMask =
        (1ull << ComponentsIndices::PROJECTILE_TAG_COMPONENT);

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
        (1ull << ComponentsIndices::PLAYER_TAG_COMPONENT);

    uint64_t projectileRequiredMask =
        (1ull << ComponentsIndices::PROJECTILE_TAG_COMPONENT);

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

    uint64_t requiredMask = (1ul << ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT);

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
        (1ul << ComponentsIndices::DIRECTIONAL_LIGHT_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT);
    Archetype* cachedSpotLigthArchetypes[32];
    uint32_t spotLightArchetypesNumber = 0;
    uint64_t spotLightRequiredMask =
        (1ul << ComponentsIndices::SPOT_LIGHT_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT);
    Archetype* cachedPointLigthArchetypes[32];
    uint32_t pointLightArchetypesNumber = 0;
    uint64_t pointLightRequiredMask =
        (1ul << ComponentsIndices::POINT_LIGHT_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT);
    Archetype* cachedAnimationActorsArchetypes[32];
    uint32_t animationActorsArchetypesNumber = 0;
    uint64_t animatedActorsRequiredMask =
        (1ul << ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << ComponentsIndices::ANIMATION_COMPONENT)
        | (1ul << ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT);
    Archetype* cachedStaticActorsArchetypes[32];
    uint32_t staticActorsArchetypesNumber = 0;
    uint64_t staticActorsRequiredMask =
        (1ul << ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << ComponentsIndices::STATIC_MESH_TAG_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT);
    Archetype* cachedPlayerArchetypes[32];
    uint32_t playerArchetypesNumber = 0;
    uint64_t playerRequiredMask =
        (1ul << ComponentsIndices::PLAYER_TAG_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::VIEW_COMPONENT);
    Archetype* cachedAnimationArchetypes[32];
    uint32_t animationArchetypesNumber = 0;
    uint64_t animationRequiredMask =
        (1ul << ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << ComponentsIndices::ANIMATION_COMPONENT)
        | (1ul << ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT);
    Archetype* cachedCrosshairActorsArchetypes[32];
    uint32_t crosshairActorsArchetypesNumber = 0;
    uint64_t crosshairRequiredMask =
        (1ul << ComponentsIndices::CROSSHAIR_TAG_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT);
    Archetype* cachedLevelChunkActorsArchetypes[32];
    uint32_t levelChunkActorsArchetypesNumber = 0;
    uint64_t levelChunkRequiredMask =
        (1ul << ComponentsIndices::MATERIAL_COMPONENT)
        | (1ul << ComponentsIndices::LEVEL_CHUNK_TAG_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT);
    Archetype* cachedProjectileActorsArchetypes[32];
    uint32_t projectileActorsArchetypesNumber = 0;
    uint64_t projectileRequiredMask =
        (1ul << ComponentsIndices::PROJECTILE_BUNDLE_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT);
    Archetype* cachedItemActorsArchetypes[32];
    uint32_t itemActorsArchetypesNumber = 0;
    uint64_t rotationItemRequiredMask =
        (1ul << ComponentsIndices::ITEM_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ul << ComponentsIndices::ROTATION_COMPONENT)
        | (1ul << ComponentsIndices::MATERIAL_COMPONENT);
    Archetype* cachedInventoryArchetypes[32];
    uint32_t inventoryArchetypesNumber = 0;
    uint64_t inventoryRequiredMask =
        (1ul << ComponentsIndices::INVENTORY_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::MATERIAL_COMPONENT);
    Archetype* cachedItemArchetypes[32];
    uint32_t itemArchetypesNumber = 0;
    uint64_t itemRequiredMask = (1ul << ComponentsIndices::ITEM_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT)
        | (1ul << ComponentsIndices::COLLIDER_COMPONENT)
        | (1ul << ComponentsIndices::COLLIDER_FLAGS_COMPONENT)
        | (1ul << ComponentsIndices::MATERIAL_COMPONENT);
    Archetype* cachedHealthBarsArchetypes[32];
    uint32_t healthBarsArchetypesNumber = 0;
    uint64_t healthBarsRequiredMask =
        (1ul << ComponentsIndices::HEALTH_COMPONENT)
        | (1ul << ComponentsIndices::MESH_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT);
    Archetype* cachedFontsArchetypes[32];
    uint32_t fontsArchetypesNumber = 0;
    uint64_t fontRequiredMask = (1ul << ComponentsIndices::FONT_COMPONENT)
        | (1ul << ComponentsIndices::TRANSFORM_COMPONENT);
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
