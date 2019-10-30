cmake_minimum_required(VERSION 3.5)
project(Tests C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons" "/usr/include/libMuse")

add_executable(Tests tests.c)

target_link_libraries(Tests "/usr/lib/libcommons.so" "/usr/lib/liblibMuse.so")
