cmake_minimum_required(VERSION 3.5)
project(cliente C)

set(CMAKE_C_STANDARD 11)

include_directories("/usr/include/altaLibreria" "/usr/include/commons")

add_executable(cliente
        cliente.c
        cliente.h)

target_link_libraries(cliente "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so")