add_library(printf-asm STATIC printf-asm.o call-printf-from-c.cpp)

target_include_directories(
  printf-asm PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

add_unit_test(printf-asm-tests printf-asm printf-asm-tests.cpp)
