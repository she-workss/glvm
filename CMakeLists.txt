# [package]
cmake_minimum_required(VERSION 3.28)
project(glvm_cpp
  VERSION 0.1.0
  DESCRIPTION "GLVM Game Engine in C++26"
  LANGUAGES C CXX
)

if(MSVC)
  add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:/std:c++latest>
    $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
  )
  set(CMAKE_CXX_STANDARD_REQUIRED OFF)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++2c>)
  set(CMAKE_CXX_STANDARD_REQUIRED OFF)
else()
  set(CMAKE_CXX_STANDARD 26)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  link_libraries(stdc++exp)
endif()

option(GLVM_BUILD_DOCS "Build API documentation with Doxygen" ${PROJECT_IS_TOP_LEVEL})
option(GLVM_BUILD_TESTS "Build GLVM tests" ${PROJECT_IS_TOP_LEVEL})
option(GLVM_BUILD_EXAMPLES "Build GLVM examples" ${PROJECT_IS_TOP_LEVEL})
option(GLVM_BUILD_BENCHES "Build GLVM benchmarks" ${PROJECT_IS_TOP_LEVEL})
option(GLVM_UNITY_BUILD "Enable unity builds" ${PROJECT_IS_TOP_LEVEL})
option(GLVM_SANITIZERS
  "Enable sanitizers in Debug builds (ASan, UBSan)"
  ${PROJECT_IS_TOP_LEVEL}
)

# Like .cargo/config.toml
include(cmake/config.cmake)

# [dependencies]
include(cmake/dependencies.cmake)

# [docs]
include(cmake/docs.cmake)

# [workspace]
include(cmake/workspace.cmake)

# [tests]
include(cmake/tests.cmake)

# [examples]
include(cmake/examples.cmake)

# [benches]
include(cmake/benches.cmake)
