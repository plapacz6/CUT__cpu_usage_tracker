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
target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:ring_buffer>)
target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:reader>)
#target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:logger>)
target_link_libraries(cut__cpu_usage_tracker PRIVATE $<TARGET_OBJECTS:watchdog>)
target_link_libraries(cut__cpu_usage_tracker PRIVATE pthread)

#if(BUILD_TESTING)
    add_subdirectory(test)
#endif()

# ###########  SIGTERM handler ###################

add_library(sigterm_handler OBJECT ./SIGTERM_handler.c)
set_property(TARGET sigterm_handler PROPERTY C_STANDARD 99)
if(CMAKE_C_COMPILER  EQUAL "GNU Compiler Collection")
    target_compile_options(sigterm_handler PUBLIC -Wall -Wextra)
elseif(CMAKE_C_COMPILER EQUAL "LLVM Clang")
    target_compile_options(sigterm_handler PUBLIC -Weverything)
endif()
target_compile_definitions(sigterm_handler PUBLIC TEST_ON)
target_compile_options(sigterm_handler PUBLIC -pthread)


# ###########  ring buffer ###################

add_library(ring_buffer OBJECT ./ring_buffer.c)
set_property(TARGET ring_buffer PROPERTY C_STANDARD 99)
if(CMAKE_C_COMPILER EQUAL "GNU Compiler Collection")
    target_compile_options((ring_buffer PUBLIC -Wall -Wextra))
elseif(CMAKE_C_COMPILER EQUAL "LLVM Clang")
    target_compile_options(ring_buffer PUBLIC -Weverything)
endif()
target_compile_definitions(ring_buffer PUBLIC TEST_ON)
target_compile_options(ring_buffer PUBLIC -pthread)


# # ###########  logger ###################

# add_library(logger OBJECT ./logger.c)
# set_property(TARGET logger PROPERTY C_STANDARD 99)
# if(CMAKE_C_COMPILER EQUAL "GNU Compiler Collection")
#     target_compile_options((logger PUBLIC -Wall -Wextra))
# elseif(CMAKE_C_COMPILER EQUAL "LLVM Clang")
#     target_compile_options(logger PUBLIC -Weverything)
# endif()
# target_compile_definitions(logger PUBLIC TEST_ON)
# target_compile_options(logger PUBLIC -pthread)


# ###########  watchdog ###################

add_library(watchdog OBJECT ./watchdog.c)
set_property(TARGET watchdog PROPERTY C_STANDARD 99)
if(CMAKE_C_COMPILER EQUAL "GNU Compiler Collection")
    target_compile_options((watchdog PUBLIC -Wall -Wextra))
elseif(CMAKE_C_COMPILER EQUAL "LLVM Clang")
    target_compile_options(watchdog PUBLIC -Weverything)
endif()
target_compile_definitions(watchdog PUBLIC TEST_ON)
target_compile_options(watchdog PUBLIC -pthread)



# ###########  reader ###################

add_library(reader OBJECT ./reader.c)
set_property(TARGET reader PROPERTY C_STANDARD 99)
if(CMAKE_C_COMPILER EQUAL "GNU Compiler Collection")
    target_compile_options((reader PUBLIC -Wall -Wextra))
elseif(CMAKE_C_COMPILER EQUAL "LLVM Clang")
    target_compile_options(reader PUBLIC -Weverything)
endif()
target_compile_definitions(reader PUBLIC TEST_ON)
target_compile_options(reader PUBLIC -pthread)
