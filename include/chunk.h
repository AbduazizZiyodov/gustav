#ifndef GUSTAV_CHUNK_H
#define GUSTAV_CHUNK_H

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
	DO(OP_PRINT)         \
	DO(OP_POP)           \
	DO(OP_DEFINE_GLOBAL) \
	DO(OP_GET_GLOBAL)    \
	DO(OP_SET_GLOBAL)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum { FOREACH_OP_CODE(GENERATE_ENUM) } op_code_t;

static const char *OP_CODE_STRING[] = { FOREACH_OP_CODE(GENERATE_STRING) };

typedef struct {
	size_t count;
	size_t capacity;
	uint8_t *code;
	value_array_t constants;
	int *lines;
} chunk_t;

void init_chunk(chunk_t *chunk);
void free_chunk(chunk_t *chunk);

void write_chunk(chunk_t *chunk, uint8_t byte, size_t line);

size_t add_constant(chunk_t *chunk, value_t value);

#endif
