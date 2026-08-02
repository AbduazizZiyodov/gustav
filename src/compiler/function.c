#include <stdint.h>

#include "chunk.h"
#include "scanner.h"
#include "value.h"
#include "vm.h"

#include "compiler.h"
#include "compiler/byte_code.h"
#include "compiler/error.h"
#include "compiler/function.h"
#include "compiler/parse.h"
#include "compiler/scope.h"
#include "compiler/utils.h"
#include "compiler/var.h"

void fun_declaration(void)
{
	uint8_t global = parse_variable("Expect function name.");
	mark_initialized();
	function(TYPE_FUNCTION);
	define_variable(global);
}

uint8_t argument_list(void)
{
	uint8_t arg_count = 0;
	if (!check(TOKEN_RIGHT_PAREN)) {
		do {
			expression();
			if (arg_count == 255) {
				compiler_error(
					"Can't have more than 255 arguments.");
			}
			arg_count++;
		} while (match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
	return arg_count;
}

void function(FunctionType type)
{
	Compiler compiler;

	init_compiler(&compiler, type);
	begin_scope();

	consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

	if (!check(TOKEN_RIGHT_PAREN)) {
		do {
			current->function->arity++;
			if (current->function->arity > 255) { // god forgive me
				error_at_current(
					"Can't have more than 255 parameters.");
			}
			uint8_t constant =
				parse_variable("Expect parameters name.");
			define_variable(constant);
		} while (match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
	consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
	block();
	function_t *function = finish_compiling();
	value_t function_value = OBJ_VAL(function);
	push(function_value);
	EMIT_BYTES(OP_CLOSURE, make_constant(function_value));
	pop();

	for (int i = 0; i < function->upvalue_count; i++) {
		EMIT_BYTES(compiler.upvalues[i].is_local ? 1 : 0,
			   compiler.upvalues[i].index)
	}
}
