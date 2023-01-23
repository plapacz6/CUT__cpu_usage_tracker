cmake_minimum_required(VERSION 3.17)
project(CUT__cpu_usage_tracker LANGUAGES C)

find_library(NAME pthread)

# #####################   SIGTERM handler ##########################
add_executable(SIGTERM_handler_test
  ./SIGTERM_handler_test.c  
)
target_link_libraries(SIGTERM_handler_test PRIVATE $<TARGET_OBJECTS:SIGTERM_handler>)
target_link_libraries(SIGTERM_handler_test PRIVATE $<TARGET_OBJECTS:ring_buffer>)
target_link_libraries(SIGTERM_handler_test PRIVATE $<TARGET_OBJECTS:mutexes>)
target_link_libraries(SIGTERM_handler_test PRIVATE pthread)


# #####################   ring buffer ##############################
add_executable(ring_buffer_test
  ./ring_buffer_test.c  
)
target_link_libraries(ring_buffer_test PRIVATE $<TARGET_OBJECTS:ring_buffer>)
target_link_libraries(ring_buffer_test PRIVATE $<TARGET_OBJECTS:mutexes>)
target_link_libraries(ring_buffer_test PRIVATE pthread)


# #####################     logger      #############################
add_executable(logger_test
  ./logger_test.c  
)
target_link_libraries(logger_test PRIVATE $<TARGET_OBJECTS:ring_buffer>)
target_link_libraries(logger_test PRIVATE $<TARGET_OBJECTS:logger>)
target_link_libraries(logger_test PRIVATE $<TARGET_OBJECTS:mutexes>)
target_link_libraries(logger_test PRIVATE pthread)

# #####################     watchdog      ###########################
add_executable(watchdog_test
  ./watchdog_test.c  
)
target_link_libraries(watchdog_test PRIVATE $<TARGET_OBJECTS:watchdog>)
target_link_libraries(watchdog_test PRIVATE pthread)


# #####################     reader      #############################
add_executable(reader_test
  ./reader_test.c  
)
#target_link_libraries(reader_test PRIVATE $<TARGET_OBJECTS:watchdog>)
#target_link_libraries(reader_test PRIVATE $<TARGET_OBJECTS:logger>)
target_link_libraries(reader_test PRIVATE $<TARGET_OBJECTS:reader>)
#target_link_libraries(reader_test PRIVATE $<TARGET_OBJECTS:SIGTERM_handler>)
target_link_libraries(reader_test PRIVATE $<TARGET_OBJECTS:ring_buffer>)
target_link_libraries(reader_test PRIVATE $<TARGET_OBJECTS:mutexes>)
target_link_libraries(reader_test PRIVATE pthread)


# #####################     analyzer      ############################
add_executable(analyzer_test
  ./analyzer_test.c  
)
target_link_libraries(analyzer_test PRIVATE $<TARGET_OBJECTS:reader>)
target_link_libraries(analyzer_test PRIVATE $<TARGET_OBJECTS:SIGTERM_handler>)
target_link_libraries(analyzer_test PRIVATE $<TARGET_OBJECTS:ring_buffer>)
target_link_libraries(analyzer_test PRIVATE $<TARGET_OBJECTS:analyzer>)
target_link_libraries(analyzer_test PRIVATE $<TARGET_OBJECTS:mutexes>)
target_link_libraries(analyzer_test PRIVATE pthread)

########################################################################

foreach(my_target IN LISTS ${my_objects})  
  if(CMAKE_C_COMPILER  EQUAL "GNU Compiler Collection")    
      target_compile_options(my_target PRIVATE -std=c11 -Wall -Wextra )
  elseif(CMAKE_C_COMPILER EQUAL "LLVM Clang")
      target_compile_options(my_target PRIVATE -std=c11 -Weverything )
  endif()
  target_compile_definitions(my_target PRIVATE TEST_ON)
  target_compile_options(my_target PRIVATE -pthread)  
endforeach()

########################################################################

# #####################  TESTS  ########################################
add_test(NAME SIGTERM_handler__test COMMAND SIGTERM_handler_test)

add_test(NAME ring_buffer__test COMMAND ring_buffer_test)

add_test(NAME logger__test COMMAND logger_test)

add_test(NAME watchdog__test COMMAND watchdog_test)
# set_tests_properties( __test
#   PROPERTIES PASS_REGULAR_EXPRESSION "*PASS*"
#   )

add_test(NAME reader__test COMMAND reader_test)

add_test(NAME analyzer__test COMMAND analyzer_test)
