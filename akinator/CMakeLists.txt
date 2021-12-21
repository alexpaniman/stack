add_library(akinator STATIC akinator.cpp)

target_link_libraries(akinator PUBLIC binary-tree lexer textlib trace simple-stack)

target_include_directories(
  akinator PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

add_unit_test(akinator-tests akinator akinator-tests.cpp)


add_executable(akinator-executable main.cpp) 

target_link_libraries(akinator-executable akinator)
