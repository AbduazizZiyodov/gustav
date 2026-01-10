#ifndef GUSTAV_COMMON_H
#define GUSTAV_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef DEBUG
#include <stdlib.h>

#define UNREACHABLE()                                                        \
	do {                                                                 \
		fprintf(stderr,                                              \
			"[%s:%d] This code should not be reached in %s()\n", \
			__FILE__, __LINE__, __func__);                       \
		abort();                                                     \
	} while (false)
#else
#define UNREACHABLE()
#endif

#endif
