add_library(binary-tree INTERFACE)

target_include_directories(
  binary-tree SYSTEM INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})

add_unit_test(binary-tree-tests binary-tree binary-tree-tests.cpp)
