add_library(textlib STATIC textlib.cpp)

target_include_directories(
  textlib SYSTEM INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  textlib PUBLIC trace)

add_unit_test(textlib-test textlib textlib-test.cpp)
