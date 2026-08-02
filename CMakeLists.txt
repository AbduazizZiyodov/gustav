cmake_minimum_required(VERSION 3.24)

set(CMAKE_C_COMPILER "clang-19")

project(
  gustav
  VERSION 2.0
  LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
  message(FATAL_ERROR "clang is required")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

option(SANITIZE "Enable address sanitizer and ubsan" ON)

option(DEBUG_STRESS_GC "Run GC on every allocation" OFF)
option(DEBUG_LOG_GC "Log GC events" OFF)
option(CLANG_TIDY "Run clang-tidy as part of the build" OFF)
if(CLANG_TIDY)
  set(CMAKE_C_CLANG_TIDY
      clang-tidy; --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy; --use-color;)
endif()
set(LINE_LENGTH
    1024
    CACHE STRING "Maximum line length for REPL")
set(HASH_TABLE_MAX_LOAD_FACTOR 0.75)

set(SOURCES
    # entrypoint
    src/main.c
    src/cli.c
    src/api.c
    # frontend
    src/scanner.c
    src/compiler/compiler.c
    src/compiler/byte_code.c
    src/compiler/class.c
    src/compiler/declaration.c
    src/compiler/error.c
    src/compiler/expression.c
    src/compiler/function.c
    src/compiler/rules.c
    src/compiler/scope.c
    src/compiler/statement.c
    src/compiler/utils.c
    src/compiler/var.c
    # backend
    src/vm.c
    src/chunk.c
    src/debug.c
    # runtime data & memory
    src/value.c
    src/object.c
    src/hash_table.c
    src/memory.c
    src/gc.c
    # stdlib, bruh
    src/native_functions.c
    src/lib/gustav_time.c
    src/lib/gustav_math.c
    # stolen
    src/log.c)

add_executable(gustav ${SOURCES})

target_include_directories(gustav PRIVATE ${CMAKE_SOURCE_DIR}/include)

configure_file("${CMAKE_SOURCE_DIR}/include/version.h.in"
               "${CMAKE_BINARY_DIR}/include/version.h" @ONLY)

target_include_directories(gustav PRIVATE ${CMAKE_BINARY_DIR}/include)

target_compile_definitions(
  gustav
  PRIVATE LOG_USE_COLOR
          LINE_LENGTH=${LINE_LENGTH}
          HASH_TABLE_MAX_LOAD_FACTOR=${HASH_TABLE_MAX_LOAD_FACTOR}
          $<$<CONFIG:Debug>:DEBUG>
          $<$<BOOL:${DEBUG_STRESS_GC}>:DEBUG_STRESS_GC>
          $<$<BOOL:${DEBUG_LOG_GC}>:DEBUG_LOG_GC>)

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
          -Wmissing-variable-declarations
          -Wnull-dereference
          -Wdouble-promotion
          -Wformat-security
          -Werror
          $<$<CONFIG:Debug>:-g
          -O0
          -fno-omit-frame-pointer
          -fno-optimize-sibling-calls>
          $<$<AND:$<CONFIG:Debug>,$<BOOL:${SANITIZE}>>:-fsanitize=address
          -fsanitize=undefined
          -fno-sanitize-recover=undefined>
          $<$<CONFIG:Release>:-O3
          -flto>)

target_link_options(
  gustav
  PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<BOOL:${SANITIZE}>>:-fsanitize=address
  -fsanitize=undefined
  -fno-sanitize-recover=undefined>$<$<CONFIG:Release>:-flto>)

find_program(LLD_LINKER NAMES ld.lld lld)
if(LLD_LINKER)
  target_link_options(gustav PRIVATE $<$<CONFIG:Release>:-fuse-ld=lld>)
endif()
target_link_libraries(gustav PRIVATE m)

include(GNUInstallDirs)
install(TARGETS gustav RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
