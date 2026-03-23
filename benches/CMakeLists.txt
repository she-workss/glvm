function(glvm_add_bench NAME)
  add_executable(${NAME} "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.cpp")
  target_link_libraries(${NAME}
    PRIVATE
      glvm
      benchmark::benchmark_main
  )

  add_custom_target(bench_${NAME}
    COMMENT "Running benchmark '${NAME}'"
    COMMAND cmake -E echo ""
    COMMAND $<TARGET_FILE:${NAME}>
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${NAME}>
    DEPENDS ${NAME}
    USES_TERMINAL
  )
endfunction()

glvm_add_bench(ecs)
