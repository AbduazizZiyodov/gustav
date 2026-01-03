#include "debug.h"
#include "common.h"
#include "value.h"

static size_t simple_instruction(const char *name, size_t offset);
static size_t constant_instruction(const char *name, Chunk *chunk,
				   size_t offset);

void disassemble_chunk(Chunk *chunk, const char *name)
{
	LOG_DEBUG("== %s ==", name);

	for (size_t offset = 0; offset < chunk->count;) {
		offset = disassemble_instruction(chunk, offset);
	}
}

size_t disassemble_instruction(Chunk *chunk, size_t offset)
{
	uint8_t instruction = chunk->code[offset];

	switch (instruction) {
	case OP_RETURN:
		return simple_instruction("OP_RETURN", offset);
	case OP_CONSTANT:
		return constant_instruction("OP_CONSTANT", chunk, offset);
	default:
		LOG_ERROR("Unknown opcode %d", instruction);
		return offset + 1;
	}
}

static size_t simple_instruction(const char *name, size_t offset)
{
	LOG_DEBUG("%04d %s", offset, name);
	return offset + 1;
}

static size_t constant_instruction(const char *name, Chunk *chunk,
				   size_t offset)
{
	uint8_t constant = chunk->code[offset + 1];
	Value value = chunk->constants.values[constant];
	LOG_DEBUG("%04d %s %04d value=%g", offset, name, constant, value);
	return offset + 2;
}
