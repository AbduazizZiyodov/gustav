#pragma once

#include "value.h"

bool is_falsey(Value value);
void vm_string_concatenate(void);

Value vm_peek(int distance);
void vm_reset_stack(void);
