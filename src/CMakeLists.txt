cmake_minimum_required(VERSION 3.10)
project(CUT__cpu_usage_tracker LANGUAGES C)

find_library(NAME pthread)

add_executable(cut__cpu_usage_tracker 
  main.c
)
set_property(TARGET cut__cpu_usage_tracker PROPERTY C_STANDARD 99)

if(CMAKE_C_COMPILER  EQUAL "GNU Compiler Collection")
    target_compile_options(cut__cpu_usage_tracker PRIVATE -Wall -Wextra)
endif()  
if(CMAKE_C_COMPILER EQUAL "LLVM Clang")
    target_compile_options(cut__cpu_usage_tracker PRIVATE -Weverything)
endif()
target_compile_definitions(cut__cpu_usage_tracker PRIVATE TEST_ON)
target_compile_options(cut__cpu_usage_tracker PRIVATE -pthread)
target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:sigterm_handler>)
target_link_libraries(cut__cpu_usage_tracker PRIVATE pthread)

#if(BUILD_TESTING)
    add_subdirectory(test)
#endif()

add_library(sigterm_handler OBJECT ./SIGTERM_handler.c)
set_property(TARGET sigterm_handler PROPERTY C_STANDARD 99)
if(CMAKE_C_COMPILER  EQUAL "GNU Compiler Collection")
    target_compile_options(sigterm_handler PUBLIC -Wall -Wextra)
endif()  
if(CMAKE_C_COMPILER EQUAL "LLVM Clang")
    target_compile_options(sigterm_handler PUBLIC -Weverything)
endif()
target_compile_definitions(sigterm_handler PUBLIC TEST_ON)
target_compile_options(sigterm_handler PUBLIC -pthread)

#if(BUILD_TESTING)
    add_library(sigterm_handler_test_obj OBJECT ./SIGTERM_handler.c)
    set_property(TARGET sigterm_handler_test_obj PROPERTY C_STANDARD 99)
    if(CMAKE_C_COMPILER  EQUAL "GNU Compiler Collection")    
        target_compile_options(sigterm_handler_test_obj PUBLIC -Wall -Wextra)
    endif()  
    if(CMAKE_C_COMPILER EQUAL "LLVM Clang")
        target_compile_options(sigterm_handler_test_obj PUBLIC -Weverything)
    endif()
    target_compile_definitions(sigterm_handler_test_obj PUBLIC TEST_ON)    
    target_compile_options(sigterm_handler_test_obj PUBLIC -pthread)
#endif()

