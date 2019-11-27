cmake_minimum_required(VERSION 3.5)
project(funciones-rodri C)

include_directories("/usr/include/altaLibreria" "/usr/include/commons")
include_directories(include)

set(CMAKE_C_STANDARD 99)

add_executable(funciones-emi
        funciones-emi.c funciones-emi.h
        tools/tools.c tools/tools.h structs.h)
target_link_libraries(funciones-emi "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so")