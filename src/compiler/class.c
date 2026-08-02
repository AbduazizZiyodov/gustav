#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "internal.h"
#include "scanner.h"

#include "byte_code.h"
#include "class.h"
#include "error.h"
#include "expression.h"
#include "function.h"
#include "scope.h"
#include "utils.h"
#include "var.h"

static Token sentinel_token(const char *text)
{
	Token token = { .start = text, .length = strlen(text) };
	return token;
}

static void method(void)
{
	token_consume(TOKEN_IDENTIFIER, "Expect method name.");
	uint8_t constant = identifier_constant(&parser_state.previous);

	FunctionType type = TYPE_METHOD;

	if (parser_state.previous.length == 4 &&
	    memcmp(parser_state.previous.start, "init", 4) == 0) {
		type = TYPE_INITIALIZER;
	}

	function_compile(type);

	EMIT_BYTES(OP_METHOD, constant);
}

void class_declaration(void)
{
	token_consume(TOKEN_IDENTIFIER, "Expect class name.");

	Token class_name = parser_state.previous;

	uint8_t name_constant = identifier_constant(&parser_state.previous);
	declare_variable();

	EMIT_BYTES(OP_CLASS, name_constant);
	define_variable(name_constant);

	ClassCompiler class_compiler;
	class_compiler.has_super_class = false;
	class_compiler.enclosing = current_class;
	current_class = &class_compiler;

	if (token_match(TOKEN_LESS)) {
		token_consume(TOKEN_IDENTIFIER, "Expect superclass name.");
		expression_variable(false);

		if (identifiers_equal(&class_name, &parser_state.previous)) {
			compiler_error("A class can't inherit from itself.");
		}

		begin_scope();
		add_local(sentinel_token("super"));
		define_variable(0);

		named_variable(class_name, false);
		emit_byte(OP_INHERIT);
		class_compiler.has_super_class = true;
	}

	named_variable(class_name, false);

	token_consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");

	while (!token_check(TOKEN_RIGHT_BRACE) && !token_check(TOKEN_EOF)) {
		method();
	}

	token_consume(TOKEN_RIGHT_BRACE, "Expect '}' before class body.");
	emit_byte(OP_POP);

	if (class_compiler.has_super_class) {
		end_scope();
	}

	current_class = current_class->enclosing;
}

void class_super(bool can_assign [[maybe_unused]])
{
	if (current_class == NULL) {
		compiler_error("Can't use 'super' outside of a class.");
	} else if (!current_class->has_super_class) {
		compiler_error(
			"Can't use 'super' in a class with no superclass.");
	}

	token_consume(TOKEN_DOT, "Expect '.' after 'super'.");
	token_consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
	uint8_t name = identifier_constant(&parser_state.previous);

	named_variable(sentinel_token("this"), false);

	if (token_match(TOKEN_LEFT_PAREN)) {
		uint8_t arg_count = argument_list();
		named_variable(sentinel_token("super"), false);
		EMIT_BYTES(OP_SUPER_INVOKE, name);
		emit_byte(arg_count);
	} else {
		named_variable(sentinel_token("super"), false);
		EMIT_BYTES(OP_GET_SUPER, name);
	}
}

void class_this(bool can_assign [[maybe_unused]])
{
	if (current_class == NULL) {
		compiler_error("Can't use 'this' outside of a class.");
		return;
	}

	expression_variable(false);
}
