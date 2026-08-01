/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define LOG_VERSION "0.1.0"

typedef struct {
	va_list ap;
	const char *fmt;
	const char *file;
	struct tm *time;
	void *udata;
	int line;
	int level;
} log_Event;

typedef void (*log_LogFn)(log_Event *ev);
typedef void (*log_LockFn)(bool lock, void *udata);

enum { LOG_TRACE_, LOG_DEBUG_, LOG_INFO_, LOG_WARN_, LOG_ERROR_, LOG_FATAL_ };

#define LOG_TRACE(...) log_log(LOG_TRACE_, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log_log(LOG_DEBUG_, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) log_log(LOG_INFO_, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) log_log(LOG_WARN_, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_log(LOG_ERROR_, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_log(LOG_FATAL_, __FILE__, __LINE__, __VA_ARGS__)

const char *log_level_string(int level);
void log_set_lock(log_LockFn fn, void *udata);
void log_set_level(int level);
void log_set_quiet(bool enable);
int log_add_callback(log_LogFn fn, void *udata, int level);
int log_add_fp(FILE *fp, int level);

void log_log(int level, const char *file, int line, const char *fmt, ...);
#endif // LOG_H

#ifdef DEBUG_LOG_GC
#define LOG_GC(...) log_log(LOG_TRACE_, __FILE__, __LINE__, __VA_ARGS__)
#else
#define LOG_GC(...)
#endif // DEBUG_LOG_GC
