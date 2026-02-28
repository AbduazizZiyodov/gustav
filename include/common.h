#ifndef GUSTAV_COMMON_H
#define GUSTAV_COMMON_H

#include <errno.h>
#include <error.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define is == // Yes, I use CPython
#define UINT8_COUNT (UINT8_MAX + 1)
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC

#ifdef DEBUG
#define UNREACHABLE()                                                        \
	do {                                                                 \
		(void)fprintf(                                               \
			stderr,                                              \
			"[%s:%d] This code should not be reached in %s()\n", \
			__FILE__, __LINE__, __func__);                       \
		abort();                                                     \
	} while (false)
#else
#define UNREACHABLE()
#endif // DEBUG

__attribute__((format(printf, 2, 3), noreturn)) static inline void
gustav_error(short code, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	(void)vfprintf(stderr, format, args);

	if (errno != 0) {
		/* NOLINTNEXTLINE(concurrency-mt-unsafe) */
		(void)fprintf(stderr, ": %s", strerror(errno));
	}

	(void)fprintf(stderr, "\n");

	va_end(args);
	_Exit(code);
}

#endif // GUSTAV_COMMON_H
