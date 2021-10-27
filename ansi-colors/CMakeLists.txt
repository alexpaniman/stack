add_library(ansi-colors INTERFACE)

target_include_directories(
  ansi-colors SYSTEM INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})
