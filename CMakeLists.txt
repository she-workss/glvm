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
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++23>)
  set(CMAKE_CXX_STANDARD_REQUIRED OFF)
else()
  set(CMAKE_CXX_STANDARD 23)
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

# # # # Create `cppcheck` directory in each build directory to store cppcheck reports
# # file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cppcheck)

# # # # Show Cppcheck status
# # find_program(cppcheck NAMES cppcheck)
# # message(STATUS "Cppcheck: ${cppcheck}")

# # # Enable Cppcheck flags
# # if(NOT(cppcheck MATCHES "NOTFOUND"))
# #     set(CMAKE_CXX_CPPCHECK "${cppcheck}"
# #         "--platform=native"
# #         "--enable=all"
# #         "--inconclusive"
# #         "--inline-suppr"
# #         "--cppcheck-build-dir=${CMAKE_BINARY_DIR}/cppcheck"
# #         "--quiet"
# #         "--suppress=missingIncludeSystem"
# #         "--suppress=unmatchedSuppression"
# #         "--suppress=unusedFunction"
# #         "--template='{file}:{line}: {severity}: {message}\\n{code}'"
# #         "--template-location='{file}:{line}: note: {info}\\n{code}'")
# # endif()

# # Setup all compilation flags
# if((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR
#   (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
#   set(CXX_FLAGS
#     -O2
#     -Wall
#     -Wcast-align
#     -Wcast-qual
#     -Wconversion
#     -Wctor-dtor-privacy
#     -Wdangling-reference
#     -Wenum-compare
#     -Wextra
#     -Wfloat-equal
#     -Wnon-virtual-dtor
#     -Wnrvo
#     -Wold-style-cast
#     -Woverloaded-virtual
#     -Wpedantic
#     -Wredundant-decls
#     -Wsign-conversion
#     -Wsign-promo
#     -Wtemplate-id-cdtor
#     # -fanalyzer
#     # -march=native
#     # -Werror
#   )

#   set(C_FLAGS
#     -fanalyzer
#     -O2
#     -Wall
#     -Wcast-align
#     -Wcast-qual
#     -Wconversion
#     -Wdangling-reference
#     -Wenum-compare
#     -Wextra
#     -Wfloat-equal
#     -Wnrvo
#     -Wpedantic
#     -Wredundant-decls
#     -Wsign-conversion
#     -Wtemplate-id-cdtor
#     # -march=native
#     # -Werror
#   )

#   add_compile_options(
#     "$<$<COMPILE_LANGUAGE:C>:${C_FLAGS}>"
#     "$<$<COMPILE_LANGUAGE:CXX>:${CXX_FLAGS}>"
#   )
# elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
#   add_compile_options(/W4 /WX)
# endif()

# # Strip binary
# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

# # Enable LTO
# include(CheckIPOSupported)
# check_ipo_supported(RESULT supported OUTPUT error)

# if(supported)
#   set_property(TARGET glvm PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
# else()
#   message(STATUS "IPO/LTO not supported: <${error}>")
# endif()

# # Additional linker flags
# target_link_options(glvm PRIVATE
#   -static
#   -static-libgcc
#   -static-libstdc++
#   -static-libgcc
#   -static-libstdc++
# )
