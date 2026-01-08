#ifndef GUSTAV_CHUNK_H
#define GUSTAV_CHUNK_H

#include "value.h"
#include <stdint.h>

typedef enum {
	OP_CONSTANT,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_RETURN,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NOT,
	OP_NEGATE,
} OpCode;

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

#define LOG_CHUNK(chunk) \
	LOG_DEBUG("Chunk(capacity=%d count=%d)", chunk.capacity, chunk.count);

#endif
