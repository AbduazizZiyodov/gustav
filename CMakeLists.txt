cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

project(none)

add_executable(none main.c)

# Add logging.c with LOG_USE_COLOR flag
add_library(logging OBJECT utils/logging.c)
target_compile_definitions(logging PRIVATE LOG_USE_COLOR)

target_link_libraries(none PRIVATE logging)