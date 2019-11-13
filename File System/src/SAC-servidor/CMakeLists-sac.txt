cmake_minimum_required(VERSION 3.5)
project(sac-server C)

include_directories("/usr/include/altaLibreria" "/usr/include/commons")
include_directories(include)

set(CMAKE_C_STANDARD 99)

add_executable(sac-server sac-server.c sac-server.h)

target_link_libraries(sac-server "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so")