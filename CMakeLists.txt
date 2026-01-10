cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_COMPILER "clang")

project(
  gustav
  VERSION 2.0
  LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/target)

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
  message(FATAL_ERROR "clang is required")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(LINE_LENGTH
    1024
    CACHE STRING "Maximum line length for REPL")
set(STACK_MAX
    512
    CACHE STRING "Stack size in VM")

add_executable(
  gustav
  src/main.c
  src/chunk.c
  src/memory.c
  src/debug.c
  src/value.c
  src/vm.c
  src/scanner.c
  src/compiler.c
  src/object.c
  src/log.c)

target_include_directories(
  gustav
  PUBLIC ${CMAKE_SOURCE_DIR}/include
  PRIVATE /usr/local/include)

target_compile_definitions(
  gustav PRIVATE LOG_USE_COLOR LINE_LENGTH=${LINE_LENGTH}
                 STACK_MAX=${STACK_MAX} $<$<CONFIG:Debug>:DEBUG>)
target_compile_options(
  gustav
  PRIVATE -Wall
          -Wextra
          -pedantic
          -Wshadow
          -Wcast-align
          -Wconversion
          -Wsign-conversion
          -Wformat=2
          -Wvla
          -Wstrict-prototypes
          -Wmissing-prototypes
          $<$<CONFIG:Debug>:-g
          -O0
          -fsanitize=address
          -fsanitize=undefined>
          $<$<CONFIG:Release>:-O3
          -DNDEBUG>)

find_library(MATH_LIBRARY m)
if(NOT MATH_LIBRARY)
  message(FATAL_ERROR "Math library not found")
endif()
target_link_libraries(gustav PRIVATE ${MATH_LIBRARY})

target_link_options(gustav PRIVATE $<$<CONFIG:Debug>:-fsanitize=address
                    -fsanitize=undefined>)

install(TARGETS gustav DESTINATION bin)