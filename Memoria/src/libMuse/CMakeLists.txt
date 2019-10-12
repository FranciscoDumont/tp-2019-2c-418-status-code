cmake_minimum_required(VERSION 3.14.5)
project(libMuse C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons" "/usr/include/altaLibreria")

add_library(libMuse SHARED libmuse.c libmuse.h)

target_link_libraries(libMuse "/usr/lib/libcommons.so" "/usr/lib/libaltaLibreria.so")

install(TARGETS libMuse DESTINATION /usr/lib)
install(FILES libmuse.h DESTINATION /usr/include/libMuse)