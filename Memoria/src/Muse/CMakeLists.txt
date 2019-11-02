cmake_minimum_required(VERSION 3.14.5)
project(Muse C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/altaLibreria" "/usr/include/commons")

add_executable(Muse muse.c)

target_link_libraries(Muse "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so")