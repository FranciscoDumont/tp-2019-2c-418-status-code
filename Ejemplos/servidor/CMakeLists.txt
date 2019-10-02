cmake_minimum_required(VERSION 3.5)
project(servidor C)

set(CMAKE_C_STANDARD 11)

include_directories("/usr/include/altaLibreria" "/usr/include/commons")

add_executable(servidor
        servidor.c
        servidor.h)

target_link_libraries(servidor "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so")