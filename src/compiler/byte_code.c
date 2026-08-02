#include <stddef.h>
#include <stdint.h>

#include "chunk.h"
#include "compiler.h"
#include "value.h"

#include "compiler/byte_code.h"
#include "compiler/error.h"
#include "compiler/function.h"
#include "compiler/utils.h"

#define EMIT_BYTES(first_byte, second_byte) \
	emit_byte(first_byte);              \
	emit_byte(second_byte);

void emit_byte(uint8_t byte)
{
	write_chunk(current_chunk(), byte, parser_state.previous.line);
}

void emit_loop(size_t loop_start)
{
	emit_byte(OP_LOOP);
	size_t offset = current_chunk()->count - loop_start + 2;

	if (offset > UINT16_MAX) {
		compiler_error("Loop body too large");
	}

	emit_byte((offset >> 8) & 0xff);
	emit_byte(offset & 0xff);
}

int emit_jump(uint8_t instruction)
{
	emit_byte(instruction);
	EMIT_BYTES(0xff, 0xff);
	return (int)current_chunk()->count - 2;
}

void emit_return(void)
{
	if (current->type == TYPE_INITIALIZER) {
		EMIT_BYTES(OP_GET_LOCAL, 0);
	} else {
		emit_byte(OP_NIL);
	}
	emit_byte(OP_RETURN);
}

uint8_t make_constant(value_t value)
{
	size_t constant = add_constant(current_chunk(), value);

	if (constant > UINT8_MAX) {
		compiler_error("Too many constants in one chunk");
		return 0;
	}

	return (uint8_t)constant;
}

void emit_constant(value_t value)
{
	EMIT_BYTES(OP_CONSTANT, make_constant(value));
}

void patch_jump(int offset)
{
	int jump = ((int)current_chunk()->count) - offset - 2;

	if (jump > UINT16_MAX) {
		compiler_error("Too much code to jump over");
	}

	current_chunk()->code[offset] = (jump >> 8) & 0xff;
	current_chunk()->code[offset + 1] = jump & 0xff;
}
