#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "scanner.h"

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

typedef void (*ParseFn)(bool can_assign);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

void expression_parse(void);
void parse_precedence(Precedence precedence);

const ParseRule *get_rule(TokenType type);

void expression_binary(bool can_assign);
void expression_call(bool can_assign);
void expression_dot(bool can_assign);
void expression_literal(bool can_assign);
void expression_grouping(bool can_assign);
void expression_number(bool can_assign);
void expression_string(bool can_assign);
void expression_unary(bool can_assign);
void expression_and(bool can_assign);
void expression_or(bool can_assign);
void expression_variable(bool can_assign);

uint8_t identifier_constant(Token *name);
bool identifiers_equal(Token *a, Token *b);
