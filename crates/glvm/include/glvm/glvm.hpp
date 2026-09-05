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
#define WIN32_LEAN_AND_MEAN // NOLINT(readability-identifier-naming)
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
constexpr uint64_t ENTITY_BITS_MASK = (1ull << ENTITY_ID_BITS) - 1;

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

constexpr uint64_t PLAYER_COMPONENT_MASK =
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

constexpr uint64_t ENEMY_COMPONENT_MASK =
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

constexpr uint64_t STATIC_MESH_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::FontComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::StaticMeshTagComponent);

constexpr uint64_t CROSSHAIR_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::CrosshairTagComponent);

constexpr uint64_t ITEM_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::RigidBodyComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::MoveComponent)
    | (1ull << ComponentsIndices::ItemComponent);

constexpr uint64_t INVENTORY_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::InventoryComponent)
    | (1ull << ComponentsIndices::MaterialComponent);

constexpr uint64_t DIRECTIONAL_LIGHT_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::DirectionalLightComponent);

constexpr uint64_t SPOT_LIGHT_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::SpotLightComponent);

constexpr uint64_t POINT_LIGHT_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::PointLightComponent);

constexpr uint64_t LEVEL_CHUNK_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::LevelChunkTagComponent);

constexpr uint64_t PROJECTILE_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::ProjectileBundleComponent)
    | (1ull << ComponentsIndices::ProjectileTagComponent);
}; // namespace glvm

namespace glvm {
struct Actor {};
} // namespace glvm

namespace glvm {
struct Animation {
    uint32_t current_animation_frame = 0;
    float frame_accumulator = 0.0f;
};
} // namespace glvm

namespace glvm {}

namespace glvm {
struct Attack {
    float damage;
};
} // namespace glvm

namespace glvm {
class Collider {
public:
    std::vector<unsigned int> colliders;
};
} // namespace glvm

namespace glvm {
struct ColliderFlags {
    // 0001 = wallCollision; 0010 = groundCollision; 0100 = roofCollision; 1000
    // = itemDrag.
    int flags : 4;
};
}; // namespace glvm

namespace glvm {
struct Controller {};
} // namespace glvm

namespace glvm {
struct Crosshair {};
}; // namespace glvm

namespace glvm {
struct Damage {
    float maximum_damage;
    float minimum_damage;
    float critical_hit_rate;
    float critical_modifier;
};
} // namespace glvm

namespace glvm {
struct Enemy {
    float detect_radius;
};
} // namespace glvm

namespace glvm {
struct Font {
    std::vector<char> font_string;
    float life_time;
    bool removeble;
};
} // namespace glvm

namespace glvm {
struct Health {
    float max_health;
    float current_health;
};
} // namespace glvm

namespace glvm {
struct Hud {
    bool hud = false;
    bool gltf = true;
};
}; // namespace glvm

namespace glvm {
struct InterfaceUi {};
}; // namespace glvm

namespace glvm {
struct InventorySlot {
    unsigned int item_entity = UINT_MAX;
};
} // namespace glvm

namespace glvm {
struct ItemSlotType {
    unsigned int height;
    unsigned int width;
};

struct Item {
    // Array that contain entities with inventorySlotComponent.
    std::vector<unsigned int> occupied_slots;
    ItemSlotType item_slot_type;
    bool is_actor;
};
} // namespace glvm

namespace glvm {
struct Physics {
    float gravity_accumulator = 0.0f;
};
}; // namespace glvm

namespace glvm {
class Projectile {
public:
    unsigned int owner;
    bool b_collision_status = false;
    float f_damage;
    float f_speed;
    float f_flying_range;
    float damage;
};
} // namespace glvm

namespace glvm {
struct Rotation {
    float yaw = 0.0f;
    float pitch = 0.0f;
};
}; // namespace glvm

namespace glvm {
struct TextureComponent {
    unsigned int id;
};
} // namespace glvm

namespace glvm {
struct MeshHandle {
    uint32_t id;
};

struct Mesh {
    MeshHandle handle;
    bool gltf = true;
};
} // namespace glvm

constexpr unsigned int K_I_UINT_MAX = 4000000000;
constexpr int K_I_NULL = 0;
constexpr int BOX_INDICES_FOR_INDEX_BUFFER[36] = {0, 1, 2, 3, 0, 2, 4, 0, 3,
                                                  7, 4, 3, 4, 5, 1, 0, 4, 1,
                                                  1, 5, 6, 2, 1, 6, 5, 4, 7,
                                                  6, 5, 7, 3, 2, 6, 7, 3, 6};

namespace glvm {

class CStack;

enum EEvents {
    EDefault,
    EKeyreleaseA,
    EKeyreleaseD,
    EKeyreleaseS,
    EKeyreleaseW,
    EKeyreleaseJump,
    EGravityCollisionFlag,
    ERender,
    EAtack,
    ESpawn,
    EJump,
    EInventory,
    EInventoryRelease,
    EMoveForward,
    EMoveBackward,
    EMoveLeft,
    EMoveRight,
    EMoveDiagonalFb,
    EMoveDiagonalFl,
    EMoveDiagonalLb,
    EMoveDiagonalBr,
    EMousePointerPosition,
    EMouseLeftButtonRelease,
    EMouseLeftButton,
    EMouseRightButtonRelease,
    EMouseRightButton,
    ECursorReleased,
    EGameLoopKill,
    EEmpty,
};

struct SMousePointerPosition {
    int position_x;
    int position_y;
    int offset_x = 0;
    int offset_y = 0;
    float pitch;
    float yaw;
};

class CEvent {
    EEvents e_event;
    EEvents next_event;

public:
    SMousePointerPosition mouse_pointer_position;
    bool next_event_flag = false;

    CEvent();
    EEvents& get_event();
    void set_event(EEvents new_event);
    void set_next_event(EEvents new_event);
    EEvents get_next_event();
    void set_last_event(CStack stack);

    bool is_left_mouse_button_released = true;
};

} // namespace glvm

template<typename T>
struct Node {
    std::string key;
    T value;
    Node* next = nullptr;

    Node(const char* node_key) : key(node_key) {
    }
};

template<typename S>
class HashMap {
    unsigned int capacity = 10;

public:
    Node<S>** hash_map = nullptr;

    HashMap() {
        hash_map = new Node<S>*[capacity];

        for (unsigned int i = 0; i < capacity; ++i) {
            hash_map[i] = nullptr;
        }
    }

    HashMap(const HashMap<S>& other) {
        capacity = other.capacity;
        hash_map = new Node<S>*[capacity];

        for (unsigned int i = 0; i < capacity; ++i) {
            hash_map[i] = nullptr;
        }

        for (unsigned int i = 0; i < capacity; ++i) {
            Node<S>* current_node = other.hash_map[i];

            while (current_node != nullptr) {
                unsigned int hash = hash_function(current_node->key.c_str());
                link(hash_map[hash], current_node->key.c_str()) =
                    current_node->value;

                current_node = current_node->next;
            }
        }
    }

    void operator=(const HashMap<S>& other) {
        capacity = other.capacity;
        hash_map = new Node<S>*[capacity];

        for (int i = 0; i < capacity; ++i) {
            hash_map[i] = nullptr;
        }

        for (int i = 0; i < capacity; ++i) {
            Node<S>* current_node = other.hash_map[i];
            while (current_node != nullptr) {
                unsigned int hash = hash_function(current_node->key_.c_str());
                link(hash_map[hash], current_node->key_.c_str()) =
                    current_node->value_;

                current_node = current_node->next_;
            }
        }
    }

    S& operator[](const char* lookup_key) {
        unsigned int hash = hash_function(lookup_key);

        if (hash >= capacity) {
            rehash(hash);
        }

        return link(hash_map[hash], lookup_key);
    }

    bool contain(const char* lookup_key) {
        unsigned int hash = hash_function(lookup_key);
        Node<S>* node = hash_map[hash];

        while (node != nullptr) {
            if (node->key == lookup_key) {
                return true;
            } else {
                node = node->next;
            }
        }

        return false;
    }

    ~HashMap() {
        for (unsigned int i = 0; i < capacity; ++i) {
            Node<S>* node = hash_map[i];
            while (node != nullptr) {
                Node<S>* node_temp = node;
                node = node->next;
                delete node_temp;
            }
        }
        delete[] hash_map;
        hash_map = nullptr;
    }

    bool search_key(const char* key) {
        for (int i = 0; i < capacity; ++i) {
            if (hash_map[i] != nullptr && hash_map[i]->key_ == key) {
                return true;
            }
        }

        return false;
    }

    unsigned int get_capacity() {
        return capacity;
    }

private:
    S& link(Node<S>*& link_node, const char* lookup_key) {
        if (link_node == nullptr) {
            link_node = new Node<S>(lookup_key);
            return link_node->value;
        } else {
            if (link_node->key == lookup_key) {
                return link_node->value;
            }

            return link(link_node->next, lookup_key);
        }
    }

    unsigned int hash_function(const char* lookup_key) {
        unsigned int sum = 0;
        unsigned int counter = 0;
        while (lookup_key[counter] != '\0') {
            sum += lookup_key[counter];
            ++counter;
        }

        unsigned int reminder = sum % capacity;
        return reminder;
    }

    void rehash(unsigned int required_capacity) {
        unsigned int reminder = required_capacity % 10;
        capacity = required_capacity + (10 - reminder);

        Node<S>** temp = new Node<S>*[capacity];

        for (unsigned int i = 0; i < capacity; ++i) {
            temp[i] = hash_map[i];
        }

        delete[] hash_map;
        hash_map = nullptr;
        hash_map = temp;
    }
};

namespace glvm {
class IChrono {
public:
    virtual ~IChrono() {
    }

    virtual double init_frequency() = 0;
    virtual double reset() = 0;
    virtual double get_elapsed() = 0;
};
} // namespace glvm

namespace glvm {
struct Scalar {
    float value;
};

// Vector in 3D PGA.
struct Plane {
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
struct Line {
    float rx;
    float ry;
    float rz;
    float ix;
    float iy;
    float iz;
};

struct Rline {
    float rx;
    float ry;
    float rz;
};

struct Iline {
    float ix;
    float iy;
    float iz;
};

// Trivector.
struct Point {
    float x;
    float y;
    float z;
    float w;
};

struct PseudoScalar {
    float w;
};

struct Motor {
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

struct Rotor {
    float rx;
    float ry;
    float rz;
    float rw;
};

struct Translator {
    float ix;
    float iy;
    float iz;
    float iw;
};

inline Line operator-(Line line) {
    return {
        .rx = -line.rx,
        .ry = -line.ry,
        .rz = -line.rz,
        .ix = -line.ix,
        .iy = -line.iy,
        .iz = -line.iz
    };
}

inline Point operator-(Point point) {
    return {.x = -point.x, .y = -point.y, .z = -point.z, .w = -point.w};
}

// Dual operator.

inline Point operator!(const Plane& plane) {
    return Point {.x = plane.x, .y = plane.y, .z = plane.z, .w = plane.w};
}

inline Plane operator!(const Point& point) {
    return Plane {.x = point.x, .y = point.y, .z = point.z, .w = point.w};
}

inline Line operator!(const Line& line) {
    return {
        .rx = line.ix,
        .ry = line.iy,
        .rz = line.iz,
        .ix = line.rx,
        .iy = line.ry,
        .iz = line.rz
    };
}

inline Scalar operator!(const PseudoScalar& pseudo_scalar) {
    return Scalar {.value = pseudo_scalar.w};
}

inline PseudoScalar operator!(const Scalar& scalar) {
    return PseudoScalar {.w = scalar.value};
}

inline Plane normalize(const Plane& plane) {
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

inline Line normalize(const Line& line) {
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
inline Plane operator~(const Plane& plane) {
    return plane;
}

inline Line operator~(const Line& line) {
    return -line;
}

inline Point operator~(const Point& point) {
    return -point;
}

inline Scalar operator~(const Scalar& scalar) {
    return scalar;
}

inline Rline operator~(const Rline& rline) {
    return rline;
}

// Inner product.

// Scalar product of the plane normals.
inline float operator|(const Plane& plane0, const Plane& plane1) {
    return plane0.x * plane1.x + plane0.y * plane1.y + plane0.z * plane1.z;
}

// This gives the oriented distance from the point to the plane (if normalized).
inline Line operator|(const Plane& plane, const Point& point) {
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
inline Point operator|(const Plane& plane, const Line& line) {
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
inline float operator|(const Line& line0, const Line& line1) {
    return -line0.rx * line1.rx - line0.ry * line1.ry - line0.rz * line1.rz;
}

// A line through a point defines a plane. the form is similar to plane ⋅ line,
// but semantically it is a plane containing l and pt
inline Plane operator|(const Line& line, const Point& point) {
    return {
        .x = -line.rx * point.w,
        .y = -line.ry * point.w,
        .z = -line.rz * point.w,
        .w = line.rx * point.x + line.ry * point.y + line.rz * point.z
    };
}

// Points do not have an inner product: it is always zero (if strictly by
// definition).
inline Scalar operator|(const Point& point0, const Point& point1) {
    return {.value = -point0.w * point1.w};
}

// Outer product.

// plane ^ plane -> line (those intersection).
inline Line operator^(const Plane& plane0, const Plane& plane1) {
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
inline PseudoScalar operator^(const Plane& plane, const Point& point) {
    return {
        .w = plane.x * point.x + plane.y * point.y + plane.z * point.z
            + plane.w * point.w
    };
}

// line ^ point -> plane.
inline float operator^(const Line& line, const Point& point) {
    return 0.0;
}

// point ∧ point → line (through two points).
inline float operator^(const Point& point0, const Point& point1) {
    return 0.0;
}

// line ∧ line → point (if intersecting). If w == 0, then the lines do not
// intersect (the result is a point at infinity).
inline PseudoScalar operator^(const Line& line0, const Line& line1) {
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
inline Point operator^(const Plane& plane, const Line& line) {
    return {
        .x = plane.y * line.iz - plane.z * line.iy - plane.w * line.rx,
        .y = -plane.x * line.iz + plane.z * line.ix - plane.w * line.ry,
        .z = plane.x * line.iy - plane.y * line.ix - plane.w * line.rz,
        .w = plane.x * line.rx + plane.y * line.ry + plane.z * line.rz
    };
}

inline Point operator^(const Line& line, const Plane& plane) {
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
inline float operator&(Plane plane0, Plane plane1) {
    return 0.0f;
}

inline Scalar operator&(Plane plane, Point point) {
    // Plane below link with dual point from outer product and point below link
    // with dual plane from outer product.
    return {
        .value = -plane.x * point.x + -plane.y * point.y + -plane.z * point.z
            + -plane.w * point.w
    };
}

inline Scalar operator&(Point point, Plane plane) {
    // Plane below link with dual point from outer product and point below link
    // with dual plane from outer product.
    return {
        .value = plane.x * point.x + plane.y * point.y + plane.z * point.z
            + plane.w * point.w
    };
}

inline Plane operator&(Point point, Line line) {
    // Point below link with dual plane from outer product and line below link
    // with dual line from outer product.
    return {
        .x = point.y * line.rz - point.z * line.ry - point.w * line.ix,
        .y = -point.x * line.rz + point.z * line.rx - point.w * line.iy,
        .z = point.x * line.ry - point.y * line.rx - point.w * line.iz,
        .w = point.x * line.ix + point.y * line.iy + point.z * line.iz
    };
}

inline Plane operator&(Line line, Point point) {
    // Point below link with dual plane from outer product and line below link
    // with dual line from outer product.
    return {
        .x = -line.ix * point.w - line.ry * point.z + line.rz * point.y,
        .y = -line.iy * point.w + line.rx * point.z - line.rz * point.x,
        .z = -line.iz * point.w - line.rx * point.y + line.ry * point.x,
        .w = line.ix * point.x + line.iy * point.y + line.iz * point.z,
    };
}

inline Line operator&(Point point0, Point point1) {
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

inline Scalar operator&(Line line0, Line line1) {
    return {
        .value = line0.rx * line1.ix + line0.ry * line1.iy + line0.rz * line1.iz
            + line0.ix * line1.rx + line0.iy * line1.ry + line0.iz * line1.rz
    };
}

inline float operator&(Plane plane, Line line) {
    return 0.0f;
}

inline float operator&(Line line, Plane plane) {
    return 0.0f;
}

// Geometric product.

inline Motor operator*(Plane plane0, Plane plane1) {
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

inline Motor operator*(Line line0, Line line1) {
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

inline Rotor operator*(Rline rline0, Rline rline1) {
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

inline Translator operator*(Point point0, Point point1) {
    return {
        .ix = point0.x * point1.w - point0.w * point1.x,
        .iy = point0.y * point1.w - point0.w * point1.y,
        .iz = point0.z * point1.w - point0.w * point1.z,
        .iw = -point0.w * point1.w
    };
}

inline Rotor exp(float theta, Rline rline) {
    float sin = std::sin(theta / 2.0f);
    return {
        .rx = rline.rx * sin,
        .ry = rline.ry * sin,
        .rz = rline.rz * sin,
        .rw = std::cos(theta / 2.0f)
    };
}

inline Translator exp(float distance, Iline iline) {
    float half = distance / 2.0f;
    return {
        .ix = iline.ix * half,
        .iy = iline.iy * half,
        .iz = iline.iz * half,
        .iw = 1.0f
    };
}

inline Point operator>>(const Rotor& rotor, const Point& point) {
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

inline Point operator>>(const Translator& translator, const Point& point) {
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
struct CrossHairTagComponent {};
}; // namespace glvm

namespace glvm {
struct LevelChunkTagComponent {};
}; // namespace glvm

namespace glvm {
struct PlayerTagComponent {};
}; // namespace glvm

namespace glvm {
struct ProjectileTagComponent {};
}; // namespace glvm

namespace glvm {
struct StaticMeshTagComponent {};
}; // namespace glvm

namespace glvm {
struct TextureHandle {
    uint32_t id;
};

struct Texture {
    // This field using to choose specific instance of texture image in Vulkan.
    unsigned int vk_available_inner_id = 0;
    unsigned int vk_inner_id_limit = 10;

    const char* path_to_image = "";
    std::vector<unsigned int> entities_owns_this_type_of_texture = {};
    unsigned int id = 0;
    unsigned int i_width = 0;
    unsigned int i_height = 0;
    unsigned int dat_length = 0;
    unsigned char* u_i_data = 0;
};
} // namespace glvm

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>;

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
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
        std::unique_lock<std::mutex> lock(queue_mutex);

        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        tasks.emplace([task]() {
            (*task)();
        });
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
struct Point2D {
    T x;
    T y;
};

struct Point3D {
    float x;
    float y;
    float z;
};

template<typename T>
T clamp(T lower_threshold, T target_value, T upper_threshold) {
    if (target_value < lower_threshold) {
        target_value = lower_threshold;
    } else if (target_value > upper_threshold) {
        target_value = upper_threshold;
    }
    return target_value;
}

template<class T2, int Var2>
class Vector;

template<class T, int Var>
class Matrix {
    T m_matrix[Var][Var] {};

public:
    Matrix(T arg = 0) {
        for (int i = 0; i < Var; ++i) {
            m_matrix[i][i] = arg;
        }
    }

    Matrix(
        Vector<T, Var> row0,
        Vector<T, Var> row1,
        Vector<T, Var> row2,
        Vector<T, Var> row3
    ) {
        m_matrix[0][0] = row0[0];
        m_matrix[0][1] = row0[1];
        m_matrix[0][2] = row0[2];
        m_matrix[0][3] = row0[3];

        m_matrix[1][0] = row1[0];
        m_matrix[1][1] = row1[1];
        m_matrix[1][2] = row1[2];
        m_matrix[1][3] = row1[3];

        m_matrix[2][0] = row2[0];
        m_matrix[2][1] = row2[1];
        m_matrix[2][2] = row2[2];
        m_matrix[2][3] = row2[3];

        m_matrix[3][0] = row3[0];
        m_matrix[3][1] = row3[1];
        m_matrix[3][2] = row3[2];
        m_matrix[3][3] = row3[3];
    }

    void self_tensor_transpose() {
        T temp_matrix[Var][Var];
        for (int p = 0; p < Var; ++p) {
            for (int u = 0; u < Var; ++u) {
                temp_matrix[p][u] = m_matrix[u][p];
            }
        }
        for (int j = 0; j < Var; ++j) {
            for (int z = 0; z < Var; ++z) {
                this->m_matrix[j][z] = temp_matrix[j][z];
            }
        }
    }

    void self_identity() {
        for (int i = 0; i < Var; ++i) {
            for (int j = 0; j < Var; ++j) {
                if (i == j) {
                    this->m_matrix[i][j] = 1.0f;
                } else {
                    this->m_matrix[i][j] = 0.0f;
                }
            }
        }
    }

    Matrix<T, Var> operator+(const Matrix& matrix);
    Matrix<T, Var> operator*(const T scalar);
    Matrix<T, Var> operator*(const Matrix& matrix);
    T* operator[](const int index);
    const T* operator[](const int index) const;
    template<class T2, int Var2>
    Vector<T2, Var2> operator*(const Vector<T2, Var2>& vector);
};

template<class T, int Var>
Matrix<T, Var> Matrix<T, Var>::operator+(const Matrix& matrix) {
    Matrix<T, Var> temp_matrix;
    for (int i = 0; i < Var; ++i) {
        for (int j = 0; j < Var; ++j) {
            temp_matrix[i][j] = this->m_matrix[i][j] + matrix.m_matrix[i][j];
        }
    }

    return temp_matrix;
}

template<class T, int Var>
Matrix<T, Var> Matrix<T, Var>::operator*(const T scalar) {
    Matrix<T, Var> temp_matrix;
    for (int i = 0; i < Var; ++i) {
        for (int j = 0; j < Var; ++j) {
            temp_matrix[i][j] = this->m_matrix[i][j] * scalar;
        }
    }

    return temp_matrix;
}

template<class T, int Var>
Matrix<T, Var> Matrix<T, Var>::operator*(const Matrix& matrix) {
    Matrix<T, Var> temp_matrix;
    for (int i = 0; i < Var; ++i) {
        for (int j = 0; j < Var; ++j) {
            for (int n = 0; n < Var; ++n) {
                temp_matrix.m_matrix[i][j] +=
                    m_matrix[i][n] * matrix.m_matrix[n][j];
            }
        }
    }
    return temp_matrix;
}

template<class T, int Var>
T* Matrix<T, Var>::operator[](const int index) {
    return m_matrix[index];
}

template<class T, int Var>
const T* Matrix<T, Var>::operator[](const int index) const {
    return m_matrix[index];
}

template<class T, int Var>
template<class T2, int Var2>
Vector<T2, Var2> Matrix<T, Var>::operator*(const Vector<T2, Var2>& vector) {
    static_assert(Var == Var2, "Size error");
    Vector<T2, Var2> temp_vector;
    for (int i = 0; i < Var2; ++i) {
        for (int j = 0; j < Var; ++j) {
            temp_vector[i] += m_matrix[i][j] * vector[j];
        }
    }
    return temp_vector;
}

template<class T2, int Dim>
class Vector {
public:
    T2 m_vector[Dim] {};

public:
    Vector(T2 x = 0, T2 y = 0, T2 z = 0, T2 w = 0) {
        T2 array[4] = {x, y, z, w};
        for (int i = 0; i < Dim; ++i) {
            m_vector[i] = array[i];
        }
    }

    T2& operator[](const int index);
    const T2& operator[](const int index) const;
    template<class T, int Dim2>
    Vector<T2, Dim> operator*(const Matrix<T, Dim2>& matrix);
    Vector<T2, Dim> operator*(const Vector<T2, Dim>& other);
    Vector<T2, Dim> operator*=(const Vector<T2, Dim>& other);
    Vector<T2, Dim> operator-(const Vector<T2, Dim>& other) const;
    Vector<T2, Dim> operator+(const Vector<T2, Dim>& other) const;
    void operator-=(const Vector<T2, Dim>& other);
    void operator+=(const Vector<T2, Dim>& other);
    Vector<T2, Dim> operator*(const T2& multiplier);
    Vector<T2, Dim> operator-();
    T2 length() const;
};

template<class T2, int Var2>
T2 Vector<T2, Var2>::length() const {
    return std::sqrt(
        m_vector[0] * m_vector[0] + m_vector[1] * m_vector[1]
        + m_vector[2] * m_vector[2]
    );
}

template<class T2, int Var2>
Vector<T2, Var2> Vector<T2, Var2>::operator-() {
    Vector<T2, Var2> temp_vector;
    for (int i = 0; i < Var2; ++i) {
        temp_vector[i] = -m_vector[i];
    }

    return temp_vector;
}

template<class T2, int Var2>
T2& Vector<T2, Var2>::operator[](const int index) {
    return m_vector[index];
}

template<class T2, int Var2>
const T2& Vector<T2, Var2>::operator[](const int index) const {
    return m_vector[index];
}

template<class T2, int Var2>
template<class T, int Var>
Vector<T2, Var2> Vector<T2, Var2>::operator*(const Matrix<T, Var>& matrix) {
    static_assert(Var == Var2, "Size error");
    Vector<T2, Var2> temp_vector;
    for (int i = 0; i < Var2; ++i) {
        for (int j = 0; j < Var; ++j) {
            temp_vector[i] += m_vector[j] * matrix[j][i];
        }
    }
    return temp_vector;
}

template<typename T2, int Var2>
Vector<T2, Var2> Vector<T2, Var2>::operator*(const Vector<T2, Var2>& other) {
    Vector<T2, Var2> temp_vector;
    for (int i = 0; i < 3; ++i) {
        temp_vector[i] = m_vector[i] * other[i];
    }

    return temp_vector;
}

template<typename T2, int Var2>
Vector<T2, Var2> Vector<T2, Var2>::operator*=(const Vector<T2, Var2>& other) {
    Vector<T2, Var2> temp_vector;
    for (int i = 0; i < 3; ++i) {
        temp_vector[i] = m_vector[i] * other[i];
    }

    return temp_vector;
}

template<typename T2, int Var2>
Vector<T2, Var2> Vector<T2, Var2>::operator-(
    const Vector<T2, Var2>& other
) const {
    Vector<T2, Var2> temp_vector(1);

    temp_vector[0] = m_vector[0] - other[0];
    temp_vector[1] = m_vector[1] - other[1];
    temp_vector[2] = m_vector[2] - other[2];

    return temp_vector;
}

template<typename T2, int Var2>
Vector<T2, Var2> Vector<T2, Var2>::operator+(
    const Vector<T2, Var2>& other
) const {
    Vector<T2, Var2> temp_vector(1);

    temp_vector[0] = m_vector[0] + other[0];
    temp_vector[1] = m_vector[1] + other[1];
    temp_vector[2] = m_vector[2] + other[2];

    return temp_vector;
}

template<typename T2, int Var2>
void Vector<T2, Var2>::operator-=(const Vector<T2, Var2>& other) {
    m_vector[0] = m_vector[0] - other[0];
    m_vector[1] = m_vector[1] - other[1];
    m_vector[2] = m_vector[2] - other[2];
}

template<typename T2, int Var2>
void Vector<T2, Var2>::operator+=(const Vector<T2, Var2>& other) {
    m_vector[0] = m_vector[0] + other[0];
    m_vector[1] = m_vector[1] + other[1];
    m_vector[2] = m_vector[2] + other[2];
}

template<typename T2, int Var2>
Vector<T2, Var2> Vector<T2, Var2>::operator*(const T2& multiplier) {
    Vector<T2, Var2> temp_vec(1.0f);

    for (int i = 0; i < Var2; ++i) {
        temp_vec[i] = m_vector[i] * multiplier;
    }

    return temp_vec;
}

template<typename T2, int Var2>
Vector<T2, Var2> operator*(const Vector<T2, Var2>& vector, const T2 multiplier) {
    Vector<T2, Var2> temp;
    for (int i = 0; i < Var2; ++i) {
        temp[i] = vector[i] * multiplier;
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
    adjoint_matrix.self_tensor_transpose();
    T determinant_of_basic_matrix = determinant_4x4<T>(matrix);
    Matrix<T, 4> inverse_matrix(0.0f);
    inverse_matrix = adjoint_matrix * (1 / determinant_of_basic_matrix);
    return inverse_matrix;
}

template<class T, class T2, int Var, int Var2>
Matrix<T, Var> look_at(Matrix<T, Var> matrix, Vector<T2, Var2> vector) {
    Matrix<T, Var> temp_matrix(1.0f);
    temp_matrix = matrix;

    const unsigned int variable = 3;
    for (int i = 0; i < Var2; ++i) {
        temp_matrix[i][variable] = -vector[i];
    }

    return temp_matrix;
}

template<class T, class T2, int Var, int Var2>
Matrix<T, Var> translate(Matrix<T, Var> matrix, Vector<T2, Var2> vector) {
    Matrix<T, Var> temp_matrix(1.0f);
    temp_matrix = matrix;
    for (int i = 0; i < Var; ++i) {
        temp_matrix[Var - 1][i] += vector[i];
    }
    return temp_matrix;
}

template<class T, class T2, int Var, int Var2>
Matrix<T, Var> scale(Matrix<T, Var> matrix, Vector<T2, Var2> vector) {
    Matrix<T, Var> temp_matrix;
    temp_matrix = matrix;
    for (int i = 0; i < Var; ++i) {
        for (int j = 0; j < Var; ++j) {
            temp_matrix[j][i] *= vector[i];
        }
    }
    return temp_matrix;
}

template<class T, int Var>
Matrix<T, Var> rotate_z(Matrix<T, Var> matrix, float angle) {
    Matrix<T, Var> temp_matrix(1.0f);
    temp_matrix[0][0] = std::cos(angle * PI / 180);
    temp_matrix[0][1] = -std::sin(angle * PI / 180);
    temp_matrix[1][0] = std::sin(angle * PI / 180);
    temp_matrix[1][1] = std::cos(angle * PI / 180);
    Matrix<T, Var> temp_matrix2;
    temp_matrix2 = matrix * temp_matrix;
    return temp_matrix2;
}

template<typename T>
Vector<T, 3> cross(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs) {
    return Vector<T, 3>(
        lhs[1] * rhs[2] - lhs[2] * rhs[1],
        lhs[2] * rhs[0] - lhs[0] * rhs[2],
        lhs[0] * rhs[1] - lhs[1] * rhs[0]
    );
}

template<typename T>
T dot(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs) {
    return (lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2]);
}

template<typename T>
T vector_length(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs) {
    T diff_x = rhs[0] - lhs[0];
    T diff_y = rhs[1] - lhs[1];
    T diff_z = rhs[2] - lhs[2];
    return std::sqrt(diff_x * diff_x + diff_y * diff_y + diff_z * diff_z);
}

template<typename T>
T vec_length(const Vector<T, 3>& vector) {
    return std::sqrt(
        vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]
    );
}

template<typename T>
Vector<T, 3> normalize(Vector<T, 3> other) {
    if (other[0] == 0 && other[1] == 0 && other[2] == 0) {
        return Vector<float, 3> {0.0f, 0.0f, 0.0f};
    }
    float range = std::sqrt(
        other[0] * other[0] + other[1] * other[1] + other[2] * other[2]
    );
    for (int l = 0; l < 3; ++l) {
        other[l] = other[l] / range;
    }
    return other;
}

template<typename T>
Matrix<T, 4> glvm_perspective_rh_no(T fov, T aspect, T near_plane, T far_plane) {
    const T tan_half_fov = std::tan(fov / static_cast<T>(2));
    Matrix<float, 4> result(static_cast<T>(0));
    result[0][0] = static_cast<T>(1) / (aspect * tan_half_fov);
    result[1][1] = static_cast<T>(1) / (tan_half_fov);
    result[2][2] = -(far_plane - near_plane) / (far_plane - near_plane);
    result[2][3] = -static_cast<T>(1);
    result[3][2] = -(static_cast<T>(2) * far_plane * near_plane)
        / (far_plane - near_plane);
    return result;
}

template<typename T>
Matrix<T, 4> perspective(T fov, T aspect, T near_plane, T far_plane) {
    return glvm_perspective_rh_no<T>(fov, aspect, near_plane, far_plane);
}

template<typename T>
Matrix<T, 4> look_at_rh(Vector<T, 3> eye, Vector<T, 3> center, Vector<T, 3> up) {
    Vector<T, 3> f = (normalize(center - eye));
    Vector<T, 3> s = (normalize(cross(f, up)));
    Vector<T, 3> u = (cross(s, f));
    Matrix<T, 4> result(1.0f);
    result[0][0] = s[0];
    result[1][0] = s[1];
    result[2][0] = s[2];
    result[0][1] = u[0];
    result[1][1] = u[1];
    result[2][1] = u[2];
    result[0][2] = -f[0];
    result[1][2] = -f[1];
    result[2][2] = -f[2];
    result[3][0] = -dot(s, eye);
    result[3][1] = -dot(u, eye);
    result[3][2] = dot(f, eye);
    return result;
}

template<typename T>
Matrix<T, 4> look_at_main(
    Vector<T, 3> eye,
    Vector<T, 3> center,
    Vector<T, 3> up
) {
    return look_at_rh<T>(eye, center, up);
}

template<typename T>
Matrix<T, 4> fp_sview(Vector<T, 3> eye, Vector<T, 3> center, Vector<T, 3> up) {
    Vector<T, 3> f(Normalize(center - eye));
    Vector<T, 3> s(Normalize(Cross(f, up)));
    Vector<T, 3> u(Cross(s, f));
    Matrix<T, 4> result(1.0f);
    result[0][0] = s[0];
    result[1][0] = s[1];
    result[2][0] = s[2];
    result[0][1] = u[0];
    result[1][1] = u[1];
    result[2][1] = u[2];
    result[0][2] = -f[0];
    result[1][2] = -f[1];
    result[2][2] = -f[2];
    result[3][0] = -Dot(s, eye);
    result[3][1] = -Dot(u, eye);
    result[3][2] = Dot(f, eye);
    return result;
}

template<typename T3>
T3 radians(T3 degrees) {
    degrees *= PI / static_cast<T3>(180);
    return degrees;
}

template<typename T>
Matrix<T, 4> fps_view_rh(Vector<T, 3> eye, float pitch_deg, float yaw_deg) {
    pitch_deg *= PI / 180;
    yaw_deg *= PI / 180;
    float f_cos_pitch = std::cos(pitch_deg);
    float f_sin_pitch = std::sin(pitch_deg);
    float f_cos_yaw = std::cos(yaw_deg);
    float f_sin_yaw = std::sin(yaw_deg);
    Vector<T, 3> x_axis(f_cos_yaw, 0, -f_sin_yaw);
    Vector<T, 3> y_axis(
        f_sin_yaw * f_sin_pitch,
        f_cos_pitch,
        f_cos_yaw * f_sin_pitch
    );
    Vector<T, 3> z_axis(
        f_sin_yaw * f_cos_pitch,
        -f_sin_pitch,
        f_cos_pitch * f_cos_yaw
    );
    Matrix<T, 4> t_view(
        Vector<T, 4>(x_axis[0], y_axis[0], z_axis[0], 0),
        Vector<T, 4>(x_axis[1], y_axis[1], z_axis[1], 0),
        Vector<T, 4>(x_axis[2], y_axis[2], z_axis[2], 0),
        Vector<T, 4>(-dot(x_axis, eye), -dot(y_axis, eye), -dot(z_axis, eye), 1)
    );
    return t_view;
}

template<class T, int Var, int VecSize>
Matrix<T, Var> rotate(Vector<T, VecSize> vector, float angle) {
    vector = (normalize(vector));
    Matrix<T, Var> temp_matrix(1.0f);
    // Transposed rotate matrix.
    temp_matrix[0][0] = std::cos(angle)
        + (vector[0] * vector[0]) * (static_cast<T>(1) - std::cos(angle));
    temp_matrix[1][0] =
        vector[0] * vector[1] * (static_cast<T>(1) - std::cos(angle))
        - vector[2] * std::sin(angle);
    temp_matrix[2][0] =
        vector[0] * vector[2] * (static_cast<T>(1) - std::cos(angle))
        + vector[1] * std::sin(angle);
    temp_matrix[3][0] = static_cast<T>(0);
    temp_matrix[0][1] =
        vector[1] * vector[0] * (static_cast<T>(1) - std::cos(angle))
        + vector[2] * std::sin(angle);
    temp_matrix[1][1] = std::cos(angle)
        + (vector[1] * vector[1]) * (static_cast<T>(1) - std::cos(angle));
    temp_matrix[2][1] =
        vector[1] * vector[2] * (static_cast<T>(1) - std::cos(angle))
        - vector[0] * std::sin(angle);
    temp_matrix[3][1] = static_cast<T>(0);
    temp_matrix[0][2] =
        vector[2] * vector[0] * (static_cast<T>(1) - std::cos(angle))
        - vector[1] * std::sin(angle);
    temp_matrix[1][2] =
        vector[2] * vector[1] * (static_cast<T>(1) - std::cos(angle))
        + vector[0] * std::sin(angle);
    temp_matrix[2][2] = std::cos(angle)
        + (vector[2] * vector[2]) * (static_cast<T>(1) - std::cos(angle));
    temp_matrix[3][2] = static_cast<T>(0);
    temp_matrix[0][3] = static_cast<T>(0);
    temp_matrix[1][3] = static_cast<T>(0);
    temp_matrix[2][3] = static_cast<T>(0);
    temp_matrix[3][3] = static_cast<T>(1);
    return temp_matrix;
}

template<class T, int Var>
Matrix<T, Var> ortho(float w, float h, float zn, float zf) {
    Matrix<T, Var> temp_matrix(1.0f);
    temp_matrix[0][0] = 2 / w;
    temp_matrix[1][1] = 2 / h;
    temp_matrix[2][2] = 1 / (zf - zn);
    return temp_matrix;
}

template<class T>
Matrix<T, 4> ortho_rh_zo(
    T left,
    T right,
    T bottom,
    T top,
    T near_plane,
    T far_plane
) {
    Matrix<float, 4> temp_matrix(1);
    temp_matrix[0][0] = static_cast<T>(2) / (right - left);
    temp_matrix[1][1] = static_cast<T>(2) / (top - bottom);
    temp_matrix[2][2] = -static_cast<T>(1) / (far_plane - near_plane);
    temp_matrix[3][0] = -(right + left) / (right - left);
    temp_matrix[3][1] = -(top + bottom) / (top - bottom);
    temp_matrix[3][2] = -near_plane / (far_plane - near_plane);

    return temp_matrix;
}

template<class T>
Matrix<T, 4> ortho(T left, T right, T bottom, T top, T near_plane, T far_plane) {
    return ortho_rh_zo<T>(left, right, bottom, top, near_plane, far_plane);
}

template<class T, int Var>
Matrix<T, Var> perspective_rh_zo(T fov, T aspect, T near_plane, T far_plane) {
    const float tan_half_fov = std::tan((fov * 0.5) * (PI / 360));
    Matrix<float, Var> temp_matrix(static_cast<T>(0));
    temp_matrix[0][0] = static_cast<T>(1) / (aspect * tan_half_fov);
    temp_matrix[1][1] = static_cast<T>(1) / tan_half_fov;
    temp_matrix[2][2] = far_plane / (near_plane - far_plane);
    temp_matrix[2][3] = static_cast<T>(1);
    temp_matrix[3][2] = -(far_plane * near_plane) / (far_plane - near_plane);
    return temp_matrix;
}

template<class T, int Var>
Matrix<T, Var> perspective(
    const T fov,
    const T aspect,
    const T near_plane,
    const T far_plane
) {
    return perspective_rh_zo(fov, aspect, near_plane, far_plane);
}

constexpr float max(float var1, float var2) {
    return var1 > var2 ? var1 : var2;
}

constexpr float min(float var1, float var2) {
    return var1 < var2 ? var1 : var2;
}

struct Quaternion {
    float w, x, y, z;
    Quaternion() = default;

    Quaternion(float qw, float qx, float qy, float qz) :
        w(qw),
        x(qx),
        y(qy),
        z(qz) {
    }

    Quaternion(float real, Vector<float, 3> imaginary) :
        w(real),
        x(imaginary[0]),
        y(imaginary[1]),
        z(imaginary[2]) {
    }
};

inline Quaternion conjugate(Quaternion quaternion) {
    quaternion.w = quaternion.w;
    quaternion.x = -quaternion.x;
    quaternion.y = -quaternion.y;
    quaternion.z = -quaternion.z;
    return quaternion;
}

inline Quaternion multiply_quaternion(const Quaternion& a, const Quaternion& b) {
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

inline float norm_quaternion(const Quaternion& quaternion) {
    return sqrt(
        quaternion.w * quaternion.w + quaternion.x * quaternion.x
        + quaternion.y * quaternion.y + quaternion.z * quaternion.z
    );
}

inline Quaternion normalize_quaternion(Quaternion quaternion) {
    float norm = norm_quaternion(quaternion);
    float inverse_norm = 1.0f / norm;
    quaternion.w *= inverse_norm;
    quaternion.x *= inverse_norm;
    quaternion.y *= inverse_norm;
    quaternion.z *= inverse_norm;
    return quaternion;
}

inline Quaternion inverse_quaternion(Quaternion quaternion) {
    Quaternion linked_value = conjugate(quaternion);
    float norm = norm_quaternion(quaternion);
    float inverse_norm = 1.0f / norm;
    quaternion.w = linked_value.w * inverse_norm;
    quaternion.x = linked_value.x * inverse_norm;
    quaternion.y = linked_value.y * inverse_norm;
    quaternion.z = linked_value.z * inverse_norm;
    return quaternion;
}

inline Quaternion euler_to_quaternion(
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

template<class T, int Var>
Matrix<T, Var> rotate_quaternion(Quaternion quaternion) {
    Matrix<float, 4> result(0.0f);

    quaternion = normalize_quaternion(quaternion);

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
    float& operator[](const unsigned int index) {
        assert(index < 3 && index >= 0 && "Wrong index");
        switch (index) {
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
    std::vector<int> vertex_index;
    std::vector<int> texture_index;
    std::vector<int> normal_index;

public:
    std::vector<int>& operator[](const unsigned int index) {
        assert(index < 3 && index >= 0 && "Wrong index");
        switch (index) {
            default:
            case 0:
                return vertex_index;
            case 1:
                return texture_index;
            case 2:
                return normal_index;
        }
    }

    const std::vector<int>& operator[](const unsigned int index) const {
        assert(index < 3 && index >= 0 && "Wrong index");
        switch (index) {
            default:
            case 0:
                return vertex_index;
            case 1:
                return texture_index;
            case 2:
                return normal_index;
        }
    }
};

class CWaveFrontObjParser {
    std::vector<SVertex> coordinate_vertices;
    std::vector<SVertex> texture_vertices;
    std::vector<SVertex> normals;
    std::vector<SFace> faces;

    std::string s_wavefront_obj_file_data;
    const char* p_wavefront_obj_file_data;
    unsigned int ui_counter = 0;

public:
    CWaveFrontObjParser();

    [[nodiscard]] const std::vector<SVertex>& get_coordinate_vertices() const;
    [[nodiscard]] const std::vector<SVertex>& get_texture_vertices() const;
    [[nodiscard]] const std::vector<SVertex>& get_normals() const;
    [[nodiscard]] const std::vector<SFace>& get_faces() const;

    void read_file(const char* file_path);
    void parse_file();
    std::vector<std::vector<char>> split(
        const char* data,
        const char separator,
        const char exit_symbol,
        unsigned int& position
    );
    SVertex parse_vertices(std::vector<std::vector<char>> words);
    SFace parse_faces(std::vector<std::vector<char>> words);
    int parse_integer(std::vector<char> digits);
    float parse_floating(std::vector<char> digits);
};
} // namespace glvm

namespace glvm {
class Inventory {
public:
    Inventory() {
        for (unsigned int i = 0; i < row; ++i) {
            slots[i] = new unsigned int[col];
        }

        for (unsigned int i = 0; i < row; ++i) {
            for (unsigned int j = 0; j < col; ++j) {
                slots[i][j] = -1;
            }
        }
    }

    Inventory(const Inventory& inv) {
        for (unsigned int i = 0; i < row; ++i) {
            this->slots[i] = new unsigned int[col];
        }

        for (unsigned int i = 0; i < row; ++i) {
            for (unsigned int j = 0; j < col; ++j) {
                this->slots[i][j] = inv.slots[i][j];
            }
        }

        this->entity_owner = inv.entity_owner;
        this->highlighted_slots = inv.highlighted_slots;
        this->is_available_highlighted_slots =
            inv.is_available_highlighted_slots;
    }

    ~Inventory() {
        for (unsigned int i = 0; i < row; ++i) {
            delete[] slots[i];
        }

        delete[] slots;
    }

    unsigned int row = 8;
    unsigned int col = 8;
    // Array with entities contained inventorySlotComponents.
    unsigned int** slots = new unsigned int*[row];
    unsigned int entity_owner = UINT_MAX;
    std::vector<unsigned int> highlighted_slots;
    bool is_available_highlighted_slots = false;
    MeshHandle slot_mesh_id;
    float slot_scale;
};
}; // namespace glvm

namespace glvm {
class MeshManager {
    static MeshManager* p_instance;
    static std::mutex mutex;

    MeshManager();
    ~MeshManager();

public:
    std::vector<const char*> paths_array;
    std::vector<const char*> paths_gltf;

    // It possibly to get only one instance of this class with this method.
    static MeshManager* get_instance();
    void set_mesh(const char* mesh_path);
    void set_mesh_gltf(const char* path_to_mesh);
};
} // namespace glvm

namespace glvm {
class CStack {
    int i_head = 0;
    static const int i_stack_range = 6;
    EEvents a_stack[i_stack_range] = {};

public:
    void push(const EEvents& event) {
        for (int i = 0; i < i_head; ++i) {
            if (a_stack[i] == event) {
                return;
            }
        }

        if (i_head == i_stack_range) {
            return;
        }

        a_stack[i_head] = event;

        ++i_head;
    }

    EEvents& pop() {
        if (i_head == 0) {
            return a_stack[0];
        }
        return a_stack[i_head - 1];
    }

    void remove(const EEvents& event) {
        EEvents a_temp_stack[i_stack_range] = {};
        bool remove_flag = false;
        int n = 0;

        for (int j = 0; j < i_stack_range; ++j) {
            a_temp_stack[j] = a_stack[j];
        }

        for (int i = 0; i < i_head; ++i) {
            if (event == a_temp_stack[i]) {
                remove_flag = true;
                continue;
            }

            a_stack[n] = a_temp_stack[i];
            ++n;
        }

        if (remove_flag) {
            --i_head;
            a_stack[i_head] = EEvents::EDefault;
        }
    }

    void control_input(CEvent& event) {
        if (!(search_element(event.get_event()) == EEmpty)) {
            return;
        }
        switch (event.get_event()) {
            case EGameLoopKill:
                push(EGameLoopKill);
                break;
            case EKeyreleaseA:
                remove(EMoveLeft);
                break;
            case EKeyreleaseD:
                remove(EMoveRight);
                break;
            case EKeyreleaseS:
                remove(EMoveBackward);
                break;
            case EKeyreleaseW:
                remove(EMoveForward);
                break;
            case EInventoryRelease:
                remove(EInventory);
                break;
            case EKeyreleaseJump:
                remove(EJump);
                break;
            case EMouseLeftButtonRelease:
                remove(EMouseLeftButton);
                break;
            case EMoveLeft:
                push(EMoveLeft);
                break;
            case EMoveRight:
                push(EMoveRight);
                break;
            case EMoveBackward:
                push(EMoveBackward);
                break;
            case EMoveForward:
                push(EMoveForward);
                break;
            case EJump:
                push(EJump);
                break;
            case EInventory:
                push(EInventory);
                break;
            case ECursorReleased:
                push(ECursorReleased);
                break;
            case EMouseLeftButton:
                push(EMouseLeftButton);
                break;
            default:
                break;
        }
    }

    EEvents search_element(EEvents element) {
        for (int i = 0; i < i_head; ++i) {
            if (a_stack[i] == element) {
                return element;
            }
        }

        return EEmpty;
    }

    EEvents& operator[](int index) {
        return a_stack[index];
    }

    void clear() {
        for (int i = 0; i < i_head; ++i) {
            a_stack[i] = EEvents::EDefault;
        }
    }
};
} // namespace glvm

namespace glvm {

class IWindow {
public:
    // Window keyboard focus, updated by each backend.
    bool is_focused = true;

    virtual ~IWindow() = default;

    virtual void swap_buffers() = 0;
    virtual void clear_display() = 0;
    virtual bool handle_event(CEvent& event) = 0;
    virtual void close() = 0;
    virtual void cursor_lock(
        int pointer_x,
        int pointer_y,
        int* out_offset_x,
        int* out_offset_y
    ) = 0;
};

} // namespace glvm

namespace glvm {
class CTimerCreator {
public:
    ~CTimerCreator() {
    }

    IChrono* create();
};
} // namespace glvm

#ifdef __linux__

namespace glvm {
class CTimerX: public IChrono {
    timespec start_;
    timespec now_;
    double frequency_;
    double seconds_;
    double nanoseconds_;

public:
    CTimerX();

    double init_frequency();
    double reset();
    double get_elapsed();
};
} // namespace glvm
#endif // __linux__

#ifdef _WIN32

namespace glvm {
class CTimerWin: public IChrono {
    __int64 i64_freq;
    __int64 i64_start;
    __int64 i64_now;

public:
    CTimerWin();

    double init_frequency();
    double reset();
    double get_elapsed();
};
} // namespace glvm
#endif // _WIN32

namespace glvm {
struct State {
    States state;
};
} // namespace glvm

namespace glvm {
struct CSoundSample {
    const char* k_path_to_file;
    unsigned int ui_duration;
    unsigned int ui_rate;
    float volume;
};

class ISoundEngine {
public:
    virtual ~ISoundEngine() {
    }

    virtual void open_device(const char* device) = 0;
    virtual void close_device() = 0;
    virtual std::vector<CSoundSample*>& get_sound_container() = 0;
    virtual void playback_sound_sample(CSoundSample& sample) = 0;
    virtual void set_master_volume(long volume) = 0;
    virtual void sound_stream() = 0;
    virtual void create_sound_sample(
        const char* file_path,
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
    uint32_t entity_count = 0;
    uint32_t component_ids[ComponentsIndices::ComponentsCount] = {};
    uint32_t component_count = 0;
    void* components[ComponentsIndices::ComponentsCount] = {};
    uint64_t mask = 0;

    uint32_t add_entity(uint64_t entity);
    uint64_t remove_entity(uint32_t index);
};

struct EntityLocation {
    Archetype* arch;
    uint32_t index;
    static const uint8_t max_grid_cell_number = 32;
    uint8_t grid_cell_counter = 0;
    Vector<float, 3> grid_cell_indicies[max_grid_cell_number];
    uint32_t cell_entity_indices[max_grid_cell_number];
    // Is entity has been moved or removed.
    bool is_dirty = false;
};
}; // namespace glvm

namespace glvm {
struct DirectionalLightComponent {
    Vector<float, 3> position;
    Vector<float, 3> direction;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;
};
} // namespace glvm

namespace glvm {
struct Material {
    TextureHandle diffuse_texture_id = {};
    TextureHandle specular_texture_id = {};
    Vector<float, 3> ambient = {0.0f, 0.0f, 0.0f};
    float shininess = 0.0f;
};
} // namespace glvm

namespace glvm {
struct Move {
    EEvents e_event = EEvents::EDefault;
    Vector<float, 3> frame_movement {0.0f, 0.0f, 0.0f};
    Vector<float, 3> gravity {0.0f, 0.0f, 0.0f};
};
} // namespace glvm

namespace glvm {
struct PointLightComponent {
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
    float f_mass = 0.0f;
    float jump_accumulator = 0.0f;
};
} // namespace glvm

namespace glvm {
struct SpotLightComponent {
    Vector<float, 3> position;
    Vector<float, 3> direction;
    float cut_off;
    float outer_cut_off;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};
} // namespace glvm

namespace glvm {
struct Transform {
    Vector<float, 3> position {0.0f, 0.0f, 0.0f};
    Vector<float, 3> forward {0.0f, 0.0f, 0.0f};
    float scale = 1.0f;
    float gravity_accumulator = 0.0f;
};
} // namespace glvm

namespace glvm {
struct Beholder {
    Vector<float, 3> position {0.0f, 0.0f, 0.0f};
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
    double f_number;
    int i_number;
    bool boolean;
    void* null;
    std::vector<JsonValue>* array;
    HashMap<JsonValue>* object;

    JsonVariant() {
    }

    JsonVariant(const JsonVariant& object) {
        memcpy((void*)this, &object, sizeof(JsonVariant));
    }

    ~JsonVariant() {
    }
};

struct JsonValue {
    JsonVariant value;
    JsonType type;

    JsonValue() {
        type = JsonInvalidValue;
    }

    JsonValue(std::string str) {
        type = JsonString;
        value.string = new std::string(str);
    }

    JsonValue(double number) {
        type = JsonFloatNumber;
        value.f_number = number;
    }

    JsonValue(int number) {
        type = JsonIntegerNumber;
        value.i_number = number;
    }

    JsonValue(bool flag) {
        type = JsonBoolean;
        value.boolean = flag;
    }

    JsonValue(const JsonValue& other) {
        type = JsonInvalidValue;

        switch (other.type) {
            case JsonObject:
                value.object = new HashMap<JsonValue>(*other.value.object);
                break;
            case JsonIntegerNumber:
                value.i_number = other.value.i_number;
                break;
            case JsonFloatNumber:
                value.f_number = other.value.f_number;
                break;
            case JsonString:
                value.string = new std::string(*other.value.string);
                break;
            case JsonBoolean:
                value.boolean = other.value.boolean;
                break;
            case JsonNull:
                value.null = other.value.null;
                break;
            case JsonArray:
                value.array = new std::vector<JsonValue>(*other.value.array);
                break;
            default:
                break;
        }
        type = other.type;
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

    void operator=(const JsonValue& other) {
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

        switch (other.type) {
            case JsonObject:
                value.object = new HashMap<JsonValue>(*other.value.object);
                break;
            case JsonIntegerNumber:
                value.i_number = other.value.i_number;
                break;
            case JsonFloatNumber:
                value.f_number = other.value.f_number;
                break;
            case JsonString:
                value.string = new std::string(*other.value.string);
                break;
            case JsonBoolean:
                value.boolean = other.value.boolean;
                break;
            case JsonNull:
                value.null = other.value.null;
                break;
            case JsonArray:
                value.array = new std::vector<JsonValue>(*other.value.array);
                break;
            default:
                break;
        }
        type = other.type;
    }

    JsonValue& operator[](std::string lookup_key) {
        switch (type) {
            case JsonObject:
                return (*value.object)[lookup_key.c_str()];
                break;
            default:
                throw std::out_of_range("Type is not a json object");
                break;
        }
    }

    JsonValue& operator[](const unsigned int index) {
        switch (type) {
            case JsonArray:
                return (*value.array)[index];
                break;
            default:
                throw std::out_of_range("Type is not a json array");
                break;
        }
    }

    bool is_invalid() {
        return type == JsonInvalidValue;
    }

    bool is_object() {
        return type == JsonObject;
    }

    bool is_float() {
        return type == JsonFloatNumber;
    }

    bool is_interger() {
        return type == JsonIntegerNumber;
    }

    bool is_string() {
        return type == JsonString;
    }

    bool is_boolean() {
        return type == JsonBoolean;
    }

    bool is_null() {
        return type == JsonNull;
    }

    bool is_array() {
        return type == JsonArray;
    }
};

class CJsonParser {
    std::string s_json_file_data;
    const char* p_json_file_data;
    char current_char;
    unsigned int global_file_counter = 0;

    std::vector<JsonValue*> stack_of_json_values;
    JsonValue* root;
    bool key_flag = true;
    std::string last_key = "";
    std::string buffer_string = "";

    void search_in_json_array(
        std::vector<JsonValue>* array_value,
        const char* key,
        std::vector<JsonValue>& result_vector
    ) const;

public:
    void search_in_json_object(
        HashMap<JsonValue>* map_value,
        const char* key,
        std::vector<JsonValue>& result_vector
    ) const;

    ~CJsonParser();

    JsonValue* get_root() {
        return root;
    }

    void read_file(const char* file_path);
    void parse();
    JsonValue create_json_hash_map();
    JsonValue create_json_array();
    std::string bool_or_null_parse();
    bool is_contain_char(std::string text, char character);
    std::string number_as_string_parse();
    std::string string_parse();
    std::vector<char> string_to_vector_of_chars(std::string text);
    int parse_integer(std::vector<char> digits);
    double parse_floating(std::vector<char> digits);
    std::vector<JsonValue> search(const char* key) const;
    void load_gltf(
        const char* paths_gltf,
        std::vector<float>& a_vertexes,
        std::vector<uint32_t>& a_indices,
        std::vector<std::vector<Matrix<float, 4>>>& joint_matrices_per_mesh,
        std::vector<float>& frames,
        bool& no_animations,
        float& top_y
    );
    void traversal_bones(
        std::vector<std::vector<int>> children,
        JsonValue joints,
        std::vector<uint32_t> node_stack,
        std::vector<uint32_t> deepness_stack,
        std::vector<std::vector<uint32_t>>& result
    );
    std::vector<std::vector<unsigned int>> make_render_joints_indices(
        std::vector<std::vector<unsigned int>>& input
    );
    bool contains_element(
        std::vector<std::vector<unsigned int>> container,
        unsigned int element
    );
    unsigned int get_joint_index(JsonValue joints, int searching_index);
};
} // namespace glvm

namespace glvm {
struct LightSpaceMatrixUBO {
    alignas(16) Matrix<float, 4> spot_space_matrix[SPOT_LIGHTS_NUMBER];
    alignas(16) uint32_t spot_lights_number;

    alignas(16) Matrix<float, 4> dir_space_matrix[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) uint32_t directional_lights_number;
};

struct alignas(64) ModelMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> view;
    Matrix<float, 4> proj;
    Matrix<float, 4> joint_matrices[MAX_JOINTS_NUMBER];

    Vector<float, 3> ambient;
    float shininess;

    alignas(16) Matrix<float, 4> spot_space_matrix[SPOT_LIGHTS_NUMBER];
    alignas(16) uint32_t spot_lights_number;

    alignas(16) Matrix<float, 4> dir_space_matrix[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) uint32_t directional_lights_number;
};

struct alignas(16) ShadowMapMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> light_space_matrix;
    Matrix<float, 4> joint_matrices[MAX_JOINTS_NUMBER];
};

struct alignas(16) SpotLightShadowMapMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> light_space_matrix;
};

struct alignas(64) PointLightShadowMapMatrixUBO {
    Matrix<float, 4> model;
    Matrix<float, 4> light_space_matrix;
    Vector<float, 3> light_position;
    float far_plane;
    Matrix<float, 4> joint_matrices[MAX_JOINTS_NUMBER];
};

struct alignas(16) UniformBufferObjectLightUBO {
    Vector<float, 3> light_position;
    float far_plane;
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
    float cut_off;
    float outer_cut_off;

    alignas(16) Vector<float, 3> ambient;
    alignas(16) Vector<float, 3> diffuse;
    alignas(16) Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};

struct alignas(64) LightData {
    Vector<float, 2> tileset_tiles_count;
    int tiles_raw;
    int tiles_column;

    alignas(16) Vector<float, 3> view_position;

    PointLight point_lights[POINT_LIGHTS_NUMBER];
    int point_lights_array_size;
    float far_plane;
    int padding0;
    int padding1;

    DirectionalLight directional_lights[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) int directional_lights_array_size;

    SpotLight spot_lights[SPOT_LIGHTS_NUMBER];
    int spot_light_array_size;
    int padding2;
    int padding3;
    int padding4;

    Vector<int, 4> indirect_texture
        [INDIRECT_TEXTURE_WIDTH * INDIRECT_TEXTURE_HEIGHT / 4 + 1];

    // Debug: 0 = off, 1 = directional, 2 = spot. When set, the main shader
    // renders the shadow map depth projected onto the scene instead of
    // lighting (visualized from the normal moving camera).
    int debug_shadow_mode;
    int debug_shadow_light;
    int shadows_enabled;
};

struct alignas(64) HudUbo {
    Matrix<float, 4> view;
    Matrix<float, 4> proj;
    Vector<float, 3> entity_position;
    int is_hud_exists;
    float max_hp;
    float current_hp;
    float highest_y;
};

struct alignas(64) HudScreenUbo {
    Matrix<float, 4> model;
};

struct alignas(64) FontUbo {
    Matrix<float, 4> view;
    Matrix<float, 4> proj;
    Vector<float, 3> position;
    float scale;
};

struct alignas(64) UiUbo {
    Matrix<float, 4> model;
    Vector<float, 3> color;
};

struct alignas(64) VirtualTextureUbo {};

struct alignas(64) SdfUbo {
    Matrix<float, 4> model;
    float i_time;
};

} // namespace glvm

#ifdef _WIN32

namespace glvm {

class WindowWinVulkan: public IWindow {
    HWND p_classic_window;
    HDC p_classic_dc;
    HGLRC p_classic_context;

    WNDCLASS window_class;
    HDC p_modern_dc;
    HGLRC p_modern_context;
    HWND p_modern_window;

    // Cursor-lock baseline: the cursor's actual position after the last warp
    // (not the computed center).
    int previous_x = 0;
    int previous_y = 0;

public:
    static WindowWinVulkan* instance;
    CStack* input_stack;
    uint32_t width = GetSystemMetrics(SM_CXSCREEN);
    uint32_t height = GetSystemMetrics(SM_CYSCREEN);
    WindowWinVulkan();

    void swap_buffers() override;
    void clear_display() override;
    bool handle_event(CEvent& event) override;
    HWND get_classic_window_hwnd();
    HWND get_modern_window_hwnd();
    void close() override;
    virtual void cursor_lock(
        int pointer_x,
        int pointer_y,
        int* out_offset_x,
        int* out_offset_y
    ) override;
    // Callback method for events handling.
    static LRESULT main_wnd_proc(
        HWND p_hwnd,
        UINT p_msg,
        WPARAM p_w_param,
        LPARAM p_l_param
    );
};
} // namespace glvm
#endif // _WIN32

#ifdef __linux__

namespace glvm {
class CSoundEngineAlsa: public ISoundEngine {
    snd_pcm_t* pcm_;
    std::vector<CSoundSample*> sound_container;

public:
    void open_device(const char* device) override;
    void close_device() override;
    void sound_stream() override;
    void playback_sound_sample(CSoundSample& sample) override;
    void set_master_volume(long volume) override;
    std::vector<CSoundSample*>& get_sound_container() override;
    void create_sound_sample(
        const char* file_path,
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
    ISoundEngine* create_sound_engine();
};

} // namespace glvm

#ifdef _WIN32
namespace glvm {
class CSoundEngineWaveform: public ISoundEngine {
    HANDLE h_data = NULL;
    HPSTR lp_data = NULL;

    std::vector<CSoundSample*> t_sound_container;

public:
    void open_device(const char* device) override;
    void close_device() override;
    void sound_stream() override;
    void playback_sound_sample(CSoundSample& sample) override;
    void set_master_volume(long volume) override;
    void create_sound_sample(
        const char* file_path,
        uint32_t duration,
        uint32_t rate,
        float volume
    ) override;
    std::vector<CSoundSample*>& get_sound_container() override;
};
} // namespace glvm
#endif // _WIN32

namespace glvm {
struct ProjectileBundle {
    Projectile projectile;
    Damage damage;
    Material material;
};
}; // namespace glvm

namespace glvm {
class TextureManager {
    static TextureManager* p_instance;
    static std::mutex mutex;

    std::vector<Texture> texture_vector;

public:
    TextureManager();

    void set_texture_vector(std::vector<Texture> textures);
    // It possibly to get only one instance of this class with this method.
    static TextureManager* get_instance();
    static TextureManager* get_hud_instance();
    void bind_texture(unsigned int entity_id, unsigned int texture_id);
    void load_texture_data(glvm::Texture& asset);
    std::vector<Texture>& get_texture_vector();
    void unbind_texture(Material component, unsigned int entity);
};
} // namespace glvm

namespace glvm {
class ComponentManager {
    static ComponentManager* p_instance;
    static std::mutex mutex;
    unsigned int number_of_base_components;

    ComponentManager();

    template<typename ComponentType>
    unsigned int create_component_container() {
        static unsigned int LOCAL_CONTAINER_ID = 0;
        static bool EXIST_COMPONENT_CONTAINER_FLAG = false;
        if (EXIST_COMPONENT_CONTAINER_FLAG) {
            return LOCAL_CONTAINER_ID;
        }
        // Give a value of global component container ID's counter to local
        // container ID of current component type.
        LOCAL_CONTAINER_ID = components_container_id;
        EXIST_COMPONENT_CONTAINER_FLAG = true;
        // Create component container of current type.
        world_components_container.push_back(
            std::make_shared<std::vector<ComponentType>>()
        );
        // Create ID's component container.
        std::vector<unsigned int>* sparse_entities_map_to_components =
            new std::vector<unsigned int>;
        world_sparse_entities_map_to_components.push_back(
            sparse_entities_map_to_components
        );
        // Create ID's component container.
        std::vector<unsigned int>* dense_entities_map_to_components =
            new std::vector<unsigned int>;
        world_dense_components_map_to_entities.push_back(
            dense_entities_map_to_components
        );
        components_types.push_back(typeid(ComponentType).name());
        ++components_container_id;
        return LOCAL_CONTAINER_ID;
    }

public:
    inline static unsigned int components_container_id = 0;
    // Contains all local containers for different types of components.
    std::vector<std::shared_ptr<void>> world_components_container;
    // Contains all local container with IDs for different types of components.
    std::vector<std::vector<unsigned int>*>
        world_sparse_entities_map_to_components;
    std::vector<std::vector<unsigned int>*>
        world_dense_components_map_to_entities;

    std::vector<const char*> components_types;
    bool is_components_collection_changed = true;

    ~ComponentManager();
    // Don't need to make cope because of singleton property.
    ComponentManager(ComponentManager& component_manager) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const ComponentManager& component_manager) = delete;
    // It possibly to get only one instance of this class with this method.
    static ComponentManager* get_instance();

    template<typename ComponentType>
    void create_component(const unsigned int& entity) {
        // Index for world components and world ID's containers.
        unsigned int local_container_id = 0;
        ComponentType component;
        local_container_id = create_component_container<ComponentType>();

        std::vector<unsigned int>& sparse =
            *static_cast<std::vector<unsigned int>*>(
                world_sparse_entities_map_to_components[local_container_id]
            );
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                world_dense_components_map_to_entities[local_container_id]
            );
        std::vector<ComponentType>& components =
            *std::static_pointer_cast<std::vector<ComponentType>>(
                world_components_container[local_container_id]
            );
        if (check_availability(sparse, dense, entity)) {
            return;
        }

        if (entity >= sparse.size()) {
            sparse.resize(entity + 1);
        }

        assert(dense.size() == components.size());

        sparse[entity] = dense.size();
        dense.push_back(entity);
        components.push_back(component);
        is_components_collection_changed = true;
    }

    bool check_availability(
        std::vector<unsigned int>& sparse,
        std::vector<unsigned int>& dense,
        unsigned int entity
    );

    // Allow to give a various components to chosen entity.
    template<typename ComponentType1, typename ComponentType2, typename... Args>
    void create_component(unsigned int& entity) {
        CreateComponent<ComponentType2, Args...>(entity);
        CreateComponent<ComponentType1>(entity);
    }

    template<typename ComponentType, typename... Args>
    std::vector<unsigned int> collect_linked_entities() {
        number_of_base_components = 0;
        unsigned int first_component_array_index =
            create_component_container<ComponentType>();
        std::vector<unsigned int>& dense = *static_cast<
            std::vector<unsigned int>*>(
            world_dense_components_map_to_entities[first_component_array_index]
        );

        if (dense.size() > 0) {
            ++number_of_base_components;
            number_of_base_components += sizeof...(Args);
        }
        std::vector<unsigned int> return_vector;
        for (unsigned int i = 0; i < dense.size(); ++i) {
            if (multiCheckAvailability<Args...>(dense[i])) {
                return_vector.push_back(dense[i]);
            }
        }
        return return_vector;
    }

    template<typename ComponentType, typename... Args>
    std::vector<unsigned int> collect_unique_linked_entities() {
        std::vector<unsigned int> base_sub_set_entities;
        base_sub_set_entities =
            collect_linked_entities<ComponentType, Args...>();
        unsigned int number_of_component_arrays = 0;
        for (unsigned int j = 0; j < base_sub_set_entities.size(); ++j) {
            number_of_component_arrays = 0;
            for (unsigned int i = 0;
                 i < world_dense_components_map_to_entities.size();
                 ++i) {
                std::vector<unsigned int>& sparse =
                    *static_cast<std::vector<unsigned int>*>(
                        world_sparse_entities_map_to_components[i]
                    );
                std::vector<unsigned int>& dense =
                    *static_cast<std::vector<unsigned int>*>(
                        world_dense_components_map_to_entities[i]
                    );

                if (check_availability(sparse, dense, base_sub_set_entities[j])) {
                    ++number_of_component_arrays;
                }
            }
            if (number_of_component_arrays > number_of_base_components) {
                base_sub_set_entities.erase(
                    base_sub_set_entities.begin() + base_sub_set_entities[j]
                );
                --j;
            }
        }
        return base_sub_set_entities;
    }

    template<typename... Args>
    bool multi_check_availability(unsigned int entity) {
        return (multiCheckAvailabilityBase<Args>(entity) && ...);
    }

    template<typename ComponentType>
    bool multi_check_availability_base(unsigned int entity) {
        unsigned int component_array_index =
            create_component_container<ComponentType>();
        std::vector<unsigned int>& sparse =
            *static_cast<std::vector<unsigned int>*>(
                world_sparse_entities_map_to_components[component_array_index]
            );
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                world_dense_components_map_to_entities[component_array_index]
            );
        return check_availability(sparse, dense, entity);
    }

    template<typename ComponentType>
    bool is_component_exists(const unsigned int& entity) {
        unsigned int local_container_id;
        local_container_id = create_component_container<ComponentType>();
        std::vector<unsigned int>& sparse =
            *static_cast<std::vector<unsigned int>*>(
                world_sparse_entities_map_to_components[local_container_id]
            );
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                world_dense_components_map_to_entities[local_container_id]
            );
        return check_availability(sparse, dense, entity);
    }

    template<typename ComponentType>
    ComponentType* get_component(const unsigned int& entity) {
        unsigned int local_container_id;
        local_container_id = create_component_container<ComponentType>();
        std::vector<unsigned int>& sparse =
            *static_cast<std::vector<unsigned int>*>(
                world_sparse_entities_map_to_components[local_container_id]
            );
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                world_dense_components_map_to_entities[local_container_id]
            );
        std::vector<ComponentType>& components =
            *std::static_pointer_cast<std::vector<ComponentType>>(
                world_components_container[local_container_id]
            );
        if (check_availability(sparse, dense, entity)) {
            unsigned int component_index = sparse[entity];
            return &components[component_index];
        } else {
            return nullptr;
        }
    }

    // Don't need to delete real component in this method. Because systems don't
    // work with component without indices for that component in ordered
    // container.
    template<typename ComponentType>
    void remove_component(unsigned int& entity) {
        unsigned int local_container_id;
        local_container_id = create_component_container<ComponentType>();
        std::vector<unsigned int>& sparse =
            *static_cast<std::vector<unsigned int>*>(
                world_sparse_entities_map_to_components[local_container_id]
            );
        std::vector<unsigned int>& dense =
            *static_cast<std::vector<unsigned int>*>(
                world_dense_components_map_to_entities[local_container_id]
            );
        std::vector<ComponentType>& components =
            *std::static_pointer_cast<std::vector<ComponentType>>(
                world_components_container[local_container_id]
            );
        if (check_availability(sparse, dense, entity)) {
            assert(dense.size() == components.size());
            unsigned int index_in_dense_of_removable_entity = sparse[entity];
            unsigned int index_in_sparse_of_swapable_entity = dense.back();
            const ComponentType& component_from_last_index = components.back();
            dense[index_in_dense_of_removable_entity] =
                index_in_sparse_of_swapable_entity;
            dense.pop_back();
            components[index_in_dense_of_removable_entity] =
                component_from_last_index;
            components.pop_back();
            sparse[index_in_sparse_of_swapable_entity] =
                index_in_dense_of_removable_entity;
            is_components_collection_changed = true;
        }
    }

    void remove_all_components(unsigned int& entity) {
        for (unsigned int i = 0; i < world_components_container.size(); ++i) {
            if (components_types[i] == typeid(Transform).name()) {
                remove_component<Transform>(entity);
            } else if (components_types[i] == typeid(Beholder).name()) {
                remove_component<Beholder>(entity);
            } else if (components_types[i] == typeid(RigidBody).name()) {
                remove_component<RigidBody>(entity);
            } else if (components_types[i] == typeid(Collider).name()) {
                remove_component<Collider>(entity);
            } else if (
                components_types[i] == typeid(DirectionalLightComponent).name()
            ) {
                remove_component<DirectionalLightComponent>(entity);
            } else if (components_types[i] == typeid(PointLightComponent).name()) {
                remove_component<PointLightComponent>(entity);
            } else if (components_types[i] == typeid(SpotLightComponent).name()) {
                remove_component<SpotLightComponent>(entity);
            } else if (components_types[i] == typeid(Material).name()) {
                remove_component<Material>(entity);
            } else if (components_types[i] == typeid(Move).name()) {
                remove_component<Move>(entity);
            } else if (components_types[i] == typeid(Mesh).name()) {
                remove_component<Mesh>(entity);
            } else if (components_types[i] == typeid(glvm::Controller).name()) {
                remove_component<glvm::Controller>(entity);
            } else if (components_types[i] == typeid(Projectile).name()) {
                remove_component<Projectile>(entity);
            } else if (components_types[i] == typeid(Enemy).name()) {
                remove_component<Enemy>(entity);
            } else if (components_types[i] == typeid(Font).name()) {
                remove_component<Font>(entity);
            } else if (components_types[i] == typeid(Health).name()) {
                remove_component<Health>(entity);
            } else if (components_types[i] == typeid(State).name()) {
                remove_component<State>(entity);
            } else if (components_types[i] == typeid(Actor).name()) {
                remove_component<Actor>(entity);
            } else {
                continue;
            }
        }
    }

    unsigned int get_container_id();

    template<typename ComponentType>
    std::vector<ComponentType>* get_component_container() {
        return std::static_pointer_cast<std::vector<ComponentType>>(
                   world_components_container
                       [create_component_container<ComponentType>()]
        )
            .get();
    }

    template<typename ComponentType>
    std::vector<unsigned int>* get_entity_container() {
        return static_cast<std::vector<unsigned int>*>(
            world_dense_components_map_to_entities
                [create_component_container<ComponentType>()]
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

    void set_to_default_values() {
        highest_x = -FLT_MAX;
        lowest_x = FLT_MAX;
        highest_y = -FLT_MAX;
        lowest_y = FLT_MAX;
        highest_z = -FLT_MAX;
        lowest_z = FLT_MAX;
    }

    void compare_per_direction_and_set_to_maximum_value_by_module(
        SVertex& vertex
    ) {
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

    void compare_per_direction_and_set_to_maximum_value_by_module(
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
    ShadowMapDirectionalLight,
    ShadowMapSpotLight,
    ShadowMapPointLight,
    HUD,
    FontRenderUbo,
    FontRenderSampler,
    HudScreen,
    UI,
    UiSamplers,
    UiIcons,
    UiIconsSamplers,
    VirtualTexturesUbo,
    VirtualTexturesTileset,
    MainRenderMatrixUbo,
    MainRenderLightDataUbo,
    MainRenderSpecularSampler,
    MainRenderDiffuseSampler,
    SdfData,
    // Not related to any pipeline values.
    RidableTextures,
    DescriptorChunksNumber
};

enum SpecificPipeline {
    DirectionalLightPipeline,
    SpotLightPipeline,
    PointLightPipeline,
    HudPipeline,
    FontPipeline,
    HudScreenPipeline,
    UiPipeline,
    UiIconsPipeline,
    VirtualTexturesPipeline,
    MainRenderPipeline,
    SdfPipeline,
    PipelinesNumber
};

struct RenderPass {
    unsigned int actual_attachment_description_number;
    VkAttachmentDescription attachment_descriptions[16];
    unsigned int actual_attachment_reference_number;
    VkAttachmentReference attachment_references[16];
    unsigned int actual_subpass_dependency_number;
    VkSubpassDependency subpass_dependencies[8];
};

struct GpuImage {
    VkImage image;
    VkDeviceMemory device_memory = {};
    std::vector<VkImageView> views = {};
    VkImageViewType view_type = {};
    VkImageCreateFlags create_flags = {};
    VkMemoryPropertyFlags memory_property_flags = {};
    VkImageUsageFlags usage_flags = {};
    VkImageAspectFlags aspect_flags = {};
    VkFormat format = {};
    VkImageTiling tiling = {};
    VkSampler sampler = {};
    VkComponentSwizzle red = {};
    VkComponentSwizzle green = {};
    VkComponentSwizzle blue = {};
    VkComponentSwizzle alpha = {};
    uint32_t array_layers = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

// Metadata for descriptor bindings.
struct DescriptorBinding {
    VkDescriptorType vk_type;
    VkShaderStageFlags shader_stage_flag;
    unsigned int binding;
    unsigned int shader_descriptors_number;
    unsigned int global_descriptor_offset;
    VkDeviceSize ubo_chunk_size;
};

// Metadata for descriptor sets.
struct DescriptorSet {
    unsigned int actual_linked_descriptor_bindings_number;
    unsigned int host_descriptor_number;
    VkDescriptorSetLayout set_layout;
    static constexpr unsigned int MAXIMUM_LINKED_DESCRIPTOR_BINDINGS_DS = 32;
    unsigned int
        descriptors_bindings_i_ds[MAXIMUM_LINKED_DESCRIPTOR_BINDINGS_DS];
    unsigned int descriptor_set_offset;
    bool is_texture;
};

struct Pipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    const char* vert_shader = nullptr;
    const char* frag_shader = nullptr;
    VkVertexInputBindingDescription binding_description;
    std::array<VkVertexInputAttributeDescription, 5> attribute_descriptions;
    unsigned int actual_linked_descriptor_sets_number;
    static constexpr unsigned int MAXIMUM_LINKED_DESCRIPTOR_SET_DS = 32;
    unsigned int linked_descriptor_set_i_ds[MAXIMUM_LINKED_DESCRIPTOR_SET_DS];
};

struct GPUBuffer {
    VkBuffer buffer;
    VkDeviceMemory device_memory;
};

union Descriptor {
    Descriptor() {};
    ~Descriptor() {};

    GPUBuffer* gpu_buffer;
    GpuImage* gpu_image;
};

struct Vertex {
    Vector<float, 3> pos;
    Vector<float, 3> color;
    Vector<float, 2> tex_coord;
    Vector<float, 4> join_indices;
    Vector<float, 4> weights;

    static VkVertexInputBindingDescription get_binding_description() {
        VkVertexInputBindingDescription binding_description {};
        binding_description.binding = 0;
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        binding_description.stride = sizeof(Vertex);

        return binding_description;
    }

    static std::array<VkVertexInputAttributeDescription, 5>
    get_attribute_descriptions() {
        std::array<VkVertexInputAttributeDescription, 5>
            attribute_descriptions {};

        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, pos);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);

        attribute_descriptions[2].binding = 0;
        attribute_descriptions[2].location = 2;
        attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

        attribute_descriptions[3].binding = 0;
        attribute_descriptions[3].location = 3;
        attribute_descriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attribute_descriptions[3].offset = offsetof(Vertex, join_indices);

        attribute_descriptions[4].binding = 0;
        attribute_descriptions[4].location = 4;
        attribute_descriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attribute_descriptions[4].offset = offsetof(Vertex, weights);

        return attribute_descriptions;
    }
};
} // namespace glvm

// Render objects.
struct RenderPlayer {
    Vector<float, 3> position;
    Vector<float, 3> forward;
};

struct RenderActor {
    Matrix<float, 4> model_matrix;
    std::vector<Matrix<float, 4>> joint_matrices;
    unsigned int mesh_id;
    unsigned int diffuse_texture_index;
    unsigned int specular_texture_index;
    Vector<float, 3> ambient;
    float shininess;
};

struct RenderDirectionalLight {
    Matrix<float, 4> directional_light_space_matrix;
    Vector<float, 4> position;
    Vector<float, 4> direction;

    Vector<float, 4> ambient;
    Vector<float, 4> diffuse;
    Vector<float, 4> specular;
};

struct RenderSpotLight {
    Matrix<float, 4> spot_ligth_space_matrix;
    Vector<float, 3> position;
    Vector<float, 3> direction;
    float cut_off;
    float outer_cut_off;

    Vector<float, 3> ambient;
    Vector<float, 3> diffuse;
    Vector<float, 3> specular;

    float constant;
    float linear;
    float quadratic;
};

struct RenderPointLight {
    Matrix<float, 4> point_light_space_matrix[CUBE_MAP_LAYER_NUMBER];
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
    float max_health;
    float current_health;
    unsigned int mesh_id;
};

struct RenderFont {
    Vector<float, 3> position;
    std::vector<char> font_string;
    float life_time;
};

struct SlotData {
    Matrix<float, 4> model;
    Vector<float, 3> color;
};

struct RenderInventory {
    std::vector<SlotData> slot_data;
    unsigned int inventory_texture_id;
    unsigned int mesh_id;
    unsigned int row;
    unsigned int col;
};

struct RenderItem {
    Matrix<float, 4> model;
    unsigned int mesh_id;
    unsigned int diffuse_texture_id;
};

struct RenderCrosshair {
    Matrix<float, 4> model;
    unsigned int mesh_id;
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
    virtual ~ISystem() {
    }

    virtual void update() = 0;
};
} // namespace glvm

extern glvm::CEvent G_E_EVENT;

// Contains all maximum absolute axis values.
extern std::vector<glvm::MeshAxisMaxAbsoluteValues> ALL_MESH_MAX_ABSOLUTE_VALUES;

extern glvm::CStack INPUT_STACK;

extern int X_POINTER;
extern int Y_POINTER;
extern int KEYS_PRESSED[6];

namespace glvm {
extern std::vector<VkDescriptorSet> DESCRIPTOR_SETS_CHUNKS;
extern std::vector<VkRenderPass> RENDER_PASSES;
extern std::vector<Descriptor> GPU_DESCRIPTORS;
} // namespace glvm

namespace glvm {
enum DeactivatedSystems { DeactivatedMovementSystem };

class CSystemManager: public ISystem {
    static CSystemManager* p_instance;
    static std::mutex mutex;
    std::vector<DeactivatedSystems> deactivated_systems;

    CSystemManager();

public:
    ~CSystemManager();
    // Don't need to make cope because of singleton property.
    CSystemManager(CSystemManager& other) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const CSystemManager& other) = delete;
    // It possibly to get only one instance of this class whith this method.
    static CSystemManager* get_instance();

    inline static unsigned int s_i_system_id = 0;
    std::vector<ISystem*> t_system_container;

    void activate_system(ISystem* system);
    void deactivate_system(DeactivatedSystems system);
    void return_system_to_activated_state(DeactivatedSystems system);

    void update() override;
};
} // namespace glvm

namespace glvm {
class DamageSystem: public ISystem {
public:
    void update() override;

    float delta_time;

    uint32_t cached_attackable_archetypes_number = 0;
    uint32_t cached_font_archetypes_number = 0;

    struct ArchView {
        Archetype* cached_attackable_archetypes[32];
        Archetype* cached_font_archetypes[32];
    } arch_view;

    struct ComponentsView {
        Attack* attackable_attacks = nullptr;
        Health* attackable_health = nullptr;
        Font* attackable_fonts = nullptr;

        Font* fonts = nullptr;
    } components_view;

    uint64_t attackable_required_mask =
        (1ul << ComponentsIndices::AttackComponent)
        | (1ul << ComponentsIndices::HealthComponent)
        | (1ul << ComponentsIndices::FontComponent);

    uint64_t font_required_mask = (1ull << ComponentsIndices::FontComponent);
};
} // namespace glvm

namespace glvm {
class CPhysicsSystem: public ISystem {
public:
    float f_acceleration_of_gravity;
    float f_delta_time;
    float& gravity;
    CStack& input_stack;

    uint32_t cached_archetypes_number = 0;

    struct ArchView {
        Archetype* cached_archetypes[32];
    } arch_view;

    struct ComponentsView {
        Transform* transforms_view = nullptr;
        Move* moves_view = nullptr;
        RigidBody* rigid_bodies_view = nullptr;
        ColliderFlags* collider_flags_view = nullptr;
        Collider* colliders_view = nullptr;
        Mesh* meshes_view = nullptr;
    } components_view;

    uint64_t required_mask = (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MoveComponent)
        | (1ul << ComponentsIndices::RigidBodyComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::MeshComponent);

    CPhysicsSystem(float& initial_gravity, CStack& stack) :
        gravity(initial_gravity),
        input_stack(stack) {
    }

    // Set Y-axis of transform component of backtracking entity to upper Y-axis
    // of ground entity.

    // This update searching for referring to colliders entities and check their
    // transform components for collision, and if collision detected check if
    // backtracking entity had gravity component for call Gravity function.
    void update() override;
    void repel(
        Transform& transform_component,
        float& delta_time,
        Beholder& view,
        CEvent& event
    );
};
} // namespace glvm

namespace glvm {
constexpr uint32_t CROSSHAIR_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Material)
       + sizeof(CrossHairTagComponent));

struct CrosshairArchetype: Archetype {
    Transform transforms[CROSSHAIR_ARCH_CHUNK_SIZE];
    Mesh meshes[CROSSHAIR_ARCH_CHUNK_SIZE];
    Material materials[CROSSHAIR_ARCH_CHUNK_SIZE];
    CrossHairTagComponent crosshair_tag_components[CROSSHAIR_ARCH_CHUNK_SIZE];

    CrosshairArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::CrosshairTagComponent] =
            crosshair_tag_components;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::CrosshairTagComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::MaterialComponent;
        component_ids[3] = ComponentsIndices::CrosshairTagComponent;
        component_count = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Material)
       + sizeof(DirectionalLightComponent));

struct DirectionalLightArchetype: Archetype {
    Transform transforms[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];
    Mesh meshes[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];
    Material materials[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];
    DirectionalLightComponent
        directional_lights[DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE];

    DirectionalLightArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::DirectionalLightComponent] =
            directional_lights;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::DirectionalLightComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::MaterialComponent;
        component_ids[3] = ComponentsIndices::DirectionalLightComponent;
        component_count = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t ENEMY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Enemy) + sizeof(State) + sizeof(Font)
       + sizeof(Animation) + sizeof(Material) + sizeof(Mesh) + sizeof(Collider)
       + sizeof(ColliderFlags) + sizeof(Health) + sizeof(RigidBody)
       + sizeof(Attack) + sizeof(Rotation) + sizeof(Move));

struct EnemyArchetype: Archetype {
    Transform transforms[ENEMY_ARCH_CHUNK_SIZE];
    Enemy enemies[ENEMY_ARCH_CHUNK_SIZE];
    State states[ENEMY_ARCH_CHUNK_SIZE];
    Font fonts[ENEMY_ARCH_CHUNK_SIZE];
    Animation animations[ENEMY_ARCH_CHUNK_SIZE];
    Material materials[ENEMY_ARCH_CHUNK_SIZE];
    Mesh meshes[ENEMY_ARCH_CHUNK_SIZE];
    Collider colliders[ENEMY_ARCH_CHUNK_SIZE];
    ColliderFlags collider_flags[ENEMY_ARCH_CHUNK_SIZE];
    Health health[ENEMY_ARCH_CHUNK_SIZE];
    RigidBody rigid_bodies[ENEMY_ARCH_CHUNK_SIZE];
    Attack attacks[ENEMY_ARCH_CHUNK_SIZE];
    Rotation rotations[ENEMY_ARCH_CHUNK_SIZE];
    Move moves[ENEMY_ARCH_CHUNK_SIZE];

    EnemyArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::EnemyComponent] = enemies;
        components[ComponentsIndices::StateComponent] = states;
        components[ComponentsIndices::FontComponent] = fonts;
        components[ComponentsIndices::AnimationComponent] = animations;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = collider_flags;
        components[ComponentsIndices::HealthComponent] = health;
        components[ComponentsIndices::RigidBodyComponent] = rigid_bodies;
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

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::EnemyComponent;
        component_ids[2] = ComponentsIndices::StateComponent;
        component_ids[3] = ComponentsIndices::FontComponent;
        component_ids[4] = ComponentsIndices::AnimationComponent;
        component_ids[5] = ComponentsIndices::MaterialComponent;
        component_ids[6] = ComponentsIndices::MeshComponent;
        component_ids[7] = ComponentsIndices::ColliderComponent;
        component_ids[8] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[9] = ComponentsIndices::HealthComponent;
        component_ids[10] = ComponentsIndices::RigidBodyComponent;
        component_ids[11] = ComponentsIndices::AttackComponent;
        component_ids[12] = ComponentsIndices::RotationComponent;
        component_ids[13] = ComponentsIndices::MoveComponent;
        component_count = 14;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t INVENTORY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Inventory) + sizeof(Material));

struct InventoryArchetype: Archetype {
    Transform transforms[INVENTORY_ARCH_CHUNK_SIZE];
    Mesh meshes[INVENTORY_ARCH_CHUNK_SIZE];
    Inventory invetories[INVENTORY_ARCH_CHUNK_SIZE];
    Material materials[INVENTORY_ARCH_CHUNK_SIZE];

    InventoryArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::InventoryComponent] = invetories;
        components[ComponentsIndices::MaterialComponent] = materials;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::InventoryComponent)
            | (1ull << ComponentsIndices::MaterialComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::InventoryComponent;
        component_ids[3] = ComponentsIndices::MaterialComponent;
        component_count = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t ITEM_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Collider) + sizeof(ColliderFlags)
       + sizeof(Mesh) + sizeof(RigidBody) + sizeof(Material) + sizeof(Rotation)
       + sizeof(Move) + sizeof(Item));

struct ItemArchetype: Archetype {
    Transform transforms[ITEM_ARCH_CHUNK_SIZE];
    Collider colliders[ITEM_ARCH_CHUNK_SIZE];
    ColliderFlags collider_flags[ITEM_ARCH_CHUNK_SIZE];
    Mesh meshes[ITEM_ARCH_CHUNK_SIZE];
    RigidBody rigid_bodies[ITEM_ARCH_CHUNK_SIZE];
    Material materials[ITEM_ARCH_CHUNK_SIZE];
    Rotation rotations[ITEM_ARCH_CHUNK_SIZE];
    Move moves[ITEM_ARCH_CHUNK_SIZE];
    Item items[ITEM_ARCH_CHUNK_SIZE];

    ItemArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = collider_flags;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::RigidBodyComponent] = rigid_bodies;
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

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::ColliderComponent;
        component_ids[2] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[3] = ComponentsIndices::MeshComponent;
        component_ids[4] = ComponentsIndices::RigidBodyComponent;
        component_ids[5] = ComponentsIndices::MaterialComponent;
        component_ids[6] = ComponentsIndices::RotationComponent;
        component_ids[7] = ComponentsIndices::MoveComponent;
        component_ids[8] = ComponentsIndices::ItemComponent;
        component_count = 9;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t LEVEL_CHUNK_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Material) + sizeof(Mesh) + sizeof(Collider)
       + sizeof(ColliderFlags) + sizeof(Rotation)
       + sizeof(LevelChunkTagComponent));

struct LevelChunkArchetype: Archetype {
    Transform transforms[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    Material materials[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    Mesh meshes[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    Collider colliders[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    ColliderFlags collider_flags[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    Rotation rotations[LEVEL_CHUNK_ARCH_CHUNK_SIZE];
    LevelChunkTagComponent
        level_chunk_tag_components[LEVEL_CHUNK_ARCH_CHUNK_SIZE];

    LevelChunkArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = collider_flags;
        components[ComponentsIndices::RotationComponent] = rotations;
        components[ComponentsIndices::LevelChunkTagComponent] =
            level_chunk_tag_components;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::RotationComponent)
            | (1ull << ComponentsIndices::LevelChunkTagComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MaterialComponent;
        component_ids[2] = ComponentsIndices::MeshComponent;
        component_ids[3] = ComponentsIndices::ColliderComponent;
        component_ids[4] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[5] = ComponentsIndices::RotationComponent;
        component_ids[6] = ComponentsIndices::LevelChunkTagComponent;
        component_count = 7;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t PLAYER_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Beholder) + sizeof(Collider)
       + sizeof(ColliderFlags) + sizeof(Mesh) + sizeof(RigidBody)
       + sizeof(Health) + sizeof(Material) + sizeof(Move) + sizeof(Attack)
       + sizeof(Animation) + sizeof(Font) + sizeof(Rotation)
       + sizeof(PlayerTagComponent));

struct PlayerArchetype: Archetype {
    Transform transforms[PLAYER_ARCH_CHUNK_SIZE];
    Beholder beholders[PLAYER_ARCH_CHUNK_SIZE];
    Collider colliders[PLAYER_ARCH_CHUNK_SIZE];
    ColliderFlags collider_flags[PLAYER_ARCH_CHUNK_SIZE];
    Mesh meshes[PLAYER_ARCH_CHUNK_SIZE];
    RigidBody rigid_bodies[PLAYER_ARCH_CHUNK_SIZE];
    Health health[PLAYER_ARCH_CHUNK_SIZE];
    Material materials[PLAYER_ARCH_CHUNK_SIZE];
    Move moves[PLAYER_ARCH_CHUNK_SIZE];
    Attack attacks[PLAYER_ARCH_CHUNK_SIZE];
    Animation animations[PLAYER_ARCH_CHUNK_SIZE];
    Font fonts[PLAYER_ARCH_CHUNK_SIZE];
    Rotation rotations[PLAYER_ARCH_CHUNK_SIZE];
    PlayerTagComponent player_tag_components[PLAYER_ARCH_CHUNK_SIZE];

    PlayerArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::ViewComponent] = beholders;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = collider_flags;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::RigidBodyComponent] = rigid_bodies;
        components[ComponentsIndices::HealthComponent] = health;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::MoveComponent] = moves;
        components[ComponentsIndices::AttackComponent] = attacks;
        components[ComponentsIndices::AnimationComponent] = animations;
        components[ComponentsIndices::FontComponent] = fonts;
        components[ComponentsIndices::RotationComponent] = rotations;
        components[ComponentsIndices::PlayerTagComponent] =
            player_tag_components;

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

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::ViewComponent;
        component_ids[2] = ComponentsIndices::ColliderComponent;
        component_ids[3] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[4] = ComponentsIndices::MeshComponent;
        component_ids[5] = ComponentsIndices::RigidBodyComponent;
        component_ids[6] = ComponentsIndices::HealthComponent;
        component_ids[7] = ComponentsIndices::MaterialComponent;
        component_ids[8] = ComponentsIndices::MoveComponent;
        component_ids[9] = ComponentsIndices::AttackComponent;
        component_ids[10] = ComponentsIndices::AnimationComponent;
        component_ids[11] = ComponentsIndices::FontComponent;
        component_ids[12] = ComponentsIndices::RotationComponent;
        component_ids[13] = ComponentsIndices::PlayerTagComponent;
        component_count = 14;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t POINT_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Material)
       + sizeof(PointLightComponent));

struct PointLightArchetype: Archetype {
    Transform transforms[POINT_LIGHT_ARCH_CHUNK_SIZE];
    Mesh meshes[POINT_LIGHT_ARCH_CHUNK_SIZE];
    Material materials[POINT_LIGHT_ARCH_CHUNK_SIZE];
    PointLightComponent point_lights[POINT_LIGHT_ARCH_CHUNK_SIZE];

    PointLightArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::PointLightComponent] = point_lights;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::PointLightComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::MaterialComponent;
        component_ids[3] = ComponentsIndices::PointLightComponent;
        component_count = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t PROJECTILE_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Collider)
       + sizeof(ColliderFlags) + sizeof(Rotation) + sizeof(ProjectileBundle)
       + sizeof(Health) + sizeof(Attack) + sizeof(Font)
       + sizeof(ProjectileTagComponent));

struct ProjectileArchetype: Archetype {
    Transform transforms[PROJECTILE_ARCH_CHUNK_SIZE];
    Mesh meshes[PROJECTILE_ARCH_CHUNK_SIZE];
    Collider colliders[PROJECTILE_ARCH_CHUNK_SIZE];
    ColliderFlags collider_flags[PROJECTILE_ARCH_CHUNK_SIZE];
    Rotation rotations[PROJECTILE_ARCH_CHUNK_SIZE];
    ProjectileBundle projectile_bundles[PROJECTILE_ARCH_CHUNK_SIZE];
    Health heath[PROJECTILE_ARCH_CHUNK_SIZE];
    Attack attacks[PROJECTILE_ARCH_CHUNK_SIZE];
    Font fonts[PROJECTILE_ARCH_CHUNK_SIZE];
    ProjectileTagComponent projectile_tag_components[PROJECTILE_ARCH_CHUNK_SIZE];

    ProjectileArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = collider_flags;
        components[ComponentsIndices::RotationComponent] = rotations;
        components[ComponentsIndices::ProjectileBundleComponent] =
            projectile_bundles;
        components[ComponentsIndices::HealthComponent] = heath;
        components[ComponentsIndices::AttackComponent] = attacks;
        components[ComponentsIndices::FontComponent] = fonts;
        components[ComponentsIndices::ProjectileTagComponent] =
            projectile_tag_components;

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

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::ColliderComponent;
        component_ids[3] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[4] = ComponentsIndices::RotationComponent;
        component_ids[5] = ComponentsIndices::ProjectileBundleComponent;
        component_ids[6] = ComponentsIndices::HealthComponent;
        component_ids[7] = ComponentsIndices::AttackComponent;
        component_ids[8] = ComponentsIndices::FontComponent;
        component_ids[9] = ComponentsIndices::ProjectileTagComponent;
        component_count = 10;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t RIGID_BODY_ARCH_CHUNK_SIZE =
    ARCHETYPE_CHUNK_SIZE / (sizeof(glvm::Transform) + sizeof(glvm::RigidBody));

struct RigidBodyArch {
    Transform transforms[RIGID_BODY_ARCH_CHUNK_SIZE];
    RigidBody rigid_bodies[RIGID_BODY_ARCH_CHUNK_SIZE];
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t SPOT_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Material)
       + sizeof(SpotLightComponent));

struct SpotLightArchetype: Archetype {
    Transform transforms[SPOT_LIGHT_ARCH_CHUNK_SIZE];
    Mesh meshes[SPOT_LIGHT_ARCH_CHUNK_SIZE];
    Material materials[SPOT_LIGHT_ARCH_CHUNK_SIZE];
    SpotLightComponent spot_lights[SPOT_LIGHT_ARCH_CHUNK_SIZE];

    SpotLightArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::SpotLightComponent] = spot_lights;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::SpotLightComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::MeshComponent;
        component_ids[2] = ComponentsIndices::MaterialComponent;
        component_ids[3] = ComponentsIndices::SpotLightComponent;
        component_count = 4;
    }
};
}; // namespace glvm

namespace glvm {
constexpr uint32_t STATIC_MESH_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Collider) + sizeof(ColliderFlags)
       + sizeof(Mesh) + sizeof(Material) + sizeof(Font) + sizeof(Rotation)
       + sizeof(StaticMeshTagComponent));

struct StaticMeshArchetype: Archetype {
    Transform transforms[STATIC_MESH_ARCH_CHUNK_SIZE];
    Collider colliders[STATIC_MESH_ARCH_CHUNK_SIZE];
    ColliderFlags collider_flags[STATIC_MESH_ARCH_CHUNK_SIZE];
    Mesh meshes[STATIC_MESH_ARCH_CHUNK_SIZE];
    Material materials[STATIC_MESH_ARCH_CHUNK_SIZE];
    Font fonts[STATIC_MESH_ARCH_CHUNK_SIZE];
    Rotation rotations[STATIC_MESH_ARCH_CHUNK_SIZE];
    StaticMeshTagComponent
        static_mesh_tag_components[STATIC_MESH_ARCH_CHUNK_SIZE];

    StaticMeshArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = collider_flags;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::MaterialComponent] = materials;
        components[ComponentsIndices::FontComponent] = fonts;
        components[ComponentsIndices::RotationComponent] = rotations;
        components[ComponentsIndices::StaticMeshTagComponent] =
            static_mesh_tag_components;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::MeshComponent)
            | (1ull << ComponentsIndices::MaterialComponent)
            | (1ull << ComponentsIndices::FontComponent)
            | (1ull << ComponentsIndices::RotationComponent)
            | (1ull << ComponentsIndices::StaticMeshTagComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::ColliderComponent;
        component_ids[2] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[3] = ComponentsIndices::MeshComponent;
        component_ids[4] = ComponentsIndices::MaterialComponent;
        component_ids[5] = ComponentsIndices::FontComponent;
        component_ids[6] = ComponentsIndices::RotationComponent;
        component_ids[7] = ComponentsIndices::StaticMeshTagComponent;
        component_count = 8;
    }
};
}; // namespace glvm

#ifdef __linux__

namespace glvm {
struct WindowWaylandVulkan: IWindow {
    CStack* input_stack = nullptr;
    WindowWaylandVulkan();
    void init();
    void close() override;
    bool handle_event(CEvent& event) override;
    static int create_anonymous_file(off_t size);
    struct wl_buffer* create_transparent_cursor(struct wl_shm* shm);
    void swap_buffers() override;
    void clear_display() override;
    void cursor_lock(
        int pointer_x,
        int pointer_y,
        int* out_offset_x,
        int* out_offset_y
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
    // Compositor may never report a size (WSLg sends 0,0); pick a default.
    uint16_t width = 1280;
    uint16_t height = 720;
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

[[nodiscard]] WindowWaylandVulkan* initialize_wayland_window();
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
    void cursor_lock(
        int pointer_x,
        int pointer_y,
        int* out_offset_x,
        int* out_offset_y
    ) override;
    void swap_buffers() override;
    void clear_display() override;
    bool handle_event(CEvent& event) override;
    void close() override;
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

    void swap_buffers() override;
    void clear_display() override;
    bool handle_event(CEvent& event) override;
    void close() override;
    void cursor_lock(
        int pointer_x,
        int pointer_y,
        int* out_offset_x,
        int* out_offset_y
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
    void create_swap_chain_resources();
    void destroy_swap_chain_resources();

    void new_frame();
    void record_command_buffer(
        VkCommandBuffer command_buffer,
        uint32_t image_index
    );

    bool wants_mouse() const;

    bool is_enabled() const {
        return initialized;
    }

    bool show_panel = true;
    bool show_actor_bounds = false;
    bool show_light_frustums = false;
    bool show_shadow_maps = false;
    bool show_spatial_grid = false;
    bool shadows_enabled = true;
    int shadow_map_mode = 0; // 0 = directional, 1 = spot.
    int shadow_map_light = 0;

private:
    CVulkanRenderer& renderer;
    bool initialized = false;

    VkRenderPass render_pass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;

    VkPipelineLayout line_layout = VK_NULL_HANDLE;
    VkPipeline line_pipeline = VK_NULL_HANDLE;

    VkBuffer vertex_buffer = VK_NULL_HANDLE;
    VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;
    void* vertex_buffer_mapped = nullptr;
    uint32_t line_vertex_count = 0;

    void create_render_pass();
    void create_line_pipeline();
    void create_vertex_buffer();
    void build_panel();
    void build_debug_vertices();
    void record_debug_draws(
        VkCommandBuffer command_buffer,
        uint32_t image_index
    );
    void record_im_gui_draws(
        VkCommandBuffer command_buffer,
        uint32_t image_index
    );
};

} // namespace glvm

namespace glvm {
inline DescriptorSet DESCRIPTOR_SETS_CONFIG[32];
inline DescriptorBinding DESCRIPTOR_BINDINGS_CONFIG[32];
inline Pipeline PIPELINE_CONFIGS[32];
inline RenderPass RENDER_PASS_CONFIGS[32];
constexpr uint32_t MAX_TEXTURES = 18;

inline void vk_config_initializer() {
    // Pipelines and its render passes. Put all meta data related to pipeline
    // here. Also needed to add meta data of descriptor sets and it's bindings
    // that will be related to specific pipeline

    DESCRIPTOR_SETS_CONFIG[ShadowMapDirectionalLight]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[ShadowMapDirectionalLight].host_descriptor_number =
        128;
    DESCRIPTOR_SETS_CONFIG[ShadowMapDirectionalLight].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[0].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[0].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[0].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[0].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[0].ubo_chunk_size = sizeof(ShadowMapMatrixUBO);

    PIPELINE_CONFIGS[DirectionalLightPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/flat_shadow_map/vertFlatShadowMap.spv";
    PIPELINE_CONFIGS[DirectionalLightPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[DirectionalLightPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[DirectionalLightPipeline]
        .actual_linked_descriptor_sets_number = 1;

    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .actual_attachment_description_number = 1;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .attachment_descriptions[0]
        .flags = 0;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .attachment_descriptions[0]
        .samples = VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .attachment_descriptions[0]
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .attachment_descriptions[0]
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .attachment_descriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .attachment_descriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .attachment_descriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .attachment_descriptions[0]
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .actual_attachment_reference_number = 1;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .attachment_references[0]
        .attachment = 0;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .attachment_references[0]
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .actual_subpass_dependency_number = 2;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[0]
        .srcSubpass = VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[0]
        .dstSubpass = 0;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[0]
        .srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[0]
        .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[0]
        .srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[0]
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[0]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[1]
        .srcSubpass = 0;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[1]
        .dstSubpass = VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[1]
        .srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[1]
        .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[1]
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[1]
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    RENDER_PASS_CONFIGS[DirectionalLightPipeline]
        .subpass_dependencies[1]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    DESCRIPTOR_SETS_CONFIG[ShadowMapSpotLight]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[ShadowMapSpotLight].host_descriptor_number = 256;
    DESCRIPTOR_SETS_CONFIG[ShadowMapSpotLight].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[1].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[1].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[1].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[1].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[1].ubo_chunk_size = sizeof(ShadowMapMatrixUBO);

    PIPELINE_CONFIGS[SpotLightPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/flat_shadow_map/vertFlatShadowMap.spv";
    PIPELINE_CONFIGS[SpotLightPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[SpotLightPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[SpotLightPipeline].actual_linked_descriptor_sets_number =
        1;

    RENDER_PASS_CONFIGS[SpotLightPipeline].actual_attachment_description_number =
        1;
    RENDER_PASS_CONFIGS[SpotLightPipeline].attachment_descriptions[0].flags = 0;
    RENDER_PASS_CONFIGS[SpotLightPipeline].attachment_descriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[SpotLightPipeline].attachment_descriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[SpotLightPipeline].attachment_descriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[SpotLightPipeline]
        .attachment_descriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[SpotLightPipeline]
        .attachment_descriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[SpotLightPipeline]
        .attachment_descriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[SpotLightPipeline]
        .attachment_descriptions[0]
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    RENDER_PASS_CONFIGS[SpotLightPipeline].actual_attachment_reference_number =
        1;
    RENDER_PASS_CONFIGS[SpotLightPipeline].attachment_references[0].attachment =
        0;
    RENDER_PASS_CONFIGS[SpotLightPipeline].attachment_references[0].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[SpotLightPipeline].actual_subpass_dependency_number = 2;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[0].srcSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[0].dstSubpass =
        0;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[0].srcAccessMask =
        VK_ACCESS_SHADER_READ_BIT;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[0].dstAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[SpotLightPipeline]
        .subpass_dependencies[0]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[1].srcSubpass =
        0;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[1].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[1].dstStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[1].srcAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[SpotLightPipeline].subpass_dependencies[1].dstAccessMask =
        VK_ACCESS_SHADER_READ_BIT;
    RENDER_PASS_CONFIGS[SpotLightPipeline]
        .subpass_dependencies[1]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    DESCRIPTOR_SETS_CONFIG[ShadowMapPointLight]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[ShadowMapPointLight].host_descriptor_number = 512;
    DESCRIPTOR_SETS_CONFIG[ShadowMapPointLight].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[2].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[2].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[2].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[2].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[2].ubo_chunk_size =
        sizeof(PointLightShadowMapMatrixUBO);

    PIPELINE_CONFIGS[PointLightPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/cube_shadow_map/vertCubeShadowMap.spv";
    PIPELINE_CONFIGS[PointLightPipeline].frag_shader =
        "../../../crates/glvm2/assets/shaders/cube_shadow_map/fragCubeShadowMap.spv";
    PIPELINE_CONFIGS[PointLightPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[PointLightPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[PointLightPipeline].actual_linked_descriptor_sets_number =
        1;

    RENDER_PASS_CONFIGS[PointLightPipeline]
        .actual_attachment_description_number = 1;
    RENDER_PASS_CONFIGS[PointLightPipeline].attachment_descriptions[0].flags =
        0;
    RENDER_PASS_CONFIGS[PointLightPipeline].attachment_descriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[PointLightPipeline].attachment_descriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[PointLightPipeline].attachment_descriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[PointLightPipeline]
        .attachment_descriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[PointLightPipeline]
        .attachment_descriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[PointLightPipeline]
        .attachment_descriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[PointLightPipeline]
        .attachment_descriptions[0]
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    RENDER_PASS_CONFIGS[PointLightPipeline].actual_attachment_reference_number =
        1;
    RENDER_PASS_CONFIGS[PointLightPipeline].attachment_references[0].attachment =
        0;
    RENDER_PASS_CONFIGS[PointLightPipeline].attachment_references[0].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[PointLightPipeline].actual_subpass_dependency_number =
        2;
    RENDER_PASS_CONFIGS[PointLightPipeline].subpass_dependencies[0].srcSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[PointLightPipeline].subpass_dependencies[0].dstSubpass =
        0;
    RENDER_PASS_CONFIGS[PointLightPipeline].subpass_dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    RENDER_PASS_CONFIGS[PointLightPipeline].subpass_dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[PointLightPipeline]
        .subpass_dependencies[0]
        .srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    RENDER_PASS_CONFIGS[PointLightPipeline]
        .subpass_dependencies[0]
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[PointLightPipeline]
        .subpass_dependencies[0]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    RENDER_PASS_CONFIGS[PointLightPipeline].subpass_dependencies[1].srcSubpass =
        0;
    RENDER_PASS_CONFIGS[PointLightPipeline].subpass_dependencies[1].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[PointLightPipeline].subpass_dependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[PointLightPipeline].subpass_dependencies[1].dstStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    RENDER_PASS_CONFIGS[PointLightPipeline]
        .subpass_dependencies[1]
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[PointLightPipeline]
        .subpass_dependencies[1]
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    RENDER_PASS_CONFIGS[PointLightPipeline]
        .subpass_dependencies[1]
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    DESCRIPTOR_SETS_CONFIG[HUD].actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[HUD].host_descriptor_number = 1024;
    DESCRIPTOR_SETS_CONFIG[HUD].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[3].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[3].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[3].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[3].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[3].ubo_chunk_size = sizeof(HudUbo);

    PIPELINE_CONFIGS[HudPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/hud/hud_vert.spv";
    PIPELINE_CONFIGS[HudPipeline].frag_shader =
        "../../../crates/glvm2/assets/shaders/hud/hud_frag.spv";
    PIPELINE_CONFIGS[HudPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[HudPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[HudPipeline].actual_linked_descriptor_sets_number = 1;

    RENDER_PASS_CONFIGS[HudPipeline].actual_attachment_description_number = 2;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[0].flags = 0;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[0].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[0].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[0].initialLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[1].flags = 0;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[1].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[1].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[1].initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_descriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[HudPipeline].actual_attachment_reference_number = 2;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_references[0].attachment = 0;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_references[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[HudPipeline].attachment_references[1].attachment = 1;
    RENDER_PASS_CONFIGS[HudPipeline].attachment_references[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[HudPipeline].actual_subpass_dependency_number = 1;
    RENDER_PASS_CONFIGS[HudPipeline].subpass_dependencies[0].srcSubpass = 0;
    RENDER_PASS_CONFIGS[HudPipeline].subpass_dependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[HudPipeline].subpass_dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[HudPipeline].subpass_dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[HudPipeline].subpass_dependencies[0].srcAccessMask = {};
    RENDER_PASS_CONFIGS[HudPipeline].subpass_dependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[HudPipeline].subpass_dependencies[0].dependencyFlags =
        {};

    DESCRIPTOR_SETS_CONFIG[FontRenderUbo]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[FontRenderUbo].host_descriptor_number = 4096;
    DESCRIPTOR_SETS_CONFIG[FontRenderUbo].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[4].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[4].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[4].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[4].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[4].ubo_chunk_size = sizeof(FontUbo);

    DESCRIPTOR_SETS_CONFIG[FontRenderSampler]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[FontRenderSampler].host_descriptor_number =
        MAX_TEXTURES;
    DESCRIPTOR_SETS_CONFIG[FontRenderSampler].is_texture = true;

    DESCRIPTOR_BINDINGS_CONFIG[5].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[5].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[5].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[5].shader_descriptors_number = 1;

    PIPELINE_CONFIGS[FontPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/font/font_vert.spv";
    PIPELINE_CONFIGS[FontPipeline].frag_shader =
        "../../../crates/glvm2/assets/shaders/font/font_frag.spv";
    PIPELINE_CONFIGS[FontPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[FontPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[FontPipeline].actual_linked_descriptor_sets_number = 2;

    RENDER_PASS_CONFIGS[FontPipeline].actual_attachment_description_number = 2;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[0].flags = 0;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[0].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[0].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[0].initialLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[1].flags = 0;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[1].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[1].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[1].initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_descriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[FontPipeline].actual_attachment_reference_number = 2;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_references[0].attachment = 0;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_references[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[FontPipeline].attachment_references[1].attachment = 1;
    RENDER_PASS_CONFIGS[FontPipeline].attachment_references[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[FontPipeline].actual_subpass_dependency_number = 1;
    RENDER_PASS_CONFIGS[FontPipeline].subpass_dependencies[0].srcSubpass = 0;
    RENDER_PASS_CONFIGS[FontPipeline].subpass_dependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[FontPipeline].subpass_dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[FontPipeline].subpass_dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[FontPipeline].subpass_dependencies[0].srcAccessMask = {};
    RENDER_PASS_CONFIGS[FontPipeline].subpass_dependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[FontPipeline].subpass_dependencies[0].dependencyFlags =
        {};

    DESCRIPTOR_SETS_CONFIG[HudScreen].actual_linked_descriptor_bindings_number =
        1;
    DESCRIPTOR_SETS_CONFIG[HudScreen].host_descriptor_number = 64;
    DESCRIPTOR_SETS_CONFIG[HudScreen].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[6].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[6].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[6].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[6].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[6].ubo_chunk_size = sizeof(HudScreenUbo);

    PIPELINE_CONFIGS[HudScreenPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/hud_screen/vert_hud_screen.spv";
    PIPELINE_CONFIGS[HudScreenPipeline].frag_shader =
        "../../../crates/glvm2/assets/shaders/hud_screen/frag_hud_screen.spv";
    PIPELINE_CONFIGS[HudScreenPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[HudScreenPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[HudScreenPipeline].actual_linked_descriptor_sets_number =
        1;

    RENDER_PASS_CONFIGS[HudScreenPipeline].actual_attachment_description_number =
        2;
    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_descriptions[0].flags = 0;
    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_descriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_descriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_descriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[HudScreenPipeline]
        .attachment_descriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[HudScreenPipeline]
        .attachment_descriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[HudScreenPipeline]
        .attachment_descriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    RENDER_PASS_CONFIGS[HudScreenPipeline]
        .attachment_descriptions[0]
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_descriptions[1].flags = 0;
    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_descriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_descriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_descriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[HudScreenPipeline]
        .attachment_descriptions[1]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[HudScreenPipeline]
        .attachment_descriptions[1]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[HudScreenPipeline]
        .attachment_descriptions[1]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[HudScreenPipeline]
        .attachment_descriptions[1]
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[HudScreenPipeline].actual_attachment_reference_number =
        2;
    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_references[0].attachment =
        0;
    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_references[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_references[1].attachment =
        1;
    RENDER_PASS_CONFIGS[HudScreenPipeline].attachment_references[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[HudScreenPipeline].actual_subpass_dependency_number = 1;
    RENDER_PASS_CONFIGS[HudScreenPipeline].subpass_dependencies[0].srcSubpass =
        0;
    RENDER_PASS_CONFIGS[HudScreenPipeline].subpass_dependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[HudScreenPipeline].subpass_dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[HudScreenPipeline].subpass_dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[HudScreenPipeline]
        .subpass_dependencies[0]
        .srcAccessMask = {};
    RENDER_PASS_CONFIGS[HudScreenPipeline].subpass_dependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[HudScreenPipeline]
        .subpass_dependencies[0]
        .dependencyFlags = {};

    DESCRIPTOR_SETS_CONFIG[UI].actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[UI].host_descriptor_number = 128;
    DESCRIPTOR_SETS_CONFIG[UI].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[7].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[7].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[7].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[7].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[7].ubo_chunk_size = sizeof(UiUbo);

    DESCRIPTOR_SETS_CONFIG[UiSamplers].actual_linked_descriptor_bindings_number =
        1;
    DESCRIPTOR_SETS_CONFIG[UiSamplers].host_descriptor_number = MAX_TEXTURES;
    DESCRIPTOR_SETS_CONFIG[UiSamplers].is_texture = true;

    DESCRIPTOR_BINDINGS_CONFIG[8].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[8].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[8].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[8].shader_descriptors_number = 1;

    PIPELINE_CONFIGS[UiPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/ui/vert_ui.spv";
    PIPELINE_CONFIGS[UiPipeline].frag_shader =
        "../../../crates/glvm2/assets/shaders/ui/frag_ui.spv";
    PIPELINE_CONFIGS[UiPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[UiPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[UiPipeline].actual_linked_descriptor_sets_number = 2;

    RENDER_PASS_CONFIGS[UiPipeline].actual_attachment_description_number = 2;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[0].flags = 0;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[0].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[0].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[0].initialLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[1].flags = 0;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[1].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[1].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[1].initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_descriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[UiPipeline].actual_attachment_reference_number = 2;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_references[0].attachment = 0;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_references[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[UiPipeline].attachment_references[1].attachment = 1;
    RENDER_PASS_CONFIGS[UiPipeline].attachment_references[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[UiPipeline].actual_subpass_dependency_number = 1;
    RENDER_PASS_CONFIGS[UiPipeline].subpass_dependencies[0].srcSubpass = 0;
    RENDER_PASS_CONFIGS[UiPipeline].subpass_dependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[UiPipeline].subpass_dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[UiPipeline].subpass_dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[UiPipeline].subpass_dependencies[0].srcAccessMask = {};
    RENDER_PASS_CONFIGS[UiPipeline].subpass_dependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[UiPipeline].subpass_dependencies[0].dependencyFlags = {};

    DESCRIPTOR_SETS_CONFIG[UiIcons].actual_linked_descriptor_bindings_number =
        1;
    DESCRIPTOR_SETS_CONFIG[UiIcons].host_descriptor_number = 128;
    DESCRIPTOR_SETS_CONFIG[UiIcons].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[9].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[9].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[9].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[9].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[9].ubo_chunk_size = sizeof(UiUbo);

    DESCRIPTOR_SETS_CONFIG[UiIconsSamplers]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[UiIconsSamplers].host_descriptor_number =
        MAX_TEXTURES;
    DESCRIPTOR_SETS_CONFIG[UiIconsSamplers].is_texture = true;

    DESCRIPTOR_BINDINGS_CONFIG[10].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[10].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[10].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[10].shader_descriptors_number = 1;

    PIPELINE_CONFIGS[UiIconsPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/ui_icons/vert_ui_icons.spv";
    PIPELINE_CONFIGS[UiIconsPipeline].frag_shader =
        "../../../crates/glvm2/assets/shaders/ui_icons/frag_ui_icons.spv";
    PIPELINE_CONFIGS[UiIconsPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[UiIconsPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[UiIconsPipeline].actual_linked_descriptor_sets_number = 2;

    RENDER_PASS_CONFIGS[UiIconsPipeline].actual_attachment_description_number =
        2;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_descriptions[0].flags = 0;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_descriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_descriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_descriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[UiIconsPipeline]
        .attachment_descriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[UiIconsPipeline]
        .attachment_descriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[UiIconsPipeline]
        .attachment_descriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_descriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_descriptions[1].flags = 0;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_descriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_descriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_descriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[UiIconsPipeline]
        .attachment_descriptions[1]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[UiIconsPipeline]
        .attachment_descriptions[1]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[UiIconsPipeline]
        .attachment_descriptions[1]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_descriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[UiIconsPipeline].actual_attachment_reference_number = 2;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_references[0].attachment =
        0;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_references[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_references[1].attachment =
        1;
    RENDER_PASS_CONFIGS[UiIconsPipeline].attachment_references[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[UiIconsPipeline].actual_subpass_dependency_number = 1;
    RENDER_PASS_CONFIGS[UiIconsPipeline].subpass_dependencies[0].srcSubpass = 0;
    RENDER_PASS_CONFIGS[UiIconsPipeline].subpass_dependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[UiIconsPipeline].subpass_dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[UiIconsPipeline].subpass_dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[UiIconsPipeline].subpass_dependencies[0].srcAccessMask =
        {};
    RENDER_PASS_CONFIGS[UiIconsPipeline].subpass_dependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[UiIconsPipeline]
        .subpass_dependencies[0]
        .dependencyFlags = {};

    DESCRIPTOR_SETS_CONFIG[VirtualTexturesUbo]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[VirtualTexturesUbo].host_descriptor_number = 128;
    DESCRIPTOR_SETS_CONFIG[VirtualTexturesUbo].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[11].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[11].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[11].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[11].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[11].ubo_chunk_size = sizeof(VirtualTextureUbo);

    DESCRIPTOR_SETS_CONFIG[VirtualTexturesTileset]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[VirtualTexturesTileset].host_descriptor_number =
        MAX_TEXTURES;
    DESCRIPTOR_SETS_CONFIG[VirtualTexturesTileset].is_texture = true;

    DESCRIPTOR_BINDINGS_CONFIG[12].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[12].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[12].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[12].shader_descriptors_number = 1;

    PIPELINE_CONFIGS[VirtualTexturesPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/virtual_textures/virtualTexturesVert.spv";
    PIPELINE_CONFIGS[VirtualTexturesPipeline].frag_shader =
        "../../../crates/glvm2/assets/shaders/virtual_textures/virtualTexturesFrag.spv";
    PIPELINE_CONFIGS[VirtualTexturesPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[VirtualTexturesPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[VirtualTexturesPipeline]
        .actual_linked_descriptor_sets_number = 2;

    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .actual_attachment_description_number = 2;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[0]
        .flags = 0;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[0]
        .samples = VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[0]
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[0]
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[0]
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[1]
        .flags = 0;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[1]
        .samples = VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[1]
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[1]
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[1]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[1]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[1]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_descriptions[1]
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .actual_attachment_reference_number = 2;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_references[0]
        .attachment = 0;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline].attachment_references[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .attachment_references[1]
        .attachment = 1;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline].attachment_references[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .actual_subpass_dependency_number = 1;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .subpass_dependencies[0]
        .srcSubpass = 0;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .subpass_dependencies[0]
        .dstSubpass = VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .subpass_dependencies[0]
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .subpass_dependencies[0]
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .subpass_dependencies[0]
        .srcAccessMask = {};
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .subpass_dependencies[0]
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[VirtualTexturesPipeline]
        .subpass_dependencies[0]
        .dependencyFlags = {};

    DESCRIPTOR_SETS_CONFIG[MainRenderMatrixUbo]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[MainRenderMatrixUbo].host_descriptor_number = 1024;
    DESCRIPTOR_SETS_CONFIG[MainRenderMatrixUbo].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[13].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[13].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[13].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[13].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[13].ubo_chunk_size = sizeof(ModelMatrixUBO);

    DESCRIPTOR_SETS_CONFIG[MainRenderLightDataUbo]
        .actual_linked_descriptor_bindings_number = 4;
    DESCRIPTOR_SETS_CONFIG[MainRenderLightDataUbo].host_descriptor_number = 2;
    DESCRIPTOR_SETS_CONFIG[MainRenderLightDataUbo].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[14].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[14].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[14].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[14].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[14].ubo_chunk_size = sizeof(LightData);

    DESCRIPTOR_BINDINGS_CONFIG[15].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[15].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[15].binding = 1;
    DESCRIPTOR_BINDINGS_CONFIG[15].shader_descriptors_number = 4;

    DESCRIPTOR_BINDINGS_CONFIG[16].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[16].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[16].binding = 5;
    DESCRIPTOR_BINDINGS_CONFIG[16].shader_descriptors_number = 32;

    DESCRIPTOR_BINDINGS_CONFIG[17].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[17].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[17].binding = 37;
    DESCRIPTOR_BINDINGS_CONFIG[17].shader_descriptors_number = 8;

    DESCRIPTOR_SETS_CONFIG[MainRenderSpecularSampler]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[MainRenderSpecularSampler].host_descriptor_number =
        MAX_TEXTURES;
    DESCRIPTOR_SETS_CONFIG[MainRenderSpecularSampler].is_texture = true;

    DESCRIPTOR_BINDINGS_CONFIG[18].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[18].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[18].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[18].shader_descriptors_number = 1;

    DESCRIPTOR_SETS_CONFIG[MainRenderDiffuseSampler]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[MainRenderDiffuseSampler].host_descriptor_number =
        MAX_TEXTURES;
    DESCRIPTOR_SETS_CONFIG[MainRenderDiffuseSampler].is_texture = true;

    DESCRIPTOR_BINDINGS_CONFIG[19].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[19].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[19].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[19].shader_descriptors_number = 1;

    PIPELINE_CONFIGS[MainRenderPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/main_renderer/vert.spv";
    PIPELINE_CONFIGS[MainRenderPipeline].frag_shader =
        "../../../crates/glvm2/assets/shaders/main_renderer/frag.spv";
    PIPELINE_CONFIGS[MainRenderPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[MainRenderPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[MainRenderPipeline].actual_linked_descriptor_sets_number =
        4;

    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .actual_attachment_description_number = 2;
    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_descriptions[0].flags =
        0;
    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_descriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_descriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_descriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .attachment_descriptions[0]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .attachment_descriptions[0]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .attachment_descriptions[0]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .attachment_descriptions[0]
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_descriptions[1].flags =
        0;
    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_descriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_descriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_descriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .attachment_descriptions[1]
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .attachment_descriptions[1]
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .attachment_descriptions[1]
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .attachment_descriptions[1]
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[MainRenderPipeline].actual_attachment_reference_number =
        2;
    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_references[0].attachment =
        0;
    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_references[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_references[1].attachment =
        1;
    RENDER_PASS_CONFIGS[MainRenderPipeline].attachment_references[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[MainRenderPipeline].actual_subpass_dependency_number =
        1;
    RENDER_PASS_CONFIGS[MainRenderPipeline].subpass_dependencies[0].srcSubpass =
        0;
    RENDER_PASS_CONFIGS[MainRenderPipeline].subpass_dependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[MainRenderPipeline].subpass_dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[MainRenderPipeline].subpass_dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .subpass_dependencies[0]
        .srcAccessMask = {};
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .subpass_dependencies[0]
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[MainRenderPipeline]
        .subpass_dependencies[0]
        .dependencyFlags = {};

    DESCRIPTOR_SETS_CONFIG[SdfData].actual_linked_descriptor_bindings_number =
        1;
    DESCRIPTOR_SETS_CONFIG[SdfData].host_descriptor_number = 64;
    DESCRIPTOR_SETS_CONFIG[SdfData].is_texture = false;

    DESCRIPTOR_BINDINGS_CONFIG[20].vk_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DESCRIPTOR_BINDINGS_CONFIG[20].shader_stage_flag =
        VK_SHADER_STAGE_VERTEX_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[20].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[20].shader_descriptors_number = 1;
    DESCRIPTOR_BINDINGS_CONFIG[20].ubo_chunk_size = sizeof(SdfUbo);

    PIPELINE_CONFIGS[SdfPipeline].vert_shader =
        "../../../crates/glvm2/assets/shaders/sdf/sdf_vert.spv";
    PIPELINE_CONFIGS[SdfPipeline].frag_shader =
        "../../../crates/glvm2/assets/shaders/sdf/sdf_frag.spv";
    PIPELINE_CONFIGS[SdfPipeline].binding_description =
        Vertex::get_binding_description();
    PIPELINE_CONFIGS[SdfPipeline].attribute_descriptions =
        Vertex::get_attribute_descriptions();
    PIPELINE_CONFIGS[SdfPipeline].actual_linked_descriptor_sets_number = 1;

    RENDER_PASS_CONFIGS[SdfPipeline].actual_attachment_description_number = 2;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[0].flags = 0;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[0].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[0].loadOp =
        VK_ATTACHMENT_LOAD_OP_LOAD;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[0].storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[0].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[0].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[0].initialLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[0].finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[1].flags = 0;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[1].samples =
        VK_SAMPLE_COUNT_1_BIT;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[1].loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[1].storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[1].stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[1].stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[1].initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_descriptions[1].finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[SdfPipeline].actual_attachment_reference_number = 2;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_references[0].attachment = 0;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_references[0].layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[SdfPipeline].attachment_references[1].attachment = 1;
    RENDER_PASS_CONFIGS[SdfPipeline].attachment_references[1].layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    RENDER_PASS_CONFIGS[SdfPipeline].actual_subpass_dependency_number = 1;
    RENDER_PASS_CONFIGS[SdfPipeline].subpass_dependencies[0].srcSubpass = 0;
    RENDER_PASS_CONFIGS[SdfPipeline].subpass_dependencies[0].dstSubpass =
        VK_SUBPASS_EXTERNAL;
    RENDER_PASS_CONFIGS[SdfPipeline].subpass_dependencies[0].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[SdfPipeline].subpass_dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    RENDER_PASS_CONFIGS[SdfPipeline].subpass_dependencies[0].srcAccessMask = {};
    RENDER_PASS_CONFIGS[SdfPipeline].subpass_dependencies[0].dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    RENDER_PASS_CONFIGS[SdfPipeline].subpass_dependencies[0].dependencyFlags =
        {};

    // Not related to any pipeline descriptor sets and its bindings.
    DESCRIPTOR_SETS_CONFIG[RidableTextures]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[RidableTextures].host_descriptor_number =
        MAX_TEXTURES;
    DESCRIPTOR_SETS_CONFIG[RidableTextures].is_texture = true;

    DESCRIPTOR_BINDINGS_CONFIG[21].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[21].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[21].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[21].shader_descriptors_number = MAX_TEXTURES;
}
}; // namespace glvm

namespace glvm {
void descriptor_set_builder();
void pipeline_builder();
void render_passes_builder();
}; // namespace glvm

namespace glvm {
uint64_t make_entity(uint32_t id, uint32_t generation);
uint32_t get_id(uint64_t entity);
uint32_t get_gen(uint64_t entity);
auto matches_required_mask(
    const uint64_t archetype_mask,
    const uint64_t& system_mask
) -> bool;

template<typename T>
void unwrap_archetype(Archetype* arch, uint64_t mask, void (*func)(T*)) {
    switch (mask) {
        case PLAYER_COMPONENT_MASK:
            func(static_cast<PlayerArchetype*>(arch));
            break;
        case ENEMY_COMPONENT_MASK:
            func(static_cast<EnemyArchetype*>(arch));
            break;
    }
}
}; // namespace glvm

namespace glvm {
VkResult create_debug_utils_messenger_ext(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator,
    VkDebugUtilsMessengerEXT* p_debug_messenger
);
void create_begin_debug_utils_label_ext(
    VkInstance instance,
    VkCommandBuffer command_buffer,
    const VkDebugUtilsLabelEXT* label_info
);
void create_end_debug_utils_label_ext(
    VkInstance instance,
    VkCommandBuffer command_buffer
);
void destroy_debug_utils_messenger_ext(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* p_allocator
);
VkResult set_debug_object_name(
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* object_name_info
);
void set_image_debug_object_name(
    VkDevice device,
    GpuImage image,
    std::string image_name
);
void set_pipeline_debug_object_name(
    VkDevice device,
    VkPipeline pipeline,
    std::string pipeline_name
);
void set_descriptor_set_object_name(
    VkDevice device,
    VkDescriptorSet descriptor_set,
    std::string descriptor_set_name,
    unsigned int index
);
void set_debug_object_names(
    VkDevice device,
    const std::vector<VkBuffer>& vertex_buffer_container,
    const std::vector<VkBuffer>& index_buffer_container,
    const std::vector<Descriptor>& gpu_descriptors,
    const std::vector<unsigned int>& font_indices_container,
    const std::vector<VkBuffer>& font_vertex_buffer_container,
    const std::vector<VkBuffer>& font_index_buffer_container
);
}; // namespace glvm

namespace glvm {
struct GridChunk {
    Vector<float, 3> position;
    static constexpr float SIZE = 32;
    std::vector<uint32_t> entities;
};

struct SpatialGrid {
    static const uint32_t width = 8;
    static const uint32_t height = 8;
    static const uint32_t depth = 8;
    GridChunk grid[width][height][depth];
};

struct World {
    World();
    ~World();

    SpatialGrid spatial_grid;
    std::vector<Archetype*> archetypes;
    std::vector<EntityLocation> entity_locations;

    void add_entity_to_archetype(uint64_t entity, Archetype* arch);
    void remove_entity(uint64_t entity);
    void search_cache_archetypes(
        uint64_t required_mask,
        Archetype* cached_archetypes[],
        uint32_t& cached_archetypes_number
    );
};

extern World WORLD;
}; // namespace glvm

namespace glvm {

struct ArchetypeEntityManager {
    inline static uint32_t next_id = 0;
    std::vector<uint32_t> generations;
    std::vector<uint32_t> free_list;

    ArchetypeEntityManager();
    static ArchetypeEntityManager* get_instance();

    uint64_t create_entity();
    void remove_entity(uint64_t entity);
    bool is_alive(uint64_t entity) const;

private:
    static ArchetypeEntityManager* p_instance;
    static std::mutex mutex;

    ~ArchetypeEntityManager();
};
}; // namespace glvm

namespace glvm {
class InventorySystem: public ISystem {
public:
    uint32_t crosshair_archetypes_number = 0;
    uint32_t inventory_archetypes_number = 0;

    struct ArchView {
        Archetype* crosshair_cached_archetype = nullptr;
        Archetype* inventory_cached_archetype = nullptr;
    } arch_view;

    struct ComponentsView {
        Transform* crosshair_transforms_view = nullptr;

        Transform* inventory_transforms_view = nullptr;
        Inventory* inventory_view = nullptr;
        Mesh* inventory_meshes_view = nullptr;
    } components_view;

    uint64_t crosshair_required_mask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::CrosshairTagComponent);

    uint64_t inventory_required_mask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::InventoryComponent)
        | (1ull << ComponentsIndices::MeshComponent);

    void update() override;
    int determine_swappable_status_and_slots(
        Item* item_component,
        Transform* inventory_transform_component,
        std::vector<unsigned int>& potential_occupied_slots,
        Transform* crosshair_transform_component,
        Point2D<int> intersection_slot,
        Inventory* inventory_component,
        const float inventory_slot_scale
    );
    void fill_inventory_slots(
        Item* item_component,
        const int item_width,
        const int item_height,
        Inventory* inventory_component,
        const int fill_value
    );
    int determine_swappable_field(
        Item* item_component,
        const int item_width,
        const int item_height,
        int pivot_row,
        int pivot_column,
        Inventory* inventory_component,
        std::vector<unsigned int>& potential_occupied_slots
    );
    int calculate_basic_offset(
        const int item_axis_size,
        const float axis_value,
        const float crosshair_axis_position,
        const int axis_slot_index,
        const float inventory_slot_scale
    );
    bool check_crosshair_inventory_intersection(
        Transform* crosshair_transform_component,
        Transform* inventory_transform_component,
        Inventory* inventory_component,
        const float inventory_slot_scale,
        const float inventory_slot_half_scale
    );
    Point2D<int> determine_actual_intersection_slot(
        Transform* crosshair_transform_component,
        Transform* inventory_transform_component,
        const float inventory_slot_scale,
        const float inventory_slot_half_scale
    );

    bool is_inventory_opened;
    int* is_item_draged;
    bool* is_left_mouse_button_released;
    bool is_left_mouse_button_pressed;
    float mouse_offset_x = 0;
    float mouse_offset_y = 0;
    // Window aspect ratio, set by engine each frame.
    float aspect_rate = 0.0f;
    Archetype* cached_crosshair_archetype;
    Archetype* cached_inventory_archetype;
};
} // namespace glvm

#ifdef __linux__
// #define VK_USE_PLATFORM_XLIB_KHR
// #define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR // NOLINT(readability-identifier-naming)
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
const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    "VK_KHR_shader_non_semantic_info"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete() {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

class CVulkanRenderer {
public:
    bool print = true;
    Vector<int, 4> indirect_texture
        [INDIRECT_TEXTURE_WIDTH * INDIRECT_TEXTURE_HEIGHT / 4 + 1];
    std::vector<unsigned int> entities_collection_linked_trn_mat_mes_act;
    std::vector<unsigned int> entities_collection_linked_trn_po_l_mes_act;

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
    std::chrono::steady_clock::time_point start_time;

    std::vector<Texture> initialize_texture_data;
    std::vector<const char*> paths_array;
    std::vector<const char*> paths_gltf;
    std::vector<std::vector<Vertex>> level_generated_vertices;
    std::vector<std::vector<uint32_t>> level_generated_indices;

    std::vector<std::vector<Vertex>> a_vertices;
    // Wavefront .obj indices.
    std::vector<std::vector<uint32_t>> a_indices;
    // GLTF indices.
    std::vector<std::vector<float>> a_vertexes_temp;
    // highest GLTF y.
    std::vector<float> highest_gltf_y;
    // Keep axis limiting values for every axis per mesh in current iteration
    // while initializing Wavefront .obj and GLTF.
    MeshAxisLimitingValues mesh_axis_limiting_values;
    std::vector<std::vector<std::vector<Matrix<float, 4>>>>
        joint_matrices_per_mesh;
    std::vector<std::vector<float>> frames;
    bool is_inventory_opened = false;
    bool is_cursor_released = false;
    Vector<float, 3> forward = {0.0f, 0.0f, -1.0f};
    float hud_screen_x = 0.0f;
    float hud_screen_y;

    unsigned int entities[32];
    std::vector<RenderActor> actors;
    std::vector<RenderDirectionalLight> directional_lights;
    std::vector<RenderSpotLight> spot_lights;
    std::vector<RenderPointLight> point_lights;
    std::vector<RenderHealth> health_bars;
    std::vector<RenderFont> fonts;
    std::vector<RenderInventory> inventories;
    std::vector<RenderItem> items;
    std::vector<RenderCrosshair> crosshairs;
    std::vector<RenderPlayer> players;
    RenderPlayer player;

    float f_yaw = -90.0f;
    float f_pitch = 0.0f;
    float prev_y = 0.0f;
    float current_y = 0.0f;
    float prev_x = 0.0f;
    float current_x = 0.0f;
    // Window aspect ratio, updated on resize.
    float aspect_rate = 0.0f;
    int dragged_item_entity;

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    glvm::WindowWaylandVulkan* window;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    glvm::WindowXCBVulkan* window = nullptr;
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
    glvm::WindowXVulkan* window;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    glvm::WindowWinVulkan* window;
#endif

    ImGuiOverlay* imgui_overlay = nullptr;

    CVulkanRenderer();
    ~CVulkanRenderer();

    void create_texture_image();
    void recreate_swap_chain();
    void draw();
    void set_mesh_data(
        std::vector<const char*> paths,
        std::vector<const char*> paths_gltf
    );
    void set_projection_matrix(Matrix<float, 4> new_projection_matrix);
    void set_view_matrix(Matrix<float, 4> new_view_matrix);
    void initialize_game_level_vertices();
    void run();

public:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    Matrix<float, 4> view_matrix;
    Matrix<float, 4> projection_matrix;
    ThreadPool* render_thread_pool;

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    VkWaylandSurfaceCreateInfoKHR create_wayland_surface_info;
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
    VkXlibSurfaceCreateInfoKHR createXlibSurfaceInfo;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    VkXcbSurfaceCreateInfoKHR createXcbSurfaceInfo;
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkWin32SurfaceCreateInfoKHR create_win32_surface_info;
#endif

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSwapchainKHR swap_chain;
    std::vector<VkImage> swap_chain_images;
    VkFormat swap_chain_image_format;
    VkExtent2D swap_chain_extent;
    std::vector<VkImageView> swap_chain_image_views;
    std::vector<VkFramebuffer> swap_chain_framebuffers;

    VkBuffer hud_uniform_buffer;
    VkDeviceMemory hud_uniform_buffers_memory;
    VkBuffer font_uniform_buffer;
    VkDeviceMemory font_uniform_buffers_memory;
    VkBuffer hud_screen_uniform_buffer;
    VkDeviceMemory hud_screen_uniform_buffers_memory;
    VkBuffer ui_uniform_buffer;
    VkDeviceMemory ui_uniform_buffers_memory;
    VkBuffer ui_icons_uniform_buffer;
    VkDeviceMemory ui_icons_uniform_buffers_memory;
    std::vector<VkDescriptorSet> virtual_textures_ubo_desctiptor_sets;
    std::vector<VkDescriptorSet> virtual_textures_samplers_desctiptor_sets;
    VkBuffer virtual_textures_uniform_buffer;
    VkDeviceMemory virtual_textures_uniform_buffer_memory;

    VkCommandPool directional_light_command_pool;
    VkCommandPool spot_light_command_pool;
    VkCommandPool point_light_command_pool;
    VkCommandPool font_command_pool;
    VkCommandPool hud_command_pool;
    VkCommandPool hud_screen_command_pool;
    VkCommandPool ui_command_pool;
    VkCommandPool ui_icons_command_pool;
    VkCommandPool main_render_command_pool;
    std::vector<VkCommandPool> secondary_buffers_command_pools;
    VkCommandPool virtual_textures_command_pool;

    // Main pipeline depth.
    VkImage main_depth_pipeline_image;
    VkDeviceMemory main_depth_pipeline_image_memory;
    VkImageView main_depth_image_view;

    // Depth variables for shadow map.
public:
    unsigned int directional_light_number = 0;
    std::vector<VkFramebuffer> directional_light_shadow_map_frame_buffers;
    VkBuffer shadow_map_directional_light_model_matrix_uniform_buffer;
    VkDeviceMemory
        shadow_map_directional_light_model_matrix_uniform_buffers_memory;
    std::vector<GpuImage> directional_light_texture_images;

    Matrix<float, 4> dir_light_space_matrix[DIRECTIONAL_LIGHTS_NUMBER];
    Matrix<float, 4> spot_light_space_matrix[SPOT_LIGHTS_NUMBER];

    unsigned int point_light_number = 0;
    std::vector<std::vector<VkFramebuffer>> point_light_shadow_map_frame_buffers;
    VkBuffer shadow_map_point_light_model_matrix_uniform_buffer;
    VkDeviceMemory shadow_map_point_light_model_matrix_uniform_buffers_memory;
    std::vector<GpuImage> point_light_texture_images;

    unsigned int spot_light_number = 0;
    std::vector<VkFramebuffer> spot_light_shadow_map_frame_buffers;
    VkBuffer shadow_map_spot_light_model_matrix_uniform_buffer;
    VkDeviceMemory shadow_map_spot_light_model_matrix_uniform_buffers_memory;
    std::vector<GpuImage> spot_light_texture_images;

    std::vector<GpuImage> texture_images;
    VkSampler texture_sampler;
    VkSampler shadow_map_sampler;
    uint32_t frame_counter = 0;

    std::vector<VkBuffer> vertex_buffer_container;
    std::vector<VkDeviceMemory> vertex_buffer_memory_container;
    std::vector<VkBuffer> index_buffer_container;
    std::vector<VkDeviceMemory> index_buffer_memory_contaner;
    uint32_t wavefront_obj_counter = 0;
    uint32_t gltf_counter = 0;

    std::vector<std::vector<Vertex>> symbol_g_vertices_container;
    std::vector<unsigned int> font_indices_container;
    std::vector<VkBuffer> font_vertex_buffer_container;
    std::vector<VkDeviceMemory> font_vertex_buffer_memory_container;
    std::vector<VkBuffer> font_index_buffer_container;
    std::vector<VkDeviceMemory> font_index_buffer_memory_contaner;

    VkBuffer model_matrix_uniform_buffer;
    VkDeviceMemory model_matrix_uniform_buffers_memory;
    VkBuffer light_data_uniform_buffer;
    VkDeviceMemory light_data_uniform_buffers_memory;

    VkDescriptorPool descriptor_pool;
    const unsigned int matrix_ubo_descriptors_number = 500;
    const unsigned int hud_ubo_descriptor_number = 500;
    const unsigned int font_ubo_descriptor_number = 128;
    const unsigned int hud_screen_ubo_descriptor_number = 32;
    const unsigned int ui_ubo_descriptors_number = 64;
    const unsigned int virtual_textures_descriptors_number = 64;
    std::vector<VkCommandBuffer> directional_light_command_buffers;
    std::vector<VkCommandBuffer> spot_light_command_buffers;
    std::vector<VkCommandBuffer> point_light_command_buffers;
    std::vector<VkCommandBuffer> font_command_buffers;
    std::vector<VkCommandBuffer> hud_command_buffers;
    std::vector<VkCommandBuffer> main_render_command_buffers;
    std::vector<VkCommandBuffer> directional_light_secondary_command_buffers;
    std::vector<VkCommandBuffer> spot_light_secondary_command_buffers;
    std::vector<VkCommandBuffer> point_light_secondary_command_buffers;
    std::vector<VkCommandBuffer> virtual_textures_command_buffers;

    // Main render pipeline sync objects.
    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> in_flight_fences;

    // Hud render pipeline sync objects.
    std::vector<VkSemaphore> hud_image_available_semaphores;
    std::vector<VkSemaphore> hud_render_finished_semaphores;
    std::vector<VkFence> hud_in_flight_fences;

    // Font render pipeline sync objects.
    std::vector<VkSemaphore> font_image_available_semaphores;
    std::vector<VkSemaphore> font_render_finished_semaphores;
    std::vector<VkFence> font_in_flight_fences;

    // Directional light shadow map sync objects.
    std::vector<VkSemaphore>
        directional_light_shadow_map_image_available_semaphores;
    std::vector<VkSemaphore>
        directional_light_shadow_map_render_finished_semaphores;
    std::vector<VkFence> directional_light_shadow_map_in_flight_fences;

    // Spotlight shadow map sync objects.
    std::vector<VkSemaphore> spot_light_shadow_map_image_available_semaphores;
    std::vector<VkSemaphore> spot_light_shadow_map_render_finished_semaphores;
    std::vector<VkFence> spot_light_shadow_map_in_flight_fences;

    // Point light shadow map sync objects.
    std::vector<VkSemaphore> point_light_shadow_map_image_available_semaphores;
    std::vector<VkSemaphore> point_light_shadow_map_render_finished_semaphores;
    std::vector<VkFence> point_light_shadow_map_in_flight_fences;

    // Virtual textures pipeline sync objects.
    std::vector<VkSemaphore> virtual_textures_image_available_semaphores;
    std::vector<VkSemaphore> virtual_textures_render_finished_semaphores;
    std::vector<VkFence> virtual_textures_in_flight_fences;

    uint32_t current_frame = 0;
    uint32_t directional_light_current_frame = 0;
    uint32_t spot_light_current_frame = 0;
    uint32_t point_light_current_frame = 0;

    std::mutex mutex0;
    std::mutex mutex1;
    std::mutex mutex2;
    std::mutex shadow_map_passes_mutex;

    bool framebuffer_resized = false;

    void init_window();
    void init_vulkan();
    void initialize_vertex_buffers_with_wavefront_data();
    void initialize_vertex_buffers_with_gltf_data();
    void initialize_vertex_buffers_with_font_data();
    void cleanup_swap_chain();
    void cleanup();
    void create_instance();
    void populate_debug_messenger_create_info(
        VkDebugUtilsMessengerCreateInfoEXT& create_info
    );
    void setup_debug_messenger();
    void create_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swap_chain();
    void create_image_views();
    void create_main_render_pass();
    void create_descriptor_set_layout();
    void create_graphics_pipeline();
    void create_render_pass_framebuffers(
        std::vector<VkImageView>& attachments,
        VkRenderPass& render_pass,
        VkFramebuffer& swap_chain_framebuffer,
        uint32_t width,
        uint32_t height
    );
    void create_framebuffers();
    void create_command_pool(VkCommandPool& command_pool);
    void create_depth_resources();
    void create_directional_light_shadow_map_depth_resources();
    void create_spot_light_shadow_map_depth_resources();
    void create_point_light_shadow_map_depth_resources();
    VkFormat find_supported_format(
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features
    );
    VkFormat find_depth_format();
    bool has_stencil_component(VkFormat format);
    void create_texture_image_view();
    void create_texture_sampler();
    void create_shadow_map_sampler();
    VkImageView create_image_view(
        GpuImage image,
        uint32_t base_array_layers,
        uint32_t layer_count
    );
    void create_image(GpuImage& image);
    void transition_image_layout(
        VkImage image,
        VkImageLayout old_layout,
        VkImageLayout new_layout
    );
    void transition_shadow_map_image_layout(
        VkImage image,
        VkImageLayout old_layout,
        VkImageLayout new_layout
    );
    void copy_buffer_to_image(
        VkBuffer& buffer,
        VkImage image,
        uint32_t width,
        uint32_t height
    );
    void create_vertex_buffer(
        VkBuffer& dst_vertex_buffer,
        VkDeviceMemory& dst_vertex_buffer_memory,
        std::vector<Vertex>& vertex_data
    );
    void create_index_buffer(
        VkBuffer& dst_index_buffer,
        VkDeviceMemory& dst_index_buffer_memory,
        const std::vector<uint32_t>& index_data
    );
    void create_main_render_uniform_buffers();
    void create_main_render_descriptor_pool();
    void allocate_descriptor_sets(
        std::vector<VkDescriptorSet>& descriptor_sets,
        VkDescriptorSetLayout set_layout,
        const unsigned int descriptor_sets_number,
        const unsigned int descriptor_offset
    );
    void update_descriptor_sets_ubo(
        VkBuffer ubo,
        const VkDeviceSize& ubo_struct_size,
        const unsigned int& ubo_descriptors_number,
        int ubo_binding,
        std::vector<VkDescriptorSet>& ubo_descriptor_sets,
        const unsigned int offset
    );
    void update_light_data_descriptor_sets(
        const DescriptorSet& current_descriptor_set1
    );
    void update_descriptor_sets_combined_image_sampler(
        const DescriptorSet& descriptor_set
    );
    void create_descriptor_image_info(
        const unsigned int descriptor_number,
        VkImageLayout image_layout,
        std::vector<GpuImage>& texture_images,
        const unsigned int image_view_index,
        VkDescriptorImageInfo descriptor_image_infos[]
    );
    VkDescriptorBufferInfo create_descriptor_buffer_info(
        VkBuffer ubo,
        uint32_t offset,
        uint32_t range
    );
    void create_main_render_descriptor_sets();
    void create_buffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& buffer_memory
    );
    VkCommandBuffer begin_single_time_commands(VkCommandPool& command_pool);
    void end_single_time_commands(
        VkCommandPool& command_pool,
        VkCommandBuffer& command_buffer
    );
    void copy_buffer(
        VkBuffer& src_buffer,
        VkBuffer& dst_buffer,
        VkDeviceSize size
    );
    uint32_t find_memory_type(
        uint32_t type_filter,
        VkMemoryPropertyFlags properties
    );
    void create_command_buffers(
        VkCommandPool& command_pool,
        std::vector<VkCommandBuffer>& command_buffers,
        uint32_t command_buffers_number,
        VkCommandBufferLevel command_buffer_level_flag
    );
    void execute_secondary_command_buffer(
        VkRenderPass render_pass,
        VkFramebuffer frame_buffer,
        VkExtent2D extent,
        VkCommandBuffer primary_command_buffer,
        VkCommandBuffer secondary_command_buffer
    );
    void update_hud_ubo(
        uint32_t offset,
        bool is_hud_exists,
        float highest_y,
        uint32_t health_counter
    );
    void update_hud_screen_ubo(uint32_t offset, uint32_t crosshair);
    void update_sdf_ubo(uint32_t offset, uint32_t crosshair);
    void update_ubo_ui(
        const unsigned int current_inventory_row,
        const unsigned int current_inventory_column,
        const unsigned int inventory,
        uint32_t offset
    );
    void update_ubo_icons_ui(uint32_t offset, uint32_t item);
    void hud_record_command_buffer(
        VkCommandBuffer& command_buffer,
        uint32_t image_index
    );
    void ui_record_command_buffer(
        VkCommandBuffer& command_buffer,
        uint32_t image_index
    );
    void ui_icons_record_command_buffer(
        VkCommandBuffer& command_buffer,
        uint32_t image_index
    );
    void hud_screen_record_command_buffer(
        VkCommandBuffer& command_buffer,
        uint32_t image_index
    );
    void sdf_record_command_buffer(
        VkCommandBuffer& command_buffer,
        uint32_t image_index
    );
    void font_record_command_buffer(
        VkCommandBuffer& command_buffer,
        uint32_t image_index
    );
    void record_command_buffer(
        VkCommandBuffer& command_buffer,
        uint32_t image_index
    );
    void create_sync_objects(
        std::vector<VkSemaphore>& image_available_semaphores,
        std::vector<VkSemaphore>& render_finished_semaphores,
        std::vector<VkFence>& in_flight_fences
    );
    void update_directional_light_shadow_map_matrix_ubo(
        uint32_t current_image,
        uint32_t current_light,
        unsigned int actor
    );
    void update_spot_light_shadow_map_matrix_ubo(
        uint32_t current_image,
        uint32_t current_light,
        unsigned int actor
    );
    void update_point_light_shadow_map_matrix_ubo(
        uint32_t current_image,
        uint32_t current_light,
        uint32_t layer,
        unsigned int actor
    );
    void update_matrix_uniform_buffer(uint32_t offset, unsigned int actor);
    void update_view_position_uniform_buffer(
        uint32_t current_image,
        uint32_t player
    );
    void main_render_draw_frame();
    void directional_light_shadow_map_draw_frame();
    void spot_light_shadow_map_draw_frame();
    void point_light_shadow_map_draw_frame();
    void directional_light_record_coomand_buffer(
        std::vector<VkCommandBuffer>& command_buffer,
        uint32_t current_frame
    );
    void spot_light_record_command_buffer(
        std::vector<VkCommandBuffer>& command_buffer,
        uint32_t current_frame
    );
    void point_light_record_command_buffer(
        std::vector<VkCommandBuffer>& command_buffers,
        uint32_t current_frame
    );
    VkShaderModule create_shader_module(const std::vector<char>& code);
    VkSurfaceFormatKHR choose_swap_surface_format(
        const std::vector<VkSurfaceFormatKHR>& available_formats
    );
    VkPresentModeKHR choose_swap_present_mode(
        const std::vector<VkPresentModeKHR>& available_present_modes
    );
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);
    bool is_device_suitable(VkPhysicalDevice device);
    bool check_device_extension_support(VkPhysicalDevice device);
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
    std::vector<const char*> get_required_extensions();
    bool check_validation_layer_support();
    VkDescriptorBufferInfo create_descriptor_buffer_info(
        VkBuffer ubo,
        const VkDeviceSize& ubo_struct_size,
        const VkDeviceSize& offset_step
    );
    VkDescriptorImageInfo create_descriptor_image_info(
        const GpuImage& texture_image,
        VkImageLayout layout,
        unsigned int texture_index,
        VkSampler texture_sampler
    );
    static std::vector<char> read_file(const std::string& filename);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void* p_user_data
    );
    Matrix<float, 4> compute_model_matrix(Transform* transform);
    void clear_vk_image(GpuImage* texture_images);
};

}; // namespace glvm

namespace glvm {
struct ItemSystem: public ISystem {
    uint32_t inventory_archetypes_number = 0;
    uint32_t item_archetypes_number = 0;
    uint32_t crosshair_archetypes_number = 0;

    struct ArchView {
        Archetype* inventory_cached_archetype = nullptr;
        Archetype* item_archetype = nullptr;
        Archetype* crosshair_archetype = nullptr;
    } arch_view;

    struct ComponentsView {
        Inventory* inventories_view = nullptr;

        Item* items_view = nullptr;
        Collider* item_colliders_view = nullptr;
        Transform* item_transforms_view = nullptr;

        Transform* crosshair_transforms = nullptr;
    } components_view;

    uint64_t inventory_required_mask =
        (1ull << ComponentsIndices::InventoryComponent);

    uint64_t item_required_mask = (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ItemComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent);

    uint64_t crosshair_required_mask =
        (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::CrosshairTagComponent);

    void update();
    bool put_item2x2(Inventory* inventory_component, unsigned int item_entity);

    CStack* input_stack;
    bool is_inventory_opened;
    int* dragged_item_entity;
    bool* is_left_mouse_button_released;
    bool is_left_mouse_button_pressed;
    float mouse_offset_x = 0;
    float mouse_offset_y = 0;
};
} // namespace glvm

namespace glvm {
bool box_collider(
    const Vector<float, 3> backtracking_position,
    const Vector<float, 3> compared_position,
    const float backtracking_scale,
    const float compared_scale,
    const MeshAxisMaxAbsoluteValues& backtracking_mesh_axis_max_absolute_values,
    const MeshAxisMaxAbsoluteValues& compared_mesh_axis_max_absolute_values
);

std::vector<Vector<float, 3>> compute_box_corner_bound_points(
    const MeshAxisMaxAbsoluteValues entity_chunk_bounds,
    Vector<float, 3> entity_position,
    const float scale
);

template<typename T>
bool is_exist(const std::vector<T>& array, const T& element) {
    for (uint32_t i0 = 0; i0 < array.size(); ++i0) {
        if (element == array[i0]) {
            return true;
        }
    }

    return false;
}

void set_mesh_bounds(MeshAxisLimitingValues mesh_axis_limiting_values);
void create_projectile(
    const Vector<float, 3>& projectile_position,
    const Vector<float, 3>& projectile_forward,
    const MeshHandle& mesh_handle,
    const Material& material,
    const Damage& damage,
    const EntityLocation& projectile_location
);
}; // namespace glvm

namespace glvm {
class CMovementSystem: public ISystem {
public:
    float delta_frame_time;
    float gravity;
    CStack& input_stack;
    float prev_delta_x = 0.0f;
    float prev_x = 0.0f;
    float current_x = 0.0f;
    Vector<float, 3> prev_forward;

    uint32_t player_archetypes_number = 0;
    uint32_t rigid_body_contained_archetypes_number = 0;

    struct MovementArchView {
        Archetype* player_cached_archetype = nullptr;
        Archetype* rigid_body_contained_archetypes_cache[32];
    } arch_view;

    struct MovementComponentsView {
        Move* player_moves = nullptr;
        Beholder* player_views = nullptr;
        ColliderFlags* player_collider_flags = nullptr;
        RigidBody* player_rigid_body = nullptr;

        // Components related to archetypes contains rigid.
        Transform* transforms = nullptr;
        RigidBody* rigid_bodies = nullptr;
        Move* moves = nullptr;
        Item* items = nullptr;
    } components_view;

    uint64_t player_required_mask =
        (1ull << ComponentsIndices::PlayerTagComponent);
    uint64_t rigid_body_required_mask =
        (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RigidBodyComponent)
        | (1ul << ComponentsIndices::MoveComponent);

    CMovementSystem(CStack& input_stack);

    void update();
    Vector<float, 3> calculate_vector_rl(Beholder& beholder);
    Vector<float, 3> calculate_vector_fb(Beholder& beholder, CEvent& event);
};
} // namespace glvm

namespace glvm {
class ProceduralLevelGeneratingSystem: public ISystem {
public:
    unsigned int level_nubmer = 0;
    bool bredo_flag = false;
    unsigned int previous_half_x_rand = 0;
    unsigned int previous_half_z_rand = 0;
    Vector<float, 3> current_level_position = {5.0f, 0.0f, 15.0f};
    Vector<float, 3> transition_bridge_position = {0.0f, 0.0f, 0.0f};
    unsigned int next_level_transition_direction = 0;
    unsigned int previous_iteration_transition_bridge_direction = 0;

    uint32_t cached_level_chunk_arch_number = 0;
    uint32_t cached_player_arch_number = 0;

    struct ProceduralLevelArchView {
        Archetype* cached_level_chunk_arch = nullptr;
        Archetype* cached_player_arch = nullptr;
    } arch_view;

    struct ComponentsView {
        Transform* player_transforms = nullptr;
    } components_view;

    uint64_t player_required_mask =
        (1ull << ComponentsIndices::PlayerTagComponent);

    uint64_t required_mask = (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::MaterialComponent)
        | (1ull << ComponentsIndices::MeshComponent)
        | (1ull << ComponentsIndices::ColliderComponent)
        | (1ull << ComponentsIndices::ColliderFlagsComponent)
        | (1ull << ComponentsIndices::LevelChunkTagComponent);

    std::vector<MeshHandle> mesh_handlers;
    std::vector<TextureHandle> texture_handlers;

    std::vector<std::vector<Vertex>> level_generated_vertices;
    // Wavefront .obj indices.
    std::vector<std::vector<uint32_t>> level_generated_indices;
    // Keep axis limiting values for every axis per mesh in current iteration
    // while initializing Wavefront .obj and GLTF.
    MeshAxisLimitingValues mesh_axis_limiting_values;
    // Contains maximum coordinate value in every direction for all generated
    // levels.
    MeshAxisLimitingValues coordinate_maximum_value_per_direction;

    void update();
    void set_half_extents_from_direction(
        float& half_x,
        float& half_z,
        const float& transition_bridge_half_width,
        const float& transition_bridge_half_height,
        const float& next_level_transition_direction
    );
    void generate_level(
        const unsigned int level_half_x,
        const unsigned int level_half_y,
        const unsigned int level_half_z,
        const float transition_bridge_half_width,
        const float transition_bridge_half_height
    );
    void generate_transition_bridge(
        const unsigned int level_half_x,
        const unsigned int level_half_y,
        const unsigned int level_half_z,
        const float transition_bridge_half_width,
        const float transition_bridge_half_height
    );
    void make_cube_object_vertices(
        Vector<float, 4> join_indices,
        Vector<float, 4> weights,
        float half_x,
        float half_y,
        float half_z,
        std::vector<Vertex>& destination_vertices_container
    );
    bool check_collision_intersection_with_maximum_coordinates(
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
    float f_delta_time;
    float gravity;
    bool is_inventory_opened;
    bool* is_item_draged;
    bool is_left_mouse_button_pressed;
    bool* is_left_mouse_button_released;
    CStack& input_stack;
    Archetype* cached_archetypes[32];
    uint32_t cached_archetypes_number = 0;

    struct CollisionComponentsView {
        Transform* backtracking_transforms = nullptr;
        Collider* backtracking_colliders = nullptr;
        ColliderFlags* backtracking_collider_flags = nullptr;
        Mesh* backtracking_meshes = nullptr;
        Move* backtracking_move = nullptr;
        Transform* compared_transforms = nullptr;
        Mesh* compared_meshes = nullptr;
        Move* compared_move = nullptr;
    } view;

    uint64_t required_mask = (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);

    CCollisionSystem(CStack& stack) : input_stack(stack) {
    }

    void update() override;
    bool upper_actor_check(
        Vector<float, 3> backtracking_position,
        Vector<float, 3> compared_position,
        float backtracking_scale,
        float compared_scale,
        MeshHandle backtracking_mesh_handle,
        MeshHandle compared_mesh_handle
    );
};
} // namespace glvm

namespace glvm {
class EnemySystem: public ISystem {
public:
    uint32_t player_archetypes_number = 0;
    uint32_t enemy_archetypes_number = 0;
    uint32_t projectile_archetypes_number = 0;

    struct ArchView {
        Archetype* player_cached_archetype = nullptr;
        Archetype* enemy_cached_archetype = nullptr;
        Archetype* projectile_archetype = nullptr;
    } arch_view;

    struct ComponentsView {
        Transform* player_transforms = nullptr;

        Transform* enemy_transforms = nullptr;
        State* enemy_states = nullptr;
        Enemy* enemies = nullptr;
    } components_view;

    uint64_t player_required_mask =
        (1ull << ComponentsIndices::PlayerTagComponent);

    uint64_t enemy_required_mask =
        (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::StateComponent)
        | (1ul << ComponentsIndices::EnemyComponent);

    uint64_t projectile_required_mask =
        (1ull << ComponentsIndices::ProjectileTagComponent);

    void update() override;
    ISoundEngine* sound_engine;
    std::vector<TextureHandle> texture_handlers;
    std::vector<MeshHandle> mesh_handlers;
    float projectile_cooldown = 5.0f;
    float delta_frame_time;
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
    float f_yaw = -90.0f;
    float f_pitch = 0.0f;
    bool b_first_mouse = true;
    CStack& input_stack;
    std::vector<TextureHandle> texture_handlers;
    std::vector<MeshHandle> mesh_handlers;
    ISoundEngine* sound_engine;
    float projectile_cooldown = 2.0f;
    float delta_frame_time;
    bool is_inventory_opened;

    uint32_t player_archetypes_number = 0;
    uint32_t projectile_archetypes_number = 0;

    struct ArchView {
        Archetype* player_cached_archetype = nullptr;
        Archetype* projectile_archetype = nullptr;
    } arch_view;

    struct ComponentsView {
        Transform* player_transforms = nullptr;
        Beholder* player_views = nullptr;

        Transform* projectile_transforms = nullptr;
        ColliderFlags* projectile_collider_flags = nullptr;
        Collider* projectile_colliders = nullptr;
        ProjectileBundle* projectile_bundles = nullptr;
        Health* projectile_health = nullptr;
        Attack* projectile_attacks = nullptr;
    } components_view;

    uint64_t player_required_mask =
        (1ull << ComponentsIndices::PlayerTagComponent);

    uint64_t projectile_required_mask =
        (1ull << ComponentsIndices::ProjectileTagComponent);

    CProjectileSystem(CStack& input_stack);
    void update() override;
    template<typename T>
        requires UnitOrEnemy<T> && HasAttack<T>
    static void mark_as_attacked(
        T* arch,
        Damage* projectile_damage,
        uint32_t entity_index
    );
};

template<typename T>
    requires UnitOrEnemy<T> && HasAttack<T>
void CProjectileSystem::mark_as_attacked(
    T* arch,
    Damage* projectile_damage,
    uint32_t enitity_index
) {
    arch->attacks[enitity_index].damage = projectile_damage->maximum_damage;
}

} // namespace glvm

namespace glvm {

class SpatialGridSystem: public ISystem {
    Archetype* cached_archetypes[32];
    uint32_t cached_archetypes_number = 0;
    bool is_initialized = false;

    struct SpatialGridComponentsView {
        Transform* transforms = nullptr;
        Mesh* meshes = nullptr;
    } view;

    uint64_t required_mask = (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);

    void update() override;
};

}; // namespace glvm

namespace glvm {
enum RendererType { VulkanRenderer };

class Engine {
    static Engine* p_instance;
    static std::mutex mutex;

    IChrono* chrono;
    ISoundEngine* sound_engine;
    std::thread sound_thread;
    std::atomic<bool> running_sound {false};
    float delta_frame_time;
    float gravity;
    bool is_left_mouse_button_pressed;
    std::vector<Texture> texture_vector;
    std::vector<const char*> paths_array;
    std::vector<const char*> paths_gltf;
    uint32_t mesh_id = 0;
    bool is_already_cached;
    bool is_inventory_key_held = false;
    bool was_inventory_opened = false;
    bool is_cursor_hidden = false;
    float hud_screen_x = 0.0f;
    float hud_screen_y;
    // If don't have any dragged item then this variable have value of -1.
    int dragged_item_entity = -1;
    float f_yaw = -90.0f;
    float f_pitch = 0.0f;
    float previous_mouse_offset_x = 0.0f;
    float previous_mouse_offset_y = 0.0f;
    CVulkanRenderer* vulkan_renderer;
    SpatialGridSystem* spatial_grid_system;
    CCollisionSystem* collision_system;
    CMovementSystem* movement_system;
    CPhysicsSystem* physics_system;
    CProjectileSystem* projectile_system;
    DamageSystem* damage_system;
    EnemySystem* enemy_sytem;
    ItemSystem* item_system;
    ProceduralLevelGeneratingSystem* procudural_level_generating_system;
    InventorySystem* inventory_system;
    Archetype* cached_directional_ligth_archetypes[32];
    uint32_t directional_light_archetypes_number = 0;
    uint64_t directional_light_required_mask =
        (1ul << ComponentsIndices::DirectionalLightComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cached_spot_ligth_archetypes[32];
    uint32_t spot_light_archetypes_number = 0;
    uint64_t spot_light_required_mask =
        (1ul << ComponentsIndices::SpotLightComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cached_point_ligth_archetypes[32];
    uint32_t point_light_archetypes_number = 0;
    uint64_t point_light_required_mask =
        (1ul << ComponentsIndices::PointLightComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cached_animation_actors_archetypes[32];
    uint32_t animation_actors_archetypes_number = 0;
    uint64_t animated_actors_required_mask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::AnimationComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cached_static_actors_archetypes[32];
    uint32_t static_actors_archetypes_number = 0;
    uint64_t static_actors_required_mask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::StaticMeshTagComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cached_player_archetypes[32];
    uint32_t player_archetypes_number = 0;
    uint64_t player_required_mask =
        (1ul << ComponentsIndices::PlayerTagComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ViewComponent);
    Archetype* cached_animation_archetypes[32];
    uint32_t animation_archetypes_number = 0;
    uint64_t animation_required_mask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::AnimationComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cached_crosshair_actors_archetypes[32];
    uint32_t crosshair_actors_archetypes_number = 0;
    uint64_t crosshair_required_mask =
        (1ul << ComponentsIndices::CrosshairTagComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cached_level_chunk_actors_archetypes[32];
    uint32_t level_chunk_actors_archetypes_number = 0;
    uint64_t level_chunk_required_mask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::LevelChunkTagComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cached_projectile_actors_archetypes[32];
    uint32_t projectile_actors_archetypes_number = 0;
    uint64_t projectile_required_mask =
        (1ul << ComponentsIndices::ProjectileBundleComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cached_item_actors_archetypes[32];
    uint32_t item_actors_archetypes_number = 0;
    uint64_t rotation_item_required_mask =
        (1ul << ComponentsIndices::ItemComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MaterialComponent);
    Archetype* cached_inventory_archetypes[32];
    uint32_t inventory_archetypes_number = 0;
    uint64_t inventory_required_mask =
        (1ul << ComponentsIndices::InventoryComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MaterialComponent);
    Archetype* cached_item_archetypes[32];
    uint32_t item_archetypes_number = 0;
    uint64_t item_required_mask = (1ul << ComponentsIndices::ItemComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::MaterialComponent);
    Archetype* cached_health_bars_archetypes[32];
    uint32_t health_bars_archetypes_number = 0;
    uint64_t health_bars_required_mask =
        (1ul << ComponentsIndices::HealthComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cached_fonts_archetypes[32];
    uint32_t fonts_archetypes_number = 0;
    uint64_t font_required_mask = (1ul << ComponentsIndices::FontComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    double fps_accumulator = 0;
    Engine();

public:
    std::vector<MeshHandle> mesh_handlers;
    std::vector<TextureHandle> texture_handlers;
    uint32_t wavefront_obj_counter = 0;

    ~Engine();

    // Don't need to make copy because of singleton property.
    Engine(Engine& other) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const Engine& other) = delete;
    // It possibly to get only one instance of this class with this method.
    static Engine* get_instance();
    void game_loop();
    void event_queue_flush();
    void render_vulkan();
    void enlarge_frame_accumulator(float value);
    void set_view_matrix();
    void set_projection_matrix();
    [[nodiscard]] std::vector<Matrix<float, 4>> update_animation_frames(
        Animation* animation_component,
        unsigned int mesh_id
    );
    Matrix<float, 4> update_directional_light_space_matrix_shadow_map_ubo(
        DirectionalLightComponent* light
    );
    Matrix<float, 4> update_spot_light_space_matrix_shadow_map_ubo(
        SpotLightComponent* light
    );
    Matrix<float, 4> update_point_light_space_matrix_shadow_map_ubo(
        PointLightComponent* light,
        uint32_t layer
    );
    SlotData update_data_ubo_ui(
        const unsigned int current_inventory_row,
        const unsigned int current_inventory_column,
        Inventory* inventory_component,
        Transform* slot_transfrom_component,
        Mesh* mesh_component
    );
    Matrix<float, 4> update_data_ubo_icons_ui(
        Transform* item_transfrom_component,
        Collider* item_collider_component,
        Item* item_component,
        const unsigned int row_inventory,
        const unsigned int column_inventory,
        Transform* inventory_transform_component,
        Mesh* item_mesh,
        int item_entity
    );
    Matrix<float, 4> update_data_hud_screen_ubo(Transform* cursor_transform);
    void set_frame_data();
    void load_wavefront_obj();
    void calculate_mesh_bounds(const Vector<float, 4>& animated_vertex);
    bool is_model_cache_exists(const std::string& model_file_path);
    void write_models_cache(const std::string& model_file_path);
    void initialize_gltf();
    void initialize_font_data();
    Matrix<float, 4> compute_model_matrix(
        Transform* transform,
        Rotation* rotation
    );
    void compute_hud_screeen_coordinates();
    TextureHandle load_texture_from_file(const char* path_to_texture_component);
    auto load_texture_from_address(
        unsigned int i_width,
        unsigned int i_height,
        unsigned int dat_length,
        unsigned char* u_i_data
    ) -> TextureHandle;
    MeshHandle load_mesh_from_obj(const char* mesh_path);
    MeshHandle load_mesh_from_gltf(const char* path_to_mesh);
    MeshHandle load_mesh();
    void game_kill();
};
} // namespace glvm
