#pragma once

#include <stddef.h>
#include <stdint.h>

#include "chunk.h"
#include "common.h"
#include "hash_table.h"
#include "object.h"
#include "value.h"

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
	StringObject *init_string;
	HashTable globals;

	size_t frame_count;
	UpvalueObject *open_upvalues;
	CallFrame frames[FRAMES_MAX];

	int exit_status;
} VM;

extern VM vm;
