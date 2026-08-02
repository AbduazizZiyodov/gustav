#pragma once

#include "chunk.h"
#include "common.h"
#include "hash_table.h"
#include "object.h"
#include "value.h"
#include <stdint.h>

#define FRAMES_MAX 65
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
	ClosureObject *closure;
	uint8_t *ip;
	Value *slots;
} CallFrame;

typedef struct {
	Chunk *chunk;
	uint8_t *ip;
	Value stack[STACK_MAX];
	Value *stack_top;
	Object *objects;
	StringObject *init_string;
	HashTable strings;
	HashTable globals;

	size_t frame_count;
	UpvalueObject *open_upvalues;
	CallFrame frames[FRAMES_MAX];

	size_t gray_count;
	size_t gray_capacity;
	Object **gray_stack;

	size_t bytes_allocated;
	size_t next_gc;
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void VM_Init(void);
void VM_Free(void);
InterpretResult VM_Interpret(const char *source);

void VM_Push(Value value);
Value VM_Pop(void);
