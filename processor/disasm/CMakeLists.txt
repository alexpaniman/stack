add_executable(disasm disasm.cpp)

target_include_directories(
  disasm PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..)
