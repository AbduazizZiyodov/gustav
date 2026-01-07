#ifndef GUSTAV_VM_H
#define GUSTAV_VM_H

#include "chunk.h"
#include "value.h"

#define STACK_MAX 512

typedef struct {
	Chunk *chunk;
	uint8_t *ip;
	Value stack[STACK_MAX];
	Value *stack_top;
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

void init_vm(void);
void free_vm(void);
InterpretResult interpret(const char *source);

void push(Value value);
Value pop(void);

#endif
