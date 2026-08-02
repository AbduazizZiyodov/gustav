#include <stddef.h>

#include "chunk.h"
#include "scanner.h"

#include "compiler/byte_code.h"
#include "compiler/loop.h"
#include "compiler/parse.h"
#include "compiler/scope.h"
#include "compiler/utils.h"
#include "compiler/var.h"

void for_statement(void)
{
	begin_scope();
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'");

	if (match(TOKEN_SEMICOLON)) {
		// ...
	} else if (match(TOKEN_VAR)) {
		var_declaration();
	} else {
		expression_statement();
	}

	size_t loop_start = current_chunk()->count;
	int exit_jump = -1;

	if (!match(TOKEN_SEMICOLON)) {
		expression();
		consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");
		exit_jump = emit_jump(OP_JUMP_IF_FALSE);
		emit_byte(OP_POP);
	}

	if (!match(TOKEN_RIGHT_PAREN)) {
		int body_jump = emit_jump(OP_JUMP);
		size_t increment_start = current_chunk()->count;
		expression();
		emit_byte(OP_POP);
		consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

		emit_loop(loop_start);
		loop_start = increment_start;
		patch_jump(body_jump);
	}

	statement();
	emit_loop(loop_start);

	if (exit_jump != -1) {
		patch_jump(exit_jump);
		emit_byte(OP_POP);
	}

	end_scope();
}

void while_statement(void)
{
	size_t loop_start = current_chunk()->count;

	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

	int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
	emit_byte(OP_POP);
	statement();
	emit_loop(loop_start);

	patch_jump(exit_jump);
	emit_byte(OP_POP);
}
