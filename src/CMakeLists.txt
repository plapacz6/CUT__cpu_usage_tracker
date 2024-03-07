cmake_minimum_required(VERSION 3.19)
project(CUT__cpu_usage_tracker LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_library(NAME pthread)

set(CMAKE_C_COMPILER ${CC})
if(CMAKE_C_COMPILER_ID  STREQUAL "GNU")
    set(compile_options -Wall -Wextra)      
    message("compile options for GNU: ${compile_options}")
elseif(CMAKE_C_COMPILER_ID STREQUAL "LLVM")
    set(compile_options -Weverything)   
    message("compile options for LLVM: ${compile_options}") 
else()
    set(compile_options "")    
    message("compile options for Other: ${compile_options}")
endif()
set(compile_options ${compile_options} -pthread)
message("compile options: ${compile_options}")

list(APPEND my_objects     
    ring_buffer
    logger
    watchdog
    reader
    analyzer 
    printer   
    mutexes
    SIGTERM_handler
)

foreach(my_object IN LISTS my_objects)    
    message("creating target: object: ${my_object}")
    add_library("${my_object}" OBJECT "./${my_object}.c")
 
    message("configuring target: object: ${my_object}")
    target_include_directories("${my_object}" PUBLIC "./")
    target_compile_options("${my_object}" PUBLIC ${compile_options})
    target_link_libraries("${my_object}" PUBLIC pthread)
endforeach()


add_executable(cut__cpu_usage_tracker 
  ./main.c
)
target_include_directories(cut__cpu_usage_tracker PRIVATE "./")
target_compile_options(cut__cpu_usage_tracker PRIVATE ${compile_options})
foreach(my_object IN LISTS my_objects)   
    list(APPEND obj_lists
        $<TARGET_OBJECTS:${my_object}>
    )    
endforeach()
message("object list for linking: ${obj_lists}")
target_link_libraries(cut__cpu_usage_tracker PRIVATE ${obj_lists})    
target_link_libraries(cut__cpu_usage_tracker PRIVATE pthread)


add_subdirectory(test)

