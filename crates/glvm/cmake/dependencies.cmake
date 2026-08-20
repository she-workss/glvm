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
