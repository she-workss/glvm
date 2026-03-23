project(glvm VERSION 0.1.0 LANGUAGES CXX)

include(cmake/dependencies.cmake)

add_library(glvm STATIC
  ../../assets/textures/glvm.cpp
  ../../assets/textures/sample1.cpp
  ../../assets/textures/sample2.cpp
  src/component_manager.cpp
  src/engine.cpp
  src/entity_manager.cpp
  src/event.cpp
  src/graphic_api/vulkan.cpp
  src/json_parser.cpp
  src/mesh_manager.cpp
  src/sound_engine_factory.cpp
  src/system_manager.cpp
  src/systems/camera_system.cpp
  src/systems/collision_system.cpp
  src/systems/movement_system.cpp
  src/systems/physics_system.cpp
  src/systems/projectile_system.cpp
  src/texture_manager.cpp
  src/timer_creator.cpp
  src/wavefront_obj_parser.cpp
  src/win_api/chrono_win.cpp
  src/win_api/sound_engine_waveform.cpp
  src/win_api/window_win_vulkan.cpp
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
    rusty
    gdi32
    winmm
    ${Vulkan_LIBRARY}
)
