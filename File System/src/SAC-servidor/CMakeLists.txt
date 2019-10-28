cmake_minimum_required(VERSION 3.5)
project(fileSystem C)

include_directories("/usr/include/altaLibreria" "/usr/include/commons")
include_directories(include)

set(CMAKE_C_STANDARD 99)

add_executable(fileSystem fileSystem.c fileSystem.h)

target_link_libraries(fileSystem "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so" )