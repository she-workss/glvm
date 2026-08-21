function(cevy_add_test NAME)
  add_executable(${NAME} "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.cpp")
  target_link_libraries(${NAME}
    PRIVATE
      cevy_log
      GTest::gtest_main
  )
  add_test(NAME test_${NAME} COMMAND ${NAME})

  add_custom_target(test_${NAME}
    COMMENT "Running unit test '${NAME}'"
    COMMAND cmake -E echo ""
    COMMAND $<TARGET_FILE:${NAME}>
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${NAME}>
    DEPENDS ${NAME}
    USES_TERMINAL
  )
endfunction()

cevy_add_test(basic_log)
