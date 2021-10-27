add_library(pseudographics STATIC ascii-tables.cpp)

target_include_directories(
  pseudographics PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  pseudographics PUBLIC
  simple-stack ansi-colors)
