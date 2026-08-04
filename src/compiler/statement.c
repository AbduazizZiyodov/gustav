#include <stddef.h>
#include <stdint.h>

#include "chunk.h"
#include "scanner.h"

#include "byte_code.h"
#include "declaration.h"
#include "error.h"
#include "expression.h"
#include "internal.h"
#include "log.h"
#include "scope.h"
#include "statement.h"
#include "utils.h"
#include "var.h"

void statement_block(void)
{
	while (!token_check(TOKEN_RIGHT_BRACE) && !token_check(TOKEN_EOF)) {
		declaration_parse();
	}

	token_consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void statement_expression(void)
{
	expression_parse();
	token_consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
	emit_byte(OP_POP);
}

static void statement_print(OpCode op)
{
	expression_parse();
	token_consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	emit_byte((uint8_t)op);
}

static void statement_if(void)
{
	token_consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
	expression_parse();
	token_consume(TOKEN_RIGHT_PAREN, "Expect ') after condition.");

	int then_jump = emit_jump(OP_JUMP_IF_FALSE);
	emit_byte(OP_POP);
	statement_parse();
	int else_jump = emit_jump(OP_JUMP);

	patch_jump(then_jump);
	emit_byte(OP_POP);

	if (token_match(TOKEN_ELSE)) {
		statement_parse();
	}

	patch_jump(else_jump);
}

static void statement_while(void)
{
	size_t loop_start = current_chunk()->count;

	token_consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
	expression_parse();
	token_consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

	int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
	emit_byte(OP_POP);
	statement_parse();
	emit_loop(loop_start);

	patch_jump(exit_jump);
	emit_byte(OP_POP);
}

typedef struct {
	
} ExitJumps;

static void statement_loop(void)
{
	size_t loop_start = current_chunk()->count;

	emit_byte(OP_TRUE); // expr is while(true) -> loop
	int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
	emit_byte(OP_POP);
	statement_parse();
	emit_loop(loop_start);

	patch_jump(exit_jump);
	emit_byte(OP_POP);
}

static void statement_for(void)
{
	begin_scope();
	token_consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'");

	if (token_match(TOKEN_SEMICOLON)) {
		// ...
	} else if (token_match(TOKEN_VAR)) {
		var_declaration();
	} else {
		statement_expression();
	}

	size_t loop_start = current_chunk()->count;
	int exit_jump = -1;

	if (!token_match(TOKEN_SEMICOLON)) {
		expression_parse();
		token_consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");
		exit_jump = emit_jump(OP_JUMP_IF_FALSE);
		emit_byte(OP_POP);
	}

	if (!token_match(TOKEN_RIGHT_PAREN)) {
		int body_jump = emit_jump(OP_JUMP);
		size_t increment_start = current_chunk()->count;
		expression_parse();
		emit_byte(OP_POP);
		token_consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

		emit_loop(loop_start);
		loop_start = increment_start;
		patch_jump(body_jump);
	}

	statement_parse();
	emit_loop(loop_start);

	if (exit_jump != -1) {
		patch_jump(exit_jump);
		emit_byte(OP_POP);
	}

	end_scope();
}

static void statement_break(void)
{
	emit_byte(OP_BREAK);
	token_consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
}

static void statement_return(void)
{
	if (token_match(TOKEN_SEMICOLON)) {
		emit_return();
	} else {
		if (current->type == TYPE_INITIALIZER) {
			compiler_error("Can't return a value from an initializer.");
		}

		expression_parse();
		token_consume(TOKEN_SEMICOLON, "Expect ';' after return value.");

		// top-level return <expr> is the script's exit status like in C
		if (current->type == TYPE_SCRIPT) {
			emit_byte(OP_RETURN_EXIT);
		} else {
			emit_byte(OP_RETURN);
		}
	}
}

void statement_parse(void)
{
	if (token_match(TOKEN_PRINT_STDOUT)) {
		statement_print(OP_PRINT_STDOUT);
	} else if (token_match(TOKEN_PRINT_STDERR)) {
		statement_print(OP_PRINT_STDERR);
	} else if (token_match(TOKEN_IF)) {
		statement_if();
	} else if (token_match(TOKEN_RETURN)) {
		statement_return();
	} else if (token_match(TOKEN_FOR)) {
		statement_for();
	} else if (token_match(TOKEN_WHILE)) {
		statement_while();
	} else if (token_match(TOKEN_LOOP)) {
		statement_loop();
	} else if (token_match(TOKEN_BREAK)) {
		statement_break();
	} else if (token_match(TOKEN_CONTINUE)) {
		LOG_TRACE("detected continue statement\n");
		token_consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
	} else if (token_match(TOKEN_LEFT_BRACE)) {
		begin_scope();
		statement_block();
		end_scope();
	} else {
		statement_expression();
	}
}
