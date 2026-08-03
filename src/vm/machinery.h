#pragma once

#include "value.h"

bool is_falsey(Value value);
void concatenate(void);

Value VM_Peek(int distance);
void reset_stack(void);
