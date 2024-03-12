cmake_minimum_required(VERSION 3.0)
project(GLVM
    VERSION 0.1.0
    DESCRIPTION "GLVM Engine"
    LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

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
        -Wall
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
        -Wall
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
include_directories(textures)

# Find packages
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

# Define the executable
add_executable(${PROJECT_NAME}
    GLPointer.c
    src/ComponentManager.cpp
    src/Engine.cpp
    src/EngineMain.cpp
    src/EntityManager.cpp
    src/Event.cpp
    src/GraphicAPI/Opengl.cpp
    src/GraphicAPI/Vulkan.cpp
    src/JsonParser.cpp
    src/MeshManager.cpp
    src/ShaderProgram.cpp
    src/SoundEngineFactory.cpp
    src/SystemManager.cpp
    src/Systems/AnimationSystem.cpp
    src/Systems/CameraSystem.cpp
    src/Systems/CollisionSystem.cpp
    src/Systems/GUISystem.cpp
    src/Systems/MovementSystem.cpp
    src/Systems/PhysicsSystem.cpp
    src/Systems/ProjectileSystem.cpp
    src/TextureManager.cpp
    src/TimerCreator.cpp
    src/WavefrontObjParser.cpp
    src/WinApi/ChronoWin.cpp
    src/WinApi/SoundEngineWaveform.cpp
    src/WinApi/WindowWinOpengl.cpp
    src/WinApi/WindowWinVulkan.cpp
    textures/glvm.cpp
    textures/sample1.cpp
    textures/sample2.cpp
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    gdi32
    opengl32
    winmm
    ${Vulkan_LIBRARY}
)

# Additional linker flags
target_link_options(${PROJECT_NAME} PRIVATE
    -static
    -static-libgcc
    -static-libstdc++
    -static-libgcc
    -static-libstdc++
)
