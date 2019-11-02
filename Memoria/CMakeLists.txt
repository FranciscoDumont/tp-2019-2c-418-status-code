cmake_minimum_required(VERSION 3.14)
project(Memoria)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(src)
include_directories(src/libMuse)
include_directories(src/Muse)
include_directories(test)

add_executable(Memoria
        src/libMuse/libmuse.c
        src/libMuse/libmuse.h
        src/Muse/main.c
        test/test.txt)
