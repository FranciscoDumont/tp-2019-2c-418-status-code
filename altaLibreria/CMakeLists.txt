cmake_minimum_required(VERSION 3.5)
project(altaLibreria C)

set(CMAKE_C_STANDARD 99)

add_library(altaLibreria SHARED connections.c connections.h structures.h)