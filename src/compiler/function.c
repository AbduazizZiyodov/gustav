#include <stdint.h>

#include "chunk.h"
#include "scanner.h"
#include "value.h"
#include "vm.h"

#include "byte_code.h"
#include "error.h"
#include "expression.h"
#include "function.h"
#include "internal.h"
#include "scope.h"
#include "statement.h"
#include "utils.h"
#include "var.h"

void fun_declaration(void)
{
	uint8_t global = parse_variable("Expect function name.");
	mark_initialized();
	function_compile(TYPE_FUNCTION);
	define_variable(global);
}

uint8_t argument_list(void)
{
	uint8_t arg_count = 0;
	if (!token_check(TOKEN_RIGHT_PAREN)) {
		do {
			expression_parse();
			if (arg_count == 255) {
				compiler_error("Can't have more than 255 arguments.");
			}
			arg_count++;
		} while (token_match(TOKEN_COMMA));
	}
	token_consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
	return arg_count;
}

void function_compile(FunctionType type)
{
	Compiler compiler;

	init_compiler(&compiler, type);
	begin_scope();

	token_consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

	if (!token_check(TOKEN_RIGHT_PAREN)) {
		do {
			current->function->arity++;
			if (current->function->arity > 255) { // god forgive me
				error_at_current("Can't have more than 255 parameters.");
			}
			uint8_t constant = parse_variable("Expect parameters name.");
			define_variable(constant);
		} while (token_match(TOKEN_COMMA));
	}
	token_consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
	token_consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
	statement_block();
	FunctionObject *function = finish_compiling();
	Value function_value = OBJ_VAL(function);

	vm_push(function_value);
	EMIT_BYTES(OP_CLOSURE, make_constant(function_value));
	vm_pop();

	for (int i = 0; i < function->upvalue_count; i++) {
		EMIT_BYTES(compiler.upvalues[i].is_local ? 1 : 0, compiler.upvalues[i].index)
	}
}
