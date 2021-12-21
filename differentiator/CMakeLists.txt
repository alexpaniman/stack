# Differentiator as a library
add_library(differentiator STATIC differentiator.cpp)

target_link_libraries(differentiator PUBLIC trace hash-table safe-alloc)

target_include_directories(
  differentiator PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

add_unit_test(differentiator-tests
  differentiator differentiator-tests.cpp)

# Differentiator as a standalone program
add_executable(differentiator-executable main.cpp) 
target_link_libraries(
  differentiator-executable akinator)
