cmake_minimum_required(VERSION 3.24)

set(CMAKE_C_COMPILER "clang-19")

project(
  gustav
  VERSION 2.0
  LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "clang is required")
endif()

set(CMAKE_C_CLANG_TIDY
    clang-tidy;
    --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy;
    --use-color;
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(LINE_LENGTH 1024 CACHE STRING "Maximum line length for REPL")
set(HASH_TABLE_MAX_LOAD_FACTOR 0.75)

set(SOURCES
    src/api.c
    src/chunk.c
    src/cli.c
    src/compiler.c
    src/debug.c
    src/hash_table.c
    src/log.c
    src/main.c
    src/memory.c
    src/object.c
    src/scanner.c
    src/value.c
    src/vm.c
    src/native_functions.c
    src/lib/gustav_time.c
)

set(HEADERS
    include/api.h
    include/chunk.h
    include/cli.h
    include/common.h
    include/compiler.h
    include/debug.h
    include/hash_table.h
    include/log.h
    include/memory.h
    include/object.h
    include/scanner.h
    include/value.h
    include/vm.h
    src/native_functions.h
)

add_executable(gustav ${SOURCES})

target_include_directories(gustav PUBLIC ${CMAKE_SOURCE_DIR}/include)

configure_file(
    "${CMAKE_SOURCE_DIR}/include/version.h.in"
    "${CMAKE_BINARY_DIR}/include/version.h"
    @ONLY
)

target_include_directories(gustav PRIVATE ${CMAKE_BINARY_DIR}/include)

target_compile_definitions(
  gustav PRIVATE
  LOG_USE_COLOR
  LINE_LENGTH=${LINE_LENGTH}
  HASH_TABLE_MAX_LOAD_FACTOR=${HASH_TABLE_MAX_LOAD_FACTOR}
  $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(
    gustav
    PRIVATE
    -Wall
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
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat-security
    $<$<CONFIG:Debug>:-g -O0 -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls>
    $<$<CONFIG:Release>:-O3 -flto>
)

find_library(MATH_LIBRARY m)
if(NOT MATH_LIBRARY)
    message(FATAL_ERROR "Math library not found")
endif()

target_link_libraries(gustav PRIVATE ${MATH_LIBRARY})

target_link_options(
    gustav PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=undefined>
    $<$<CONFIG:Release>:-flto>
)

install(TARGETS gustav DESTINATION bin)
