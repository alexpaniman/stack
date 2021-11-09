file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/null.cpp "")

add_library(protected-stack STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/null.cpp)

target_include_directories(
  protected-stack SYSTEM INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  protected-stack PUBLIC trace crypto)

# Add unit tests to protected-stack
# add_unit_test(protected-stack-test
#   protected-stack protected-stack-tests.cpp)
