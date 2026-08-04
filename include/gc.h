#pragma once

#include <stddef.h>

#include "value.h"

#define GC_HEAP_GROW_FACTOR 2

typedef struct {
	Object *objects;
	size_t bytes_allocated;
	size_t next_gc;

	size_t gray_count;
	size_t gray_capacity;
	Object **gray_stack;
} GC;

extern GC gc;

void gc_init(void);
void gc_collect(void);

void gc_mark_object(Object *object);
void gc_mark_value(Value value);
