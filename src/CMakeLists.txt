cmake_minimum_required(VERSION 3.17)
project(CUT__cpu_usage_tracker LANGUAGES C)

find_library(NAME pthread)


add_executable(cut__cpu_usage_tracker 
  main.c
)
set_property(TARGET cut__cpu_usage_tracker PROPERTY C_STANDARD 11)

if(CMAKE_C_COMPILER  EQUAL "GNU Compiler Collection")
    target_compile_options(cut__cpu_usage_tracker PRIVATE -std=c11 -Wall -Wextra )
endif()  
if(CMAKE_C_COMPILER EQUAL "LLVM Clang")
    target_compile_options(cut__cpu_usage_tracker PRIVATE -std=c11 -Weverything )
endif()
target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:SIGTERM_handler>)
target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:ring_buffer>)
target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:reader>)
target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:analyzer>)
target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:printer>)
#target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:logger>)
target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:watchdog>)
target_link_libraries(cut__cpu_usage_tracker PRIVATE pthread)
target_compile_options(cut__cpu_usage_tracker PRIVATE -pthread)


#if(BUILD_TESTING)
add_subdirectory(test)
#endif()

add_library( SIGTERM_handler OBJECT ./SIGTERM_handler.c)
add_library( ring_buffer OBJECT ./ring_buffer.c)
#add_library( logger OBJECT ./logger.c)
add_library( watchdog OBJECT ./watchdog.c)
add_library( reader OBJECT ./reader.c)
add_library( analyzer OBJECT ./analyzer.c)
add_library( printer OBJECT ./printer.c)

list(APPEND my_objects 
    SIGTERM_handler
    ring_buffer
    #logger
    watchdog
    reader
    analyzer 
    printer   
)
foreach(my_object IN LISTS ${my_objects} )    
    set_property(TARGET ${my_object} PROPERTY C_STANDARD 11)
    if(CMAKE_C_COMPILER  EQUAL "GNU Compiler Collection")
        target_compile_options(${my_object} PUBLIC -std=c11 -Wall -Wextra )
    elseif(CMAKE_C_COMPILER EQUAL "LLVM Clang")
        target_compile_options(${my_object} PUBLIC -std=c11 -Weverything )
    endif()
    target_compile_definitions(${my_object} PUBLIC TEST_ON)
    target_compile_options(${my_object} PUBLIC -pthread)
endforeach()
