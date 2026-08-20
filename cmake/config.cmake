if(NOT MSVC AND NOT EMSCRIPTEN AND NOT APPLE)
  find_program(LLD_PATH NAMES ld.lld lld-20 lld-18 lld)
  if(LLD_PATH)
    add_link_options(-fuse-ld=lld)
    message(STATUS "Linker: lld (${LLD_PATH})")
  else()
    message(STATUS "Linker: lld not found, using default")
  endif()
endif()

# GCC's -fmodules-ts emits duplicate std::logic_error symbols on MinGW static
# linking; this project doesn't use C++ modules, so disable module scanning.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # MinGW + static libstdc++: GCC 16 emits a strong definition of
  # std::logic_error's copy ctor (C++26 constexpr-exceptions branch) that
  # collides with libstdc++.a(cow-stdexcept.o).
  add_link_options(-Wl,--allow-multiple-definition)
endif()

find_program(CCACHE_PATH ccache)
if(CCACHE_PATH AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
  set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PATH}" CACHE STRING "")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PATH}" CACHE STRING "")
  message(STATUS "Compiler cache: ccache (${CCACHE_PATH})")
endif()

option(GLVM_UNITY_BUILD "Enable unity builds" ON)
if(GLVM_UNITY_BUILD)
  message(STATUS "Unity build: ON")
endif()

if(GLVM_SANITIZERS)
  if(MSVC)
    # MSVC only supports AddressSanitizer (no UBSan)
    set(GLVM_SAN_CFLAGS /fsanitize=address)
    set(GLVM_SAN_LFLAGS /fsanitize=address)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(GLVM_SAN_CFLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
    set(GLVM_SAN_LFLAGS -fsanitize=address,undefined)
  else()
    message(WARNING "Sanitizers not supported by ${CMAKE_CXX_COMPILER_ID}, ignored")
    set(GLVM_SANITIZERS OFF)
  endif()

  if(GLVM_SANITIZERS)
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_FLAGS "${GLVM_SAN_CFLAGS}")
    set(CMAKE_REQUIRED_LINK_OPTIONS "${GLVM_SAN_LFLAGS}")
    check_cxx_source_compiles("int main() { return 0; }" GLVM_SAN_LINKABLE)
    unset(CMAKE_REQUIRED_FLAGS)
    unset(CMAKE_REQUIRED_LINK_OPTIONS)
    if(NOT GLVM_SAN_LINKABLE)
      message(WARNING "Sanitizer runtime not available with ${CMAKE_CXX_COMPILER_ID}, disabling GLVM_SANITIZERS")
      set(GLVM_SANITIZERS OFF)
    endif()
  endif()
endif()

if(GLVM_SANITIZERS)
  if(MSVC)
    add_compile_options($<$<CONFIG:Debug>:${GLVM_SAN_CFLAGS}>)
    add_link_options($<$<CONFIG:Debug>:${GLVM_SAN_LFLAGS}>)
  else()
    add_compile_options(
      "$<$<CONFIG:Debug>:-fsanitize=address,undefined>"
      "$<$<CONFIG:Debug>:-fno-omit-frame-pointer>"
    )
    add_link_options("$<$<CONFIG:Debug>:-fsanitize=address,undefined>")
  endif()
  message(STATUS "Sanitizers: ON (Debug builds)")
endif()
