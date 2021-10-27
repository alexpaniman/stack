add_library(simple-stack INTERFACE)

target_include_directories(
  simple-stack SYSTEM INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})
