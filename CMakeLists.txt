cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(stack VERSION 1.0)

add_subdirectory(simple-stack)
add_subdirectory(pseudographics)
add_subdirectory(ansi-colors)
add_subdirectory(protected-stack)

add_executable(stack main.cpp)

target_include_directories(stack PUBLIC ./simple-stack/)

target_link_libraries(
  stack PUBLIC
  pseudographics)
