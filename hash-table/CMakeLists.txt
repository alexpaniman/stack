add_library(hash-table STATIC default-hash-functions.cpp)

target_include_directories(
  hash-table PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(hash-table linked-list safe-alloc macro-utils)

add_unit_test(hash-table-tests hash-table hash-table-tests.cpp)
