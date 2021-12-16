add_library(graphviz STATIC graphviz.cpp)

target_include_directories(
  graphviz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(graphviz linked-list hash-table textlib)

add_unit_test(graphviz-tests graphviz graphviz-tests.cpp)
