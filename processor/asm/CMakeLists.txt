add_library(assembler STATIC lexer.cpp assembler.cpp)

target_include_directories(
  assembler PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(assembler PUBLIC textlib simple-stack)

add_unit_test(lexer-tests assembler lexer-tests.cpp)
add_unit_test(assembler-tests assembler assembler-tests.cpp)
