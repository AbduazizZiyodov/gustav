#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "compiler.h"
#include "scanner.h"

#include "compiler/byte_code.h"
#include "compiler/class.h"
#include "compiler/error.h"
#include "compiler/function.h"
#include "compiler/parse.h"
#include "compiler/scope.h"
#include "compiler/utils.h"
#include "compiler/var.h"

static token_t
sentinel_token(const char *text) // original name was "synthetic_token"
{
	token_t token = { .start = text, .length = strlen(text) };
	return token;
}

void method()
{
	consume(TOKEN_IDENTIFIER, "Expect method name.");
	uint8_t constant = identifier_constant(&parser_state.previous);

	FunctionType type = TYPE_METHOD;

	if (parser_state.previous.length == 4 &&
	    memcmp(parser_state.previous.start, "init", 4) == 0) {
		type = TYPE_INITIALIZER;
	}

	function(type);

	EMIT_BYTES(OP_METHOD, constant);
}

void class_declaration()
{
	consume(TOKEN_IDENTIFIER, "Expect class name.");

	token_t class_name = parser_state.previous;

	uint8_t name_constant = identifier_constant(&parser_state.previous);
	declare_variable();

	EMIT_BYTES(OP_CLASS, name_constant);
	define_variable(name_constant);

	ClassCompiler class_compiler;
	class_compiler.has_super_class = false;
	class_compiler.enclosing = current_class;
	current_class = &class_compiler;

	if (match(TOKEN_LESS)) {
		consume(TOKEN_IDENTIFIER, "Expect superclass name.");
		variable(false);

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

	consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");

	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
		method();
	}

	consume(TOKEN_RIGHT_BRACE, "Expect '}' before class body.");
	emit_byte(OP_POP);

	if (class_compiler.has_super_class) {
		end_scope();
	}

	current_class = current_class->enclosing;
}

void super(bool can_assign [[maybe_unused]])
{
	if (current_class == NULL) {
		compiler_error("Can't use 'super' outside of a class.");
	} else if (!current_class->has_super_class) {
		compiler_error(
			"Can't use 'super' in a class with no superclass.");
	}

	consume(TOKEN_DOT, "Expect '.' after 'super'.");
	consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
	uint8_t name = identifier_constant(&parser_state.previous);

	named_variable(sentinel_token("this"), false);

	if (match(TOKEN_LEFT_PAREN)) {
		uint8_t arg_count = argument_list();
		named_variable(sentinel_token("super"), false);
		EMIT_BYTES(OP_SUPER_INVOKE, name);
		emit_byte(arg_count);
	} else {
		named_variable(sentinel_token("super"), false);
		EMIT_BYTES(OP_GET_SUPER, name);
	}
}

void this(bool can_assign [[maybe_unused]])
{
	if (current_class == NULL) {
		compiler_error("Can't use 'this' outside of a class.");
		return;
	}

	variable(false);
}
