cmake_minimum_required(VERSION 3.15)
project(libMuse C)

set(CMAKE_C_STANDARD 99)

add_library(libMuse library.c library.h)