cmake_minimum_required(VERSION 3.5)
project(funciones-emi C)

include_directories("/usr/include/altaLibreria" "/usr/include/commons")
include_directories(include)

set(CMAKE_C_STANDARD 99)

add_executable(funciones-rodri
        funciones-rodri.c funciones-rodri.h
        tools/tools.c tools/tools.h)
target_link_libraries(funciones-rodri "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so")
