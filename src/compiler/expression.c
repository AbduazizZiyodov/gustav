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

#include "byte_code.h"
#include "error.h"
#include "expression.h"
#include "function.h"
#include "internal.h"
#include "utils.h"
#include "var.h"

void expression_binary(bool can_assign [[maybe_unused]])
{
	TokenType operator_type = parser_state.previous.type;

	const ParseRule *rule = get_rule(operator_type);

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

void expression_call(bool can_assign [[maybe_unused]])
{
	uint8_t arg_count = argument_list();
	EMIT_BYTES(OP_CALL, arg_count);
}

void expression_dot(bool can_assign)
{
	token_consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
	uint8_t name = identifier_constant(&parser_state.previous);

	if (can_assign && token_match(TOKEN_EQUAL)) {
		expression_parse();
		EMIT_BYTES(OP_SET_PROPERTY, name);
	} else if (token_match(TOKEN_LEFT_PAREN)) {
		uint8_t arg_count = argument_list();
		EMIT_BYTES(OP_INVOKE, name);
		emit_byte(arg_count);
	} else {
		EMIT_BYTES(OP_GET_PROPERTY, name);
	}
}

void expression_literal(bool can_assign [[maybe_unused]])
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

void expression_grouping(bool can_assign [[maybe_unused]])
{
	expression_parse();
	token_consume(TOKEN_RIGHT_PAREN, "Expect '}' after expression.");
}

void expression_number(bool can_assign [[maybe_unused]])
{
	errno = 0;
	double value = strtod(parser_state.previous.start, NULL);

	if (errno == ERANGE) {
		error_at(&parser_state.previous, "Number literal out of range");
		return;
	}

	emit_constant(NUMBER_VAL(value));
}

void expression_string(bool can_assign [[maybe_unused]])
{
	StringObject *str =
		String_FromChars(parser_state.previous.start + 1, parser_state.previous.length - 2);

	// NOTE(Abduaziz): temporary push the value, fixes GC bug
	VM_Push(OBJ_VAL(str));
	emit_constant(OBJ_VAL(str));
	VM_Pop();
}

uint8_t identifier_constant(Token *name)
{
	StringObject *string_val = String_FromChars(name->start, name->length);
	VM_Push(OBJ_VAL(string_val)); // NOTE(Abduaziz): fixes same GC error above
	uint8_t constant = make_constant(OBJ_VAL(string_val));
	VM_Pop();
	return constant;
}

bool identifiers_equal(Token *a, Token *b)
{
	if (a->length != b->length) {
		return false;
	}
	return memcmp(a->start, b->start, a->length) == 0;
}

void expression_unary(bool can_assign [[maybe_unused]])
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

void expression_and(bool can_assign [[maybe_unused]])
{
	int end_jump = emit_jump(OP_JUMP_IF_FALSE);

	emit_byte(OP_POP);
	parse_precedence(PREC_AND);

	patch_jump(end_jump);
}

void expression_or(bool can_assign [[maybe_unused]])
{
	int else_jump = emit_jump(OP_JUMP_IF_FALSE);
	int end_jump = emit_jump(OP_JUMP);

	patch_jump(else_jump);
	emit_byte(OP_POP);

	parse_precedence(PREC_OR);
	patch_jump(end_jump);
}

void expression_variable(bool can_assign)
{
	named_variable(parser_state.previous, can_assign);
}

void parse_precedence(Precedence precedence)
{
	token_advance();
	ParseFn prefix_rule = get_rule(parser_state.previous.type)->prefix;

	if (prefix_rule is NULL) {
		compiler_error("Expect expression.");
		return;
	}

	bool can_assign = (bool)(precedence <= PREC_ASSIGNMENT);
	prefix_rule(can_assign);

	while (precedence <= get_rule(parser_state.current.type)->precedence) {
		token_advance();
		ParseFn infix_rule = get_rule(parser_state.previous.type)->infix;
		infix_rule(can_assign);
	}

	if (can_assign && token_match(TOKEN_EQUAL)) {
		compiler_error("Invalid assignment target.");
	}
}

void expression_parse(void)
{
	parse_precedence(PREC_ASSIGNMENT);
}
