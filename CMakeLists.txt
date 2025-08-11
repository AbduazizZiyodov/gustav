cmake_minimum_required(VERSION 3.10)
project(gustav)

set(CMAKE_C_STANDARD 11)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/target)

set(
    SOURCES
    src/main.c
)

add_executable(gustav ${SOURCES})