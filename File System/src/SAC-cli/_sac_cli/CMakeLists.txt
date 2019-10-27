cmake_minimum_required(VERSION 3.5)
project(sac-cli)

include_directories("/usr/include/altaLibreria" "/usr/include/commons")
include_directories(include)

set(CMAKE_C_STANDARD 99)

add_executable(sac-cli fuse_example.c fuse_example.h)

target_link_libraries(sac-cli "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so" )