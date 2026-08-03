#ifdef DEBUG

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "log.h"
#include "object.h"
#include "value.h"

static uint8_t current_instruction;

static int constant_instruction(const char *name, Chunk *chunk, int offset);
static int byte_instruction(const char *name, Chunk *chunk, int offset);
static int jump_instruction(const char *name, int sign, Chunk *chunk, int offset);
static int invoke_instruction(const char *name, Chunk *chunk, int offset);

void Debug_DisassembleChunk(Chunk *chunk, const char *name)
{
	printf("\n");

	LOG_DEBUG("== [%s] ==\n", name);
	for (int offset = 0; offset < (int)chunk->count;) {
		offset = Debug_DisassembleInstruction(chunk, offset);
	}
	LOG_DEBUG("== [/%s] ==\n\n", name);
}

int Debug_DisassembleInstruction(Chunk *chunk, int offset)
{
	current_instruction = chunk->code[offset];

	const char *op_string = OP_CODE_STRING[current_instruction];

	switch (current_instruction) {
	case OP_RETURN:
	case OP_RETURN_EXIT:
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
	case OP_PRINT_STDOUT:
	case OP_PRINT_STDERR:
	case OP_POP:
	case OP_CLOSE_UPVALUE:
	case OP_INHERIT:
	case OP_UNINITIALIZED:
		LOG_DEBUG("%04d %s\n", offset, op_string);
		return offset + 1;
	case OP_CONSTANT:
	case OP_GET_GLOBAL:
	case OP_DEFINE_GLOBAL:
	case OP_CLASS:
	case OP_SET_GLOBAL:
	case OP_GET_PROPERTY:
	case OP_SET_PROPERTY:
	case OP_METHOD:
	case OP_GET_SUPER:
		return constant_instruction(op_string, chunk, offset);
	case OP_GET_LOCAL:
	case OP_SET_LOCAL:
	case OP_GET_UPVALUE:
	case OP_SET_UPVALUE:
	case OP_CALL:
		return byte_instruction(op_string, chunk, offset);
	case OP_JUMP:
	case OP_JUMP_IF_FALSE:
		return jump_instruction(op_string, 1, chunk, offset);
	case OP_LOOP:
		return jump_instruction(op_string, -1, chunk, offset);
	case OP_CLOSURE: {
		offset++;
		uint8_t constant = chunk->code[offset++];
		LOG_DEBUG("%-16s %4d ", "OP_CLOSURE", constant);
		Value_Print(stdout, chunk->constants.values[constant]);
		printf("\n");

		FunctionObject *function = AS_FUNCTION(chunk->constants.values[constant]);

		for (int i = 0; i < function->upvalue_count; i++) {
			int is_local = chunk->code[offset++];
			int index = chunk->code[offset++];

			LOG_DEBUG("%04d\t\t|\t%s %d\n", (int)offset - 2,
				  is_local ? "local" : "upvalue", index);
		}

		return offset;
	}
	case OP_INVOKE:
	case OP_SUPER_INVOKE:
		return invoke_instruction(op_string, chunk, offset);
	default:
		LOG_ERROR("Unknown opcode %d\n", current_instruction);
		return offset + 1;
	}
}

static int invoke_instruction(const char *name, Chunk *chunk, int offset)
{
	uint8_t constant = chunk->code[offset + 1];
	uint8_t arg_count = chunk->code[offset + 2];
	LOG_DEBUG("%-16s (%d args) %4d '", name, arg_count, constant);
	Value_Print(stdout, chunk->constants.values[constant]);
	printf("\n");

	return offset + 3;
}

static int constant_instruction(const char *name, Chunk *chunk, int offset)
{
	uint8_t constant = chunk->code[offset + 1];
	Value value = chunk->constants.values[constant];

	LOG_DEBUG("%04d %s %04d value=", offset, name, constant);
	Value_Print(stdout, value);
	printf("\n");

	return offset + 2;
}

static int byte_instruction(const char *name, Chunk *chunk, int offset)
{
	uint8_t slot = chunk->code[offset + 1];
	LOG_DEBUG("%04d %s slot=%04d\n", offset, name, slot);
	return offset + 2;
}

static int jump_instruction(const char *name, int sign, Chunk *chunk, int offset)
{
	uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
	jump |= chunk->code[offset + 2];

	int target = offset + 3 + (sign * (int)jump);

	printf("%-16s %4ld -> %ld\n", name, (long)offset, (long)target);
	return offset + 3;
}

#else

#include "chunk.h"
#include "debug.h"

void Debug_DisassembleChunk(Chunk *chunk [[maybe_unused]], const char *name [[maybe_unused]])
{
}
#endif // DEBUG
