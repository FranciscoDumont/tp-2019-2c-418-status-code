cmake_minimum_required(VERSION 3.14.5)
project(libMuse C)

set(CMAKE_C_STANDARD 99)

add_library(libMuse libmuse.c libmuse.h)