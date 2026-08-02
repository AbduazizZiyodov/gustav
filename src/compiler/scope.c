#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "chunk.h"
#include "common.h"
#include "scanner.h"

#include "byte_code.h"
#include "error.h"
#include "expression.h"
#include "internal.h"
#include "scope.h"

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

void add_local(Token name)
{
	if (current->local_count == UINT8_COUNT) {
		compiler_error("Too many local variables in function.");
		return;
	}
	Local *local = &current->locals[current->local_count++];
	local->name = name;
	local->depth = -1;
	local->is_captured = false;
}

void mark_initialized(void)
{
	if (current->scope_depth == 0) {
		return;
	}
	current->locals[current->local_count - 1].depth = current->scope_depth;
}

int resolve_local(Compiler *compiler, Token *name)
{
	for (int i = compiler->local_count - 1; i >= 0; i--) {
		Local *local = &compiler->locals[i];

		if (identifiers_equal(name, &local->name)) {
			if (local->depth == -1) {
				compiler_error(

					"Can't read local variable in its own initializer.");
			}
			return i;
		}
	}
	return -1;
}

static int add_upvalue(Compiler *compiler, uint8_t index, bool is_local)
{
	int upvalue_count = compiler->function->upvalue_count;

	for (int i = 0; i < upvalue_count; i++) {
		Upvalue *upvalue = &compiler->upvalues[i];
		if (upvalue->index == index && upvalue->is_local == is_local) {
			return i;
		}
	}

	if (upvalue_count == UINT8_MAX) {
		compiler_error("Too many closure variables in function.");
		return 0;
	}

	compiler->upvalues[upvalue_count].is_local = is_local;
	compiler->upvalues[upvalue_count].index = index;

	return compiler->function->upvalue_count++;
}

int resolve_upvalue(Compiler *compiler, Token *name)
{
	if (compiler->enclosing == NULL) {
		return -1;
	}

	int local = resolve_local(compiler->enclosing, name);

	if (local != -1) {
		compiler->enclosing->locals[local].is_captured = true;
		return add_upvalue(compiler, (uint8_t)local, true);
	}

	int upvalue = resolve_upvalue(compiler->enclosing, name);

	if (upvalue != -1) {
		return add_upvalue(compiler, (uint8_t)upvalue, false);
	}

	return -1;
}
