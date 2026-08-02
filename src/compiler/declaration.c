#include "scanner.h"

#include "class.h"
#include "declaration.h"
#include "function.h"
#include "internal.h"
#include "statement.h"
#include "utils.h"
#include "var.h"

static void synchronize(void)
{
	parser_state.panic_mode = false;

	while (parser_state.current.type != TOKEN_EOF) {
		if (parser_state.previous.type == TOKEN_SEMICOLON) {
			return;
		}

		switch (parser_state.current.type) {
		case TOKEN_CLASS:
		case TOKEN_FUN:
		case TOKEN_VAR:
		case TOKEN_FOR:
		case TOKEN_IF:
		case TOKEN_WHILE:
		case TOKEN_PRINT_STDOUT:
		case TOKEN_PRINT_STDERR:
		case TOKEN_RETURN:
			return;
		default:;
		}
		token_advance();
	}
}

void declaration_parse(void)
{
	if (token_match(TOKEN_CLASS)) {
		class_declaration();
	} else if (token_match(TOKEN_FUN)) {
		fun_declaration();
	} else if (token_match(TOKEN_VAR)) {
		var_declaration();
	} else {
		statement_parse();
	}

	if (parser_state.panic_mode) {
		synchronize();
	}
}
