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




add_library(atomic_variables_for_tests STATIC ./atomic_variables_for_tests.c)

# #####################   SIGTERM handler ##########################
add_executable(SIGTERM_handler_test
  ./SIGTERM_handler_test.c    
)
list(APPEND tests_list SIGTERM_handler_test)
# target_compile_options(SIGTERM_handler_test PRIVATE -fno-pic)
target_link_libraries(SIGTERM_handler_test PRIVATE   
  atomic_variables_for_tests
  $<TARGET_OBJECTS:SIGTERM_handler>
  $<TARGET_OBJECTS:ring_buffer>    
  $<TARGET_OBJECTS:mutexes>
  pthread  
)

# #####################   ring buffer ##############################
add_executable(ring_buffer_test
  ./ring_buffer_test.c  
)
list(APPEND tests_list ring_buffer_test)
target_link_libraries(ring_buffer_test PRIVATE 
  $<TARGET_OBJECTS:ring_buffer>
  $<TARGET_OBJECTS:mutexes>
  pthread
)

# #####################     logger      #############################
add_executable(logger_test
  ./logger_test.c  
)
list(APPEND tests_list logger_test)
target_link_libraries(logger_test PRIVATE 
  $<TARGET_OBJECTS:ring_buffer>
  $<TARGET_OBJECTS:logger>  
  $<TARGET_OBJECTS:mutexes>
  pthread
)

# #####################     watchdog      ###########################
add_executable(watchdog_test
  ./watchdog_test.c  
)
list(APPEND tests_list watchdog_test)
target_link_libraries(watchdog_test PRIVATE    
  $<TARGET_OBJECTS:ring_buffer>
  $<TARGET_OBJECTS:logger>  
  $<TARGET_OBJECTS:mutexes>
  $<TARGET_OBJECTS:watchdog>
  pthread
)

# #####################     reader      #############################
add_executable(reader_test
  ./reader_test.c  
)
list(APPEND tests_list reader_test)
target_link_libraries(reader_test PRIVATE 
  $<TARGET_OBJECTS:logger>
  $<TARGET_OBJECTS:reader>
  $<TARGET_OBJECTS:ring_buffer>
  $<TARGET_OBJECTS:mutexes>
  pthread
)

# #####################     analyzer      ############################
add_executable(analyzer_test
  ./analyzer_test.c  
)
list(APPEND tests_list analyzer_test)
target_link_libraries(analyzer_test PRIVATE 
  # atomic_variables_for_tests
  $<TARGET_OBJECTS:reader>
  $<TARGET_OBJECTS:SIGTERM_handler>
  $<TARGET_OBJECTS:ring_buffer>
  $<TARGET_OBJECTS:logger>
  $<TARGET_OBJECTS:printer>
  $<TARGET_OBJECTS:analyzer>
  $<TARGET_OBJECTS:mutexes>
  pthread
)




########################################################################

foreach(test_target IN LISTS tests_list)  
  target_compile_options(${test_target} PRIVATE ${compile_options})

  target_compile_definitions(${test_target} PUBLIC __ISOC11_SOURCE)  
  list(APPEND tests_runner_list "${test_target}_runner")
endforeach()

########################################################################

# #####################  TESTS  ########################################
foreach(test_target test_target_runner 
        IN ZIP_LISTS 
        tests_list tests_runner_list
)  
  add_test(NAME ${test_target_runner} COMMAND ${test_target})
endforeach()

# set_tests_properties( ${test_target_runner}
#   PROPERTIES PASS_REGULAR_EXPRESSION "*PASS*"
#   )
