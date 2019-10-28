cmake_minimum_required(VERSION 3.15)
project(SAC-cli C)

find_package(Threads)

include_directories("/usr/include/altaLibreria" "/usr/include/commons" "/usr/include/fuse")
include_directories(include)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=27")
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_executable(SAC-cli fuse_example.c operaciones.c fuse_example.h operaciones.h)

target_link_libraries(SAC-cli "/usr/lib/libaltaLibreria.so" "/usr/lib/libcommons.so" "-lfuse" "-pthread")