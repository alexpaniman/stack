add_library(trace STATIC trace.cpp)

target_include_directories(
  trace PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  trace PUBLIC ansi-colors)
