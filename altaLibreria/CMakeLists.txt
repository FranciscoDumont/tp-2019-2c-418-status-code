cmake_minimum_required(VERSION 3.5)
project(altaLibreria C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons")

add_library(altaLibreria SHARED connections.c connections.h structures.h)

target_link_libraries(altaLibreria "/usr/lib/libcommons.so" pthread)

install(TARGETS altaLibreria DESTINATION /usr/lib)
install(FILES connections.h structures.h DESTINATION /usr/include/altaLibreria)