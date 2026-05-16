#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "log.h"
#include "memory.h"
#include "object.h"
#include "scanner.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG
#include "debug.h"
#endif

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

typedef void (*ParseFn)(bool can_assign);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

typedef struct {
	token_t name;
	int depth;
	bool is_captured;
} Local;

typedef struct {
	uint8_t index;
	bool is_local;
} Upvalue;

typedef enum {
	TYPE_FUNCTION,
	TYPE_SCRIPT,
	TYPE_METHOD,
	TYPE_INITIALIZER
} FunctionType;

typedef struct Compiler {
	struct Compiler *enclosing;

	function_t *function;
	FunctionType type;

	Local locals[UINT8_COUNT];
	int local_count;
	Upvalue upvalues[UINT8_MAX];
	int scope_depth;
} Compiler;

typedef struct ClassCompiler {
	struct ClassCompiler *enclosing;
} ClassCompiler;

ParserState parser_state;
Compiler *current = NULL;
ClassCompiler *current_class = NULL;

chunk_t *compiling_chunk;

static void statement(void);
static void declaration(void);
static uint8_t argument_list(void);
static uint8_t identifier_constant(token_t *name);

static chunk_t *current_chunk(void)
{
	return &current->function->chunk;
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
		LOG_DEBUG("line=%04d %-20s <=> '%.*s'\n",
			  parser_state.current.line,
			  TOKEN_TYPE_STRING[parser_state.current.type],
			  parser_state.current.length,
			  parser_state.current.start);

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

static bool check(TokenType type)
{
	return parser_state.current.type == type;
}

static bool match(TokenType type)
{
	if (!check(type)) {
		return false;
	}
	advance();
	return true;
}

static void emit_byte(uint8_t byte)
{
	write_chunk(current_chunk(), byte, parser_state.previous.line);
}

static void emit_loop(size_t loop_start)
{
	emit_byte(OP_LOOP);
	size_t offset = current_chunk()->count - loop_start + 2;

	if (offset > UINT16_MAX) {
		gustav_error(1, "Loop body too large");
	}

	emit_byte((offset >> 8) & 0xff);
	emit_byte(offset & 0xff);
}

static int emit_jump(uint8_t instruction)
{
	emit_byte(instruction);
	EMIT_BYTES(0xff, 0xff);
	return (int)current_chunk()->count - 2;
}

static void emit_return(void)
{
	if (current->type == TYPE_INITIALIZER) {
		EMIT_BYTES(OP_GET_LOCAL, 0);
	} else {
		emit_byte(OP_NIL);
	}
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

static void patch_jump(int offset)
{
	int jump = ((int)current_chunk()->count) - offset - 2;

	if (jump > UINT16_MAX) {
		gustav_error(1, "Too much code to jump over");
	}

	current_chunk()->code[offset] = (jump >> 8) & 0xff;
	current_chunk()->code[offset + 1] = jump & 0xff;
}

static void init_compiler(Compiler *compiler, FunctionType type)
{
	compiler->enclosing = current;

	compiler->function = NULL;
	compiler->type = type;

	compiler->local_count = 0;
	compiler->scope_depth = 0;

	compiler->function = new_function();

	current = compiler;

	if (type != TYPE_SCRIPT) {
		current->function->name =
			copy_string(parser_state.previous.start,
				    parser_state.previous.length);
	}

	Local *local = &current->locals[current->local_count++];
	local->depth = 0;
	local->is_captured = false;

	if (type != TYPE_FUNCTION) {
		local->name.start = "this";
		local->name.length = 4;
	} else {
		local->name.start = "";
		local->name.length = 0;
	}
}

static function_t *finish_compiling(void)
{
	emit_return();
	function_t *function = current->function;

#ifdef DEBUG
	if (!parser_state.had_error) {
		disassemble_chunk(current_chunk(),
				  function->name != NULL ?
					  function->name->chars :
					  "<script>");
	}
#endif // DEBUG
	current = current->enclosing;

	return function;
}

static void begin_scope(void)
{
	current->scope_depth++;
}

static void end_scope(void)
{
	current->scope_depth--;

	while (current->local_count > 0 &&
	       current->locals[current->local_count - 1].depth >
		       current->scope_depth) {
		if (current->locals[current->local_count - 1].is_captured) {
			emit_byte(OP_CLOSE_UPVALUE);
		} else {
			emit_byte(OP_POP);
		}
		current->local_count--;
	}
}

static void expression(void);
static ParseRule *get_rule(TokenType type);
static void parse_precedence(Precedence precedence);

static void binary(bool can_assign [[maybe_unused]])
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

static void call(bool can_assign [[maybe_unused]])
{
	uint8_t arg_count = argument_list();
	EMIT_BYTES(OP_CALL, arg_count);
}

static void dot(bool can_assign)
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

static void literal(bool can_assign [[maybe_unused]])
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

static void grouping(bool can_assign [[maybe_unused]])
{
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect '}' after expression.");
}

static void number(bool can_assign [[maybe_unused]])
{
	errno = 0;
	double value = strtod(parser_state.previous.start, NULL);

	if (errno == ERANGE) {
		error_at(&parser_state.previous, "Number literal out of range");
		return;
	}

	emit_constant(NUMBER_VAL(value));
}

static void string(bool can_assign [[maybe_unused]])
{
	string_t *str = copy_string(parser_state.previous.start + 1,
				    parser_state.previous.length - 2);

	// NOTE(abduaziz): temporary push the value, fixes GC bug
	push(OBJ_VAL(str));
	emit_constant(OBJ_VAL(str));
	pop();
}

static uint8_t identifier_constant(token_t *name)
{
	string_t *string_val = copy_string(name->start, name->length);
	push(OBJ_VAL(string_val)); // NOTE(abduaziz): fixes same GC error above
	uint8_t constant = make_constant(OBJ_VAL(string_val));
	pop();
	return constant;
}

static bool identifiers_equal(token_t *a, token_t *b)
{
	if (a->length != b->length) {
		return false;
	}
	return memcmp(a->start, b->start, a->length) == 0;
}

static int resolve_local(Compiler *compiler, token_t *name)
{
	for (int i = compiler->local_count - 1; i >= 0; i--) {
		Local *local = &compiler->locals[i];

		if (identifiers_equal(name, &local->name)) {
			if (local->depth == -1) {
				gustav_error(
					1,
					"Can't read local variable in its own initializer.");
			}
			return i;
		}
	}
	return -1;
}

static int add_upvalue(Compiler *compiler, uint8_t index, bool is_local)
{
	int upvalue_count = compiler->function->upvalue_count;

	for (int i = 0; i < upvalue_count; i++) {
		Upvalue *upvalue = &compiler->upvalues[i];
		if (upvalue->index == index && upvalue->is_local == is_local) {
			return i;
		}
	}

	if (upvalue_count == UINT8_MAX) {
		gustav_error(1, "Too many closure variables in function.");
		return 0;
	}

	compiler->upvalues[upvalue_count].is_local = is_local;
	compiler->upvalues[upvalue_count].index = index;

	return compiler->function->upvalue_count++;
}

static int resolve_upvalue(Compiler *compiler, token_t *name)
{
	if (compiler->enclosing == NULL) {
		return -1;
	}

	int local = resolve_local(compiler->enclosing, name);

	if (local != -1) {
		compiler->enclosing->locals[local].is_captured = true;
		return add_upvalue(compiler, (uint8_t)local, true);
	}

	int upvalue = resolve_upvalue(compiler->enclosing, name);

	if (upvalue != -1) {
		return add_upvalue(compiler, (uint8_t)upvalue, false);
	}

	return -1;
}

static void add_local(token_t name)
{
	if (current->local_count == UINT8_COUNT) {
		// NOTE(abduaziz): error, gustav_error ...
		gustav_error(1, "Too many local variables in function.");
		return;
	}
	Local *local = &current->locals[current->local_count++];
	local->name = name;
	local->depth = -1;
	local->is_captured = false;
}

static void declare_variable(void)
{
	if (current->scope_depth == 0) {
		return;
	}

	token_t *name = &parser_state.previous;

	for (int i = current->local_count - 1; i >= 0; i--) {
		Local *local = &current->locals[i];

		if (local->depth != -1 && local->depth < current->scope_depth) {
			break;
		}

		if (identifiers_equal(name, &local->name)) {
			gustav_error(
				1,
				"Already a variable with this name in this scope.");
		}
	}

	add_local(*name);
}

static void named_variable(token_t name, bool can_assign)
{
	uint8_t get_op;
	uint8_t set_op;

	int arg = resolve_local(current, &name);

	if (arg != -1) {
		get_op = OP_GET_LOCAL;
		set_op = OP_SET_LOCAL;
		/* NOLINTNEXTLINE(bugprone-assignment-in-if-condition) */
	} else if ((arg = resolve_upvalue(current, &name)) != -1) {
		get_op = OP_GET_UPVALUE;
		set_op = OP_SET_UPVALUE;
	}

	else {
		arg = identifier_constant(&name);
		get_op = OP_GET_GLOBAL;
		set_op = OP_SET_GLOBAL;
	}

	if (can_assign && match(TOKEN_EQUAL)) {
		expression();
		EMIT_BYTES(set_op, (uint8_t)arg);
	} else {
		EMIT_BYTES(get_op, (uint8_t)arg);
	}
}

static void variable(bool can_assign)
{
	named_variable(parser_state.previous, can_assign);
}

static void this(bool can_assign [[maybe_unused]])
{
	if (current_class == NULL) {
		gustav_error(-1, "Can't use 'this' outside of a class.");
		return;
	}

	variable(false);
}

static void unary(bool can_assign [[maybe_unused]])
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

static void and_(bool can_assign [[maybe_unused]])
{
	int end_jump = emit_jump(OP_JUMP_IF_FALSE);

	emit_byte(OP_POP);
	parse_precedence(PREC_AND);

	patch_jump(end_jump);
}

static void or_(bool can_assign [[maybe_unused]])
{
	int else_jump = emit_jump(OP_JUMP_IF_FALSE);
	int end_jump = emit_jump(OP_JUMP);

	patch_jump(else_jump);
	emit_byte(OP_POP);

	parse_precedence(PREC_OR);
	patch_jump(end_jump);
}

ParseRule rules[] = {
	[TOKEN_LEFT_PAREN] = { grouping, call, PREC_CALL },
	[TOKEN_RIGHT_PAREN] = { NULL, NULL, PREC_NONE },
	[TOKEN_LEFT_BRACE] = { NULL, NULL, PREC_NONE },
	[TOKEN_RIGHT_BRACE] = { NULL, NULL, PREC_NONE },
	[TOKEN_COMMA] = { NULL, NULL, PREC_NONE },
	[TOKEN_DOT] = { NULL, dot, PREC_CALL },
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
	[TOKEN_IDENTIFIER] = { variable, NULL, PREC_NONE },
	[TOKEN_STRING] = { string, NULL, PREC_NONE },
	[TOKEN_NUMBER] = { number, NULL, PREC_NONE },
	[TOKEN_AND] = { NULL, and_, PREC_AND },
	[TOKEN_CLASS] = { NULL, NULL, PREC_NONE },
	[TOKEN_ELSE] = { NULL, NULL, PREC_NONE },
	[TOKEN_FALSE] = { literal, NULL, PREC_NONE },
	[TOKEN_FOR] = { NULL, NULL, PREC_NONE },
	[TOKEN_FUN] = { NULL, NULL, PREC_NONE },
	[TOKEN_IF] = { NULL, NULL, PREC_NONE },
	[TOKEN_NIL] = { literal, NULL, PREC_NONE },
	[TOKEN_OR] = { NULL, or_, PREC_OR },
	[TOKEN_PRINT] = { NULL, NULL, PREC_NONE },
	[TOKEN_RETURN] = { NULL, NULL, PREC_NONE },
	[TOKEN_SUPER] = { NULL, NULL, PREC_NONE },
	[TOKEN_THIS] = { this, NULL, PREC_NONE },
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
		gustav_error(1, "Invalid assignment target.");
	}
}

static void expression(void)
{
	parse_precedence(PREC_ASSIGNMENT);
}

static void block(void)
{
	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
		declaration();
	}

	consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static uint8_t parse_variable(const char *error_message)
{
	consume(TOKEN_IDENTIFIER, error_message);

	declare_variable();

	if (current->scope_depth > 0) {
		return 0;
	}

	return identifier_constant(&parser_state.previous);
}

static void mark_initialized(void)
{
	if (current->scope_depth == 0) {
		return;
	}
	current->locals[current->local_count - 1].depth = current->scope_depth;
}

static void define_variable(uint8_t global)
{
	if (current->scope_depth > 0) {
		mark_initialized();
		return;
	}
	EMIT_BYTES(OP_DEFINE_GLOBAL, global);
}

static uint8_t argument_list(void)
{
	uint8_t arg_count = 0;
	if (!check(TOKEN_RIGHT_PAREN)) {
		do {
			expression();
			if (arg_count == 255) {
				gustav_error(
					1,
					"Can't have more than 255 arguments.");
			}
			arg_count++;
		} while (match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
	return arg_count;
}

static void function(FunctionType type)
{
	Compiler compiler;

	init_compiler(&compiler, type);
	begin_scope();

	consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

	if (!check(TOKEN_RIGHT_PAREN)) {
		do {
			current->function->arity++;
			if (current->function->arity > 255) { // god forgive me
				error_at_current(
					"Can't have more than 255 parameters.");
			}
			uint8_t constant =
				parse_variable("Expect parameters name.");
			define_variable(constant);
		} while (match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
	consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
	block();
	function_t *function = finish_compiling();
	value_t function_value = OBJ_VAL(function);
	push(function_value);
	EMIT_BYTES(OP_CLOSURE, make_constant(function_value));
	pop();

	for (int i = 0; i < function->upvalue_count; i++) {
		EMIT_BYTES(compiler.upvalues[i].is_local ? 1 : 0,
			   compiler.upvalues[i].index)
	}
}

static void method()
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

static void class_declaration()
{
	consume(TOKEN_IDENTIFIER, "Expect class name.");

	token_t class_name = parser_state.previous;

	uint8_t name_constant = identifier_constant(&parser_state.previous);
	declare_variable();

	EMIT_BYTES(OP_CLASS, name_constant);
	define_variable(name_constant);

	ClassCompiler class_compiler;
	class_compiler.enclosing = current_class;
	current_class = &class_compiler;

	named_variable(class_name, false);

	consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");

	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
		method();
	}

	consume(TOKEN_RIGHT_BRACE, "Expect '}' before class body.");
	emit_byte(OP_POP);

	current_class = current_class->enclosing;
}

static void fun_declaration(void)
{
	uint8_t global = parse_variable("Expect function name.");
	mark_initialized();
	function(TYPE_FUNCTION);
	define_variable(global);
}

static void var_declaration(void)
{
	uint8_t global = parse_variable("Expect variable name.");

	if (match(TOKEN_EQUAL)) {
		expression();
	} else {
		emit_byte(OP_NIL);
	}

	consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");
	define_variable(global);
}

static void expression_statement(void)
{
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
	emit_byte(OP_POP);
}

static void for_statement(void)
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

static void if_statement(void)
{
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ') after condition.");

	int then_jump = emit_jump(OP_JUMP_IF_FALSE);
	emit_byte(OP_POP);
	statement();
	int else_jump = emit_jump(OP_JUMP);

	patch_jump(then_jump);
	emit_byte(OP_POP);

	if (match(TOKEN_ELSE)) {
		statement();
	}

	patch_jump(else_jump);
}

static void print_statement(void)
{
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	emit_byte(OP_PRINT);
}

static void return_statement(void)
{
	// NOTE(abduaziz): return via exit-code, from script (top-level code) ?
	if (current->type == TYPE_SCRIPT) {
		gustav_error(1, "Can't return from top-level code.");
	}

	if (match(TOKEN_SEMICOLON)) {
		emit_return();
	} else {
		if (current->type == TYPE_INITIALIZER) {
			gustav_error(
				-1,
				"Can't return a value from an initializer.");
		}
		expression();
		consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
		emit_byte(OP_RETURN);
	}
}

static void while_statement(void)
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

static void statement(void)
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

static void declaration(void)
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

static ParseRule *get_rule(TokenType type)
{
	return &rules[type];
}

function_t *compile(const char *source)
{
	init_scanner(source);
	Compiler compiler;
	init_compiler(&compiler, TYPE_SCRIPT);

	parser_state.panic_mode = false;
	parser_state.had_error = false;

	LOG_INFO("Begin scanning\n");
	LOG_DEBUG("== [scanner] ==\n");

	advance();

	while (!match(TOKEN_EOF)) {
		declaration();
	}
	LOG_DEBUG("== [/scanner] ==\n");

	function_t *function = finish_compiling();

	if (parser_state.had_error) {
		return NULL;
	}
	return function;
}

void mark_compiler_roots(void)
{
	Compiler *compiler = current;

	while (compiler != NULL) {
		mark_object((obj_t *)compiler->function);
		compiler = compiler->enclosing;
	}
}
