add_library(lexer STATIC lexer.cpp dfs-visualizer.cpp)

target_include_directories(
  lexer PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(lexer PUBLIC simple-stack trace hash-table hash-set graphviz)

# Add unit tests for lexer and it's visualizer

add_unit_test(lexer-tests lexer lexer-tests.cpp)

add_unit_test(dfs-visualizer-tests
  lexer dfs-visualizer-tests.cpp)
