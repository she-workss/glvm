project(glvm VERSION 0.1.0 LANGUAGES CXX)

include(cmake/dependencies.cmake)

add_library(glvm STATIC
  src/archetyped_ecs/arch_ecs_utils.cpp
  src/archetyped_ecs/arch_ecs_world.cpp
  src/archetyped_ecs/archetype_entity_manager.cpp
  src/archetyped_ecs/archetype_interface.cpp
  src/common/common_functions.cpp
  src/component_manager.cpp
  src/engine.cpp
  src/entity_manager.cpp
  src/event.cpp
  src/graphic_api/render_data.cpp
  src/graphic_api/vk_builders.cpp
  src/graphic_api/vk_debug_utils.cpp
  src/graphic_api/imgui_overlay.cpp
  src/graphic_api/vulkan.cpp
  src/json_parser.cpp
  src/mesh_manager.cpp
  src/procedural_level_generating_system.cpp
  src/sound_engine_factory.cpp
  src/system_manager.cpp
  src/systems/collision_system.cpp
  src/systems/damage_system.cpp
  src/systems/enemy_system.cpp
  src/systems/inventory_system.cpp
  src/systems/item_system.cpp
  src/systems/movement_system.cpp
  src/systems/physics_system.cpp
  src/systems/projectile_system.cpp
  src/systems/spatial_grid_system.cpp
  src/texture_manager.cpp
  src/thread_pool.cpp
  src/timer_creator.cpp
  src/win_api/chrono_win.cpp
  src/win_api/sound_engine_waveform.cpp
  src/win_api/window_win_vulkan.cpp
  src/vertex_data.cpp
  src/wavefront_obj_parser.cpp
  ../../assets/textures/chelik.cpp
  ../../assets/textures/container2_specular.cpp
  ../../assets/textures/container2.cpp
  ../../assets/textures/crosshair.cpp
  ../../assets/textures/fontAtlas.cpp
  ../../assets/textures/gray.cpp
  ../../assets/textures/inventorySlot.cpp
  ../../assets/textures/tileset.cpp
  ../../assets/textures/witch.cpp
)

# target_compile_options(glvm PRIVATE -fno-exceptions -fno-rtti)

if(GLVM_UNITY_BUILD)
  set_target_properties(glvm PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_BATCH_SIZE 8
  )
endif()

target_include_directories(glvm
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(glvm
  PUBLIC
    dwmapi
    gdi32
    imgui
    rusty
    winmm
    ${Vulkan_LIBRARY}
)

# [tests]
include(cmake/tests.cmake)
