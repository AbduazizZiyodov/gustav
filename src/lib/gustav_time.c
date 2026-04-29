/* NOLINTNEXTLINE(bugprone-reserved-identifier) */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
/* NOLINTNEXTLINE(misc-include-cleaner) */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "native_functions.h"
#include "value.h"

value_t gustav_clock_native(size_t arg_count [[maybe_unused]],
			    value_t *args [[maybe_unused]])
{
	struct timespec ts;
	/* NOLINTNEXTLINE(misc-include-cleaner) */
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
		return NIL_VAL;
	}

	double seconds = (double)ts.tv_sec + ((double)ts.tv_nsec / 1e9);
	return NUMBER_VAL(seconds);
}

value_t gustav_sleep_native(size_t arg_count [[maybe_unused]],
			    value_t *args [[maybe_unused]])
{
	// TODO(abduaziz): better error handling, <0, no arg ...
	if (arg_count < 1) {
		return NIL_VAL;
	}

	double seconds = AS_NUMBER(args[0]);

	if (!(seconds > 0.0)) {
		return NIL_VAL;
	}

	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
		return NIL_VAL;
	}

	long add_sec = (long)seconds;
	double frac = seconds - (double)add_sec;

	if (frac < 0) {
		add_sec -= 1;
		frac += 1.0;
	}

	struct timespec target;
	target.tv_sec = now.tv_sec + add_sec;
	target.tv_nsec = now.tv_nsec + (long)(frac * 1e9);

	if (target.tv_nsec >= 1000000000L) {
		target.tv_sec += target.tv_nsec / 1000000000L;
		target.tv_nsec = target.tv_nsec % 1000000000L;
	}

	while (1) {
		/* NOLINTNEXTLINE(misc-include-cleaner) */
		int r = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &target,
					NULL);
		if (r == 0) {
			break;
		}

		if (r == EINTR) {
			continue;
		}

		errno = r;
		break;
	}

	return NIL_VAL;
}
