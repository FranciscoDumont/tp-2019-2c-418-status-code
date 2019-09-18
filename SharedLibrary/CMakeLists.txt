cmake_minimum_required(VERSION 3.15)
project(SharedLibrary C)

set(CMAKE_C_STANDARD 99)

add_library(SharedLibrary SHARED protocolo.c protocolo.h)