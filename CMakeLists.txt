cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "clang")

project(gustav VERSION 2.0 LANGUAGES C)

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

add_executable(gustav 
    src/main.c 
    src/chunk.c 
    src/memory.c
    src/log.c
    src/debug.c
    src/value.c
    src/vm.c
)

target_include_directories(gustav 
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    PRIVATE /usr/local/include
)

target_compile_definitions(gustav PRIVATE 
    LOG_USE_COLOR
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:Release>:RELEASE_BUILD>
)

target_compile_options(gustav PRIVATE
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
    $<$<CONFIG:Debug>:-g -O0 -fsanitize=address -fsanitize=undefined>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_link_options(gustav PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=undefined>
)

install(TARGETS gustav DESTINATION bin)

message(STATUS "project=${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "build_type=${CMAKE_BUILD_TYPE}")
message(STATUS "c_compiler=${CMAKE_C_COMPILER}")
message(STATUS "compiler_id=${CMAKE_C_COMPILER_ID}")
message(STATUS "compiler_version=${CMAKE_C_COMPILER_VERSION}")
message(STATUS "c_standard=C${CMAKE_C_STANDARD}")
message(STATUS "output_dir=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")