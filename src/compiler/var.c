#include <stddef.h>
#include <stdint.h>

#include "chunk.h"
#include "scanner.h"

#include "byte_code.h"
#include "error.h"
#include "expression.h"
#include "internal.h"
#include "scope.h"
#include "utils.h"
#include "var.h"

uint8_t parse_variable(const char *error_message)
{
	token_consume(TOKEN_IDENTIFIER, error_message);

	declare_variable();

	if (current->scope_depth > 0) {
		return 0;
	}

	return identifier_constant(&parser_state.previous);
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

	if (token_match(TOKEN_EQUAL)) {
		expression_parse();
	} else {
		emit_byte(OP_UNINITIALIZED);
	}

	token_consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");
	define_variable(global);
}

void declare_variable(void)
{
	if (current->scope_depth == 0) {
		return;
	}

	Token *name = &parser_state.previous;

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

void named_variable(Token name, bool can_assign)
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

	if (can_assign && token_match(TOKEN_EQUAL)) {
		expression_parse();
		EMIT_BYTES(set_op, (uint8_t)arg);
	} else {
		EMIT_BYTES(get_op, (uint8_t)arg);
	}
}
