#include <stddef.h>
#include <stdint.h>

#include "chunk.h"
#include "common.h"
#include "scanner.h"

#include "compiler.h"
#include "compiler/byte_code.h"
#include "compiler/error.h"
#include "compiler/parse.h"
#include "compiler/utils.h"
#include "compiler/var.h"

uint8_t parse_variable(const char *error_message)
{
	consume(TOKEN_IDENTIFIER, error_message);

	declare_variable();

	if (current->scope_depth > 0) {
		return 0;
	}

	return identifier_constant(&parser_state.previous);
}

void mark_initialized(void)
{
	if (current->scope_depth == 0) {
		return;
	}
	current->locals[current->local_count - 1].depth = current->scope_depth;
}

void define_variable(uint8_t global)
{
	if (current->scope_depth > 0) {
		mark_initialized();
		return;
	}
	EMIT_BYTES(OP_DEFINE_GLOBAL, global);
}

void var_declaration(void)
{
	uint8_t global = parse_variable("Expect variable name.");

	if (match(TOKEN_EQUAL)) {
		expression();
	} else {
		emit_byte(OP_NIL);
	}

	consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");
	define_variable(global);
}

void declare_variable(void)
{
	if (current->scope_depth == 0) {
		return;
	}

	token_t *name = &parser_state.previous;

	for (int i = current->local_count - 1; i >= 0; i--) {
		Local *local = &current->locals[i];

		if (local->depth != -1 && local->depth < current->scope_depth) {
			break;
		}

		if (identifiers_equal(name, &local->name)) {
			compiler_error(

				"Already a variable with this name in this scope.");
		}
	}

	add_local(*name);
}

void named_variable(token_t name, bool can_assign)
{
	uint8_t get_op;
	uint8_t set_op;

	int arg = resolve_local(current, &name);

	if (arg != -1) {
		get_op = OP_GET_LOCAL;
		set_op = OP_SET_LOCAL;
		/* NOLINTNEXTLINE(bugprone-assignment-in-if-condition) */
	} else if ((arg = resolve_upvalue(current, &name)) != -1) {
		get_op = OP_GET_UPVALUE;
		set_op = OP_SET_UPVALUE;
	}

	else {
		arg = identifier_constant(&name);
		get_op = OP_GET_GLOBAL;
		set_op = OP_SET_GLOBAL;
	}

	if (can_assign && match(TOKEN_EQUAL)) {
		expression();
		EMIT_BYTES(set_op, (uint8_t)arg);
	} else {
		EMIT_BYTES(get_op, (uint8_t)arg);
	}
}

void add_local(token_t name)
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

int resolve_local(Compiler *compiler, token_t *name)
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

int add_upvalue(Compiler *compiler, uint8_t index, bool is_local)
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

int resolve_upvalue(Compiler *compiler, token_t *name)
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
