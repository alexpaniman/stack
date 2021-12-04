file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/null.cpp "")

add_library(hash-table STATIC ${CMAKE_CURRENT_BINARY_DIR}/null.cpp)

target_include_directories(
  hash-table SYSTEM INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(hash-table linked-list safe-alloc)

add_unit_test(hash-table-tests hash-table hash-table-tests.cpp)
