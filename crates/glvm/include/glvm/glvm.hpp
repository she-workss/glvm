#pragma once

#include "rusty/prelude.hpp"

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

using namespace rusty::prelude;

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
constexpr auto NUMBER_OF_DRAWING_VERTICES = 36;
constexpr auto NUMBER_OF_MATRICES = 1;
constexpr auto PI = std::numbers::pi_v<f32>;
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
constexpr auto ENTITY_BITS_MASK = (1ull << ENTITY_ID_BITS) - 1;

struct ComponentsIndices {
    enum Types : u32 {
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

constexpr auto PLAYER_COMPONENT_MASK =
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

constexpr auto ENEMY_COMPONENT_MASK =
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

constexpr auto STATIC_MESH_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::FontComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::StaticMeshTagComponent);

constexpr auto CROSSHAIR_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::CrosshairTagComponent);

constexpr auto ITEM_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::RigidBodyComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::MoveComponent)
    | (1ull << ComponentsIndices::ItemComponent);

constexpr auto INVENTORY_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::InventoryComponent)
    | (1ull << ComponentsIndices::MaterialComponent);

constexpr auto DIRECTIONAL_LIGHT_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::DirectionalLightComponent);

constexpr auto SPOT_LIGHT_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::SpotLightComponent);

constexpr auto POINT_LIGHT_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::PointLightComponent);

constexpr auto LEVEL_CHUNK_COMPONENT_MASK =
    (1ull << ComponentsIndices::TransformComponent)
    | (1ull << ComponentsIndices::MaterialComponent)
    | (1ull << ComponentsIndices::MeshComponent)
    | (1ull << ComponentsIndices::ColliderComponent)
    | (1ull << ComponentsIndices::ColliderFlagsComponent)
    | (1ull << ComponentsIndices::RotationComponent)
    | (1ull << ComponentsIndices::LevelChunkTagComponent);

constexpr auto PROJECTILE_COMPONENT_MASK =
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
    u32 current_animation_frame = 0;
    f32 frame_accumulator = 0.0f;
};
} // namespace glvm

namespace glvm {}

namespace glvm {
struct Attack {
    f32 damage;
};
} // namespace glvm

namespace glvm {
struct Collider {
public:
    Vec<u32> colliders;
};
} // namespace glvm

namespace glvm {
struct ColliderFlags {
    // 0001 = wallCollision; 0010 = groundCollision; 0100 = roofCollision; 1000
    // = itemDragging.
    i32 flags : 4;
};
}; // namespace glvm

namespace glvm {
struct Controller {};

struct CameraComponent {};
} // namespace glvm

namespace glvm {
struct Crosshair {};
}; // namespace glvm

namespace glvm {
struct Damage {
    f32 maximum_damage;
    f32 minimum_damage;
    f32 critical_hit_rate;
    f32 critical_modifier;
};
} // namespace glvm

namespace glvm {
struct Enemy {
    f32 detect_radius;
};
} // namespace glvm

namespace glvm {
struct Font {
    Vec<char> font_string;
    f32 lifetime;
    bool removable;
};
} // namespace glvm

namespace glvm {
struct Health {
    f32 max_health;
    f32 current_health;
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
    u32 item_entity = UINT_MAX;
};
} // namespace glvm

namespace glvm {
struct ItemSlotType {
    u32 height;
    u32 width;
};

struct Item {
    // Array that contains entities with InventorySlotComponent.
    Vec<u32> occupied_slots;
    ItemSlotType item_slot_type;
    bool is_actor;
};
} // namespace glvm

namespace glvm {
struct Physics {
    f32 gravity_accumulator = 0.0f;
};
}; // namespace glvm

namespace glvm {
struct Projectile {
public:
    u32 owner;
    bool collision_status = false;
};
} // namespace glvm

namespace glvm {
struct Rotation {
    f32 yaw = 0.0f;
    f32 pitch = 0.0f;
};
}; // namespace glvm

namespace glvm {
struct TextureComponent {
    u32 id;
};
} // namespace glvm

namespace glvm {
struct MeshHandle {
    u32 id;
};

struct Mesh {
    MeshHandle handle;
    bool gltf = true;
};
} // namespace glvm

constexpr auto INVALID_ENTITY_ID = 4000000000;
constexpr i32 BOX_INDICES_FOR_INDEX_BUFFER[36] = {0, 1, 2, 3, 0, 2, 4, 0, 3,
                                                  7, 4, 3, 4, 5, 1, 0, 4, 1,
                                                  1, 5, 6, 2, 1, 6, 5, 4, 7,
                                                  6, 5, 7, 3, 2, 6, 7, 3, 6};

namespace glvm {

struct EventStack;

enum EventKind {
    Default,
    KeyReleaseA,
    KeyReleaseD,
    KeyReleaseS,
    KeyReleaseW,
    KeyReleaseJump,
    GravityCollisionFlag,
    Render,
    AttackEvent,
    Spawn,
    Jump,
    InventoryToggle,
    InventoryRelease,
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    MoveDiagonalFb,
    MoveDiagonalFl,
    MoveDiagonalLb,
    MoveDiagonalBr,
    MouseMoved,
    MouseLeftButtonRelease,
    MouseLeftButton,
    MouseRightButtonRelease,
    MouseRightButton,
    CursorReleased,
    GameLoopKill,
    Empty,
};

struct MousePointerPosition {
    i32 position_x;
    i32 position_y;
    i32 offset_x = 0;
    i32 offset_y = 0;
    f32 pitch;
    f32 yaw;
};

struct Event {
private:
    EventKind event;
    EventKind next_event;

public:
    MousePointerPosition mouse_pointer_position;
    bool next_event_flag = false;

    Event();
    auto get_event() -> EventKind&;
    auto set_event(EventKind new_event) -> void;
    auto set_next_event(EventKind new_event) -> void;
    auto get_next_event() -> EventKind;
    auto set_last_event(EventStack stack) -> void;

    bool is_left_mouse_button_released = true;
};

} // namespace glvm

namespace glvm {
struct Chrono {
public:
    virtual ~Chrono() {
    }

    virtual auto init_frequency() -> f64 = 0;
    virtual auto reset() -> f64 = 0;
    virtual auto get_elapsed() -> f64 = 0;
};
} // namespace glvm

namespace glvm {
struct Scalar {
    f32 value;
};

// Vector in 3D PGA.
struct Plane {
    // e1 basis vector.
    f32 x;
    // e2 basis vector.
    f32 y;
    // e3 basis vector.
    f32 z;
    // e0 projective plane in infinity.
    f32 w;
};

// Bivector.
struct Line {
    f32 rx;
    f32 ry;
    f32 rz;
    f32 ix;
    f32 iy;
    f32 iz;
};

struct RLine {
    f32 rx;
    f32 ry;
    f32 rz;
};

struct ILine {
    f32 ix;
    f32 iy;
    f32 iz;
};

// Trivector.
struct Point {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
};

struct PseudoScalar {
    f32 w;
};

struct Motor {
    f32 rx;
    f32 ry;
    f32 rz;
    // Scalar.
    f32 rw;
    f32 ix;
    f32 iy;
    f32 iz;
    // Pseudoscalar.
    f32 iw;
};

struct Rotor {
    f32 rx;
    f32 ry;
    f32 rz;
    f32 rw;
};

struct Translator {
    f32 ix;
    f32 iy;
    f32 iz;
    f32 iw;
};

inline auto operator-(Line line) -> Line {
    return {
        .rx = -line.rx,
        .ry = -line.ry,
        .rz = -line.rz,
        .ix = -line.ix,
        .iy = -line.iy,
        .iz = -line.iz
    };
}

inline auto operator-(Point point) -> Point {
    return {.x = -point.x, .y = -point.y, .z = -point.z, .w = -point.w};
}

// Dual operator.

inline auto operator!(const Plane& plane) -> Point {
    return Point {.x = plane.x, .y = plane.y, .z = plane.z, .w = plane.w};
}

inline auto operator!(const Point& point) -> Plane {
    return Plane {.x = point.x, .y = point.y, .z = point.z, .w = point.w};
}

inline auto operator!(const Line& line) -> Line {
    return {
        .rx = line.ix,
        .ry = line.iy,
        .rz = line.iz,
        .ix = line.rx,
        .iy = line.ry,
        .iz = line.rz
    };
}

inline auto operator!(const PseudoScalar& pseudo_scalar) -> Scalar {
    return Scalar {.value = pseudo_scalar.w};
}

inline auto operator!(const Scalar& scalar) -> PseudoScalar {
    return PseudoScalar {.w = scalar.value};
}

inline auto normalize(const Plane& plane) -> Plane {
    f32 length =
        std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
    assert(length != 0);
    return {
        .x = plane.x / length,
        .y = plane.y / length,
        .z = plane.z / length,
        .w = plane.w / length
    };
}

inline auto normalize(const Line& line) -> Line {
    f32 length =
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
inline auto operator~(const Plane& plane) -> Plane {
    return plane;
}

inline auto operator~(const Line& line) -> Line {
    return -line;
}

inline auto operator~(const Point& point) -> Point {
    return -point;
}

inline auto operator~(const Scalar& scalar) -> Scalar {
    return scalar;
}

inline auto operator~(const RLine& rline) -> RLine {
    return rline;
}

// Inner product.

// Scalar product of the plane normals.
inline auto operator|(const Plane& plane0, const Plane& plane1) -> f32 {
    return plane0.x * plane1.x + plane0.y * plane1.y + plane0.z * plane1.z;
}

// This gives the oriented distance from the point to the plane (if normalized).
inline auto operator|(const Plane& plane, const Point& point) -> Line {
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
inline auto operator|(const Plane& plane, const Line& line) -> Point {
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
inline auto operator|(const Line& line0, const Line& line1) -> f32 {
    return -line0.rx * line1.rx - line0.ry * line1.ry - line0.rz * line1.rz;
}

// A line through a point defines a plane. The form is similar to plane ⋅ line,
// but semantically it is a plane containing l and pt
inline auto operator|(const Line& line, const Point& point) -> Plane {
    return {
        .x = -line.rx * point.w,
        .y = -line.ry * point.w,
        .z = -line.rz * point.w,
        .w = line.rx * point.x + line.ry * point.y + line.rz * point.z
    };
}

// Points do not have an inner product: it is always zero (if strictly by
// definition).
inline auto operator|(const Point& point0, const Point& point1) -> Scalar {
    return {.value = -point0.w * point1.w};
}

// Outer product.

// plane ^ plane -> line (their intersection).
inline auto operator^(const Plane& plane0, const Plane& plane1) -> Line {
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

// plane ∧ point -> pseudoscalar (signed distance scaled by the plane norm).
inline auto operator^(const Plane& plane, const Point& point) -> PseudoScalar {
    return {
        .w = plane.x * point.x + plane.y * point.y + plane.z * point.z
            + plane.w * point.w
    };
}

// line ^ point -> f32 (stub, always zero).
inline auto operator^(const Line& line, const Point& point) -> f32 {
    return 0.0f;
}

// point ∧ point → f32 (stub, always zero).
inline auto operator^(const Point& point0, const Point& point1) -> f32 {
    return 0.0f;
}

// line ∧ line → point (if intersecting). If w == 0, then the lines do not
// intersect (the result is a point at infinity).
inline auto operator^(const Line& line0, const Line& line1) -> PseudoScalar {
    // e0 ^ (e1 ^ (e2 ^ e3)).
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
inline auto operator^(const Plane& plane, const Line& line) -> Point {
    return {
        .x = plane.y * line.iz - plane.z * line.iy - plane.w * line.rx,
        .y = -plane.x * line.iz + plane.z * line.ix - plane.w * line.ry,
        .z = plane.x * line.iy - plane.y * line.ix - plane.w * line.rz,
        .w = plane.x * line.rx + plane.y * line.ry + plane.z * line.rz
    };
}

inline auto operator^(const Line& line, const Plane& plane) -> Point {
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
inline auto operator&(Plane plane0, Plane plane1) -> f32 {
    return 0.0f;
}

inline auto operator&(Plane plane, Point point) -> Scalar {
    // Plane linked with dual point from outer product and point linked
    // with dual plane from outer product.
    return {
        .value = -plane.x * point.x + -plane.y * point.y + -plane.z * point.z
            + -plane.w * point.w
    };
}

inline auto operator&(Point point, Plane plane) -> Scalar {
    // Plane linked with dual point from outer product and point linked
    // with dual plane from outer product.
    return {
        .value = plane.x * point.x + plane.y * point.y + plane.z * point.z
            + plane.w * point.w
    };
}

inline auto operator&(Point point, Line line) -> Plane {
    // Point linked with dual plane from outer product and line linked
    // with dual line from outer product.
    return {
        .x = point.y * line.rz - point.z * line.ry - point.w * line.ix,
        .y = -point.x * line.rz + point.z * line.rx - point.w * line.iy,
        .z = point.x * line.ry - point.y * line.rx - point.w * line.iz,
        .w = point.x * line.ix + point.y * line.iy + point.z * line.iz
    };
}

inline auto operator&(Line line, Point point) -> Plane {
    // Point linked with dual plane from outer product and line linked
    // with dual line from outer product.
    return {
        .x = -line.ix * point.w - line.ry * point.z + line.rz * point.y,
        .y = -line.iy * point.w + line.rx * point.z - line.rz * point.x,
        .z = -line.iz * point.w - line.rx * point.y + line.ry * point.x,
        .w = line.ix * point.x + line.iy * point.y + line.iz * point.z,
    };
}

inline auto operator&(Point point0, Point point1) -> Line {
    // point0 linked with dual plane0 from outer product and point1 below
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

inline auto operator&(Line line0, Line line1) -> Scalar {
    return {
        .value = line0.rx * line1.ix + line0.ry * line1.iy + line0.rz * line1.iz
            + line0.ix * line1.rx + line0.iy * line1.ry + line0.iz * line1.rz
    };
}

inline auto operator&(Plane plane, Line line) -> f32 {
    return 0.0f;
}

inline auto operator&(Line line, Plane plane) -> f32 {
    return 0.0f;
}

// Geometric product.

inline auto operator*(Plane plane0, Plane plane1) -> Motor {
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
        .iw = 0.0f
    };
}

inline auto operator*(Line line0, Line line1) -> Motor {
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

inline auto operator*(RLine rline0, RLine rline1) -> Rotor {
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

inline auto operator*(Point point0, Point point1) -> Translator {
    return {
        .ix = point0.x * point1.w - point0.w * point1.x,
        .iy = point0.y * point1.w - point0.w * point1.y,
        .iz = point0.z * point1.w - point0.w * point1.z,
        .iw = -point0.w * point1.w
    };
}

inline auto exp(f32 theta, RLine rline) -> Rotor {
    f32 sin = std::sin(theta / 2.0f);
    return {
        .rx = rline.rx * sin,
        .ry = rline.ry * sin,
        .rz = rline.rz * sin,
        .rw = std::cos(theta / 2.0f)
    };
}

inline auto exp(f32 distance, ILine iline) -> Translator {
    f32 half = distance / 2.0f;
    return {
        .ix = iline.ix * half,
        .iy = iline.iy * half,
        .iz = iline.iz * half,
        .iw = 1.0f
    };
}

inline auto operator>>(const Rotor& rotor, const Point& point) -> Point {
    const auto d0 =
        point.x * rotor.rw + point.y * rotor.rz - point.z * rotor.ry;
    const auto d1 =
        point.x * rotor.ry - point.y * rotor.rx + point.z * rotor.rw;
    const auto d2 =
        -point.x * rotor.rz + point.y * rotor.rw + point.z * rotor.rx;
    return {
        .x = point.x + 2.0f * (-rotor.ry * d1 + rotor.rz * d2),
        .y = point.y + 2.0f * (-rotor.rz * d0 + rotor.rx * d1),
        .z = point.z + 2.0f * (-rotor.rx * d2 + rotor.ry * d0),
        .w = point.w
    };
}

inline auto operator>>(const Translator& translator, const Point& point)
    -> Point {
    const auto pwrw = point.w * translator.iw;
    const auto rww = translator.iw * translator.iw;
    return {
        .x = point.x * rww - 2.0f * pwrw * translator.ix,
        .y = point.y * rww - 2.0f * pwrw * translator.iy,
        .z = point.z * rww - 2.0f * pwrw * translator.iz,
        .w = point.w * rww
    };
}
}; // namespace glvm

namespace glvm {
enum States : u8 { IDLE, ATTACK, ROAMING };
} // namespace glvm

namespace glvm {
struct CrosshairTagComponent {};
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
    u32 id;
};

struct Texture {
    // This field is used to choose specific instance of texture image in Vulkan.
    u32 vk_available_inner_id = 0;
    u32 vk_inner_id_limit = 10;

    const char* path_to_image = "";
    Vec<u32> entities_own_this_type_of_texture = {};
    u32 id = 0;
    u32 width = 0;
    u32 height = 0;
    u32 data_length = 0;
    u8* data = 0;
};
} // namespace glvm

struct ThreadPool {
public:
    explicit ThreadPool(usize num_threads);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    auto operator=(const ThreadPool&) -> ThreadPool& = delete;

    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>;

private:
    Vec<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    Mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>> {
    auto task = std::make_shared<
        std::packaged_task<typename std::invoke_result_t<F, Args...>()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<typename std::invoke_result_t<F, Args...>> res =
        task->get_future();
    {
        std::unique_lock<Mutex> lock(queue_mutex);

        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        tasks.emplace([task]() -> void {
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

template<typename T>
struct Point2D {
    T x;
    T y;
};

struct Point3D {
    f32 x;
    f32 y;
    f32 z;
};

template<typename T>
auto clamp(T lower_threshold, T target_value, T upper_threshold) -> T {
    if (target_value < lower_threshold) {
        target_value = lower_threshold;
    } else if (target_value > upper_threshold) {
        target_value = upper_threshold;
    }
    return target_value;
}

template<typename T2, i32 Var2>
struct Vector;

template<typename T, i32 Var>
struct Matrix {
private:
    T elements[Var][Var] {};

public:
    Matrix(T arg = 0) {
        for (i32 i = 0; i < Var; ++i) {
            elements[i][i] = arg;
        }
    }

    Matrix(
        Vector<T, Var> row0,
        Vector<T, Var> row1,
        Vector<T, Var> row2,
        Vector<T, Var> row3
    ) {
        elements[0][0] = row0[0];
        elements[0][1] = row0[1];
        elements[0][2] = row0[2];
        elements[0][3] = row0[3];

        elements[1][0] = row1[0];
        elements[1][1] = row1[1];
        elements[1][2] = row1[2];
        elements[1][3] = row1[3];

        elements[2][0] = row2[0];
        elements[2][1] = row2[1];
        elements[2][2] = row2[2];
        elements[2][3] = row2[3];

        elements[3][0] = row3[0];
        elements[3][1] = row3[1];
        elements[3][2] = row3[2];
        elements[3][3] = row3[3];
    }

    auto self_tensor_transpose() -> void {
        T temp_matrix[Var][Var];
        for (i32 p = 0; p < Var; ++p) {
            for (i32 u = 0; u < Var; ++u) {
                temp_matrix[p][u] = elements[u][p];
            }
        }
        for (i32 j = 0; j < Var; ++j) {
            for (i32 z = 0; z < Var; ++z) {
                this->elements[j][z] = temp_matrix[j][z];
            }
        }
    }

    auto self_identity() -> void {
        for (i32 i = 0; i < Var; ++i) {
            for (i32 j = 0; j < Var; ++j) {
                if (i == j) {
                    this->elements[i][j] = 1.0f;
                } else {
                    this->elements[i][j] = 0.0f;
                }
            }
        }
    }

    auto operator+(const Matrix& matrix) -> Matrix<T, Var>;
    auto operator*(const T scalar) -> Matrix<T, Var>;
    auto operator*(const Matrix& matrix) -> Matrix<T, Var>;
    auto operator[](const i32 index) -> T*;
    auto operator[](const i32 index) const -> const T*;
    template<typename T2, i32 Var2>
    auto operator*(const Vector<T2, Var2>& vector) -> Vector<T2, Var2>;
};

template<typename T, i32 Var>
auto Matrix<T, Var>::operator+(const Matrix& matrix) -> Matrix<T, Var> {
    Matrix<T, Var> temp_matrix;
    for (i32 i = 0; i < Var; ++i) {
        for (i32 j = 0; j < Var; ++j) {
            temp_matrix[i][j] = this->elements[i][j] + matrix.elements[i][j];
        }
    }

    return temp_matrix;
}

template<typename T, i32 Var>
auto Matrix<T, Var>::operator*(const T scalar) -> Matrix<T, Var> {
    Matrix<T, Var> temp_matrix;
    for (i32 i = 0; i < Var; ++i) {
        for (i32 j = 0; j < Var; ++j) {
            temp_matrix[i][j] = this->elements[i][j] * scalar;
        }
    }

    return temp_matrix;
}

template<typename T, i32 Var>
auto Matrix<T, Var>::operator*(const Matrix& matrix) -> Matrix<T, Var> {
    Matrix<T, Var> temp_matrix;
    for (i32 i = 0; i < Var; ++i) {
        for (i32 j = 0; j < Var; ++j) {
            for (i32 n = 0; n < Var; ++n) {
                temp_matrix.elements[i][j] +=
                    elements[i][n] * matrix.elements[n][j];
            }
        }
    }
    return temp_matrix;
}

template<typename T, i32 Var>
auto Matrix<T, Var>::operator[](const i32 index) -> T* {
    return elements[index];
}

template<typename T, i32 Var>
auto Matrix<T, Var>::operator[](const i32 index) const -> const T* {
    return elements[index];
}

template<typename T, i32 Var>
template<typename T2, i32 Var2>
auto Matrix<T, Var>::operator*(const Vector<T2, Var2>& vector)
    -> Vector<T2, Var2> {
    static_assert(Var == Var2, "Size error");
    Vector<T2, Var2> temp_vector;
    for (i32 i = 0; i < Var2; ++i) {
        for (i32 j = 0; j < Var; ++j) {
            temp_vector[i] += elements[i][j] * vector[j];
        }
    }
    return temp_vector;
}

template<typename T2, i32 Dim>
struct Vector {
public:
    T2 elements[Dim] {};

public:
    Vector(T2 x = 0, T2 y = 0, T2 z = 0, T2 w = 0) {
        T2 array[4] = {x, y, z, w};
        for (i32 i = 0; i < Dim; ++i) {
            elements[i] = array[i];
        }
    }

    auto operator[](const i32 index) -> T2&;
    auto operator[](const i32 index) const -> const T2&;
    template<typename T, i32 Dim2>
    auto operator*(const Matrix<T, Dim2>& matrix) -> Vector<T2, Dim>;
    auto operator*(const Vector<T2, Dim>& other) -> Vector<T2, Dim>;
    auto operator*=(const Vector<T2, Dim>& other) -> Vector<T2, Dim>;
    auto operator-(const Vector<T2, Dim>& other) const -> Vector<T2, Dim>;
    auto operator+(const Vector<T2, Dim>& other) const -> Vector<T2, Dim>;
    auto operator-=(const Vector<T2, Dim>& other) -> void;
    auto operator+=(const Vector<T2, Dim>& other) -> void;
    auto operator*(const T2& multiplier) -> Vector<T2, Dim>;
    auto operator-() -> Vector<T2, Dim>;
    auto length() const -> T2;
};

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::length() const -> T2 {
    return std::sqrt(
        elements[0] * elements[0] + elements[1] * elements[1]
        + elements[2] * elements[2]
    );
}

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::operator-() -> Vector<T2, Var2> {
    Vector<T2, Var2> temp_vector;
    for (i32 i = 0; i < Var2; ++i) {
        temp_vector[i] = -elements[i];
    }

    return temp_vector;
}

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::operator[](const i32 index) -> T2& {
    return elements[index];
}

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::operator[](const i32 index) const -> const T2& {
    return elements[index];
}

template<typename T2, i32 Var2>
template<typename T, i32 Var>
auto Vector<T2, Var2>::operator*(const Matrix<T, Var>& matrix)
    -> Vector<T2, Var2> {
    static_assert(Var == Var2, "Size error");
    Vector<T2, Var2> temp_vector;
    for (i32 i = 0; i < Var2; ++i) {
        for (i32 j = 0; j < Var; ++j) {
            temp_vector[i] += elements[j] * matrix[j][i];
        }
    }
    return temp_vector;
}

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::operator*(const Vector<T2, Var2>& other)
    -> Vector<T2, Var2> {
    Vector<T2, Var2> temp_vector;
    for (i32 i = 0; i < 3; ++i) {
        temp_vector[i] = elements[i] * other[i];
    }

    return temp_vector;
}

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::operator*=(const Vector<T2, Var2>& other)
    -> Vector<T2, Var2> {
    Vector<T2, Var2> temp_vector;
    for (i32 i = 0; i < 3; ++i) {
        temp_vector[i] = elements[i] * other[i];
    }

    return temp_vector;
}

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::operator-(const Vector<T2, Var2>& other) const
    -> Vector<T2, Var2> {
    Vector<T2, Var2> temp_vector(1);

    temp_vector[0] = elements[0] - other[0];
    temp_vector[1] = elements[1] - other[1];
    temp_vector[2] = elements[2] - other[2];

    return temp_vector;
}

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::operator+(const Vector<T2, Var2>& other) const
    -> Vector<T2, Var2> {
    Vector<T2, Var2> temp_vector(1);

    temp_vector[0] = elements[0] + other[0];
    temp_vector[1] = elements[1] + other[1];
    temp_vector[2] = elements[2] + other[2];

    return temp_vector;
}

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::operator-=(const Vector<T2, Var2>& other) -> void {
    elements[0] = elements[0] - other[0];
    elements[1] = elements[1] - other[1];
    elements[2] = elements[2] - other[2];
}

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::operator+=(const Vector<T2, Var2>& other) -> void {
    elements[0] = elements[0] + other[0];
    elements[1] = elements[1] + other[1];
    elements[2] = elements[2] + other[2];
}

template<typename T2, i32 Var2>
auto Vector<T2, Var2>::operator*(const T2& multiplier) -> Vector<T2, Var2> {
    Vector<T2, Var2> temp_vec(1.0f);

    for (i32 i = 0; i < Var2; ++i) {
        temp_vec[i] = elements[i] * multiplier;
    }

    return temp_vec;
}

template<typename T2, i32 Var2>
auto operator*(const Vector<T2, Var2>& vector, const T2 multiplier)
    -> Vector<T2, Var2> {
    Vector<T2, Var2> temp;
    for (i32 i = 0; i < Var2; ++i) {
        temp[i] = vector[i] * multiplier;
    }
    return temp;
}

template<typename T>
auto determinant_2x2(Matrix<T, 2> matrix) -> T {
    return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
}

template<typename T>
auto determinant_3x3(Matrix<T, 3> matrix) -> T {
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
auto determinant_4x4(Matrix<T, 4> matrix) -> T {
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
    T determinant = matrix[0][0] * determinant_3x3<f32>(remove_1row_1col)
        - matrix[0][1] * determinant_3x3<f32>(remove_1row_2col)
        + matrix[0][2] * determinant_3x3<f32>(remove_1row_3col)
        - matrix[0][3] * determinant_3x3<f32>(remove_1row_4col);
    return determinant;
}

template<typename T>
auto inverse_matrix_4x4(Matrix<T, 4> matrix) -> Matrix<T, 4> {
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
    for (u32 i = 0; i < 4; ++i) {
        for (u32 j = 0; j < 4; ++j) {
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

template<typename T, typename T2, i32 Var, i32 Var2>
auto look_at(Matrix<T, Var> matrix, Vector<T2, Var2> vector) -> Matrix<T, Var> {
    Matrix<T, Var> temp_matrix(1.0f);
    temp_matrix = matrix;

    const auto variable = 3;
    for (i32 i = 0; i < Var2; ++i) {
        temp_matrix[i][variable] = -vector[i];
    }

    return temp_matrix;
}

template<typename T, typename T2, i32 Var, i32 Var2>
auto translate(Matrix<T, Var> matrix, Vector<T2, Var2> vector)
    -> Matrix<T, Var> {
    Matrix<T, Var> temp_matrix(1.0f);
    temp_matrix = matrix;
    for (i32 i = 0; i < Var; ++i) {
        temp_matrix[Var - 1][i] += vector[i];
    }
    return temp_matrix;
}

template<typename T, typename T2, i32 Var, i32 Var2>
auto scale(Matrix<T, Var> matrix, Vector<T2, Var2> vector) -> Matrix<T, Var> {
    Matrix<T, Var> temp_matrix;
    temp_matrix = matrix;
    for (i32 i = 0; i < Var; ++i) {
        for (i32 j = 0; j < Var; ++j) {
            temp_matrix[j][i] *= vector[i];
        }
    }
    return temp_matrix;
}

template<typename T, i32 Var>
auto rotate_z(Matrix<T, Var> matrix, f32 angle) -> Matrix<T, Var> {
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
auto cross(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs) -> Vector<T, 3> {
    return Vector<T, 3>(
        lhs[1] * rhs[2] - lhs[2] * rhs[1],
        lhs[2] * rhs[0] - lhs[0] * rhs[2],
        lhs[0] * rhs[1] - lhs[1] * rhs[0]
    );
}

template<typename T>
auto dot(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs) -> T {
    return (lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2]);
}

template<typename T>
auto vector_length(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs) -> T {
    T diff_x = rhs[0] - lhs[0];
    T diff_y = rhs[1] - lhs[1];
    T diff_z = rhs[2] - lhs[2];
    return std::sqrt(diff_x * diff_x + diff_y * diff_y + diff_z * diff_z);
}

template<typename T>
auto vec_length(const Vector<T, 3>& vector) -> T {
    return std::sqrt(
        vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]
    );
}

template<typename T>
auto normalize(Vector<T, 3> other) -> Vector<T, 3> {
    if (other[0] == 0 && other[1] == 0 && other[2] == 0) {
        return Vector<f32, 3> {0.0f, 0.0f, 0.0f};
    }
    f32 range = std::sqrt(
        other[0] * other[0] + other[1] * other[1] + other[2] * other[2]
    );
    for (i32 l = 0; l < 3; ++l) {
        other[l] = other[l] / range;
    }
    return other;
}

template<typename T>
auto glvm_perspective_rh_no(T fov, T aspect, T near_plane, T far_plane)
    -> Matrix<T, 4> {
    const T tan_half_fov = std::tan(fov / static_cast<T>(2));
    Matrix<f32, 4> result(static_cast<T>(0));
    result[0][0] = static_cast<T>(1) / (aspect * tan_half_fov);
    result[1][1] = static_cast<T>(1) / (tan_half_fov);
    result[2][2] = -(far_plane - near_plane) / (far_plane - near_plane);
    result[2][3] = -static_cast<T>(1);
    result[3][2] = -(static_cast<T>(2) * far_plane * near_plane)
        / (far_plane - near_plane);
    return result;
}

template<typename T>
auto perspective(T fov, T aspect, T near_plane, T far_plane) -> Matrix<T, 4> {
    return glvm_perspective_rh_no<T>(fov, aspect, near_plane, far_plane);
}

template<typename T>
auto look_at_rh(Vector<T, 3> eye, Vector<T, 3> center, Vector<T, 3> up)
    -> Matrix<T, 4> {
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
auto look_at_main(Vector<T, 3> eye, Vector<T, 3> center, Vector<T, 3> up)
    -> Matrix<T, 4> {
    return look_at_rh<T>(eye, center, up);
}

template<typename T>
auto fps_view(Vector<T, 3> eye, Vector<T, 3> center, Vector<T, 3> up)
    -> Matrix<T, 4> {
    Vector<T, 3> f(normalize(center - eye));
    Vector<T, 3> s(normalize(cross(f, up)));
    Vector<T, 3> u(cross(s, f));
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

template<typename T3>
auto radians(T3 degrees) -> T3 {
    degrees *= PI / static_cast<T3>(180);
    return degrees;
}

template<typename T>
auto fps_view_rh(Vector<T, 3> eye, f32 pitch_deg, f32 yaw_deg) -> Matrix<T, 4> {
    pitch_deg *= PI / 180;
    yaw_deg *= PI / 180;
    f32 cos_pitch = std::cos(pitch_deg);
    f32 sin_pitch = std::sin(pitch_deg);
    f32 cos_yaw = std::cos(yaw_deg);
    f32 sin_yaw = std::sin(yaw_deg);
    Vector<T, 3> x_axis(cos_yaw, 0, -sin_yaw);
    Vector<T, 3> y_axis(sin_yaw * sin_pitch, cos_pitch, cos_yaw * sin_pitch);
    Vector<T, 3> z_axis(sin_yaw * cos_pitch, -sin_pitch, cos_pitch * cos_yaw);
    Matrix<T, 4> view(
        Vector<T, 4>(x_axis[0], y_axis[0], z_axis[0], 0),
        Vector<T, 4>(x_axis[1], y_axis[1], z_axis[1], 0),
        Vector<T, 4>(x_axis[2], y_axis[2], z_axis[2], 0),
        Vector<T, 4>(-dot(x_axis, eye), -dot(y_axis, eye), -dot(z_axis, eye), 1)
    );
    return view;
}

template<typename T, i32 Var, i32 VecSize>
auto rotate(Vector<T, VecSize> vector, f32 angle) -> Matrix<T, Var> {
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

template<typename T, i32 Var>
auto ortho(f32 w, f32 h, f32 zn, f32 zf) -> Matrix<T, Var> {
    Matrix<T, Var> temp_matrix(1.0f);
    temp_matrix[0][0] = 2 / w;
    temp_matrix[1][1] = 2 / h;
    temp_matrix[2][2] = 1 / (zf - zn);
    return temp_matrix;
}

template<typename T>
auto ortho_rh_zo(T left, T right, T bottom, T top, T near_plane, T far_plane)
    -> Matrix<T, 4> {
    Matrix<f32, 4> temp_matrix(1);
    temp_matrix[0][0] = static_cast<T>(2) / (right - left);
    temp_matrix[1][1] = static_cast<T>(2) / (top - bottom);
    temp_matrix[2][2] = -static_cast<T>(1) / (far_plane - near_plane);
    temp_matrix[3][0] = -(right + left) / (right - left);
    temp_matrix[3][1] = -(top + bottom) / (top - bottom);
    temp_matrix[3][2] = -near_plane / (far_plane - near_plane);

    return temp_matrix;
}

template<typename T>
auto ortho(T left, T right, T bottom, T top, T near_plane, T far_plane)
    -> Matrix<T, 4> {
    return ortho_rh_zo<T>(left, right, bottom, top, near_plane, far_plane);
}

template<typename T, i32 Var>
auto perspective_rh_zo(T fov, T aspect, T near_plane, T far_plane)
    -> Matrix<T, Var> {
    const auto tan_half_fov = std::tan((fov * 0.5f) * (PI / 360));
    Matrix<f32, Var> temp_matrix(static_cast<T>(0));
    temp_matrix[0][0] = static_cast<T>(1) / (aspect * tan_half_fov);
    temp_matrix[1][1] = static_cast<T>(1) / tan_half_fov;
    temp_matrix[2][2] = far_plane / (near_plane - far_plane);
    temp_matrix[2][3] = static_cast<T>(1);
    temp_matrix[3][2] = -(far_plane * near_plane) / (far_plane - near_plane);
    return temp_matrix;
}

template<typename T, i32 Var>
auto perspective(
    const T fov,
    const T aspect,
    const T near_plane,
    const T far_plane
) -> Matrix<T, Var> {
    return perspective_rh_zo(fov, aspect, near_plane, far_plane);
}

constexpr auto max(f32 var1, f32 var2) -> f32 {
    return var1 > var2 ? var1 : var2;
}

constexpr auto min(f32 var1, f32 var2) -> f32 {
    return var1 < var2 ? var1 : var2;
}

struct Quaternion {
    f32 w, x, y, z;
    Quaternion() = default;

    Quaternion(f32 qw, f32 qx, f32 qy, f32 qz) : w(qw), x(qx), y(qy), z(qz) {
    }

    Quaternion(f32 real, Vector<f32, 3> imaginary) :
        w(real),
        x(imaginary[0]),
        y(imaginary[1]),
        z(imaginary[2]) {
    }
};

inline auto conjugate(Quaternion quaternion) -> Quaternion {
    quaternion.w = quaternion.w;
    quaternion.x = -quaternion.x;
    quaternion.y = -quaternion.y;
    quaternion.z = -quaternion.z;
    return quaternion;
}

inline auto multiply_quaternion(const Quaternion& a, const Quaternion& b)
    -> Quaternion {
    Quaternion result;
    result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    return result;
}

inline auto operator*(const Quaternion& a, const Quaternion& b) -> Quaternion {
    Quaternion result;
    result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    return result;
}

inline auto norm_quaternion(const Quaternion& quaternion) -> f32 {
    return std::sqrt(
        quaternion.w * quaternion.w + quaternion.x * quaternion.x
        + quaternion.y * quaternion.y + quaternion.z * quaternion.z
    );
}

inline auto normalize_quaternion(Quaternion quaternion) -> Quaternion {
    f32 norm = norm_quaternion(quaternion);
    f32 inverse_norm = 1.0f / norm;
    quaternion.w *= inverse_norm;
    quaternion.x *= inverse_norm;
    quaternion.y *= inverse_norm;
    quaternion.z *= inverse_norm;
    return quaternion;
}

inline auto inverse_quaternion(Quaternion quaternion) -> Quaternion {
    Quaternion linked_value = conjugate(quaternion);
    f32 norm = norm_quaternion(quaternion);
    f32 inverse_norm = 1.0f / norm;
    quaternion.w = linked_value.w * inverse_norm;
    quaternion.x = linked_value.x * inverse_norm;
    quaternion.y = linked_value.y * inverse_norm;
    quaternion.z = linked_value.z * inverse_norm;
    return quaternion;
}

inline auto euler_to_quaternion(const f32 roll, const f32 pitch, const f32 yaw)
    -> Quaternion {
    const auto cr = std::cos(roll * 0.5f);
    const auto sr = std::sin(roll * 0.5f);
    const auto cp = std::cos(pitch * 0.5f);
    const auto sp = std::sin(pitch * 0.5f);
    const auto cy = std::cos(yaw * 0.5f);
    const auto sy = std::sin(yaw * 0.5f);
    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    return q;
}

template<typename T, i32 Var>
auto rotate_quaternion(Quaternion quaternion) -> Matrix<T, Var> {
    Matrix<f32, 4> result(0.0f);

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
struct Position {
private:
    f32 x;
    f32 y;
    f32 z;

public:
    auto operator[](const u32 index) -> f32& {
        assert(index < 3 && index >= 0 && "wrong index");
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

struct Face {
private:
    Vec<i32> vertex_index;
    Vec<i32> texture_index;
    Vec<i32> normal_index;

public:
    auto operator[](const u32 index) -> Vec<i32>& {
        assert(index < 3 && index >= 0 && "wrong index");
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

    auto operator[](const u32 index) const -> const Vec<i32>& {
        assert(index < 3 && index >= 0 && "wrong index");
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

struct WavefrontObjParser {
private:
    Vec<Position> coordinate_vertices;
    Vec<Position> texture_vertices;
    Vec<Position> normals;
    Vec<Face> faces;

    String wavefront_obj_file_data;
    const char* wavefront_obj_file_data_ptr;
    u32 cursor = 0;

public:
    WavefrontObjParser();

    [[nodiscard]] auto get_coordinate_vertices() const -> const Vec<Position>&;
    [[nodiscard]] auto get_texture_vertices() const -> const Vec<Position>&;
    [[nodiscard]] auto get_normals() const -> const Vec<Position>&;
    [[nodiscard]] auto get_faces() const -> const Vec<Face>&;

    auto read_file(const char* file_path) -> void;
    auto parse_file() -> void;
    auto split(
        const char* data,
        const char separator,
        const char exit_symbol,
        u32& position
    ) -> Vec<Vec<char>>;
    auto parse_vertices(Vec<Vec<char>> words) -> Position;
    auto parse_faces(Vec<Vec<char>> words) -> Face;
    auto parse_integer(Vec<char> digits) -> i32;
    auto parse_float(Vec<char> digits) -> f32;
};
} // namespace glvm

namespace glvm {
struct Inventory {
public:
    Inventory() {
        for (u32 i = 0; i < row; ++i) {
            slots[i] = new u32[col];
        }

        for (u32 i = 0; i < row; ++i) {
            for (u32 j = 0; j < col; ++j) {
                slots[i][j] = -1;
            }
        }
    }

    Inventory(const Inventory& inv) {
        for (u32 i = 0; i < row; ++i) {
            this->slots[i] = new u32[col];
        }

        for (u32 i = 0; i < row; ++i) {
            for (u32 j = 0; j < col; ++j) {
                this->slots[i][j] = inv.slots[i][j];
            }
        }

        this->entity_owner = inv.entity_owner;
        this->highlighted_slots = inv.highlighted_slots;
        this->is_available_highlighted_slots =
            inv.is_available_highlighted_slots;
    }

    ~Inventory() {
        for (u32 i = 0; i < row; ++i) {
            delete[] slots[i];
        }

        delete[] slots;
    }

    u32 row = 8;
    u32 col = 8;
    // Array with entities containing InventorySlotComponents.
    u32** slots = new u32*[row];
    u32 entity_owner = UINT_MAX;
    Vec<u32> highlighted_slots;
    bool is_available_highlighted_slots = false;
    MeshHandle slot_mesh_id;
    f32 slot_scale;
};
}; // namespace glvm

namespace glvm {
struct MeshManager {
private:
    static MeshManager* instance;
    static Mutex mutex;

    MeshManager();
    ~MeshManager();

public:
    Vec<const char*> paths_array;
    Vec<const char*> paths_gltf;

    // It is possible to get only one instance of this struct with this method.
    static auto get_instance() -> MeshManager*;
    auto set_mesh(const char* mesh_path) -> void;
    auto set_mesh_gltf(const char* path_to_mesh) -> void;
};
} // namespace glvm

namespace glvm {
struct EventStack {
private:
    i32 head = 0;
    static const auto STACK_RANGE = 6;
    EventKind stack[STACK_RANGE] = {};

public:
    auto push(const EventKind& event) -> void {
        for (i32 i = 0; i < head; ++i) {
            if (stack[i] == event) {
                return;
            }
        }

        if (head == STACK_RANGE) {
            return;
        }

        stack[head] = event;

        ++head;
    }

    auto pop() -> EventKind& {
        if (head == 0) {
            return stack[0];
        }
        return stack[head - 1];
    }

    auto remove(const EventKind& event) -> void {
        EventKind temp_stack[STACK_RANGE] = {};
        bool remove_flag = false;
        i32 n = 0;

        for (i32 j = 0; j < STACK_RANGE; ++j) {
            temp_stack[j] = stack[j];
        }

        for (i32 i = 0; i < head; ++i) {
            if (event == temp_stack[i]) {
                remove_flag = true;
                continue;
            }

            stack[n] = temp_stack[i];
            ++n;
        }

        if (remove_flag) {
            --head;
            stack[head] = EventKind::Default;
        }
    }

    auto control_input(Event& event) -> void {
        if (!(search_element(event.get_event()) == Empty)) {
            return;
        }
        switch (event.get_event()) {
            case GameLoopKill:
                push(GameLoopKill);
                break;
            case KeyReleaseA:
                remove(MoveLeft);
                break;
            case KeyReleaseD:
                remove(MoveRight);
                break;
            case KeyReleaseS:
                remove(MoveBackward);
                break;
            case KeyReleaseW:
                remove(MoveForward);
                break;
            case InventoryRelease:
                remove(InventoryToggle);
                break;
            case KeyReleaseJump:
                remove(Jump);
                break;
            case MouseLeftButtonRelease:
                remove(MouseLeftButton);
                break;
            case MoveLeft:
                push(MoveLeft);
                break;
            case MoveRight:
                push(MoveRight);
                break;
            case MoveBackward:
                push(MoveBackward);
                break;
            case MoveForward:
                push(MoveForward);
                break;
            case Jump:
                push(Jump);
                break;
            case InventoryToggle:
                push(InventoryToggle);
                break;
            case CursorReleased:
                push(CursorReleased);
                break;
            case MouseLeftButton:
                push(MouseLeftButton);
                break;
            default:
                break;
        }
    }

    auto search_element(EventKind element) -> EventKind {
        for (i32 i = 0; i < head; ++i) {
            if (stack[i] == element) {
                return element;
            }
        }

        return Empty;
    }

    auto operator[](i32 index) -> EventKind& {
        return stack[index];
    }

    auto clear() -> void {
        for (i32 i = 0; i < head; ++i) {
            stack[i] = EventKind::Default;
        }
    }
};
} // namespace glvm

namespace glvm {

struct WindowInterface {
public:
    // Window keyboard focus, updated by each backend.
    bool is_focused = true;

    virtual ~WindowInterface() = default;

    virtual auto swap_buffers() -> void = 0;
    virtual auto clear_display() -> void = 0;
    virtual auto handle_event(Event& event) -> bool = 0;
    virtual auto close() -> void = 0;
    virtual auto cursor_lock(
        i32 pointer_x,
        i32 pointer_y,
        i32* out_offset_x,
        i32* out_offset_y
    ) -> void = 0;
};

} // namespace glvm

namespace glvm {
struct TimerCreator {
public:
    ~TimerCreator() {
    }

    auto create() -> Chrono*;
};
} // namespace glvm

#ifdef __linux__

namespace glvm {
struct TimerX: public Chrono {
private:
    timespec start_;
    timespec now_;
    f64 frequency_;
    f64 seconds_;
    f64 nanoseconds_;

public:
    TimerX();

    auto init_frequency() -> f64;
    auto reset() -> f64;
    auto get_elapsed() -> f64;
};
} // namespace glvm
#endif // __linux__

#ifdef _WIN32

namespace glvm {
struct TimerWin: public Chrono {
private:
    __int64 i64_freq;
    __int64 i64_start;
    __int64 i64_now;

public:
    TimerWin();

    auto init_frequency() -> f64;
    auto reset() -> f64;
    auto get_elapsed() -> f64;
};
} // namespace glvm
#endif // _WIN32

namespace glvm {
struct State {
    States state;
};
} // namespace glvm

namespace glvm {
struct SoundSample {
    const char* path_to_file;
    u32 ui_duration;
    u32 ui_rate;
    f32 volume;
};

struct SoundEngine {
public:
    virtual ~SoundEngine() {
    }

    virtual auto open_device(const char* device) -> void = 0;
    virtual auto close_device() -> void = 0;
    virtual auto get_sound_container() -> Vec<SoundSample*>& = 0;
    virtual auto playback_sound_sample(SoundSample& sample) -> void = 0;
    virtual auto set_master_volume(long volume) -> void = 0;
    virtual auto sound_stream() -> void = 0;
    virtual auto create_sound_sample(
        const char* file_path,
        u32 duration,
        u32 rate,
        f32 volume
    ) -> void = 0;
};
} // namespace glvm

namespace glvm {
struct Archetype {
    virtual ~Archetype() = default;

    static constexpr auto CAPACITY = 1024;

    u64 entities[CAPACITY];
    u32 entity_count = 0;
    u32 component_ids[ComponentsIndices::ComponentsCount] = {};
    u32 component_count = 0;
    void* components[ComponentsIndices::ComponentsCount] = {};
    u64 mask = 0;

    auto add_entity(u64 entity) -> u32;
    auto remove_entity(u32 index) -> u64;
};

struct EntityLocation {
    Archetype* arch;
    u32 index;
    static const auto max_grid_cell_number = 32;
    u8 grid_cell_counter = 0;
    Vector<f32, 3> grid_cell_indices[max_grid_cell_number];
    u32 cell_entity_indices[max_grid_cell_number];
    // Whether the entity has been moved or removed.
    bool is_dirty = false;
};
}; // namespace glvm

namespace glvm {
struct DirectionalLightComponent {
    Vector<f32, 3> position;
    Vector<f32, 3> direction;

    Vector<f32, 3> ambient;
    Vector<f32, 3> diffuse;
    Vector<f32, 3> specular;
};
} // namespace glvm

namespace glvm {
struct Material {
    TextureHandle diffuse_texture_id = {};
    TextureHandle specular_texture_id = {};
    Vector<f32, 3> ambient = {0.0f, 0.0f, 0.0f};
    f32 shininess = 0.0f;
};
} // namespace glvm

namespace glvm {
struct Move {
    EventKind event = EventKind::Default;
    Vector<f32, 3> frame_movement {0.0f, 0.0f, 0.0f};
    Vector<f32, 3> gravity {0.0f, 0.0f, 0.0f};
};
} // namespace glvm

namespace glvm {
struct PointLightComponent {
    Vector<f32, 3> position;

    Vector<f32, 3> ambient;
    Vector<f32, 3> diffuse;
    Vector<f32, 3> specular;

    f32 constant;
    f32 linear;
    f32 quadratic;
};
} // namespace glvm

namespace glvm {
struct RigidBody {
public:
    f32 mass = 0.0f;
    f32 jump_accumulator = 0.0f;
};
} // namespace glvm

namespace glvm {
struct SpotLightComponent {
    Vector<f32, 3> position;
    Vector<f32, 3> direction;
    f32 cut_off;
    f32 outer_cut_off;

    Vector<f32, 3> ambient;
    Vector<f32, 3> diffuse;
    Vector<f32, 3> specular;

    f32 constant;
    f32 linear;
    f32 quadratic;
};
} // namespace glvm

namespace glvm {
struct Transform {
    Vector<f32, 3> position {0.0f, 0.0f, 0.0f};
    Vector<f32, 3> forward {0.0f, 0.0f, 0.0f};
    f32 scale = 1.0f;
    f32 gravity_accumulator = 0.0f;
};
} // namespace glvm

namespace glvm {
struct Beholder {
    Vector<f32, 3> position {0.0f, 0.0f, 0.0f};
    Vector<f32, 3> forward {0.0f, 0.0f, 0.0f};
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
    String* string;
    f64 float_number;
    i32 int_number;
    bool boolean;
    void* null;
    Vec<JsonValue>* array;
    HashMap<String, JsonValue>* object;

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

    JsonValue(String str) {
        type = JsonString;
        value.string = new String(str);
    }

    JsonValue(f64 number) {
        type = JsonFloatNumber;
        value.float_number = number;
    }

    JsonValue(i32 number) {
        type = JsonIntegerNumber;
        value.int_number = number;
    }

    JsonValue(bool flag) {
        type = JsonBoolean;
        value.boolean = flag;
    }

    JsonValue(const JsonValue& other) {
        type = JsonInvalidValue;

        switch (other.type) {
            case JsonObject:
                value.object =
                    new HashMap<String, JsonValue>(*other.value.object);
                break;
            case JsonIntegerNumber:
                value.int_number = other.value.int_number;
                break;
            case JsonFloatNumber:
                value.float_number = other.value.float_number;
                break;
            case JsonString:
                value.string = new String(*other.value.string);
                break;
            case JsonBoolean:
                value.boolean = other.value.boolean;
                break;
            case JsonNull:
                value.null = other.value.null;
                break;
            case JsonArray:
                value.array = new Vec<JsonValue>(*other.value.array);
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
                value.object =
                    new HashMap<String, JsonValue>(*other.value.object);
                break;
            case JsonIntegerNumber:
                value.int_number = other.value.int_number;
                break;
            case JsonFloatNumber:
                value.float_number = other.value.float_number;
                break;
            case JsonString:
                value.string = new String(*other.value.string);
                break;
            case JsonBoolean:
                value.boolean = other.value.boolean;
                break;
            case JsonNull:
                value.null = other.value.null;
                break;
            case JsonArray:
                value.array = new Vec<JsonValue>(*other.value.array);
                break;
            default:
                break;
        }
        type = other.type;
    }

    auto operator[](String lookup_key) -> JsonValue& {
        switch (type) {
            case JsonObject:
                return (*value.object)[lookup_key];
                break;
            default:
                throw std::out_of_range("Type is not a JSON object");
                break;
        }
    }

    auto operator[](const u32 index) -> JsonValue& {
        switch (type) {
            case JsonArray:
                return (*value.array)[index];
                break;
            default:
                throw std::out_of_range("Type is not a JSON array");
                break;
        }
    }

    auto is_invalid() -> bool {
        return type == JsonInvalidValue;
    }

    auto is_object() -> bool {
        return type == JsonObject;
    }

    auto is_float() -> bool {
        return type == JsonFloatNumber;
    }

    auto is_integer() -> bool {
        return type == JsonIntegerNumber;
    }

    auto is_string() -> bool {
        return type == JsonString;
    }

    auto is_boolean() -> bool {
        return type == JsonBoolean;
    }

    auto is_null() -> bool {
        return type == JsonNull;
    }

    auto is_array() -> bool {
        return type == JsonArray;
    }
};

struct JsonParser {
private:
    String json_file_data;
    const char* json_file_data_ptr;
    char current_char;
    u32 file_counter = 0;

    Vec<JsonValue*> stack_of_json_values;
    JsonValue* root;
    bool key_flag = true;
    String last_key = "";
    String buffer_string = "";

    auto search_in_json_array(
        Vec<JsonValue>* array_value,
        const char* key,
        Vec<JsonValue>& result_vector
    ) const -> void;

public:
    auto search_in_json_object(
        HashMap<String, JsonValue>* map_value,
        const char* key,
        Vec<JsonValue>& result_vector
    ) const -> void;

    ~JsonParser();

    auto get_root() -> JsonValue* {
        return root;
    }

    auto read_file(const char* file_path) -> void;
    auto parse() -> void;
    auto create_json_hash_map() -> JsonValue;
    auto create_json_array() -> JsonValue;
    auto parse_bool_or_null() -> String;
    auto contains_char(String text, char character) -> bool;
    auto parse_number_as_string() -> String;
    auto parse_string() -> String;
    auto string_to_vector_of_chars(String text) -> Vec<char>;
    auto parse_integer(Vec<char> digits) -> i32;
    auto parse_float(Vec<char> digits) -> f64;
    auto search(const char* key) const -> Vec<JsonValue>;
    auto load_gltf(
        const char* paths_gltf,
        Vec<f32>& vertices,
        Vec<u32>& indices,
        Vec<Vec<Matrix<f32, 4>>>& joint_matrices_per_mesh,
        Vec<f32>& frames,
        bool& no_animations,
        f32& top_y
    ) -> void;
    auto traverse_bones(
        Vec<Vec<i32>> children,
        JsonValue joints,
        Vec<u32> node_stack,
        Vec<u32> depth_stack,
        Vec<Vec<u32>>& result
    ) -> void;
    auto make_render_joints_indices(Vec<Vec<u32>>& input) -> Vec<Vec<u32>>;
    auto contains_element(Vec<Vec<u32>> container, u32 element) -> bool;
    auto get_joint_index(JsonValue joints, i32 searching_index) -> u32;
};
} // namespace glvm

namespace glvm {
struct LightSpaceMatrixUBO {
    alignas(16) Matrix<f32, 4> spot_space_matrix[SPOT_LIGHTS_NUMBER];
    alignas(16) u32 spot_lights_number;

    alignas(16) Matrix<f32, 4> dir_space_matrix[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) u32 directional_lights_number;
};

struct alignas(64) ModelMatrixUBO {
    Matrix<f32, 4> model;
    Matrix<f32, 4> view;
    Matrix<f32, 4> proj;
    Matrix<f32, 4> joint_matrices[MAX_JOINTS_NUMBER];

    Vector<f32, 3> ambient;
    f32 shininess;

    alignas(16) Matrix<f32, 4> spot_space_matrix[SPOT_LIGHTS_NUMBER];
    alignas(16) u32 spot_lights_number;

    alignas(16) Matrix<f32, 4> dir_space_matrix[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) u32 directional_lights_number;
};

struct alignas(16) ShadowMapMatrixUBO {
    Matrix<f32, 4> model;
    Matrix<f32, 4> light_space_matrix;
    Matrix<f32, 4> joint_matrices[MAX_JOINTS_NUMBER];
};

struct alignas(16) SpotLightShadowMapMatrixUBO {
    Matrix<f32, 4> model;
    Matrix<f32, 4> light_space_matrix;
};

struct alignas(64) PointLightShadowMapMatrixUBO {
    Matrix<f32, 4> model;
    Matrix<f32, 4> light_space_matrix;
    Vector<f32, 3> light_position;
    f32 far_plane;
    Matrix<f32, 4> joint_matrices[MAX_JOINTS_NUMBER];
};

struct alignas(16) UniformBufferObjectLightUBO {
    Vector<f32, 3> light_position;
    f32 far_plane;
};

struct alignas(16) DirectionalLight {
    Vector<f32, 4> position;
    Vector<f32, 4> direction;

    Vector<f32, 4> ambient;
    Vector<f32, 4> diffuse;
    Vector<f32, 4> specular;
};

struct alignas(16) PointLight {
    Vector<f32, 3> position;
    f32 padding0;

    Vector<f32, 3> ambient;
    f32 padding1;
    Vector<f32, 3> diffuse;
    f32 padding2;

    Vector<f32, 3> specular;
    f32 constant;
    f32 linear;
    f32 quadratic;
};

struct alignas(16) SpotLight {
    alignas(16) Vector<f32, 3> position;
    alignas(16) Vector<f32, 3> direction;
    f32 cut_off;
    f32 outer_cut_off;

    alignas(16) Vector<f32, 3> ambient;
    alignas(16) Vector<f32, 3> diffuse;
    alignas(16) Vector<f32, 3> specular;

    f32 constant;
    f32 linear;
    f32 quadratic;
};

struct alignas(64) LightData {
    Vector<f32, 2> tileset_tiles_count;
    i32 tiles_row;
    i32 tiles_column;

    alignas(16) Vector<f32, 3> view_position;

    PointLight point_lights[POINT_LIGHTS_NUMBER];
    i32 point_lights_array_size;
    f32 far_plane;
    i32 padding0;
    i32 padding1;

    DirectionalLight directional_lights[DIRECTIONAL_LIGHTS_NUMBER];
    alignas(16) i32 directional_lights_array_size;

    SpotLight spot_lights[SPOT_LIGHTS_NUMBER];
    i32 spot_light_array_size;
    i32 padding2;
    i32 padding3;
    i32 padding4;

    Vector<i32, 4> indirect_texture
        [INDIRECT_TEXTURE_WIDTH * INDIRECT_TEXTURE_HEIGHT / 4 + 1];

    // Debug: 0 = off, 1 = directional, 2 = spot. When set, the main shader
    // renders the shadow map depth projected onto the scene instead of
    // lighting (visualized from the normal moving camera).
    i32 debug_shadow_mode;
    i32 debug_shadow_light;
    i32 shadows_enabled;
};

struct alignas(64) HudUbo {
    Matrix<f32, 4> view;
    Matrix<f32, 4> proj;
    Vector<f32, 3> entity_position;
    i32 hud_exists;
    f32 max_hp;
    f32 current_hp;
    f32 highest_y;
};

struct alignas(64) HudScreenUbo {
    Matrix<f32, 4> model;
};

struct alignas(64) FontUbo {
    Matrix<f32, 4> view;
    Matrix<f32, 4> proj;
    Vector<f32, 3> position;
    f32 scale;
};

struct alignas(64) UiUbo {
    Matrix<f32, 4> model;
    Vector<f32, 3> color;
};

struct alignas(64) VirtualTextureUbo {};

struct alignas(64) SdfUbo {
    Matrix<f32, 4> model;
    f32 time;
};

} // namespace glvm

#ifdef _WIN32

namespace glvm {

struct WindowWinVulkan: public WindowInterface {
private:
    HWND classic_window;
    HDC classic_dc;
    HGLRC classic_context;

    WNDCLASS window_class;
    HDC modern_dc;
    HGLRC modern_context;
    HWND modern_window;

    // Cursor-lock baseline: the cursor's actual position after the last warp
    // (not the computed center).
    i32 previous_x = 0;
    i32 previous_y = 0;

public:
    static WindowWinVulkan* instance;
    EventStack* input_stack;
    u32 width = GetSystemMetrics(SM_CXSCREEN);
    u32 height = GetSystemMetrics(SM_CYSCREEN);
    WindowWinVulkan();

    auto swap_buffers() -> void override;
    auto clear_display() -> void override;
    auto handle_event(Event& event) -> bool override;
    auto get_classic_window_hwnd() -> HWND;
    auto get_modern_window_hwnd() -> HWND;
    auto close() -> void override;
    virtual auto cursor_lock(
        i32 pointer_x,
        i32 pointer_y,
        i32* out_offset_x,
        i32* out_offset_y
    ) -> void override;
    // Callback method for handling events.
    static auto main_wnd_proc(
        HWND hwnd,
        UINT message,
        WPARAM w_param,
        LPARAM l_param
    ) -> LRESULT;
};
} // namespace glvm
#endif // _WIN32

#ifdef __linux__

namespace glvm {
struct SoundEngineAlsa: public SoundEngine {
private:
    snd_pcm_t* pcm;
    Vec<SoundSample*> sound_container;

public:
    auto open_device(const char* device) -> void override;
    auto close_device() -> void override;
    auto sound_stream() -> void override;
    auto playback_sound_sample(SoundSample& sample) -> void override;
    auto set_master_volume(long volume) -> void override;
    auto get_sound_container() -> Vec<SoundSample*>& override;
    auto create_sound_sample(
        const char* file_path,
        u32 duration,
        u32 rate,
        f32 volume
    ) -> void override;

    ~SoundEngineAlsa();
};
} // namespace glvm
#endif // __linux__

namespace glvm {
struct SoundEngineFactory {
public:
    auto create_sound_engine() -> SoundEngine*;
};

} // namespace glvm

#ifdef _WIN32
namespace glvm {
struct SoundEngineWaveform: public SoundEngine {
private:
    Vec<SoundSample*> sound_container;

public:
    auto open_device(const char* device) -> void override;
    auto close_device() -> void override;
    auto sound_stream() -> void override;
    auto playback_sound_sample(SoundSample& sample) -> void override;
    auto set_master_volume(long volume) -> void override;
    auto create_sound_sample(
        const char* file_path,
        u32 duration,
        u32 rate,
        f32 volume
    ) -> void override;
    auto get_sound_container() -> Vec<SoundSample*>& override;
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
struct TextureManager {
private:
    static TextureManager* instance;
    static Mutex mutex;

    Vec<Texture> texture_vector;

public:
    TextureManager();

    auto set_texture_vector(Vec<Texture> textures) -> void;
    // It is possible to get only one instance of this struct with this method.
    static auto get_instance() -> TextureManager*;
    static auto get_hud_instance() -> TextureManager*;
    auto bind_texture(u32 entity_id, u32 texture_id) -> void;
    auto load_texture_data(glvm::Texture& asset) -> void;
    auto get_texture_vector() -> Vec<Texture>&;
    auto unbind_texture(Material component, u32 entity) -> void;
};
} // namespace glvm

namespace glvm {
struct ComponentManager {
private:
    static ComponentManager* instance;
    static Mutex mutex;
    u32 number_of_base_components;

    ComponentManager();

    template<typename ComponentType>
    auto create_component_container() -> u32 {
        static u32 LOCAL_CONTAINER_ID = 0;
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
            std::make_shared<Vec<ComponentType>>()
        );
        // Create ID's component container.
        Vec<u32>* sparse_entities_map_to_components = new Vec<u32>;
        world_sparse_entities_map_to_components.push_back(
            sparse_entities_map_to_components
        );
        // Create dense map from component index to entity.
        Vec<u32>* dense_entities_map_to_components = new Vec<u32>;
        world_dense_components_map_to_entities.push_back(
            dense_entities_map_to_components
        );
        components_types.push_back(typeid(ComponentType).name());
        ++components_container_id;
        return LOCAL_CONTAINER_ID;
    }

public:
    inline static u32 components_container_id = 0;
    // Contains all local containers for different types of components.
    Vec<Rc<void>> world_components_container;
    // Contains all local container with IDs for different types of components.
    Vec<Vec<u32>*> world_sparse_entities_map_to_components;
    Vec<Vec<u32>*> world_dense_components_map_to_entities;

    Vec<const char*> components_types;
    bool is_components_collection_changed = true;

    ~ComponentManager();
    // No need to make a copy because of singleton property.
    ComponentManager(ComponentManager& component_manager) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const ComponentManager& component_manager) = delete;
    // It is possible to get only one instance of this struct with this method.
    static auto get_instance() -> ComponentManager*;

    template<typename ComponentType>
    auto create_component(const u32& entity) -> void {
        // Index for world components and world ID's containers.
        u32 local_container_id = 0;
        ComponentType component;
        local_container_id = create_component_container<ComponentType>();

        Vec<u32>& sparse = *static_cast<Vec<u32>*>(
            world_sparse_entities_map_to_components[local_container_id]
        );
        Vec<u32>& dense = *static_cast<Vec<u32>*>(
            world_dense_components_map_to_entities[local_container_id]
        );
        Vec<ComponentType>& components =
            *std::static_pointer_cast<Vec<ComponentType>>(
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

    auto check_availability(Vec<u32>& sparse, Vec<u32>& dense, u32 entity)
        -> bool;

    // Allows giving various components to a chosen entity.
    template<typename ComponentType1, typename ComponentType2, typename... Args>
    auto create_component(u32& entity) -> void {
        create_component<ComponentType2, Args...>(entity);
        create_component<ComponentType1>(entity);
    }

    template<typename ComponentType, typename... Args>
    auto collect_linked_entities() -> Vec<u32> {
        number_of_base_components = 0;
        u32 first_component_array_index =
            create_component_container<ComponentType>();
        Vec<u32>& dense = *static_cast<Vec<u32>*>(
            world_dense_components_map_to_entities[first_component_array_index]
        );

        if (dense.size() > 0) {
            ++number_of_base_components;
            number_of_base_components += sizeof...(Args);
        }
        Vec<u32> return_vector;
        for (u32 i = 0; i < dense.size(); ++i) {
            if (multi_check_availability<Args...>(dense[i])) {
                return_vector.push_back(dense[i]);
            }
        }
        return return_vector;
    }

    template<typename ComponentType, typename... Args>
    auto collect_unique_linked_entities() -> Vec<u32> {
        Vec<u32> base_sub_set_entities;
        base_sub_set_entities =
            collect_linked_entities<ComponentType, Args...>();
        u32 number_of_component_arrays = 0;
        for (u32 j = 0; j < base_sub_set_entities.size(); ++j) {
            number_of_component_arrays = 0;
            for (u32 i = 0; i < world_dense_components_map_to_entities.size();
                 ++i) {
                Vec<u32>& sparse = *static_cast<Vec<u32>*>(
                    world_sparse_entities_map_to_components[i]
                );
                Vec<u32>& dense = *static_cast<Vec<u32>*>(
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
    auto multi_check_availability(u32 entity) -> bool {
        return (multi_check_availability_base<Args>(entity) && ...);
    }

    template<typename ComponentType>
    auto multi_check_availability_base(u32 entity) -> bool {
        u32 component_array_index = create_component_container<ComponentType>();
        Vec<u32>& sparse = *static_cast<Vec<u32>*>(
            world_sparse_entities_map_to_components[component_array_index]
        );
        Vec<u32>& dense = *static_cast<Vec<u32>*>(
            world_dense_components_map_to_entities[component_array_index]
        );
        return check_availability(sparse, dense, entity);
    }

    template<typename ComponentType>
    auto is_component_exists(const u32& entity) -> bool {
        u32 local_container_id;
        local_container_id = create_component_container<ComponentType>();
        Vec<u32>& sparse = *static_cast<Vec<u32>*>(
            world_sparse_entities_map_to_components[local_container_id]
        );
        Vec<u32>& dense = *static_cast<Vec<u32>*>(
            world_dense_components_map_to_entities[local_container_id]
        );
        return check_availability(sparse, dense, entity);
    }

    template<typename ComponentType>
    auto get_component(const u32& entity) -> ComponentType* {
        u32 local_container_id;
        local_container_id = create_component_container<ComponentType>();
        Vec<u32>& sparse = *static_cast<Vec<u32>*>(
            world_sparse_entities_map_to_components[local_container_id]
        );
        Vec<u32>& dense = *static_cast<Vec<u32>*>(
            world_dense_components_map_to_entities[local_container_id]
        );
        Vec<ComponentType>& components =
            *std::static_pointer_cast<Vec<ComponentType>>(
                world_components_container[local_container_id]
            );
        if (check_availability(sparse, dense, entity)) {
            u32 component_index = sparse[entity];
            return &components[component_index];
        } else {
            return nullptr;
        }
    }

    // No need to delete the real component in this method, because systems
    // don't work with components lacking indices in the ordered container.
    template<typename ComponentType>
    auto remove_component(u32& entity) -> void {
        u32 local_container_id;
        local_container_id = create_component_container<ComponentType>();
        Vec<u32>& sparse = *static_cast<Vec<u32>*>(
            world_sparse_entities_map_to_components[local_container_id]
        );
        Vec<u32>& dense = *static_cast<Vec<u32>*>(
            world_dense_components_map_to_entities[local_container_id]
        );
        Vec<ComponentType>& components =
            *std::static_pointer_cast<Vec<ComponentType>>(
                world_components_container[local_container_id]
            );
        if (check_availability(sparse, dense, entity)) {
            assert(dense.size() == components.size());
            u32 index_in_dense_of_removable_entity = sparse[entity];
            u32 index_in_sparse_of_swappable_entity = dense.back();
            const ComponentType& component_from_last_index = components.back();
            dense[index_in_dense_of_removable_entity] =
                index_in_sparse_of_swappable_entity;
            dense.pop_back();
            components[index_in_dense_of_removable_entity] =
                component_from_last_index;
            components.pop_back();
            sparse[index_in_sparse_of_swappable_entity] =
                index_in_dense_of_removable_entity;
            is_components_collection_changed = true;
        }
    }

    auto remove_all_components(u32& entity) -> void {
        for (u32 i = 0; i < world_components_container.size(); ++i) {
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

    auto get_container_id() -> u32;

    template<typename ComponentType>
    auto get_component_container() -> Vec<ComponentType>* {
        return std::static_pointer_cast<Vec<ComponentType>>(
                   world_components_container
                       [create_component_container<ComponentType>()]
        )
            .get();
    }

    template<typename ComponentType>
    auto get_entity_container() -> Vec<u32>* {
        return static_cast<Vec<u32>*>(
            world_dense_components_map_to_entities
                [create_component_container<ComponentType>()]
        );
    }
};

} // namespace glvm

namespace glvm {
struct MeshAxisMaxAbsoluteValues {
    f32 absolute_x = 0.0f;
    f32 absolute_y = 0.0f;
    f32 absolute_z = 0.0f;

    f32 origin_offset_x = 0.0f;
    f32 origin_offset_y = 0.0f;
    f32 origin_offset_z = 0.0f;
};

struct MeshAxisLimitingValues {
    f32 lowest_x = FLT_MAX;
    f32 highest_x = -FLT_MAX;
    f32 lowest_y = FLT_MAX;
    f32 highest_y = -FLT_MAX;
    f32 lowest_z = FLT_MAX;
    f32 highest_z = -FLT_MAX;

    auto set_to_default_values() -> void {
        highest_x = -FLT_MAX;
        lowest_x = FLT_MAX;
        highest_y = -FLT_MAX;
        lowest_y = FLT_MAX;
        highest_z = -FLT_MAX;
        lowest_z = FLT_MAX;
    }

    auto compare_per_direction_and_set_to_maximum_value_by_module(
        Position& vertex
    ) -> void {
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

    auto compare_per_direction_and_set_to_maximum_value_by_module(
        Vector<f32, 3> position,
        f32 half_x,
        f32 half_y,
        f32 half_z
    ) -> void {
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
    ReadableTextures,
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
    u32 actual_attachment_description_number;
    VkAttachmentDescription attachment_descriptions[16];
    u32 actual_attachment_reference_number;
    VkAttachmentReference attachment_references[16];
    u32 actual_subpass_dependency_number;
    VkSubpassDependency subpass_dependencies[8];
};

struct GpuImage {
    VkImage image;
    VkDeviceMemory device_memory = {};
    Vec<VkImageView> views = {};
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
    u32 array_layers = 0;
    u32 width = 0;
    u32 height = 0;
};

// Metadata for descriptor bindings.
struct DescriptorBinding {
    VkDescriptorType vk_type;
    VkShaderStageFlags shader_stage_flag;
    u32 binding;
    u32 shader_descriptors_number;
    u32 global_descriptor_offset;
    VkDeviceSize ubo_chunk_size;
};

// Metadata for descriptor sets.
struct DescriptorSet {
    u32 actual_linked_descriptor_bindings_number;
    u32 host_descriptor_number;
    VkDescriptorSetLayout set_layout;
    static constexpr auto MAXIMUM_LINKED_DESCRIPTOR_BINDINGS_DS = 32;
    u32 descriptors_bindings_ids[MAXIMUM_LINKED_DESCRIPTOR_BINDINGS_DS];
    u32 descriptor_set_offset;
    bool is_texture;
};

struct Pipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    const char* vert_shader = nullptr;
    const char* frag_shader = nullptr;
    VkVertexInputBindingDescription binding_description;
    Array<VkVertexInputAttributeDescription, 5> attribute_descriptions;
    u32 actual_linked_descriptor_sets_number;
    static constexpr auto MAXIMUM_LINKED_DESCRIPTOR_SET_DS = 32;
    u32 linked_descriptor_set_ids[MAXIMUM_LINKED_DESCRIPTOR_SET_DS];
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
    Vector<f32, 3> pos;
    Vector<f32, 3> color;
    Vector<f32, 2> tex_coord;
    Vector<f32, 4> joint_indices;
    Vector<f32, 4> weights;

    static auto get_binding_description() -> VkVertexInputBindingDescription {
        VkVertexInputBindingDescription binding_description {};
        binding_description.binding = 0;
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        binding_description.stride = sizeof(Vertex);

        return binding_description;
    }

    static auto get_attribute_descriptions()
        -> Array<VkVertexInputAttributeDescription, 5> {
        Array<VkVertexInputAttributeDescription, 5> attribute_descriptions {};

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
        attribute_descriptions[3].offset = offsetof(Vertex, joint_indices);

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
    Vector<f32, 3> position;
    Vector<f32, 3> forward;
};

struct RenderActor {
    Matrix<f32, 4> model_matrix;
    Vec<Matrix<f32, 4>> joint_matrices;
    u32 mesh_id;
    u32 diffuse_texture_index;
    u32 specular_texture_index;
    Vector<f32, 3> ambient;
    f32 shininess;
};

struct RenderDirectionalLight {
    Matrix<f32, 4> directional_light_space_matrix;
    Vector<f32, 4> position;
    Vector<f32, 4> direction;

    Vector<f32, 4> ambient;
    Vector<f32, 4> diffuse;
    Vector<f32, 4> specular;
};

struct RenderSpotLight {
    Matrix<f32, 4> spot_light_space_matrix;
    Vector<f32, 3> position;
    Vector<f32, 3> direction;
    f32 cut_off;
    f32 outer_cut_off;

    Vector<f32, 3> ambient;
    Vector<f32, 3> diffuse;
    Vector<f32, 3> specular;

    f32 constant;
    f32 linear;
    f32 quadratic;
};

struct RenderPointLight {
    Matrix<f32, 4> point_light_space_matrix[CUBE_MAP_LAYER_NUMBER];
    Vector<f32, 3> position;

    Vector<f32, 3> ambient;
    Vector<f32, 3> diffuse;
    Vector<f32, 3> specular;

    f32 constant;
    f32 linear;
    f32 quadratic;
};

struct RenderHealth {
    Vector<f32, 3> position;
    f32 max_health;
    f32 current_health;
    u32 mesh_id;
};

struct RenderFont {
    Vector<f32, 3> position;
    Vec<char> font_string;
    f32 lifetime;
};

struct SlotData {
    Matrix<f32, 4> model;
    Vector<f32, 3> color;
};

struct RenderInventory {
    Vec<SlotData> slot_data;
    u32 inventory_texture_id;
    u32 mesh_id;
    u32 row;
    u32 col;
};

struct RenderItem {
    Matrix<f32, 4> model;
    u32 mesh_id;
    u32 diffuse_texture_id;
};

struct RenderCrosshair {
    Matrix<f32, 4> model;
    u32 mesh_id;
};

namespace glvm {
struct EntityManager {
private:
    static EntityManager* instance;
    static Mutex mutex;

    inline static u32 id = 0;
    Vec<u32> removed_entity_registry;
    Vec<u32> active_entity_registry;

    EntityManager();

public: // TODO: Delete this.
    ~EntityManager();
    // No need to make a copy because of singleton property.
    EntityManager(EntityManager& entity_manager) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const EntityManager& entity_manager) = delete;
    // It is possible to get only one instance of this struct with this method.
    static auto get_instance() -> EntityManager*;
    [[nodiscard]] auto create_entity() -> u32;
    auto remove_entity(u32& entity_id, ComponentManager* component_manager)
        -> void;
    bool is_entities_collection_changed = true;
};
} // namespace glvm

namespace glvm {
struct System {
    virtual ~System() {
    }

    virtual auto update() -> void = 0;
};
} // namespace glvm

extern glvm::Event global_event;

// Contains all maximum absolute axis values.
extern Vec<glvm::MeshAxisMaxAbsoluteValues> all_mesh_max_absolute_values;

extern glvm::EventStack global_input_stack;

extern i32 global_pointer_x;
extern i32 global_pointer_y;
extern i32 KEYS_PRESSED[6];

namespace glvm {
extern Vec<VkDescriptorSet> DESCRIPTOR_SETS_CHUNKS;
extern Vec<VkRenderPass> RENDER_PASSES;
extern Vec<Descriptor> GPU_DESCRIPTORS;
} // namespace glvm

namespace glvm {
enum DeactivatedSystems { DeactivatedMovementSystem };

struct SystemManager: public System {
private:
    static SystemManager* instance;
    static Mutex mutex;
    Vec<DeactivatedSystems> deactivated_systems;

    SystemManager();

public:
    ~SystemManager();
    // No need to make a copy because of singleton property.
    SystemManager(SystemManager& other) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const SystemManager& other) = delete;
    // It is possible to get only one instance of this struct with this method.
    static auto get_instance() -> SystemManager*;

    inline static u32 system_count = 0;
    Vec<System*> system_container;

    auto activate_system(System* system) -> void;
    auto deactivate_system(DeactivatedSystems system) -> void;
    auto return_system_to_activated_state(DeactivatedSystems system) -> void;

    auto update() -> void override;
};
} // namespace glvm

namespace glvm {
struct DamageSystem: public System {
public:
    auto update() -> void override;

    f32 delta_time;

    u32 cached_attackable_archetypes_number = 0;
    u32 cached_font_archetypes_number = 0;

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

    u64 attackable_required_mask = (1ul << ComponentsIndices::AttackComponent)
        | (1ul << ComponentsIndices::HealthComponent)
        | (1ul << ComponentsIndices::FontComponent);

    u64 font_required_mask = (1ull << ComponentsIndices::FontComponent);
};
} // namespace glvm

namespace glvm {
struct PhysicsSystem: public System {
public:
    f32 acceleration_of_gravity;
    f32 delta_time;
    f32& gravity;
    EventStack& input_stack;

    u32 cached_archetypes_number = 0;

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

    u64 required_mask = (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MoveComponent)
        | (1ul << ComponentsIndices::RigidBodyComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::MeshComponent);

    PhysicsSystem(f32& initial_gravity, EventStack& stack) :
        gravity(initial_gravity),
        input_stack(stack) {
    }

    // Sets the backtracking entity's transform Y to the ground entity's upper Y.

    // This update searches for entities referring to colliders and checks their
    // transform components for collisions; if a collision is detected, it
    // checks whether the backtracking entity has a gravity component to call
    // the gravity function.
    auto update() -> void override;
    auto repel(
        Transform& transform_component,
        f32& delta_time,
        Beholder& view,
        Event& event
    ) -> void;
};
} // namespace glvm

namespace glvm {
constexpr auto CROSSHAIR_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Material)
       + sizeof(CrosshairTagComponent));

struct CrosshairArchetype: Archetype {
    Transform transforms[CROSSHAIR_ARCH_CHUNK_SIZE];
    Mesh meshes[CROSSHAIR_ARCH_CHUNK_SIZE];
    Material materials[CROSSHAIR_ARCH_CHUNK_SIZE];
    CrosshairTagComponent crosshair_tag_components[CROSSHAIR_ARCH_CHUNK_SIZE];

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
constexpr auto DIRECTIONAL_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
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
constexpr auto ENEMY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
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
constexpr auto INVENTORY_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Mesh) + sizeof(Inventory) + sizeof(Material));

struct InventoryArchetype: Archetype {
    Transform transforms[INVENTORY_ARCH_CHUNK_SIZE];
    Mesh meshes[INVENTORY_ARCH_CHUNK_SIZE];
    Inventory inventories[INVENTORY_ARCH_CHUNK_SIZE];
    Material materials[INVENTORY_ARCH_CHUNK_SIZE];

    InventoryArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::MeshComponent] = meshes;
        components[ComponentsIndices::InventoryComponent] = inventories;
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
constexpr auto ITEM_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
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
constexpr auto LEVEL_CHUNK_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
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
constexpr auto PLAYER_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
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
constexpr auto POINT_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
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
constexpr auto PROJECTILE_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
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
    Health health[PROJECTILE_ARCH_CHUNK_SIZE];
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
        components[ComponentsIndices::HealthComponent] = health;
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
constexpr auto RIGID_BODY_ARCH_CHUNK_SIZE =
    ARCHETYPE_CHUNK_SIZE / (sizeof(glvm::Transform) + sizeof(glvm::RigidBody));

struct RigidBodyArch {
    Transform transforms[RIGID_BODY_ARCH_CHUNK_SIZE];
    RigidBody rigid_bodies[RIGID_BODY_ARCH_CHUNK_SIZE];
};
}; // namespace glvm

namespace glvm {
constexpr auto SPOT_LIGHT_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
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
constexpr auto STATIC_MESH_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
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

namespace glvm {
constexpr auto COLLIDER_ARCH_CHUNK_SIZE =
    ARCHETYPE_CHUNK_SIZE / (sizeof(Collider) + sizeof(ColliderFlags));

struct ColliderArchetype: Archetype {
    Collider colliders[COLLIDER_ARCH_CHUNK_SIZE];
    ColliderFlags collider_flags[COLLIDER_ARCH_CHUNK_SIZE];

    ColliderArchetype() {
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = collider_flags;

        mask = (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent);

        component_ids[0] = ComponentsIndices::ColliderComponent;
        component_ids[1] = ComponentsIndices::ColliderFlagsComponent;
        component_count = 2;
    }
};
}; // namespace glvm

namespace glvm {
constexpr auto DAMAGE_ARCH_CHUNK_SIZE =
    ARCHETYPE_CHUNK_SIZE / (sizeof(Attack) + sizeof(Health) + sizeof(Font));

struct DamageArchetype: Archetype {
    Attack attacks[DAMAGE_ARCH_CHUNK_SIZE];
    Health health[DAMAGE_ARCH_CHUNK_SIZE];
    Font fonts[DAMAGE_ARCH_CHUNK_SIZE];

    DamageArchetype() {
        components[ComponentsIndices::AttackComponent] = attacks;
        components[ComponentsIndices::HealthComponent] = health;
        components[ComponentsIndices::FontComponent] = fonts;

        mask = (1ull << ComponentsIndices::AttackComponent)
            | (1ull << ComponentsIndices::HealthComponent)
            | (1ull << ComponentsIndices::FontComponent);

        component_ids[0] = ComponentsIndices::AttackComponent;
        component_ids[1] = ComponentsIndices::HealthComponent;
        component_ids[2] = ComponentsIndices::FontComponent;
        component_count = 3;
    }
};
}; // namespace glvm

namespace glvm {
constexpr auto PHYSICS_ARCH_CHUNK_SIZE = ARCHETYPE_CHUNK_SIZE
    / (sizeof(Transform) + sizeof(Collider) + sizeof(ColliderFlags)
       + sizeof(Move) + sizeof(RigidBody));

struct PhysicsArchetype: Archetype {
    Transform transforms[PHYSICS_ARCH_CHUNK_SIZE];
    Collider colliders[PHYSICS_ARCH_CHUNK_SIZE];
    ColliderFlags collider_flags[PHYSICS_ARCH_CHUNK_SIZE];
    Move moves[PHYSICS_ARCH_CHUNK_SIZE];
    RigidBody rigid_bodies[PHYSICS_ARCH_CHUNK_SIZE];

    PhysicsArchetype() {
        components[ComponentsIndices::TransformComponent] = transforms;
        components[ComponentsIndices::ColliderComponent] = colliders;
        components[ComponentsIndices::ColliderFlagsComponent] = collider_flags;
        components[ComponentsIndices::MoveComponent] = moves;
        components[ComponentsIndices::RigidBodyComponent] = rigid_bodies;

        mask = (1ull << ComponentsIndices::TransformComponent)
            | (1ull << ComponentsIndices::ColliderComponent)
            | (1ull << ComponentsIndices::ColliderFlagsComponent)
            | (1ull << ComponentsIndices::MoveComponent)
            | (1ull << ComponentsIndices::RigidBodyComponent);

        component_ids[0] = ComponentsIndices::TransformComponent;
        component_ids[1] = ComponentsIndices::ColliderComponent;
        component_ids[2] = ComponentsIndices::ColliderFlagsComponent;
        component_ids[3] = ComponentsIndices::MoveComponent;
        component_ids[4] = ComponentsIndices::RigidBodyComponent;
        component_count = 5;
    }
};
}; // namespace glvm

#ifdef __linux__

namespace glvm {
struct WindowWaylandVulkan: WindowInterface {
    EventStack* input_stack = nullptr;
    WindowWaylandVulkan();
    auto init() -> void;
    auto close() -> void override;
    auto handle_event(Event& event) -> bool override;
    static auto create_anonymous_file(off_t size) -> i32;
    auto create_transparent_cursor(struct wl_shm* shm) -> struct wl_buffer*;
    auto swap_buffers() -> void override;
    auto clear_display() -> void override;
    auto cursor_lock(
        i32 pointer_x,
        i32 pointer_y,
        i32* out_offset_x,
        i32* out_offset_y
    ) -> void override;
    bool hide_and_lock_pointer = false;
    struct xdg_toplevel_listener xdg_toplevel_listener;
    struct xdg_surface_listener xdg_surface_listener;
    struct wl_callback_listener callback_listener;
    struct xdg_wm_base_listener shell_listener;
    struct wl_keyboard_listener keyboard_listener;
    struct zwp_relative_pointer_v1_listener relative_pointer_listener;
    struct wl_pointer_listener pointer_listener;
    struct wl_seat_listener seat_listener;
    struct wl_registry_listener registry_listener;
    struct wl_output_listener output_listener;

    struct wl_surface* wl_surface;
    struct wl_compositor* compositor;
    struct xdg_toplevel* xdg_toplevel;
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
    u16 width = 1280;
    u16 height = 720;
    u8 constant_byte = 0;
    u8 close_xdg_toplevel;
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_callback* frame_callback;
    struct xdg_surface* xdg_surface;
};

auto xdg_toplevel_configure(
    void* data,
    struct xdg_toplevel* xdg_toplevel,
    i32 new_width,
    i32 new_height,
    struct wl_array* state
) -> void;
auto xdg_toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel) -> void;
auto allocate_shared_memory(u64 size) -> i32;
auto resize(void* data) -> void;
auto draw(void* data) -> void;
auto xdg_surface_configure(
    void* data,
    struct xdg_surface* xdg_surface,
    u32 serial
) -> void;
auto new_frame(void* data, struct wl_callback* frame_callback, u32 callback_data)
    -> void;
auto shell_ping(void* data, struct xdg_wm_base* shell, u32 serial) -> void;
auto keyboard_keymap(
    void* data,
    struct wl_keyboard* keyboard,
    u32 format,
    i32 keymap_file_descriptor,
    u32 size
) -> void;
auto keyboard_enter(
    void* data,
    struct wl_keyboard* keyboard,
    u32 serial,
    struct wl_surface* surface,
    struct wl_array* keys
) -> void;
auto keyboard_leave(
    void* data,
    struct wl_keyboard* keyboard,
    u32 serial,
    struct wl_surface* surface
) -> void;
auto keyboard_key(
    void* data,
    struct wl_keyboard* keyboard,
    u32 serial,
    u32 time,
    u32 key,
    u32 state
) -> void;
auto keyboard_modifiers(
    void* data,
    struct wl_keyboard* keyboard,
    u32 serial,
    u32 mods_depressed,
    u32 mods_latched,
    u32 mods_locked,
    u32 group
) -> void;
auto keyboard_repeat_info(
    void* data,
    struct wl_keyboard* keyboard,
    i32 rate,
    i32 delay
) -> void;
auto pointer_enter(
    void* data,
    struct wl_pointer* pointer,
    u32 serial,
    struct wl_surface* surface,
    wl_fixed_t sx,
    wl_fixed_t sy
) -> void;
auto pointer_leave(
    void* data,
    struct wl_pointer* pointer,
    u32 serial,
    struct wl_surface* surface
) -> void;
auto pointer_motion(
    void* data,
    struct wl_pointer* pointer,
    u32 time,
    wl_fixed_t sx,
    wl_fixed_t sy
) -> void;
auto pointer_axis(
    void* data,
    struct wl_pointer* pointer,
    u32 time,
    u32 axis,
    wl_fixed_t value
) -> void;
auto pointer_button(
    void* data,
    struct wl_pointer* pointer,
    u32 serial,
    u32 time,
    u32 button,
    u32 state
) -> void;
auto handle_relative_motion(
    void* data,
    struct zwp_relative_pointer_v1* rel_pointer,
    u32 utime_hi,
    u32 utime_lo,
    wl_fixed_t dx,
    wl_fixed_t dy,
    wl_fixed_t dx_unaccel,
    wl_fixed_t dy_unaccel
) -> void;
auto seat_capabilities(void* data, struct wl_seat* seat, u32 capabilities)
    -> void;
auto seat_name(void* data, struct wl_seat* seat, const char* name) -> void;
auto registry_global(
    void* data,
    struct wl_registry* registry,
    u32 name,
    const char* interface,
    u32 version
) -> void;
auto registry_global_remove(void* data, struct wl_registry* registry, u32 name)
    -> void;

[[nodiscard]] auto initialize_wayland_window() -> WindowWaylandVulkan*;
}; // namespace glvm
#endif // __linux__

#ifdef __linux__

namespace glvm {
struct WindowXVulkan: public WindowInterface {
private:
    XWindowAttributes x_window_attributes;
    Window root_window;
    XSetWindowAttributes set_window_attributes;

public:
    Display* display;
    Window win;
    u32 width;
    u32 height;

    WindowXVulkan();
    ~WindowXVulkan();

    auto get_window() -> Window;
    auto get_display() -> Display*;
    auto cursor_lock(
        i32 pointer_x,
        i32 pointer_y,
        i32* out_offset_x,
        i32* out_offset_y
    ) -> void override;
    auto swap_buffers() -> void override;
    auto clear_display() -> void override;
    auto handle_event(Event& event) -> bool override;
    auto close() -> void override;
};
} // namespace glvm
#endif // __linux__

#ifdef __linux__

namespace glvm {
struct WindowXCBVulkan: public WindowInterface {
private:
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    u32 window;
    xcb_key_symbols_t* key_symbols;
    xcb_generic_event_t* next_generic_event = nullptr;

public:
    u32 width;
    u32 height;
    bool is_window_resize_read = false;

    WindowXCBVulkan();

    auto configure_window() -> void;
    auto hide_cursor() -> void;
    auto get_connection() -> xcb_connection_t*;
    auto get_window() -> u32;
    auto disconnect() -> void;

    auto swap_buffers() -> void override;
    auto clear_display() -> void override;
    auto handle_event(Event& event) -> bool override;
    auto close() -> void override;
    auto cursor_lock(
        i32 pointer_x,
        i32 pointer_y,
        i32* out_offset_x,
        i32* out_offset_y
    ) -> void override;
};
} // namespace glvm
#endif // __linux__

namespace glvm {
struct Renderer;

struct DebugVertex {
    f32 x, y, z;
    f32 r, g, b;
};

struct ImGuiOverlay {
public:
    explicit ImGuiOverlay(Renderer& renderer);

    auto init() -> void;
    auto shutdown() -> void;
    auto create_swap_chain_resources() -> void;
    auto destroy_swap_chain_resources() -> void;

    auto new_frame() -> void;
    auto record_command_buffer(VkCommandBuffer command_buffer, u32 image_index)
        -> void;

    [[nodiscard]] auto wants_mouse() const -> bool;

    [[nodiscard]] auto is_enabled() const -> bool {
        return initialized;
    }

    bool show_panel = true;
    bool show_actor_bounds = false;
    bool show_light_frustums = false;
    bool show_shadow_maps = false;
    bool show_spatial_grid = false;
    bool shadows_enabled = true;
    i32 shadow_map_mode = 0; // 0 = directional, 1 = spot.
    i32 shadow_map_light = 0;

private:
    Renderer& renderer;
    bool initialized = false;

    VkRenderPass render_pass = VK_NULL_HANDLE;
    Vec<VkFramebuffer> framebuffers;

    VkPipelineLayout line_layout = VK_NULL_HANDLE;
    VkPipeline line_pipeline = VK_NULL_HANDLE;

    VkBuffer vertex_buffer = VK_NULL_HANDLE;
    VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;
    void* vertex_buffer_mapped = nullptr;
    u32 line_vertex_count = 0;

    auto create_render_pass() -> void;
    auto create_line_pipeline() -> void;
    auto create_vertex_buffer() -> void;
    auto build_panel() -> void;
    auto build_debug_vertices() -> void;
    auto record_debug_draws(VkCommandBuffer command_buffer, u32 image_index)
        -> void;
    auto record_im_gui_draws(VkCommandBuffer command_buffer, u32 image_index)
        -> void;
};

} // namespace glvm

namespace glvm {
inline DescriptorSet DESCRIPTOR_SETS_CONFIG[32];
inline DescriptorBinding DESCRIPTOR_BINDINGS_CONFIG[32];
inline Pipeline PIPELINE_CONFIGS[32];
inline RenderPass RENDER_PASS_CONFIGS[32];
constexpr auto MAX_TEXTURES = 18;

inline auto vk_config_initializer() -> void {
    // Pipelines and their render passes. Put all metadata related to pipelines
    // here. Also needed to add metadata of descriptor sets and their bindings
    // that will be related to a specific pipeline.

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
        "../../../crates/glvm/assets/shaders/flat_shadow_map/vertFlatShadowMap.spv";
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
        "../../../crates/glvm/assets/shaders/flat_shadow_map/vertFlatShadowMap.spv";
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
        "../../../crates/glvm/assets/shaders/cube_shadow_map/vertCubeShadowMap.spv";
    PIPELINE_CONFIGS[PointLightPipeline].frag_shader =
        "../../../crates/glvm/assets/shaders/cube_shadow_map/fragCubeShadowMap.spv";
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
        "../../../crates/glvm/assets/shaders/hud/hud_vert.spv";
    PIPELINE_CONFIGS[HudPipeline].frag_shader =
        "../../../crates/glvm/assets/shaders/hud/hud_frag.spv";
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
        "../../../crates/glvm/assets/shaders/font/font_vert.spv";
    PIPELINE_CONFIGS[FontPipeline].frag_shader =
        "../../../crates/glvm/assets/shaders/font/font_frag.spv";
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
        "../../../crates/glvm/assets/shaders/hud_screen/vert_hud_screen.spv";
    PIPELINE_CONFIGS[HudScreenPipeline].frag_shader =
        "../../../crates/glvm/assets/shaders/hud_screen/frag_hud_screen.spv";
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
        "../../../crates/glvm/assets/shaders/ui/vert_ui.spv";
    PIPELINE_CONFIGS[UiPipeline].frag_shader =
        "../../../crates/glvm/assets/shaders/ui/frag_ui.spv";
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
        "../../../crates/glvm/assets/shaders/ui_icons/vert_ui_icons.spv";
    PIPELINE_CONFIGS[UiIconsPipeline].frag_shader =
        "../../../crates/glvm/assets/shaders/ui_icons/frag_ui_icons.spv";
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
        "../../../crates/glvm/assets/shaders/virtual_textures/virtualTexturesVert.spv";
    PIPELINE_CONFIGS[VirtualTexturesPipeline].frag_shader =
        "../../../crates/glvm/assets/shaders/virtual_textures/virtualTexturesFrag.spv";
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
        "../../../crates/glvm/assets/shaders/main_renderer/vert.spv";
    PIPELINE_CONFIGS[MainRenderPipeline].frag_shader =
        "../../../crates/glvm/assets/shaders/main_renderer/frag.spv";
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
        "../../../crates/glvm/assets/shaders/sdf/sdf_vert.spv";
    PIPELINE_CONFIGS[SdfPipeline].frag_shader =
        "../../../crates/glvm/assets/shaders/sdf/sdf_frag.spv";
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
    DESCRIPTOR_SETS_CONFIG[ReadableTextures]
        .actual_linked_descriptor_bindings_number = 1;
    DESCRIPTOR_SETS_CONFIG[ReadableTextures].host_descriptor_number =
        MAX_TEXTURES;
    DESCRIPTOR_SETS_CONFIG[ReadableTextures].is_texture = true;

    DESCRIPTOR_BINDINGS_CONFIG[21].vk_type =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DESCRIPTOR_BINDINGS_CONFIG[21].shader_stage_flag =
        VK_SHADER_STAGE_FRAGMENT_BIT;
    DESCRIPTOR_BINDINGS_CONFIG[21].binding = 0;
    DESCRIPTOR_BINDINGS_CONFIG[21].shader_descriptors_number = MAX_TEXTURES;
}
}; // namespace glvm

namespace glvm {
auto descriptor_set_builder() -> void;
auto pipeline_builder() -> void;
auto render_passes_builder() -> void;
}; // namespace glvm

namespace glvm {
auto make_entity(u32 id, u32 generation) -> u64;
auto get_id(u64 entity) -> u32;
auto get_gen(u64 entity) -> u32;
auto matches_required_mask(const u64 archetype_mask, const u64& system_mask)
    -> bool;

template<typename T>
auto unwrap_archetype(Archetype* arch, u64 mask, void (*func)(T*)) -> void {
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
auto create_debug_utils_messenger_ext(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger
) -> VkResult;
auto create_begin_debug_utils_label_ext(
    VkInstance instance,
    VkCommandBuffer command_buffer,
    const VkDebugUtilsLabelEXT* label_info
) -> void;
auto create_end_debug_utils_label_ext(
    VkInstance instance,
    VkCommandBuffer command_buffer
) -> void;
auto destroy_debug_utils_messenger_ext(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* allocator
) -> void;
auto set_debug_object_name(
    VkDevice device,
    const VkDebugUtilsObjectNameInfoEXT* object_name_info
) -> VkResult;
auto set_image_debug_object_name(
    VkDevice device,
    GpuImage image,
    String image_name
) -> void;
auto set_pipeline_debug_object_name(
    VkDevice device,
    VkPipeline pipeline,
    String pipeline_name
) -> void;
auto set_descriptor_set_object_name(
    VkDevice device,
    VkDescriptorSet descriptor_set,
    String descriptor_set_name,
    u32 index
) -> void;
auto set_debug_object_names(
    VkDevice device,
    const Vec<VkBuffer>& vertex_buffer_container,
    const Vec<VkBuffer>& index_buffer_container,
    const Vec<Descriptor>& gpu_descriptors,
    const Vec<u32>& font_indices_container,
    const Vec<VkBuffer>& font_vertex_buffer_container,
    const Vec<VkBuffer>& font_index_buffer_container
) -> void;
}; // namespace glvm

namespace glvm {
struct GridChunk {
    Vector<f32, 3> position;
    static constexpr auto SIZE = 32;
    Vec<u32> entities;
};

struct SpatialGrid {
    static const auto width = 8;
    static const auto height = 8;
    static const auto depth = 8;
    GridChunk grid[width][height][depth];
};

struct World {
    World();
    ~World();

    SpatialGrid spatial_grid;
    Vec<Archetype*> archetypes;
    Vec<EntityLocation> entity_locations;

    auto add_entity_to_archetype(u64 entity, Archetype* arch) -> void;
    auto remove_entity(u64 entity) -> void;
    auto search_cache_archetypes(
        u64 required_mask,
        Archetype* cached_archetypes[],
        u32& cached_archetypes_number
    ) -> void;
};

extern World world;
}; // namespace glvm

namespace glvm {

struct ArchetypeEntityManager {
    inline static u32 next_id = 0;
    Vec<u32> generations;
    Vec<u32> free_list;

    ArchetypeEntityManager();
    static auto get_instance() -> ArchetypeEntityManager*;

    auto create_entity() -> u64;
    auto remove_entity(u64 entity) -> void;
    auto is_alive(u64 entity) const -> bool;

private:
    static ArchetypeEntityManager* instance;
    static Mutex mutex;

    ~ArchetypeEntityManager();
};
}; // namespace glvm

namespace glvm {
struct InventorySystem: public System {
public:
    u32 crosshair_archetypes_number = 0;
    u32 inventory_archetypes_number = 0;

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

    u64 crosshair_required_mask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::CrosshairTagComponent);

    u64 inventory_required_mask =
        (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::InventoryComponent)
        | (1ull << ComponentsIndices::MeshComponent);

    auto update() -> void override;
    auto determine_swappable_status_and_slots(
        Item* item_component,
        Transform* inventory_transform_component,
        Vec<u32>& potential_occupied_slots,
        Transform* crosshair_transform_component,
        Point2D<i32> intersection_slot,
        Inventory* inventory_component,
        const f32 inventory_slot_scale
    ) -> i32;
    auto fill_inventory_slots(
        Item* item_component,
        const i32 item_width,
        const i32 item_height,
        Inventory* inventory_component,
        const i32 fill_value
    ) -> void;
    auto determine_swappable_field(
        Item* item_component,
        const i32 item_width,
        const i32 item_height,
        i32 pivot_row,
        i32 pivot_column,
        Inventory* inventory_component,
        Vec<u32>& potential_occupied_slots
    ) -> i32;
    auto calculate_basic_offset(
        const i32 item_axis_size,
        const f32 axis_value,
        const f32 crosshair_axis_position,
        const i32 axis_slot_index,
        const f32 inventory_slot_scale
    ) -> i32;
    auto check_crosshair_inventory_intersection(
        Transform* crosshair_transform_component,
        Transform* inventory_transform_component,
        Inventory* inventory_component,
        const f32 inventory_slot_scale,
        const f32 inventory_slot_half_scale
    ) -> bool;
    auto determine_actual_intersection_slot(
        Transform* crosshair_transform_component,
        Transform* inventory_transform_component,
        const f32 inventory_slot_scale,
        const f32 inventory_slot_half_scale
    ) -> Point2D<i32>;

    bool is_inventory_opened;
    i32* is_item_dragged;
    bool* is_left_mouse_button_released;
    bool is_left_mouse_button_pressed;
    f32 mouse_offset_x = 0;
    f32 mouse_offset_y = 0;
    // Window aspect ratio, set by engine each frame.
    f32 aspect_ratio = 0.0f;
    Archetype* crosshair_cached_archetype;
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
const auto MAX_FRAMES_IN_FLIGHT = 2;
const Vec<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

const Vec<const char*> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    "VK_KHR_shader_non_semantic_info"
};

#ifdef NDEBUG
const bool enable_validation_layers = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

struct QueueFamilyIndices {
    Option<u32> graphics_family;
    Option<u32> present_family;

    auto is_complete() -> bool {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    Vec<VkSurfaceFormatKHR> formats;
    Vec<VkPresentModeKHR> present_modes;
};

struct Renderer {
public:
    bool print = true;
    Vector<i32, 4> indirect_texture
        [INDIRECT_TEXTURE_WIDTH * INDIRECT_TEXTURE_HEIGHT / 4 + 1];
    Vec<u32> entities_collection_linked_trn_mat_mes_act;
    Vec<u32> entities_collection_linked_trn_po_l_mes_act;

    char glyphs[128] = {'A',  'B',  'C', 'D', 'E', 'F', 'G',  'H',  'I', 'J',
                        'K',  'L',  'M', 'N', 'O', 'P', 'Q',  'R',  'S', 'T',
                        'U',  'V',  'W', 'X', 'Y', 'Z', 'a',  'b',  'c', 'd',
                        'e',  'f',  'g', 'h', 'i', 'j', 'k',  'l',  'm', 'n',
                        'o',  'p',  'q', 'r', 's', 't', 'u',  'v',  'w', 'x',
                        'y',  'z',  '0', '1', '2', '3', '4',  '5',  '6', '7',
                        '8',  '9',  '.', ',', '"', '"', '\'', '\'', '"', '"',
                        '\'', '\'', '?', '!', '_', '$', '(',  ')',  '+', '-',
                        '/',  ':',  ';', '<', '>', '=', '[',  ']',  '\\'};
    // FIXME: Padding that may be needed for alignment.
    const Vec<Vertex> padding[128];
    const Vec<u32> symbol_g_indices = {0, 1, 2, 2, 1, 3};
    std::chrono::steady_clock::time_point start_time;

    Vec<Texture> initialize_texture_data;
    Vec<const char*> paths_array;
    Vec<const char*> paths_gltf;
    Vec<Vec<Vertex>> level_generated_vertices;
    Vec<Vec<u32>> level_generated_indices;

    Vec<Vec<Vertex>> vertices;
    // Wavefront .obj indices.
    Vec<Vec<u32>> indices;
    // GLTF indices.
    Vec<Vec<f32>> vertices_temp;
    // highest GLTF y.
    Vec<f32> highest_gltf_y;
    // Keep axis limiting values for every axis per mesh in current iteration
    // while initializing Wavefront .obj and GLTF.
    MeshAxisLimitingValues mesh_axis_limiting_values;
    Vec<Vec<Vec<Matrix<f32, 4>>>> joint_matrices_per_mesh;
    Vec<Vec<f32>> frames;
    bool is_inventory_opened = false;
    bool is_cursor_released = false;
    Vector<f32, 3> forward = {0.0f, 0.0f, -1.0f};
    f32 hud_screen_x = 0.0f;
    f32 hud_screen_y;

    u32 entities[32];
    Vec<RenderActor> actors;
    Vec<RenderDirectionalLight> directional_lights;
    Vec<RenderSpotLight> spot_lights;
    Vec<RenderPointLight> point_lights;
    Vec<RenderHealth> health_bars;
    Vec<RenderFont> fonts;
    Vec<RenderInventory> inventories;
    Vec<RenderItem> items;
    Vec<RenderCrosshair> crosshairs;
    Vec<RenderPlayer> players;
    RenderPlayer player;

    f32 yaw = -90.0f;
    f32 pitch = 0.0f;
    f32 prev_y = 0.0f;
    f32 current_y = 0.0f;
    f32 prev_x = 0.0f;
    f32 current_x = 0.0f;
    // Window aspect ratio, updated on resize.
    f32 aspect_ratio = 0.0f;
    i32 dragged_item_entity;

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

    Renderer();
    ~Renderer();

    auto create_texture_image() -> void;
    auto recreate_swap_chain() -> void;
    auto draw() -> void;
    auto set_mesh_data(Vec<const char*> paths, Vec<const char*> paths_gltf)
        -> void;
    auto set_projection_matrix(Matrix<f32, 4> new_projection_matrix) -> void;
    auto set_view_matrix(Matrix<f32, 4> new_view_matrix) -> void;
    auto initialize_game_level_vertices() -> void;
    auto run() -> void;

public:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    Matrix<f32, 4> view_matrix;
    Matrix<f32, 4> projection_matrix;
    ThreadPool* render_thread_pool;

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    VkWaylandSurfaceCreateInfoKHR create_wayland_surface_info;
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
    VkXlibSurfaceCreateInfoKHR create_xlib_surface_info;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
    VkXcbSurfaceCreateInfoKHR create_xcb_surface_info;
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
    Vec<VkImage> swap_chain_images;
    VkFormat swap_chain_image_format;
    VkExtent2D swap_chain_extent;
    Vec<VkImageView> swap_chain_image_views;
    Vec<VkFramebuffer> swap_chain_framebuffers;

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
    Vec<VkDescriptorSet> virtual_textures_ubo_descriptor_sets;
    Vec<VkDescriptorSet> virtual_textures_samplers_descriptor_sets;
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
    Vec<VkCommandPool> secondary_buffers_command_pools;
    VkCommandPool virtual_textures_command_pool;

    // Main pipeline depth.
    VkImage main_depth_pipeline_image;
    VkDeviceMemory main_depth_pipeline_image_memory;
    VkImageView main_depth_image_view;

    // Depth variables for shadow map.
public:
    u32 directional_light_number = 0;
    Vec<VkFramebuffer> directional_light_shadow_map_frame_buffers;
    VkBuffer shadow_map_directional_light_model_matrix_uniform_buffer;
    VkDeviceMemory
        shadow_map_directional_light_model_matrix_uniform_buffers_memory;
    Vec<GpuImage> directional_light_texture_images;

    Matrix<f32, 4> dir_light_space_matrix[DIRECTIONAL_LIGHTS_NUMBER];
    Matrix<f32, 4> spot_light_space_matrix[SPOT_LIGHTS_NUMBER];

    u32 point_light_number = 0;
    Vec<Vec<VkFramebuffer>> point_light_shadow_map_frame_buffers;
    VkBuffer shadow_map_point_light_model_matrix_uniform_buffer;
    VkDeviceMemory shadow_map_point_light_model_matrix_uniform_buffers_memory;
    Vec<GpuImage> point_light_texture_images;

    u32 spot_light_number = 0;
    Vec<VkFramebuffer> spot_light_shadow_map_frame_buffers;
    VkBuffer shadow_map_spot_light_model_matrix_uniform_buffer;
    VkDeviceMemory shadow_map_spot_light_model_matrix_uniform_buffers_memory;
    Vec<GpuImage> spot_light_texture_images;

    Vec<GpuImage> texture_images;
    VkSampler texture_sampler;
    VkSampler shadow_map_sampler;
    u32 frame_counter = 0;

    Vec<VkBuffer> vertex_buffer_container;
    Vec<VkDeviceMemory> vertex_buffer_memory_container;
    Vec<VkBuffer> index_buffer_container;
    Vec<VkDeviceMemory> index_buffer_memory_container;
    u32 wavefront_obj_counter = 0;
    u32 gltf_counter = 0;

    Vec<Vec<Vertex>> symbol_g_vertices_container;
    Vec<u32> font_indices_container;
    Vec<VkBuffer> font_vertex_buffer_container;
    Vec<VkDeviceMemory> font_vertex_buffer_memory_container;
    Vec<VkBuffer> font_index_buffer_container;
    Vec<VkDeviceMemory> font_index_buffer_memory_container;

    VkBuffer model_matrix_uniform_buffer;
    VkDeviceMemory model_matrix_uniform_buffers_memory;
    VkBuffer light_data_uniform_buffer;
    VkDeviceMemory light_data_uniform_buffers_memory;

    VkDescriptorPool descriptor_pool;
    const u32 matrix_ubo_descriptors_number = 500;
    const u32 hud_ubo_descriptor_number = 500;
    const u32 font_ubo_descriptor_number = 128;
    const u32 hud_screen_ubo_descriptor_number = 32;
    const u32 ui_ubo_descriptors_number = 64;
    const u32 virtual_textures_descriptors_number = 64;
    Vec<VkCommandBuffer> directional_light_command_buffers;
    Vec<VkCommandBuffer> spot_light_command_buffers;
    Vec<VkCommandBuffer> point_light_command_buffers;
    Vec<VkCommandBuffer> font_command_buffers;
    Vec<VkCommandBuffer> hud_command_buffers;
    Vec<VkCommandBuffer> main_render_command_buffers;
    Vec<VkCommandBuffer> directional_light_secondary_command_buffers;
    Vec<VkCommandBuffer> spot_light_secondary_command_buffers;
    Vec<VkCommandBuffer> point_light_secondary_command_buffers;
    Vec<VkCommandBuffer> virtual_textures_command_buffers;

    // Main render pipeline sync objects.
    Vec<VkSemaphore> image_available_semaphores;
    Vec<VkSemaphore> render_finished_semaphores;
    Vec<VkFence> in_flight_fences;

    // Hud render pipeline sync objects.
    Vec<VkSemaphore> hud_image_available_semaphores;
    Vec<VkSemaphore> hud_render_finished_semaphores;
    Vec<VkFence> hud_in_flight_fences;

    // Font render pipeline sync objects.
    Vec<VkSemaphore> font_image_available_semaphores;
    Vec<VkSemaphore> font_render_finished_semaphores;
    Vec<VkFence> font_in_flight_fences;

    // Directional light shadow map sync objects.
    Vec<VkSemaphore> directional_light_shadow_map_image_available_semaphores;
    Vec<VkSemaphore> directional_light_shadow_map_render_finished_semaphores;
    Vec<VkFence> directional_light_shadow_map_in_flight_fences;

    // Spotlight shadow map sync objects.
    Vec<VkSemaphore> spot_light_shadow_map_image_available_semaphores;
    Vec<VkSemaphore> spot_light_shadow_map_render_finished_semaphores;
    Vec<VkFence> spot_light_shadow_map_in_flight_fences;

    // Point light shadow map sync objects.
    Vec<VkSemaphore> point_light_shadow_map_image_available_semaphores;
    Vec<VkSemaphore> point_light_shadow_map_render_finished_semaphores;
    Vec<VkFence> point_light_shadow_map_in_flight_fences;

    // Virtual textures pipeline sync objects.
    Vec<VkSemaphore> virtual_textures_image_available_semaphores;
    Vec<VkSemaphore> virtual_textures_render_finished_semaphores;
    Vec<VkFence> virtual_textures_in_flight_fences;

    u32 current_frame = 0;
    u32 directional_light_current_frame = 0;
    u32 spot_light_current_frame = 0;
    u32 point_light_current_frame = 0;

    Mutex mutex0;
    Mutex mutex1;
    Mutex mutex2;
    Mutex shadow_map_passes_mutex;

    bool framebuffer_resized = false;

    auto init_window() -> void;
    auto init_vulkan() -> void;
    auto initialize_vertex_buffers_with_wavefront_data() -> void;
    auto initialize_vertex_buffers_with_gltf_data() -> void;
    auto initialize_vertex_buffers_with_font_data() -> void;
    auto cleanup_swap_chain() -> void;
    auto cleanup() -> void;
    auto create_instance() -> void;
    auto populate_debug_messenger_create_info(
        VkDebugUtilsMessengerCreateInfoEXT& create_info
    ) -> void;
    auto setup_debug_messenger() -> void;
    auto create_surface() -> void;
    auto pick_physical_device() -> void;
    auto create_logical_device() -> void;
    auto create_swap_chain() -> void;
    auto create_image_views() -> void;
    auto create_main_render_pass() -> void;
    auto create_descriptor_set_layout() -> void;
    auto create_graphics_pipeline() -> void;
    auto create_render_pass_framebuffers(
        Vec<VkImageView>& attachments,
        VkRenderPass& render_pass,
        VkFramebuffer& swap_chain_framebuffer,
        u32 width,
        u32 height
    ) -> void;
    auto create_framebuffers() -> void;
    auto create_command_pool(VkCommandPool& command_pool) -> void;
    auto create_depth_resources() -> void;
    auto create_directional_light_shadow_map_depth_resources() -> void;
    auto create_spot_light_shadow_map_depth_resources() -> void;
    auto create_point_light_shadow_map_depth_resources() -> void;
    auto find_supported_format(
        const Vec<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features
    ) -> VkFormat;
    auto find_depth_format() -> VkFormat;
    auto has_stencil_component(VkFormat format) -> bool;
    auto create_texture_image_view() -> void;
    auto create_texture_sampler() -> void;
    auto create_shadow_map_sampler() -> void;
    auto create_image_view(
        GpuImage image,
        u32 base_array_layers,
        u32 layer_count
    ) -> VkImageView;
    auto create_image(GpuImage& image) -> void;
    auto transition_image_layout(
        VkImage image,
        VkImageLayout old_layout,
        VkImageLayout new_layout
    ) -> void;
    auto transition_shadow_map_image_layout(
        VkImage image,
        VkImageLayout old_layout,
        VkImageLayout new_layout
    ) -> void;
    auto copy_buffer_to_image(
        VkBuffer& buffer,
        VkImage image,
        u32 width,
        u32 height
    ) -> void;
    auto create_vertex_buffer(
        VkBuffer& dst_vertex_buffer,
        VkDeviceMemory& dst_vertex_buffer_memory,
        Vec<Vertex>& vertex_data
    ) -> void;
    auto create_index_buffer(
        VkBuffer& dst_index_buffer,
        VkDeviceMemory& dst_index_buffer_memory,
        const Vec<u32>& index_data
    ) -> void;
    auto create_main_render_uniform_buffers() -> void;
    auto create_main_render_descriptor_pool() -> void;
    auto allocate_descriptor_sets(
        Vec<VkDescriptorSet>& descriptor_sets,
        VkDescriptorSetLayout set_layout,
        const u32 descriptor_sets_number,
        const u32 descriptor_offset
    ) -> void;
    auto update_descriptor_sets_ubo(
        VkBuffer ubo,
        const VkDeviceSize& ubo_struct_size,
        const u32& ubo_descriptors_number,
        i32 ubo_binding,
        Vec<VkDescriptorSet>& ubo_descriptor_sets,
        const u32 offset
    ) -> void;
    auto update_light_data_descriptor_sets(
        const DescriptorSet& current_descriptor_set1
    ) -> void;
    auto update_descriptor_sets_combined_image_sampler(
        const DescriptorSet& descriptor_set
    ) -> void;
    auto create_descriptor_image_info(
        const u32 descriptor_number,
        VkImageLayout image_layout,
        Vec<GpuImage>& texture_images,
        const u32 image_view_index,
        VkDescriptorImageInfo descriptor_image_infos[]
    ) -> void;
    auto create_descriptor_buffer_info(VkBuffer ubo, u32 offset, u32 range)
        -> VkDescriptorBufferInfo;
    auto create_main_render_descriptor_sets() -> void;
    auto create_buffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& buffer_memory
    ) -> void;
    auto begin_single_time_commands(VkCommandPool& command_pool)
        -> VkCommandBuffer;
    auto end_single_time_commands(
        VkCommandPool& command_pool,
        VkCommandBuffer& command_buffer
    ) -> void;
    auto copy_buffer(
        VkBuffer& src_buffer,
        VkBuffer& dst_buffer,
        VkDeviceSize size
    ) -> void;
    auto find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties)
        -> u32;
    auto create_command_buffers(
        VkCommandPool& command_pool,
        Vec<VkCommandBuffer>& command_buffers,
        u32 command_buffers_number,
        VkCommandBufferLevel command_buffer_level_flag
    ) -> void;
    auto execute_secondary_command_buffer(
        VkRenderPass render_pass,
        VkFramebuffer frame_buffer,
        VkExtent2D extent,
        VkCommandBuffer primary_command_buffer,
        VkCommandBuffer secondary_command_buffer
    ) -> void;
    auto update_hud_ubo(
        u32 offset,
        bool hud_exists,
        f32 highest_y,
        u32 health_counter
    ) -> void;
    auto update_hud_screen_ubo(u32 offset, u32 crosshair) -> void;
    auto update_sdf_ubo(u32 offset, u32 crosshair) -> void;
    auto update_ubo_ui(
        const u32 current_inventory_row,
        const u32 current_inventory_column,
        const u32 inventory,
        u32 offset
    ) -> void;
    auto update_ubo_icons_ui(u32 offset, u32 item) -> void;
    auto hud_record_command_buffer(
        VkCommandBuffer& command_buffer,
        u32 image_index
    ) -> void;
    auto ui_record_command_buffer(
        VkCommandBuffer& command_buffer,
        u32 image_index
    ) -> void;
    auto ui_icons_record_command_buffer(
        VkCommandBuffer& command_buffer,
        u32 image_index
    ) -> void;
    auto hud_screen_record_command_buffer(
        VkCommandBuffer& command_buffer,
        u32 image_index
    ) -> void;
    auto sdf_record_command_buffer(
        VkCommandBuffer& command_buffer,
        u32 image_index
    ) -> void;
    auto font_record_command_buffer(
        VkCommandBuffer& command_buffer,
        u32 image_index
    ) -> void;
    auto record_command_buffer(VkCommandBuffer& command_buffer, u32 image_index)
        -> void;
    auto create_sync_objects(
        Vec<VkSemaphore>& image_available_semaphores,
        Vec<VkSemaphore>& render_finished_semaphores,
        Vec<VkFence>& in_flight_fences
    ) -> void;
    auto update_directional_light_shadow_map_matrix_ubo(
        u32 current_image,
        u32 current_light,
        u32 actor
    ) -> void;
    auto update_spot_light_shadow_map_matrix_ubo(
        u32 current_image,
        u32 current_light,
        u32 actor
    ) -> void;
    auto update_point_light_shadow_map_matrix_ubo(
        u32 current_image,
        u32 current_light,
        u32 layer,
        u32 actor
    ) -> void;
    auto update_matrix_uniform_buffer(u32 offset, u32 actor) -> void;
    auto update_view_position_uniform_buffer(u32 current_image, u32 player)
        -> void;
    auto main_render_draw_frame() -> void;
    auto directional_light_shadow_map_draw_frame() -> void;
    auto spot_light_shadow_map_draw_frame() -> void;
    auto point_light_shadow_map_draw_frame() -> void;
    auto directional_light_record_command_buffer(
        Vec<VkCommandBuffer>& command_buffer,
        u32 current_frame
    ) -> void;
    auto spot_light_record_command_buffer(
        Vec<VkCommandBuffer>& command_buffer,
        u32 current_frame
    ) -> void;
    auto point_light_record_command_buffer(
        Vec<VkCommandBuffer>& command_buffers,
        u32 current_frame
    ) -> void;
    auto create_shader_module(const Vec<char>& code) -> VkShaderModule;
    auto choose_swap_surface_format(
        const Vec<VkSurfaceFormatKHR>& available_formats
    ) -> VkSurfaceFormatKHR;
    auto choose_swap_present_mode(
        const Vec<VkPresentModeKHR>& available_present_modes
    ) -> VkPresentModeKHR;
    auto choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
        -> VkExtent2D;
    auto query_swap_chain_support(VkPhysicalDevice device)
        -> SwapChainSupportDetails;
    auto is_device_suitable(VkPhysicalDevice device) -> bool;
    auto check_device_extension_support(VkPhysicalDevice device) -> bool;
    auto find_queue_families(VkPhysicalDevice device) -> QueueFamilyIndices;
    auto get_required_extensions() -> Vec<const char*>;
    auto check_validation_layer_support() -> bool;
    auto create_descriptor_buffer_info(
        VkBuffer ubo,
        const VkDeviceSize& ubo_struct_size,
        const VkDeviceSize& offset_step
    ) -> VkDescriptorBufferInfo;
    auto create_descriptor_image_info(
        const GpuImage& texture_image,
        VkImageLayout layout,
        u32 texture_index,
        VkSampler texture_sampler
    ) -> VkDescriptorImageInfo;
    static auto read_file(const String& filename) -> Vec<char>;
    static auto VKAPI_ATTR VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data
    ) -> VkBool32;
    auto compute_model_matrix(Transform* transform) -> Matrix<f32, 4>;
    auto clear_vk_image(GpuImage* texture_images) -> void;
};

}; // namespace glvm

namespace glvm {
struct ItemSystem: public System {
    u32 inventory_archetypes_number = 0;
    u32 item_archetypes_number = 0;
    u32 crosshair_archetypes_number = 0;

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

    u64 inventory_required_mask =
        (1ull << ComponentsIndices::InventoryComponent);

    u64 item_required_mask = (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ItemComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent);

    u64 crosshair_required_mask = (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::CrosshairTagComponent);

    auto update() -> void;
    auto put_item2x2(Inventory* inventory_component, u32 item_entity) -> bool;

    EventStack* input_stack;
    bool is_inventory_opened;
    i32* dragged_item_entity;
    bool* is_left_mouse_button_released;
    bool is_left_mouse_button_pressed;
    f32 mouse_offset_x = 0;
    f32 mouse_offset_y = 0;
};
} // namespace glvm

namespace glvm {
auto box_collider(
    const Vector<f32, 3> backtracking_position,
    const Vector<f32, 3> compared_position,
    const f32 backtracking_scale,
    const f32 compared_scale,
    const MeshAxisMaxAbsoluteValues& backtracking_mesh_axis_max_absolute_values,
    const MeshAxisMaxAbsoluteValues& compared_mesh_axis_max_absolute_values
) -> bool;

auto compute_box_corner_bound_points(
    const MeshAxisMaxAbsoluteValues entity_chunk_bounds,
    Vector<f32, 3> entity_position,
    const f32 scale
) -> Vec<Vector<f32, 3>>;

template<typename T>
auto is_exist(const Vec<T>& array, const T& element) -> bool {
    for (u32 i0 = 0; i0 < array.size(); ++i0) {
        if (element == array[i0]) {
            return true;
        }
    }

    return false;
}

auto set_mesh_bounds(MeshAxisLimitingValues mesh_axis_limiting_values) -> void;
auto create_projectile(
    const Vector<f32, 3>& projectile_position,
    const Vector<f32, 3>& projectile_forward,
    const MeshHandle& mesh_handle,
    const Material& material,
    const Damage& damage,
    const EntityLocation& projectile_location
) -> void;
}; // namespace glvm

namespace glvm {
struct MovementSystem: public System {
public:
    f32 delta_frame_time;
    f32 gravity;
    EventStack& input_stack;
    f32 prev_delta_x = 0.0f;
    f32 prev_x = 0.0f;
    f32 current_x = 0.0f;
    Vector<f32, 3> prev_forward;

    u32 player_archetypes_number = 0;
    u32 rigid_body_contained_archetypes_number = 0;

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

    u64 player_required_mask = (1ull << ComponentsIndices::PlayerTagComponent);
    u64 rigid_body_required_mask =
        (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RigidBodyComponent)
        | (1ul << ComponentsIndices::MoveComponent);

    MovementSystem(EventStack& input_stack);

    auto update() -> void;
    auto calculate_vector_rl(Beholder& beholder) -> Vector<f32, 3>;
    auto calculate_vector_fb(Beholder& beholder, Event& event)
        -> Vector<f32, 3>;
};
} // namespace glvm

namespace glvm {
struct ProceduralLevelGeneratingSystem: public System {
public:
    u32 level_number = 0;
    bool stupid_flag = false;
    u32 previous_half_x_rand = 0;
    u32 previous_half_z_rand = 0;
    Vector<f32, 3> current_level_position = {5.0f, 0.0f, 15.0f};
    Vector<f32, 3> transition_bridge_position = {0.0f, 0.0f, 0.0f};
    u32 next_level_transition_direction = 0;
    u32 previous_iteration_transition_bridge_direction = 0;

    u32 cached_level_chunk_arch_number = 0;
    u32 cached_player_arch_number = 0;

    struct ProceduralLevelArchView {
        Archetype* cached_level_chunk_arch = nullptr;
        Archetype* cached_player_arch = nullptr;
    } arch_view;

    struct ComponentsView {
        Transform* player_transforms = nullptr;
    } components_view;

    u64 player_required_mask = (1ull << ComponentsIndices::PlayerTagComponent);

    u64 required_mask = (1ull << ComponentsIndices::TransformComponent)
        | (1ull << ComponentsIndices::MaterialComponent)
        | (1ull << ComponentsIndices::MeshComponent)
        | (1ull << ComponentsIndices::ColliderComponent)
        | (1ull << ComponentsIndices::ColliderFlagsComponent)
        | (1ull << ComponentsIndices::LevelChunkTagComponent);

    Vec<MeshHandle> mesh_handles;
    Vec<TextureHandle> texture_handlers;

    Vec<Vec<Vertex>> level_generated_vertices;
    // Wavefront .obj indices.
    Vec<Vec<u32>> level_generated_indices;
    // Keep axis limiting values for every axis per mesh in current iteration
    // while initializing Wavefront .obj and GLTF.
    MeshAxisLimitingValues mesh_axis_limiting_values;
    // Contains maximum coordinate value in every direction for all generated
    // levels.
    MeshAxisLimitingValues coordinate_maximum_value_per_direction;

    auto update() -> void;
    auto set_half_extents_from_direction(
        f32& half_x,
        f32& half_z,
        const f32& transition_bridge_half_width,
        const f32& transition_bridge_half_height,
        const f32& next_level_transition_direction
    ) -> void;
    auto generate_level(
        const u32 level_half_x,
        const u32 level_half_y,
        const u32 level_half_z,
        const f32 transition_bridge_half_width,
        const f32 transition_bridge_half_height
    ) -> void;
    auto generate_transition_bridge(
        const u32 level_half_x,
        const u32 level_half_y,
        const u32 level_half_z,
        const f32 transition_bridge_half_width,
        const f32 transition_bridge_half_height
    ) -> void;
    auto make_cube_object_vertices(
        Vector<f32, 4> joint_indices,
        Vector<f32, 4> weights,
        f32 half_x,
        f32 half_y,
        f32 half_z,
        Vec<Vertex>& destination_vertices_container
    ) -> void;
    auto check_collision_intersection_with_maximum_coordinates(
        Vector<f32, 3> position,
        f32 half_x,
        f32 half_y,
        f32 half_z
    ) -> bool;
};
} // namespace glvm

namespace glvm {
struct CollisionSystem: public System {
public:
    f32 delta_time;
    f32 gravity;
    bool is_inventory_opened;
    bool* is_item_dragged;
    bool is_left_mouse_button_pressed;
    bool* is_left_mouse_button_released;
    EventStack& input_stack;
    Archetype* cached_archetypes[32];
    u32 cached_archetypes_number = 0;

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

    u64 required_mask = (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);

    CollisionSystem(EventStack& stack) : input_stack(stack) {
    }

    auto update() -> void override;
    auto upper_actor_check(
        Vector<f32, 3> backtracking_position,
        Vector<f32, 3> compared_position,
        f32 backtracking_scale,
        f32 compared_scale,
        MeshHandle backtracking_mesh_handle,
        MeshHandle compared_mesh_handle
    ) -> bool;
};
} // namespace glvm

namespace glvm {
struct EnemySystem: public System {
public:
    u32 player_archetypes_number = 0;
    u32 enemy_archetypes_number = 0;
    u32 projectile_archetypes_number = 0;

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

    u64 player_required_mask = (1ull << ComponentsIndices::PlayerTagComponent);

    u64 enemy_required_mask = (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::StateComponent)
        | (1ul << ComponentsIndices::EnemyComponent);

    u64 projectile_required_mask =
        (1ull << ComponentsIndices::ProjectileTagComponent);

    auto update() -> void override;
    SoundEngine* sound_engine;
    Vec<TextureHandle> texture_handlers;
    Vec<MeshHandle> mesh_handles;
    f32 projectile_cooldown = 5.0f;
    f32 delta_frame_time;
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

struct ProjectileSystem: public System {
public:
    f32 yaw = -90.0f;
    f32 pitch = 0.0f;
    bool first_mouse = true;
    EventStack& input_stack;
    Vec<TextureHandle> texture_handlers;
    Vec<MeshHandle> mesh_handles;
    SoundEngine* sound_engine;
    f32 projectile_cooldown = 2.0f;
    f32 delta_frame_time;
    bool is_inventory_opened;

    u32 player_archetypes_number = 0;
    u32 projectile_archetypes_number = 0;

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

    u64 player_required_mask = (1ull << ComponentsIndices::PlayerTagComponent);

    u64 projectile_required_mask =
        (1ull << ComponentsIndices::ProjectileTagComponent);

    ProjectileSystem(EventStack& input_stack);
    auto update() -> void override;
    template<typename T>
        requires UnitOrEnemy<T> && HasAttack<T>
    static auto mark_as_attacked(
        T* arch,
        Damage* projectile_damage,
        u32 entity_index
    ) -> void;
};

template<typename T>
    requires UnitOrEnemy<T> && HasAttack<T>
auto ProjectileSystem::mark_as_attacked(
    T* arch,
    Damage* projectile_damage,
    u32 entity_index
) -> void {
    arch->attacks[entity_index].damage = projectile_damage->maximum_damage;
}

} // namespace glvm

namespace glvm {

struct SpatialGridSystem: public System {
private:
    Archetype* cached_archetypes[32];
    u32 cached_archetypes_number = 0;
    bool is_initialized = false;

    struct SpatialGridComponentsView {
        Transform* transforms = nullptr;
        Mesh* meshes = nullptr;
    } view;

    u64 required_mask = (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);

    auto update() -> void override;
};

}; // namespace glvm

namespace glvm {
enum RendererType { VulkanRenderer };

struct Engine {
private:
    static Engine* instance;
    static Mutex mutex;

    Chrono* chrono;
    SoundEngine* sound_engine;
    std::thread sound_thread;
    AtomicBool running_sound {false};
    f32 delta_frame_time;
    f32 gravity;
    bool is_left_mouse_button_pressed;
    Vec<Texture> texture_vector;
    Vec<const char*> paths_array;
    Vec<const char*> paths_gltf;
    u32 mesh_id = 0;
    bool is_already_cached;
    bool is_inventory_key_held = false;
    bool was_inventory_opened = false;
    bool is_cursor_hidden = false;
    f32 hud_screen_x = 0.0f;
    f32 hud_screen_y;
    // If don't have any dragged item then this variable have value of -1.
    i32 dragged_item_entity = -1;
    f32 yaw = -90.0f;
    f32 pitch = 0.0f;
    f32 previous_mouse_offset_x = 0.0f;
    f32 previous_mouse_offset_y = 0.0f;
    Renderer* vulkan_renderer;
    SpatialGridSystem* spatial_grid_system;
    CollisionSystem* collision_system;
    MovementSystem* movement_system;
    PhysicsSystem* physics_system;
    ProjectileSystem* projectile_system;
    DamageSystem* damage_system;
    EnemySystem* enemy_system;
    ItemSystem* item_system;
    ProceduralLevelGeneratingSystem* procedural_level_generating_system;
    InventorySystem* inventory_system;
    Archetype* cached_directional_light_archetypes[32];
    u32 directional_light_archetypes_number = 0;
    u64 directional_light_required_mask =
        (1ul << ComponentsIndices::DirectionalLightComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cached_spot_light_archetypes[32];
    u32 spot_light_archetypes_number = 0;
    u64 spot_light_required_mask =
        (1ul << ComponentsIndices::SpotLightComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cached_point_light_archetypes[32];
    u32 point_light_archetypes_number = 0;
    u64 point_light_required_mask =
        (1ul << ComponentsIndices::PointLightComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cached_animation_actors_archetypes[32];
    u32 animation_actors_archetypes_number = 0;
    u64 animated_actors_required_mask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::AnimationComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cached_static_actors_archetypes[32];
    u32 static_actors_archetypes_number = 0;
    u64 static_actors_required_mask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::StaticMeshTagComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cached_player_archetypes[32];
    u32 player_archetypes_number = 0;
    u64 player_required_mask = (1ul << ComponentsIndices::PlayerTagComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ViewComponent);
    Archetype* cached_animation_archetypes[32];
    u32 animation_archetypes_number = 0;
    u64 animation_required_mask = (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::AnimationComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cached_crosshair_actors_archetypes[32];
    u32 crosshair_actors_archetypes_number = 0;
    u64 crosshair_required_mask =
        (1ul << ComponentsIndices::CrosshairTagComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cached_level_chunk_actors_archetypes[32];
    u32 level_chunk_actors_archetypes_number = 0;
    u64 level_chunk_required_mask =
        (1ul << ComponentsIndices::MaterialComponent)
        | (1ul << ComponentsIndices::LevelChunkTagComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cached_projectile_actors_archetypes[32];
    u32 projectile_actors_archetypes_number = 0;
    u64 projectile_required_mask =
        (1ul << ComponentsIndices::ProjectileBundleComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MeshComponent);
    Archetype* cached_item_actors_archetypes[32];
    u32 item_actors_archetypes_number = 0;
    u64 rotation_item_required_mask = (1ul << ComponentsIndices::ItemComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::RotationComponent)
        | (1ul << ComponentsIndices::MaterialComponent);
    Archetype* cached_inventory_archetypes[32];
    u32 inventory_archetypes_number = 0;
    u64 inventory_required_mask = (1ul << ComponentsIndices::InventoryComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::MaterialComponent);
    Archetype* cached_item_archetypes[32];
    u32 item_archetypes_number = 0;
    u64 item_required_mask = (1ul << ComponentsIndices::ItemComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent)
        | (1ul << ComponentsIndices::ColliderComponent)
        | (1ul << ComponentsIndices::ColliderFlagsComponent)
        | (1ul << ComponentsIndices::MaterialComponent);
    Archetype* cached_health_bars_archetypes[32];
    u32 health_bars_archetypes_number = 0;
    u64 health_bars_required_mask = (1ul << ComponentsIndices::HealthComponent)
        | (1ul << ComponentsIndices::MeshComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    Archetype* cached_fonts_archetypes[32];
    u32 fonts_archetypes_number = 0;
    u64 font_required_mask = (1ul << ComponentsIndices::FontComponent)
        | (1ul << ComponentsIndices::TransformComponent);
    f64 fps_accumulator = 0;
    Engine();

public:
    Vec<MeshHandle> mesh_handles;
    Vec<TextureHandle> texture_handlers;
    u32 wavefront_obj_counter = 0;

    ~Engine();

    // No need to make copy because of singleton property.
    Engine(Engine& other) = delete;
    // Don't need assignment operator because of singleton property.
    void operator=(const Engine& other) = delete;
    // It is possible to get only one instance of this struct with this method.
    static auto get_instance() -> Engine*;
    auto game_loop() -> void;
    auto event_queue_flush() -> void;
    auto render_vulkan() -> void;
    auto enlarge_frame_accumulator(f32 value) -> void;
    auto set_view_matrix() -> void;
    auto set_projection_matrix() -> void;
    [[nodiscard]] auto update_animation_frames(
        Animation* animation_component,
        u32 mesh_id
    ) -> Vec<Matrix<f32, 4>>;
    auto update_directional_light_space_matrix_shadow_map_ubo(
        DirectionalLightComponent* light
    ) -> Matrix<f32, 4>;
    auto update_spot_light_space_matrix_shadow_map_ubo(SpotLightComponent* light)
        -> Matrix<f32, 4>;
    auto update_point_light_space_matrix_shadow_map_ubo(
        PointLightComponent* light,
        u32 layer
    ) -> Matrix<f32, 4>;
    auto update_data_ubo_ui(
        const u32 current_inventory_row,
        const u32 current_inventory_column,
        Inventory* inventory_component,
        Transform* slot_transform_component,
        Mesh* mesh_component
    ) -> SlotData;
    auto update_data_ubo_icons_ui(
        Transform* item_transform_component,
        Collider* item_collider_component,
        Item* item_component,
        const u32 row_inventory,
        const u32 column_inventory,
        Transform* inventory_transform_component,
        Mesh* item_mesh,
        i32 item_entity
    ) -> Matrix<f32, 4>;
    auto update_data_hud_screen_ubo(Transform* cursor_transform)
        -> Matrix<f32, 4>;
    auto set_frame_data() -> void;
    auto load_wavefront_obj() -> void;
    auto calculate_mesh_bounds(const Vector<f32, 4>& animated_vertex) -> void;
    auto is_model_cache_exists(const String& model_file_path) -> bool;
    auto write_models_cache(const String& model_file_path) -> void;
    auto initialize_gltf() -> void;
    auto initialize_font_data() -> void;
    auto compute_model_matrix(Transform* transform, Rotation* rotation)
        -> Matrix<f32, 4>;
    auto compute_hud_screen_coordinates() -> void;
    auto load_texture_from_file(const char* path_to_texture_component)
        -> TextureHandle;
    auto load_texture_from_address(
        u32 width,
        u32 height,
        u32 data_length,
        u8* data
    ) -> TextureHandle;
    auto load_mesh_from_obj(const char* mesh_path) -> MeshHandle;
    auto load_mesh_from_gltf(const char* path_to_mesh) -> MeshHandle;
    auto load_mesh() -> MeshHandle;
    auto game_kill() -> void;
};
} // namespace glvm
