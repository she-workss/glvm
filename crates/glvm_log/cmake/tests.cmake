if(NOT GLVM_BUILD_TESTS)
  return()
endif()

enable_testing()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(tests)
