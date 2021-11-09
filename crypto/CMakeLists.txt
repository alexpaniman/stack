add_library(crypto STATIC sha256.cpp)

target_include_directories(
  crypto PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})
