#ifndef GUSTAV_CHUNK_H
#define GUSTAV_CHUNK_H

#include <stdint.h>

#include "value.h"

// Source - https://stackoverflow.com/a/10966395
// Posted by Terrence M, modified by community. See post 'Timeline' for change history
// Retrieved 2026-01-08, License - CC BY-SA 3.0
#define str(x) #x
#define xstr(x) str(x)
#define FOREACH_OP_CODE(DO) \
	DO(OP_CONSTANT)     \
	DO(OP_NIL)          \
	DO(OP_TRUE)         \
	DO(OP_FALSE)        \
	DO(OP_EQUAL)        \
	DO(OP_GREATER)      \
	DO(OP_LESS)         \
	DO(OP_RETURN)       \
	DO(OP_ADD)          \
	DO(OP_SUBTRACT)     \
	DO(OP_MULTIPLY)     \
	DO(OP_DIVIDE)       \
	DO(OP_IS)           \
	DO(OP_POW)          \
	DO(OP_CONCAT)       \
	DO(OP_NOT)          \
	DO(OP_NEGATE)

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

void init_chunk(Chunk *chunk);
void free_chunk(Chunk *chunk);

void write_chunk(Chunk *chunk, uint8_t byte, size_t line);

size_t add_constant(Chunk *chunk, Value value);

#endif
