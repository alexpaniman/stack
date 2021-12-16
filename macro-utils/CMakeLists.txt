add_library(macro-utils INTERFACE)

target_include_directories(
  macro-utils SYSTEM INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})
