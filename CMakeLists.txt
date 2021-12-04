cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(stack VERSION 1.0)

# ------------------------------- DEBUG FLAGS -------------------------------

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D _DEBUG -ggdb3 -std=c++14 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++0x-compat -Wc++11-compat -Wc++14-compat -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlarger-than=8192 -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstack-usage=8192 -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -fcheck-new -fsized-deallocation -fstack-check -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -fPIE -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=null -fsanitize=object-size -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr -O0")

set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -D _DEBUG -ggdb3 -std=c++14 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++0x-compat -Wc++11-compat -Wc++14-compat -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlarger-than=8192 -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstack-usage=8192 -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -fcheck-new -fsized-deallocation -fstack-check -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -fPIE -fsanitize=address -fsanitize=alignment -fsanitize=bool -fsanitize=bounds -fsanitize=enum -fsanitize=float-cast-overflow -fsanitize=float-divide-by-zero -fsanitize=integer-divide-by-zero -fsanitize=leak -fsanitize=nonnull-attribute -fsanitize=null -fsanitize=object-size -fsanitize=return -fsanitize=returns-nonnull-attribute -fsanitize=shift -fsanitize=signed-integer-overflow -fsanitize=undefined -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=vptr -O0")

# --------------------------------- TESTING ---------------------------------

# Make test-framework available for linking
add_subdirectory(test-framework)

# Enable ctest in current project
enable_testing()

# Macro to add test and remeber it's targets
# It's not very useful now, but it will be, when we add more tests
macro(add_unit_test target lib-under-test target-cpp)
  # Create test executable
  add_executable(${target} ${target-cpp})

  # Link library that we're testing to that executable
  target_link_libraries(${target} PUBLIC ${lib-under-test} test-framework)

  # Keep track of test targets
  set(UNIT_TEST_TARGETS ${UNIT_TEST_TARGETS} ${target} PARENT_SCOPE)

  # Add this unit test to other ones
  add_test(target ${CMAKE_CURRENT_BINARY_DIR}/${target})
endmacro(add_unit_test)

# Add target that depends on tests' tagets added with add_unit_test
add_custom_target(all_tests ALL DEPENDS ${UNIT_TEST_TARGETS})

# Run tests automatically after each build:
# Possible flags to use with ctest:
#   --verbose           always outputs full information about test
#   --output-on-failure does so only when test fails
# add_custom_command(TARGET all_tests COMMENT "Run all tests"
# 		   POST_BUILD COMMAND ctest --verbose
# 		   WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# ------------------------------ DEPENDENCIES -------------------------------

# Simple stack implementation
add_subdirectory(simple-stack)

# Library for keeping track of errors
add_subdirectory(trace)

# Library for cryptographic hashing
add_subdirectory(crypto)

# Header only library for coloring terminal text
add_subdirectory(ansi-colors)

# TODO ...
add_subdirectory(pseudographics)

# Secure stack implementation
add_subdirectory(protected-stack)

# Processor
# add_subdirectory(processor)

# Fully featured doubly linked list in array
add_subdirectory(linked-list)

# ----------------------------- MAIN EXECUTABLE -----------------------------

add_executable(stack main.cpp)

target_include_directories(stack PUBLIC ./simple-stack/)

target_link_libraries(stack PUBLIC pseudographics)
