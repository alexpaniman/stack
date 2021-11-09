add_library(test-framework INTERFACE)

target_include_directories(test-framework SYSTEM INTERFACE
                           ${CMAKE_CURRENT_SOURCE_DIR})
