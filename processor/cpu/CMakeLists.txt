add_executable(cpu cpu.cpp)

target_include_directories(
  cpu PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..)
