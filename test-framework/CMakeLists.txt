file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/null.cpp "")

add_library(test-framework STATIC
  ${CMAKE_CURRENT_BINARY_DIR}/null.cpp)

target_link_libraries(test-framework PUBLIC ansi-colors simple-stack trace)

target_include_directories(test-framework SYSTEM INTERFACE
                           ${CMAKE_CURRENT_SOURCE_DIR})
