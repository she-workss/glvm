function(glvm_add_test NAME)
  add_executable(${NAME} "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.cpp")
  target_link_libraries(${NAME}
    PRIVATE
      glvm
      GTest::gtest_main
  )
  add_test(NAME test_${NAME} COMMAND ${NAME})

  add_custom_target(test_${NAME}
    COMMENT "Running test '${NAME}'"
    COMMAND cmake -E echo ""
    COMMAND $<TARGET_FILE:${NAME}>
    WORKING_DIRECTORY $<TARGET_FILE_DIR:${NAME}>
    DEPENDS ${NAME}
    USES_TERMINAL
  )
endfunction()

glvm_add_test(basic)
glvm_add_test(rotation_math)
