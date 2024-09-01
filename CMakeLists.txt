cmake_minimum_required(VERSION 3.9)
project(GLVM
    VERSION 0.1.0
    DESCRIPTION "GLVM Engine"
    LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

include(CTest)
enable_testing()

# # Create `cppcheck` directory in each build directory to store cppcheck reports
# file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cppcheck)

# # Show Cppcheck status
# find_program(cppcheck NAMES cppcheck)
# message(STATUS "Cppcheck: ${cppcheck}")

# # Enable Cppcheck flags
# if(NOT(cppcheck MATCHES "NOTFOUND"))
#     set(CMAKE_CXX_CPPCHECK "${cppcheck}"
#         "--platform=native"
#         "--enable=all"
#         "--inconclusive"
#         "--inline-suppr"
#         "--cppcheck-build-dir=${CMAKE_BINARY_DIR}/cppcheck"
#         "--quiet"
#         "--suppress=missingIncludeSystem"
#         "--suppress=unmatchedSuppression"
#         "--suppress=unusedFunction"
#         "--template='{file}:{line}: {severity}: {message}\\n{code}'"
#         "--template-location='{file}:{line}: note: {info}\\n{code}'")
# endif()

# Setup all compilation flags
if((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR
    (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    set(CXX_FLAGS
        -O2
        # -march=native
        -Wall
        -Wnrvo
        -Wdangling-reference
        -Wtemplate-id-cdtor
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wenum-compare
        -Wextra
        # -Werror
        -Wfloat-equal
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpedantic
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo)
        # -fanalyzer)

    set(C_FLAGS
        -O2
        # -march=native
        -Wall
        -Wnrvo
        -Wdangling-reference
        -Wtemplate-id-cdtor
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wenum-compare
        # -Werror
        -Wextra
        -Wfloat-equal
        -Wpedantic
        -Wredundant-decls
        -Wsign-conversion)
        # -fanalyzer)

    add_compile_options(
        "$<$<COMPILE_LANGUAGE:C>:${C_FLAGS}>"
        "$<$<COMPILE_LANGUAGE:CXX>:${CXX_FLAGS}>")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /WX)
endif()

# Strip binary
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Sanitize options
if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-fsanitize=null -fno-omit-frame-pointer)
endif()

# Include directories
include_directories(include)
include_directories(assets/textures)

# Find packages
find_package(Vulkan REQUIRED)

# Define the executable
add_executable(glvm
    assets/textures/glvm.cpp
    assets/textures/sample1.cpp
    assets/textures/sample2.cpp
    src/component_manager.cpp
    src/engine.cpp
    src/engine_main.cpp
    src/entity_manager.cpp
    src/event.cpp
    src/graphic_api/vulkan.cpp
    src/json_parser.cpp
    src/mesh_manager.cpp
    src/sound_engine_factory.cpp
    src/system_manager.cpp
    src/systems/animation_system.cpp
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

# Enable LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

if(supported)
    set_property(TARGET glvm PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO/LTO not supported: <${error}>")
endif()

# Link libraries
target_link_libraries(glvm
    gdi32
    winmm
    ${Vulkan_LIBRARY}
)

# Additional linker flags
target_link_options(glvm PRIVATE
    -static
    -static-libgcc
    -static-libstdc++
    -static-libgcc
    -static-libstdc++
)
