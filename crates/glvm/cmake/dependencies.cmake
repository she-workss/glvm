# Vulkan without a preinstalled SDK: headers and the volk loader come from
# CPM, so MSVC builds work with nothing but a compiler. volk loads
# vulkan-1.dll at runtime, no link-time dependency on the SDK.
CPMAddPackage(
  NAME Vulkan-Headers
  GITHUB_REPOSITORY KhronosGroup/Vulkan-Headers
  GIT_TAG vulkan-sdk-1.4.357.0
  SYSTEM YES
)

CPMAddPackage(
  NAME volk
  GITHUB_REPOSITORY zeux/volk
  GIT_TAG vulkan-sdk-1.4.357.0
  OPTIONS
    "VOLK_INSTALL OFF"
    # Headers come from the Vulkan-Headers package above (linked below),
    # never from a host SDK: same headers on every machine.
    "VOLK_PULL_IN_VULKAN OFF"
  SYSTEM YES
)

if(volk_ADDED)
  # volk.c includes <vulkan/vulkan.h>; give it the CPM headers.
  target_link_libraries(volk PRIVATE Vulkan::Headers)
  target_include_directories(volk PUBLIC
    $<BUILD_INTERFACE:${volk_SOURCE_DIR}>
  )
  # volk declares/defines the vk* globals (incl. platform surface entry
  # points) behind VK_USE_PLATFORM_* guards: the volk TU and every consumer
  # including volk.h must see the same guard, hence PUBLIC.
  if(WIN32)
    target_compile_definitions(volk PUBLIC VK_USE_PLATFORM_WIN32_KHR)
  elseif(UNIX AND NOT APPLE)
    # Exactly one Linux backend: glvm.hpp picks the window class and the
    # VkSurface creator from this macro.
    if(GLVM_WINDOW_SYSTEM STREQUAL "X11")
      target_compile_definitions(volk PUBLIC VK_USE_PLATFORM_XLIB_KHR)
    elseif(GLVM_WINDOW_SYSTEM STREQUAL "XCB")
      target_compile_definitions(volk PUBLIC VK_USE_PLATFORM_XCB_KHR)
    else()
      target_compile_definitions(volk PUBLIC VK_USE_PLATFORM_WAYLAND_KHR)
    endif()
  endif()
endif()

CPMAddPackage(
  NAME imgui
  GITHUB_REPOSITORY ocornut/imgui
  GIT_TAG v1.92.9b
  DOWNLOAD_ONLY YES
  SYSTEM YES
)

if(imgui_ADDED AND NOT TARGET imgui)
  set(_imgui_sources
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
  )
  if(WIN32)
    list(APPEND _imgui_sources ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp)
  endif()
  add_library(imgui STATIC ${_imgui_sources})
  target_include_directories(imgui
    PUBLIC
      ${imgui_SOURCE_DIR}
      ${imgui_SOURCE_DIR}/backends
  )
  # imgui_impl_vulkan.cpp calls vk* directly: with VK_NO_PROTOTYPES those
  # resolve to volk's runtime-loaded pointers (loaded by glvm at startup).
  # IMGUI_IMPL_VULKAN_USE_VOLK makes the backend include volk.h itself, so
  # it uses volk globals instead of demanding ImGui_ImplVulkan_LoadFunctions.
  target_compile_definitions(imgui PUBLIC
    VK_NO_PROTOTYPES
    IMGUI_IMPL_VULKAN_USE_VOLK
  )
  target_link_libraries(imgui PUBLIC volk Vulkan::Headers)
endif()

if(UNIX AND NOT APPLE)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
  pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)
  pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
  pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb)
  pkg_check_modules(XCB_CURSOR REQUIRED IMPORTED_TARGET xcb-cursor)
  pkg_check_modules(XCB_KEYSYMS REQUIRED IMPORTED_TARGET xcb-keysyms)

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
    unstable/relative-pointer/relative-pointer-unstable-v1
    unstable/pointer-constraints/pointer-constraints-unstable-v1
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
