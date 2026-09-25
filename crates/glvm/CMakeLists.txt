project(glvm VERSION 0.1.0 LANGUAGES C;CXX)

include(cmake/dependencies.cmake)

add_library(glvm STATIC
  src/glvm.cpp
)

# target_compile_options(glvm PRIVATE -fno-exceptions -fno-rtti)

target_include_directories(glvm
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(glvm
  PUBLIC
    glvm_log
    imgui
    rusty
    volk
    Vulkan::Headers
)

# Vulkan functions are loaded at runtime by volk: no vk* prototypes from
# vulkan.h, everything goes through volk's function pointers. PUBLIC so all
# consumers (examples, tests) include the headers the same way.
target_compile_definitions(glvm PUBLIC VK_NO_PROTOTYPES)

if(WIN32)
  target_link_libraries(glvm PUBLIC dwmapi gdi32 winmm)
endif()

if(TARGET glvm-wayland-protocols)
  target_link_libraries(glvm
    PUBLIC
      glvm-wayland-protocols
      PkgConfig::WAYLAND_CLIENT
  )
endif()

if(UNIX AND NOT APPLE)
  target_link_libraries(glvm
    PUBLIC
      PkgConfig::ALSA
      PkgConfig::X11
      PkgConfig::XCB
      PkgConfig::XCB_CURSOR
      PkgConfig::XCB_KEYSYMS
  )
endif()

# [tests]
include(cmake/tests.cmake)
