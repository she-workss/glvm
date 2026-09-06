project(glvm_log VERSION 0.1.0 LANGUAGES CXX)

add_library(glvm_log STATIC
  src/lib.cpp
)

# target_compile_options(glvm_log PRIVATE -fno-exceptions -fno-rtti)

if(GLVM_UNITY_BUILD)
  set_target_properties(glvm_log PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_BATCH_SIZE 8
  )
endif()

target_include_directories(glvm_log
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(glvm_log
  PUBLIC
    rusty
)

# [tests]
include(cmake/tests.cmake)
