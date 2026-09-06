# Builds GLSL shaders to SPIR-V at build time with glslangValidator
# (Vulkan SDK). Replaces the old per-directory compile.sh scripts, so no
# checked-in .spv files are needed. Must be included after the glvm target
# exists (workspace.cmake).

find_program(
  GLVM_GLSLANG_VALIDATOR
  NAMES glslangValidator
  HINTS "$ENV{VULKAN_SDK}/bin" "$ENV{VULKAN_SDK}/Bin"
)
if(NOT GLVM_GLSLANG_VALIDATOR)
  message(
    FATAL_ERROR
    "glslangValidator not found. Install the Vulkan SDK or put it on PATH."
  )
endif()

set(GLVM_SHADER_SRC_DIR ${PROJECT_SOURCE_DIR}/crates/glvm/assets/shaders)
set(GLVM_SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shaders)

# Entries are "<subdir>/<source>=<output.spv>", mirroring the old compile.sh
# names. Same flags as the old scripts (-V -g).
set(_glvm_shaders
  "cube_shadow_map/cubeShadowMap.vert=vertCubeShadowMap.spv"
  "cube_shadow_map/cubeShadowMap.frag=fragCubeShadowMap.spv"
  "debug/debug.vert=debug_vert.spv"
  "debug/debug.frag=debug_frag.spv"
  "flat_shadow_map/flatShadowMap.vert=vertFlatShadowMap.spv"
  "flat_shadow_map/flatShadowMap.frag=fragFlatShadowMap.spv"
  "font/font_shader.vert=font_vert.spv"
  "font/font_shader.frag=font_frag.spv"
  "hud/hud_shader.vert=hud_vert.spv"
  "hud/hud_shader.frag=hud_frag.spv"
  "hud_screen/shader_hud_screen.vert=vert_hud_screen.spv"
  "hud_screen/shader_hud_screen.frag=frag_hud_screen.spv"
  "main_renderer/shader.vert=vert.spv"
  "main_renderer/shader.frag=frag.spv"
  "sdf/sdf.vert=sdf_vert.spv"
  "sdf/sdf.frag=sdf_frag.spv"
  "ui/ui_shader.vert=vert_ui.spv"
  "ui/ui_shader.frag=frag_ui.spv"
  "ui_icons/ui_icons_shader.vert=vert_ui_icons.spv"
  "ui_icons/ui_icons_shader.frag=frag_ui_icons.spv"
  "virtual_textures/virtualTextures.vert=virtualTexturesVert.spv"
  "virtual_textures/virtualTextures.frag=virtualTexturesFrag.spv"
)

set(_glvm_spv_outputs)
foreach(_entry IN LISTS _glvm_shaders)
  string(REPLACE "=" ";" _parts "${_entry}")
  list(GET _parts 0 _rel_src)
  list(GET _parts 1 _out_name)
  get_filename_component(_subdir "${_rel_src}" DIRECTORY)
  set(_src "${GLVM_SHADER_SRC_DIR}/${_rel_src}")
  set(_out "${GLVM_SHADER_OUT_DIR}/${_subdir}/${_out_name}")
  add_custom_command(
    OUTPUT ${_out}
    COMMAND ${GLVM_GLSLANG_VALIDATOR} -V -g ${_src} -o ${_out}
    DEPENDS ${_src}
    COMMENT "Compiling shader ${_rel_src}"
    VERBATIM
  )
  list(APPEND _glvm_spv_outputs ${_out})
endforeach()

add_custom_target(glvm_shaders ALL DEPENDS ${_glvm_spv_outputs})
add_dependencies(glvm glvm_shaders)
target_compile_definitions(glvm PUBLIC GLVM_SHADER_DIR="${GLVM_SHADER_OUT_DIR}")
