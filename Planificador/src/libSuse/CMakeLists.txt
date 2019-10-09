cmake_minimum_required(VERSION 3.5)
project(libSuse C)

set(CMAKE_C_STANDARD 99)

include_directories("/usr/include/commons" "/usr/include/altaLibreria" "/usr/include/hilolay")

add_library(libSuse SHARED libSuse.c libSuse.h libSuseStructures.h)

target_link_libraries(libSuse "/usr/lib/libcommons.so" "/usr/lib/libaltaLibreria.so" "/usr/lib/libhilolay.so")

install(TARGETS libSuse DESTINATION /usr/lib)
install(FILES libSuse.h libSuseStructures.h DESTINATION /usr/include/libSuse)