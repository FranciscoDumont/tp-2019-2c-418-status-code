cmake_minimum_required(VERSION 3.5)
project(suse C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/altaLibreria" "/usr/include/commons")

add_executable(suse suse.c suse.h)

target_link_libraries(suse "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so")