#pragma once

#include <signal.h>

#if defined(__clang__)
#define COMPILER_NAME "Clang"
#define COMPILER_VERSION __clang_version__
#elif defined(__GNUC__)
#define COMPILER_NAME "GCC"
#define COMPILER_VERSION __VERSION__
#endif

#ifdef __OPTIMIZE__
#define OPT_LEVEL "optimized"
#else
#define OPT_LEVEL "not optimized"
#endif

#if defined(__x86_64__)
#define ARCH "x86_64"
#elif defined(__aarch64__)
#define ARCH "ARM64"
#else
#define ARCH "unknown"
#endif

#if defined(__linux__)
#define OS "Linux"
#elif defined(_WIN32)
#define OS "Windows"
#else
#define OS "unknown"
#endif

void repl(void);
void show_gustav_info(void);
void run_file(const char *path);

extern volatile sig_atomic_t shutdown_requested;
