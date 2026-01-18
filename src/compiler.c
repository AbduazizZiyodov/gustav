#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "log.h"
#include "object.h"
#include "scanner.h"
#include "value.h"

#define EMIT_BYTES(first_byte, second_byte) \
	emit_byte(first_byte);              \
	emit_byte(second_byte);

typedef struct {
	token_t current;
	token_t previous;
	bool had_error;
	bool panic_mode;
} ParserState;

typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT, // =
	PREC_OR, // or
	PREC_AND, // and
	PREC_EQUALITY, // == !=
	PREC_COMPARISON, // < > <= >=
	PREC_TERM, // + -
	PREC_FACTOR, // * /
	PREC_UNARY, // ! -
	PREC_CALL, // . ()
	PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(void);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

ParserState parser_state;

chunk_t *compiling_chunk;

static chunk_t *current_chunk(void)
{
	return compiling_chunk;
}

static void error_at(token_t *token, const char *message)
{
	if (parser_state.panic_mode) {
		return;
	}

	parser_state.panic_mode = true;

	(void)fprintf(stderr, "[line %lu] Error", token->line);

	if (token->type == TOKEN_EOF) {
		(void)fprintf(stderr, " at end");
	} else if (token->type == TOKEN_ERROR) {
		//
	} else {
		(void)fprintf(stderr, " at '%.*s'", (int)token->length,
			      token->start);
	}

	(void)fprintf(stderr, ": %s\n", message);

	parser_state.had_error = true;
}

static void compiler_error(const char *message)
{
	error_at(&parser_state.previous, message);
}

static void error_at_current(const char *what)
{
	error_at(&parser_state.current, what);
}

static void advance(void)
{
	parser_state.previous = parser_state.current;

	while (true) {
		parser_state.current = scan_token();

		if (parser_state.current.type != TOKEN_ERROR) {
			break;
		}

		error_at_current(parser_state.current.start);
	}
}

static void consume(TokenType type, const char *message)
{
	if (parser_state.current.type == type) {
		advance();
		return;
	}

	error_at_current(message);
}

static void emit_byte(uint8_t byte)
{
	write_chunk(current_chunk(), byte, parser_state.previous.line);
}
static void emit_return(void)
{
	emit_byte(OP_RETURN);
}

static uint8_t make_constant(value_t value)
{
	size_t constant = add_constant(current_chunk(), value);

	if (constant > UINT8_MAX) {
		compiler_error("Too many constants in one chunk");
		return 0;
	}

	return (uint8_t)constant;
}

static void emit_constant(value_t value)
{
	EMIT_BYTES(OP_CONSTANT, make_constant(value));
}

static void finish_compiling(void)
{
	emit_return();

	if (!parser_state.had_error) {
		disassemble_chunk(current_chunk(), "code");
	}
}

static void expression(void);
static ParseRule *get_rule(TokenType type);
static void parse_precedence(Precedence precedence);

static void binary(void)
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

static void literal(void)
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

static void grouping(void)
{
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect '}' after expression.");
}

static void number(void)
{
	errno = 0;
	double value = strtod(parser_state.previous.start, NULL);

	if (errno == ERANGE) {
		error_at(&parser_state.previous, "Number literal out of range");
		return;
	}

	emit_constant(NUMBER_VAL(value));
}

static void string(void)
{
	emit_constant(OBJ_VAL(copy_string(parser_state.previous.start + 1,
					  parser_state.previous.length - 2)));
}

static void unary(void)
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

ParseRule rules[] = {
	[TOKEN_LEFT_PAREN] = { grouping, NULL, PREC_NONE },
	[TOKEN_RIGHT_PAREN] = { NULL, NULL, PREC_NONE },
	[TOKEN_LEFT_BRACE] = { NULL, NULL, PREC_NONE },
	[TOKEN_RIGHT_BRACE] = { NULL, NULL, PREC_NONE },
	[TOKEN_COMMA] = { NULL, NULL, PREC_NONE },
	[TOKEN_DOT] = { NULL, NULL, PREC_NONE },
	[TOKEN_MINUS] = { unary, binary, PREC_TERM },
	[TOKEN_PLUS] = { NULL, binary, PREC_TERM },
	[TOKEN_PLUS_PLUS] = { NULL, binary, PREC_TERM },
	[TOKEN_SEMICOLON] = { NULL, NULL, PREC_NONE },
	[TOKEN_SLASH] = { NULL, binary, PREC_FACTOR },
	[TOKEN_STAR] = { NULL, binary, PREC_FACTOR },
	[TOKEN_BANG] = { unary, NULL, PREC_NONE },
	[TOKEN_BANG_EQUAL] = { NULL, binary, PREC_EQUALITY },
	[TOKEN_EQUAL] = { NULL, NULL, PREC_NONE },
	[TOKEN_EQUAL_EQUAL] = { NULL, binary, PREC_EQUALITY },
	[TOKEN_GREATER] = { NULL, binary, PREC_COMPARISON },
	[TOKEN_GREATER_EQUAL] = { NULL, binary, PREC_COMPARISON },
	[TOKEN_LESS] = { NULL, binary, PREC_COMPARISON },
	[TOKEN_LESS_EQUAL] = { NULL, binary, PREC_COMPARISON },
	[TOKEN_IDENTIFIER] = { NULL, NULL, PREC_NONE },
	[TOKEN_STRING] = { string, NULL, PREC_NONE },
	[TOKEN_NUMBER] = { number, NULL, PREC_NONE },
	[TOKEN_AND] = { NULL, NULL, PREC_NONE },
	[TOKEN_CLASS] = { NULL, NULL, PREC_NONE },
	[TOKEN_ELSE] = { NULL, NULL, PREC_NONE },
	[TOKEN_FALSE] = { literal, NULL, PREC_NONE },
	[TOKEN_FOR] = { NULL, NULL, PREC_NONE },
	[TOKEN_FUN] = { NULL, NULL, PREC_NONE },
	[TOKEN_IF] = { NULL, NULL, PREC_NONE },
	[TOKEN_NIL] = { literal, NULL, PREC_NONE },
	[TOKEN_OR] = { NULL, NULL, PREC_NONE },
	[TOKEN_PRINT] = { NULL, NULL, PREC_NONE },
	[TOKEN_RETURN] = { NULL, NULL, PREC_NONE },
	[TOKEN_SUPER] = { NULL, NULL, PREC_NONE },
	[TOKEN_THIS] = { NULL, NULL, PREC_NONE },
	[TOKEN_TRUE] = { literal, NULL, PREC_NONE },
	[TOKEN_VAR] = { NULL, NULL, PREC_NONE },
	[TOKEN_WHILE] = { NULL, NULL, PREC_NONE },
	[TOKEN_ERROR] = { NULL, NULL, PREC_NONE },
	[TOKEN_POW] = { NULL, binary, PREC_FACTOR },
	[TOKEN_EOF] = { NULL, NULL, PREC_NONE },
};

static void parse_precedence(Precedence precedence)
{
	advance();
	ParseFn prefix_rule = get_rule(parser_state.previous.type)->prefix;

	if (prefix_rule == NULL) {
		compiler_error("Expect expression.");
		return;
	}

	prefix_rule();

	while (precedence <= get_rule(parser_state.current.type)->precedence) {
		advance();
		ParseFn infix_rule =
			get_rule(parser_state.previous.type)->infix;
		infix_rule();
	}
}

static void expression(void)
{
	parse_precedence(PREC_ASSIGNMENT);
}

static ParseRule *get_rule(TokenType type)
{
	return &rules[type];
}

bool compile(const char *source, chunk_t *chunk)
{
	init_scanner(source);
	compiling_chunk = chunk;

	parser_state.panic_mode = false;
	parser_state.had_error = false;

	advance();
	expression();
	consume(TOKEN_EOF, "Expect end of expression");

	finish_compiling();
	return !parser_state.had_error;
}
