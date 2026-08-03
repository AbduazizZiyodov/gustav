#pragma once

#include <stdint.h>

#include "value.h"

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
	INTERPRET_EXIT
} InterpretResult;

void VM_Init(void);
void VM_Free(void);
InterpretResult VM_Interpret(const char *source);

void VM_Push(Value value);
Value VM_Pop(void);

void VM_MarkRoots(void);
int VM_ExitStatus(void);
