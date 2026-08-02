#pragma once

// private state

#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "object.h"
#include "scanner.h"

typedef enum {
	TYPE_FUNCTION,
	TYPE_SCRIPT,
	TYPE_METHOD,
	TYPE_INITIALIZER
} FunctionType;

typedef struct {
	Token name;
	int depth;
	bool is_captured;
} Local;

typedef struct {
	uint8_t index;
	bool is_local;
} Upvalue;

typedef struct Compiler {
	struct Compiler *enclosing;

	FunctionObject *function;
	FunctionType type;

	Local locals[UINT8_COUNT];
	int local_count;
	Upvalue upvalues[UINT8_MAX];
	int scope_depth;
} Compiler;

typedef struct {
	Token current;
	Token previous;
	bool had_error;
	bool panic_mode;
} ParserState;

typedef struct ClassCompiler {
	struct ClassCompiler *enclosing;
	bool has_super_class;
} ClassCompiler;

extern ParserState parser_state;
extern Compiler *current;
extern ClassCompiler *current_class;

void init_compiler(Compiler *compiler, FunctionType type);

FunctionObject *finish_compiling(void);
