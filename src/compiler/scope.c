#include "chunk.h"

#include "compiler.h"
#include "compiler/byte_code.h"
#include "compiler/scope.h"

void begin_scope(void)
{
	current->scope_depth++;
}

void end_scope(void)
{
	current->scope_depth--;

	while (current->local_count > 0 &&
	       current->locals[current->local_count - 1].depth >
		       current->scope_depth) {
		if (current->locals[current->local_count - 1].is_captured) {
			emit_byte(OP_CLOSE_UPVALUE);
		} else {
			emit_byte(OP_POP);
		}
		current->local_count--;
	}
}
