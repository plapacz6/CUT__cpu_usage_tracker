cmake_minimum_required(VERSION 3.10)
project(CUT__cpu_usage_tracker LANGUAGES C)

find_library(NAME pthread)


add_executable(SIGTERM_handler_test
  ./SIGTERM_handler_test.c  
)
target_compile_definitions(SIGTERM_handler_test PUBLIC TEST_ON)
if(CMAKE_C_COMPILER  EQUAL "GNU Compiler Collection")    
    target_compile_options(SIGTERM_handler_test PRIVATE -Wall -Wextra)
endif()  
if(CMAKE_C_COMPILER EQUAL "LLVM Clang")
    target_compile_options(SIGTERM_handler_test PRIVATE -Weverything)
endif()
target_compile_definitions(SIGTERM_handler_test PRIVATE TEST_ON)
target_compile_options(SIGTERM_handler_test PRIVATE -pthread)
target_link_libraries(SIGTERM_handler_test PRIVATE $<TARGET_OBJECTS:sigterm_handler_test_obj>)
target_link_libraries(SIGTERM_handler_test PRIVATE pthread)



add_test(NAME SIGTERM_handler__test COMMAND SIGTERM_handler_test)
# set_tests_properties(SIGTERM_handler__test
#   PROPERTIES PASS_REGULAR_EXPRESSION "*PASS*"
#   )
