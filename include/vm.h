#pragma once

#include <stdint.h>

#include "value.h"

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
	INTERPRET_EXIT
} InterpretResult;

void init_vm(void);
void free_vm(void);
InterpretResult VM_Interpret(const char *source);

void vm_push(Value value);
Value vm_pop(void);

void vm_mark_roots(void);
int vm_exit_status(void);
