#include <stddef.h>
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
static size_t byte_instruction(const char *name, chunk_t *chunk, size_t offset);
static size_t jump_instruction(const char *name, int sign, chunk_t *chunk,
			       size_t offset);

void disassemble_chunk(chunk_t *chunk, const char *name)
{
	printf("\n");

	LOG_DEBUG("== [%s] ==\n", name);
	for (size_t offset = 0; offset < chunk->count;) {
		offset = disassemble_instruction(chunk, offset);
	}
	LOG_DEBUG("== [/%s] ==\n\n", name);
}

size_t disassemble_instruction(chunk_t *chunk, size_t offset)
{
	current_instruction = chunk->code[offset];

	const char *op_string = OP_CODE_STRING[current_instruction];

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
	case OP_PRINT:
	case OP_POP:
		LOG_DEBUG("%04d %s\n", offset, op_string);
		return offset + 1;
	case OP_CONSTANT:
	case OP_GET_GLOBAL:
	case OP_DEFINE_GLOBAL:
	case OP_SET_GLOBAL:
		return constant_instruction(op_string, chunk, offset);
	case OP_GET_LOCAL:
	case OP_SET_LOCAL:
	case OP_CALL:
		return byte_instruction(op_string, chunk, offset);
	case OP_JUMP:
	case OP_JUMP_IF_FALSE:
		return jump_instruction(op_string, 1, chunk, offset);
	case OP_LOOP:
		return jump_instruction(op_string, -1, chunk, offset);
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

	LOG_DEBUG("%04d %s %04d value=", offset, name, constant);
	print_value(value);
	printf("\n");

	return offset + 2;
}

static size_t byte_instruction(const char *name, chunk_t *chunk, size_t offset)
{
	uint8_t slot = chunk->code[offset + 1];
	LOG_DEBUG("%04d %s slot=%04d\n", offset, name, slot);
	return offset + 2;
}

static size_t jump_instruction(const char *name, int sign, chunk_t *chunk,
			       size_t offset)
{
	uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
	jump |= chunk->code[offset + 2];

	size_t target = offset + 3 + (size_t)(sign * (int)jump);

	printf("%-16s %4ld -> %ld\n", name, (long)offset, (long)target);
	return offset + 3;
}

#else

#include "chunk.h"
#include "debug.h"

void disassemble_chunk(chunk_t *chunk __attribute__((unused)),
		       const char *name __attribute__((unused)))
{
}
#endif // DEBUG
