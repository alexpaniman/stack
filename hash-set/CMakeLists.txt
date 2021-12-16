file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/null.cpp "")

add_library(hash-set STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/null.cpp)

target_include_directories(
  hash-set PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(hash-set hash-table)

add_unit_test(hash-set-tests hash-set hash-set-tests.cpp)
