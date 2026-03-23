project(rusty VERSION 0.1.0 LANGUAGES CXX)

add_library(rusty
  src/lib.cpp
)

target_compile_options(rusty PRIVATE -fno-exceptions -fno-rtti)

if(GLVM_UNITY_BUILD)
  set_target_properties(rusty PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_BATCH_SIZE 8
  )
endif()

target_include_directories(rusty
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
