project(glvm VERSION 0.1.0 LANGUAGES CXX)

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
    dwmapi
    gdi32
    imgui
    rusty
    winmm
    ${Vulkan_LIBRARY}
)

if(TARGET glvm-wayland-protocols)
  target_link_libraries(glvm
    PUBLIC
      glvm-wayland-protocols
      PkgConfig::WAYLAND_CLIENT
  )
endif()

# [tests]
include(cmake/tests.cmake)
