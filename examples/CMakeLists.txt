function(glvm_add_example NAME)
  add_executable(${NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.cpp"
    assets/textures/human.cpp
    assets/textures/container2_specular.cpp
    assets/textures/container2.cpp
    assets/textures/crosshair.cpp
    assets/textures/font_atlas.cpp
    assets/textures/gray.cpp
    assets/textures/inventory_slot.cpp
    assets/textures/tileset.cpp
    assets/textures/witch.cpp
  )
  target_link_libraries(${NAME}
    PRIVATE
      glvm
  )

  # -static only on Windows: Linux examples link the shared system libs
  # (Wayland, X11, XCB, ALSA), which cannot be pulled into a static link.
  target_link_options(${NAME} PRIVATE
    $<$<BOOL:${WIN32}>:-static>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-s>
    $<$<BOOL:${WIN32}>:-Wl,-Bstatic,--whole-archive>
    $<$<BOOL:${WIN32}>:-lwinpthread>
    $<$<BOOL:${WIN32}>:-Wl,--no-whole-archive>
  )

  add_custom_target(run_${NAME}
    COMMENT "Running example '${NAME}'"
    COMMAND cmake -E echo ""
    COMMAND $<TARGET_FILE:${NAME}>
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${NAME}>
    DEPENDS ${NAME}
    USES_TERMINAL
  )
endfunction()

glvm_add_example(hello_world)
glvm_add_example(tps)
