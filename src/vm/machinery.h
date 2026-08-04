#pragma once

#include "value.h"

bool is_falsey(Value value);
void VM_String_Concatenate(void);

Value VM_Peek(int distance);
void VM_Reset_Stack(void);
