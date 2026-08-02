#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "object.h"
#include "scanner.h"
#include "value.h"
#include "vm.h"

#include "compiler.h"
#include "compiler/byte_code.h"
#include "compiler/error.h"
#include "compiler/function.h"
#include "compiler/if.h"
#include "compiler/loop.h"
#include "compiler/parse.h"
#include "compiler/print.h"
#include "compiler/scope.h"
#include "compiler/utils.h"
#include "compiler/var.h"

void binary(bool can_assign [[maybe_unused]])
{
	TokenType operator_type = parser_state.previous.type;

	ParseRule *rule = get_rule(operator_type);

	parse_precedence((Precedence)(rule->precedence + 1));

	switch (operator_type) {
	case TOKEN_BANG_EQUAL:
		EMIT_BYTES(OP_EQUAL, OP_NOT);
		break;
	case TOKEN_EQUAL_EQUAL:
		emit_byte(OP_EQUAL);
		break;
	case TOKEN_GREATER:
		emit_byte(OP_GREATER);
		break;
	case TOKEN_GREATER_EQUAL:
		EMIT_BYTES(OP_LESS, OP_NOT);
		break;
	case TOKEN_LESS:
		emit_byte(OP_LESS);
		break;
	case TOKEN_LESS_EQUAL:
		EMIT_BYTES(OP_GREATER, OP_NOT);
		break;
	case TOKEN_PLUS:
		emit_byte(OP_ADD);
		break;
	case TOKEN_PLUS_PLUS:
		emit_byte(OP_CONCAT);
		break;
	case TOKEN_MINUS:
		emit_byte(OP_SUBTRACT);
		break;
	case TOKEN_STAR:
		emit_byte(OP_MULTIPLY);
		break;
	case TOKEN_POW:
		emit_byte(OP_POW);
		break;
	case TOKEN_SLASH:
		emit_byte(OP_DIVIDE);
		break;
	default:
		UNREACHABLE();
	}
}

void call(bool can_assign [[maybe_unused]])
{
	uint8_t arg_count = argument_list();
	EMIT_BYTES(OP_CALL, arg_count);
}

void dot(bool can_assign)
{
	consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
	uint8_t name = identifier_constant(&parser_state.previous);

	if (can_assign && match(TOKEN_EQUAL)) {
		expression();
		EMIT_BYTES(OP_SET_PROPERTY, name);
	} else if (match(TOKEN_LEFT_PAREN)) {
		uint8_t arg_count = argument_list();
		EMIT_BYTES(OP_INVOKE, name);
		emit_byte(arg_count);
	} else {
		EMIT_BYTES(OP_GET_PROPERTY, name);
	}
}

void literal(bool can_assign [[maybe_unused]])
{
	switch (parser_state.previous.type) {
	case TOKEN_FALSE:
		emit_byte(OP_FALSE);
		break;
	case TOKEN_TRUE:
		emit_byte(OP_TRUE);
		break;
	case TOKEN_NIL:
		emit_byte(OP_NIL);
		break;
	default:
		UNREACHABLE();
	}
}

void grouping(bool can_assign [[maybe_unused]])
{
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect '}' after expression.");
}

void number(bool can_assign [[maybe_unused]])
{
	errno = 0;
	double value = strtod(parser_state.previous.start, NULL);

	if (errno == ERANGE) {
		error_at(&parser_state.previous, "Number literal out of range");
		return;
	}

	emit_constant(NUMBER_VAL(value));
}

void string(bool can_assign [[maybe_unused]])
{
	string_t *str = copy_string(parser_state.previous.start + 1,
				    parser_state.previous.length - 2);

	// NOTE(abduaziz): temporary push the value, fixes GC bug
	push(OBJ_VAL(str));
	emit_constant(OBJ_VAL(str));
	pop();
}

uint8_t identifier_constant(token_t *name)
{
	string_t *string_val = copy_string(name->start, name->length);
	push(OBJ_VAL(string_val)); // NOTE(abduaziz): fixes same GC error above
	uint8_t constant = make_constant(OBJ_VAL(string_val));
	pop();
	return constant;
}

bool identifiers_equal(token_t *a, token_t *b)
{
	if (a->length != b->length) {
		return false;
	}
	return memcmp(a->start, b->start, a->length) == 0;
}

void unary(bool can_assign [[maybe_unused]])
{
	TokenType operator_type = parser_state.previous.type;

	parse_precedence(PREC_UNARY);

	switch (operator_type) {
	case TOKEN_MINUS:
		emit_byte(OP_NEGATE);
		break;
	case TOKEN_BANG:
		emit_byte(OP_NOT);
		break;
	default:
		UNREACHABLE();
	}
}

void and_(bool can_assign [[maybe_unused]])
{
	int end_jump = emit_jump(OP_JUMP_IF_FALSE);

	emit_byte(OP_POP);
	parse_precedence(PREC_AND);

	patch_jump(end_jump);
}

void or_(bool can_assign [[maybe_unused]])
{
	int else_jump = emit_jump(OP_JUMP_IF_FALSE);
	int end_jump = emit_jump(OP_JUMP);

	patch_jump(else_jump);
	emit_byte(OP_POP);

	parse_precedence(PREC_OR);
	patch_jump(end_jump);
}

void parse_precedence(Precedence precedence)
{
	advance();
	ParseFn prefix_rule = get_rule(parser_state.previous.type)->prefix;

	if (prefix_rule is NULL) {
		compiler_error("Expect expression.");
		return;
	}

	bool can_assign = (bool)(precedence <= PREC_ASSIGNMENT);
	prefix_rule(can_assign);

	while (precedence <= get_rule(parser_state.current.type)->precedence) {
		advance();
		ParseFn infix_rule =
			get_rule(parser_state.previous.type)->infix;
		infix_rule(can_assign);
	}

	if (can_assign && match(TOKEN_EQUAL)) {
		compiler_error("Invalid assignment target.");
	}
}

void expression(void)
{
	parse_precedence(PREC_ASSIGNMENT);
}

void block(void)
{
	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
		declaration();
	}

	consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void expression_statement(void)
{
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
	emit_byte(OP_POP);
}

void return_statement(void)
{
	// NOTE(abduaziz): return via exit-code, from script (top-level code) ?
	if (current->type == TYPE_SCRIPT) {
		compiler_error("Can't return from top-level code.");
	}

	if (match(TOKEN_SEMICOLON)) {
		emit_return();
	} else {
		if (current->type == TYPE_INITIALIZER) {
			compiler_error(

				"Can't return a value from an initializer.");
		}
		expression();
		consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
		emit_byte(OP_RETURN);
	}
}

void statement(void)
{
	if (match(TOKEN_PRINT)) {
		print_statement();
	} else if (match(TOKEN_IF)) {
		if_statement();
	} else if (match(TOKEN_RETURN)) {
		return_statement();
	} else if (match(TOKEN_FOR)) {
		for_statement();
	} else if (match(TOKEN_WHILE)) {
		while_statement();
	} else if (match(TOKEN_LEFT_BRACE)) {
		begin_scope();
		block();
		end_scope();
	} else {
		expression_statement();
	}
}

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
		case TOKEN_PRINT:
		case TOKEN_RETURN:
			return;
		default:;
		}
		advance();
	}
}

void declaration(void)
{
	if (match(TOKEN_CLASS)) {
		class_declaration();
	} else if (match(TOKEN_FUN)) {
		fun_declaration();
	} else if (match(TOKEN_VAR)) {
		var_declaration();
	} else {
		statement();
	}

	if (parser_state.panic_mode) {
		synchronize();
	}
}

ParseRule *get_rule(TokenType type)
{
	return &rules[type];
}

void variable(bool can_assign)
{
	named_variable(parser_state.previous, can_assign);
}
