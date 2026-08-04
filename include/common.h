#pragma once

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define is == // Yes, I use CPython
#define UINT8_COUNT (UINT8_MAX + 1)
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef GUSTAV_DEBUG
#define UNREACHABLE()                                                                      \
	do {                                                                               \
		(void)fprintf(stderr, "[%s:%d] This code should not be reached in %s()\n", \
			      __FILE__, __LINE__, __func__);                               \
		abort();                                                                   \
	} while (false)
#else
#define UNREACHABLE()
#endif // GUSTAV_DEBUG

__attribute__((format(printf, 2, 3), noreturn)) static inline void
Gustav_Error(short code, const char *format, ...)
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
