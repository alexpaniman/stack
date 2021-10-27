add_library(sha-256 STATIC sha256.cpp)

target_include_directories(
  sha-256 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})
