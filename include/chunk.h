#pragma once

#include <stddef.h>
#include <stdint.h>

#include "value.h"

// Source - https://stackoverflow.com/a/10966395
// Posted by Terrence M, modified by community. See post 'Timeline' for change
// history Retrieved 2026-01-08, License - CC BY-SA 3.0
#define FOREACH_OP_CODE(DO)  \
	DO(OP_CONSTANT)      \
	DO(OP_NIL)           \
	DO(OP_TRUE)          \
	DO(OP_FALSE)         \
	DO(OP_EQUAL)         \
	DO(OP_GREATER)       \
	DO(OP_LESS)          \
	DO(OP_RETURN)        \
	DO(OP_ADD)           \
	DO(OP_SUBTRACT)      \
	DO(OP_MULTIPLY)      \
	DO(OP_DIVIDE)        \
	DO(OP_POW)           \
	DO(OP_CONCAT)        \
	DO(OP_NOT)           \
	DO(OP_NEGATE)        \
	DO(OP_PRINT_STDOUT)  \
	DO(OP_PRINT_STDERR)  \
	DO(OP_POP)           \
	DO(OP_DEFINE_GLOBAL) \
	DO(OP_GET_GLOBAL)    \
	DO(OP_SET_GLOBAL)    \
	DO(OP_GET_LOCAL)     \
	DO(OP_SET_LOCAL)     \
	DO(OP_JUMP_IF_FALSE) \
	DO(OP_JUMP)          \
	DO(OP_LOOP)          \
	DO(OP_CALL)          \
	DO(OP_CLOSURE)       \
	DO(OP_GET_UPVALUE)   \
	DO(OP_SET_UPVALUE)   \
	DO(OP_CLOSE_UPVALUE) \
	DO(OP_CLASS)         \
	DO(OP_GET_PROPERTY)  \
	DO(OP_SET_PROPERTY)  \
	DO(OP_METHOD)        \
	DO(OP_INVOKE)        \
	DO(OP_INHERIT)       \
	DO(OP_GET_SUPER)     \
	DO(OP_SUPER_INVOKE)  \
	DO(OP_UNINITIALIZED)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum { FOREACH_OP_CODE(GENERATE_ENUM) } OpCode;

static const char *OP_CODE_STRING[] = { FOREACH_OP_CODE(GENERATE_STRING) };

typedef struct {
	size_t count;
	size_t capacity;
	uint8_t *code;
	ValueArray constants;
	int *lines;
} Chunk;

void Chunk_Init(Chunk *chunk);
void Chunk_Free(Chunk *chunk);

void Chunk_Write(Chunk *chunk, uint8_t byte, size_t line);

size_t Chunk_AddConstant(Chunk *chunk, Value value);
