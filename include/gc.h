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

void GC_Init(void);
void GC_Collect(void);

void GC_MarkObject(Object *object);
void GC_MarkValue(Value value);
