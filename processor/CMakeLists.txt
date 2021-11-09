cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(processor VERSION 1.0)

add_subdirectory(textlib)

add_subdirectory(cpu)
add_subdirectory(asm)
add_subdirectory(disasm)
