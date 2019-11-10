cmake_minimum_required(VERSION 3.5)
project(tools C)

include_directories("/usr/include/altaLibreria" "/usr/include/commons")
include_directories(include)

set(CMAKE_C_STANDARD 99)

add_executable(tools tools.c tools.h)

target_link_libraries(tools "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so" )