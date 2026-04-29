#ifndef GUSTAV_VM_H
#define GUSTAV_VM_H

#include "chunk.h"
#include "common.h"
#include "hash_table.h"
#include "object.h"
#include "value.h"
#include <stdint.h>

#define FRAMES_MAX 65
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
	ObjClosure *closure;
	uint8_t *ip;
	value_t *slots;
} call_frame_t;

typedef struct {
	chunk_t *chunk;
	uint8_t *ip;
	value_t stack[STACK_MAX];
	value_t *stack_top;
	obj_t *objects;
	hash_table_t strings;
	hash_table_t globals;

	size_t frame_count;
	ObjUpvalue *open_upvalues;
	call_frame_t frames[FRAMES_MAX];

	size_t gray_count;
	size_t gray_capacity;
	obj_t **gray_stack;

	size_t bytes_allocated;
	size_t next_gc;
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} interpreter_result_t;

extern VM vm;

void init_vm(void);
void free_vm(void);
interpreter_result_t interpret(const char *source);

void push(value_t value);
value_t pop(void);

#endif // GUSTAV_VM_H
