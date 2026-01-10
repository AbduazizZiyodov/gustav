/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifdef DEBUG

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

enum { _LOG_TRACE, _LOG_DEBUG, _LOG_INFO, _LOG_WARN, _LOG_ERROR, _LOG_FATAL };

#define LOG_TRACE(...) log_log(_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) log_log(_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) log_log(_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) log_log(_LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_log(_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) log_log(_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

const char *log_level_string(int level);
void log_set_lock(log_LockFn fn, void *udata);
void log_set_level(int level);
void log_set_quiet(bool enable);
int log_add_callback(log_LogFn fn, void *udata, int level);
int log_add_fp(FILE *fp, int level);

void log_log(int level, const char *file, int line, const char *fmt, ...);
#endif

#else

#define LOG_TRACE(...)
#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_FATAL(...)

void log_log(int level __attribute__((unused)),
	     const char *file __attribute__((unused)),
	     int line __attribute__((unused)),
	     const char *fmt __attribute__((unused)), ...);

#endif
