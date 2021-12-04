file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/null.cpp "")

add_library(safe-alloc STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/null.cpp)

target_link_libraries(safe-alloc trace)

target_include_directories(
  safe-alloc PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})
