#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "log.h"
#include "value.h"

static size_t simple_instruction(const char *name, size_t offset);
static size_t constant_instruction(const char *name, Chunk *chunk,
				   size_t offset);

void disassemble_chunk(Chunk *chunk, const char *name)
{
	printf("\n");

	LOG_DEBUG("== %s ==", name);
	for (size_t offset = 0; offset < chunk->count;) {
		offset = disassemble_instruction(chunk, offset);
	}
	LOG_DEBUG("== end ==\n", name);
}

size_t disassemble_instruction(Chunk *chunk, size_t offset)
{
	uint8_t instruction = chunk->code[offset];

	switch (instruction) {
	case OP_RETURN:
		return simple_instruction("OP_RETURN", offset);
	case OP_CONSTANT:
		return constant_instruction("OP_CONSTANT", chunk, offset);
	case OP_NIL:
		return simple_instruction("OP_NIL", offset);
	case OP_FALSE:
		return simple_instruction("OP_FALSE", offset);
	case OP_EQUAL:
		return simple_instruction("OP_EQUAL", offset);
	case OP_IS:
		return simple_instruction("OP_IS", offset);
	case OP_GREATER:
		return simple_instruction("OP_GREATER", offset);
	case OP_LESS:
		return simple_instruction("OP_LESS", offset);
	case OP_TRUE:
		return simple_instruction("OP_TRUE", offset);
	case OP_ADD:
		return simple_instruction("OP_ADD", offset);
	case OP_SUBTRACT:
		return simple_instruction("OP_SUBTRACT", offset);
	case OP_MULTIPLY:
		return simple_instruction("OP_MULTIPLY", offset);
	case OP_DIVIDE:
		return simple_instruction("OP_DIVIDE", offset);
	case OP_NOT:
		return simple_instruction("OP_NEGATE", offset);
	case OP_NEGATE:
		return simple_instruction("OP_NEGATE", offset);
	default:
		LOG_ERROR("Unknown opcode %d", instruction);
		return offset + 1;
	}
}

static size_t simple_instruction(const char *name, size_t offset)
{
	LOG_TRACE("%04d %s", offset, name);
	return offset + 1;
}

static size_t constant_instruction(const char *name, Chunk *chunk,
				   size_t offset)
{
	uint8_t constant = chunk->code[offset + 1];
	Value value = chunk->constants.values[constant];
	LOG_TRACE("%04d %s %04d value=%g", offset, name, constant,
		  AS_NUMBER(value));
	return offset + 2;
}
