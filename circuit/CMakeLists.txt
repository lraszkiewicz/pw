cmake_minimum_required(VERSION 3.5.1)
project(circuit)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c parse_input.c parse_input.h clear_memory.c clear_memory.h)
add_executable(circuit ${SOURCE_FILES})