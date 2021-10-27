add_library(protected-stack INTERFACE)

target_include_directories(
  protected-stack SYSTEM INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})
