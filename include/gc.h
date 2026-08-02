#pragma once

#include "value.h"

#define GC_HEAP_GROW_FACTOR 2

void GC_Collect(void);

void GC_MarkObject(Object *object);
void GC_MarkValue(Value value);
