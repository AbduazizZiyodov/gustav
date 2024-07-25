cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

project(none)

add_executable(none main.c)

add_library(logging OBJECT utils/logging.c)
target_compile_definitions(logging PRIVATE LOG_USE_COLOR)
target_link_libraries(none PRIVATE logging)


add_library(errors OBJECT utils/errors.c)
target_link_libraries(none PRIVATE errors)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)