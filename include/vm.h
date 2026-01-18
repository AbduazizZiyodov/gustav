#ifndef GUSTAV_VM_H
#define GUSTAV_VM_H

#include "chunk.h"
#include "value.h"

typedef struct {
	chunk_t *chunk;
	uint8_t *ip;
	value_t stack[STACK_MAX];
	value_t *stack_top;
	obj_t *objects;
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

#endif
