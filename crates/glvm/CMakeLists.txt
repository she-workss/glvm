project(glvm VERSION 0.1.0 LANGUAGES CXX)

include(cmake/dependencies.cmake)

add_library(glvm STATIC
  src/ArchetypedECS/ArchECS_Utils.cpp
  src/ArchetypedECS/ArchECS_World.cpp
  src/ArchetypedECS/ArchetypeEntityManager.cpp
  src/ArchetypedECS/ArchetypeInterface.cpp
  src/Common/CommonFunctions.cpp
  src/ComponentManager.cpp
  src/Engine.cpp
  src/EntityManager.cpp
  src/Event.cpp
  src/GraphicAPI/RenderData.cpp
  src/GraphicAPI/VkBuilders.cpp
  src/GraphicAPI/VkDebugUtils.cpp
  src/GraphicAPI/Vulkan.cpp
  src/JsonParser.cpp
  src/MeshManager.cpp
  src/ProceduralLevelGeneratingSystem.cpp
  src/SoundEngineFactory.cpp
  src/SystemManager.cpp
  src/Systems/AnimationSystem.cpp
  src/Systems/CollisionSystem.cpp
  src/Systems/DamageSystem.cpp
  src/Systems/EnemySystem.cpp
  src/Systems/InventorySystem.cpp
  src/Systems/ItemSystem.cpp
  src/Systems/MovementSystem.cpp
  src/Systems/PhysicsSystem.cpp
  src/Systems/ProjectileSystem.cpp
  src/Systems/SpatialGridSystem.cpp
  src/TextureManager.cpp
  src/ThreadPool.cpp
  src/TimerCreator.cpp
  src/WinApi/ChronoWin.cpp
  src/WinApi/SoundEngineWaveform.cpp
  src/WinApi/WindowWinVulkan.cpp
  src/VertexData.cpp
  src/WavefrontObjParser.cpp
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
    rusty
    gdi32
    winmm
    ${Vulkan_LIBRARY}
)
