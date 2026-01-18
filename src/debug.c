#ifdef DEBUG

#include <stdint.h>
#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "log.h"
#include "value.h"

static uint8_t current_instruction;

static size_t constant_instruction(const char *name, chunk_t *chunk,
				   size_t offset);

void disassemble_chunk(chunk_t *chunk, const char *name)
{
	printf("\n");

	LOG_DEBUG("== [%s] ==\n", name);
	for (size_t offset = 0; offset < chunk->count;) {
		offset = disassemble_instruction(chunk, offset);
	}
	LOG_DEBUG("== end ==\n\n", name);
}

size_t disassemble_instruction(chunk_t *chunk, size_t offset)
{
	current_instruction = chunk->code[offset];

	switch (current_instruction) {
	case OP_RETURN:
	case OP_NIL:
	case OP_FALSE:
	case OP_EQUAL:
	case OP_GREATER:
	case OP_LESS:
	case OP_TRUE:
	case OP_ADD:
	case OP_SUBTRACT:
	case OP_MULTIPLY:
	case OP_POW:
	case OP_CONCAT:
	case OP_DIVIDE:
	case OP_NOT:
	case OP_NEGATE:
		LOG_TRACE("%04d %s\n", offset,
			  OP_CODE_STRING[current_instruction]);
		return offset + 1;
	case OP_CONSTANT:
		return constant_instruction("OP_CONSTANT", chunk, offset);
	default:
		LOG_ERROR("Unknown opcode %d\n", current_instruction);
		return offset + 1;
	}
}

static size_t constant_instruction(const char *name, chunk_t *chunk,
				   size_t offset)
{
	uint8_t constant = chunk->code[offset + 1];
	value_t value = chunk->constants.values[constant];

	LOG_TRACE("%04d %s %04d value=", offset, name, constant);
	print_value(value);
	printf("\n");

	return offset + 2;
}

#else

#include "chunk.h"
#include "debug.h"

void disassemble_chunk(chunk_t *chunk __attribute__((unused)),
		       const char *name __attribute__((unused)))
{
}
#endif
