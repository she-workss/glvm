# Find packages
find_package(Vulkan REQUIRED)

CPMAddPackage(
  NAME imgui
  GITHUB_REPOSITORY ocornut/imgui
  GIT_TAG v1.92.9b
  DOWNLOAD_ONLY YES
  SYSTEM YES
)

if(imgui_ADDED AND NOT TARGET imgui)
  add_library(imgui
    STATIC
      ${imgui_SOURCE_DIR}/imgui.cpp
      ${imgui_SOURCE_DIR}/imgui_demo.cpp
      ${imgui_SOURCE_DIR}/imgui_draw.cpp
      ${imgui_SOURCE_DIR}/imgui_tables.cpp
      ${imgui_SOURCE_DIR}/imgui_widgets.cpp
      ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
      ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  )
  target_include_directories(imgui
    PUBLIC
      ${imgui_SOURCE_DIR}
      ${imgui_SOURCE_DIR}/backends
  )
endif()

if(UNIX AND NOT APPLE)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)

  find_program(WAYLAND_SCANNER NAMES wayland-scanner REQUIRED)

  CPMAddPackage(
    NAME wayland-protocols
    GIT_REPOSITORY https://gitlab.freedesktop.org/wayland/wayland-protocols.git
    GIT_TAG 1.45
    DOWNLOAD_ONLY YES
    SYSTEM YES
  )

  set(_wl_gen_dir ${CMAKE_CURRENT_BINARY_DIR}/generated/wayland)
  file(MAKE_DIRECTORY ${_wl_gen_dir})

  set(_wl_protos
    stable/xdg-shell/xdg-shell
    unstable/relative-pointer-unstable-v1/relative-pointer-unstable-v1
    unstable/pointer-constraints-unstable-v1/pointer-constraints-unstable-v1
  )

  foreach(_proto ${_wl_protos})
    get_filename_component(_name ${_proto} NAME)
    set(_xml ${wayland-protocols_SOURCE_DIR}/${_proto}.xml)
    set(_hdr ${_wl_gen_dir}/${_name}-client-protocol.h)
    set(_src ${_wl_gen_dir}/${_name}-client-protocol.c)
    add_custom_command(
      OUTPUT ${_hdr}
      COMMAND ${WAYLAND_SCANNER} client-header ${_xml} ${_hdr}
      DEPENDS ${_xml}
    )
    add_custom_command(
      OUTPUT ${_src}
      COMMAND ${WAYLAND_SCANNER} private-code ${_xml} ${_src}
      DEPENDS ${_xml}
    )
    list(APPEND _wl_sources ${_hdr} ${_src})
  endforeach()

  add_library(glvm-wayland-protocols STATIC ${_wl_sources})
  target_include_directories(glvm-wayland-protocols PUBLIC ${_wl_gen_dir})
endif()
