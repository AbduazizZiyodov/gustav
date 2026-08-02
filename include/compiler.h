#pragma once

#include "common.h"
#include "object.h"
#include "scanner.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "compiler/class.h"
#include "compiler/function.h"

typedef struct {
	token_t name;
	int depth;
	bool is_captured;
} Local;

typedef struct {
	uint8_t index;
	bool is_local;
} Upvalue;

typedef struct Compiler {
	struct Compiler *enclosing;

	function_t *function;
	FunctionType type;

	Local locals[UINT8_COUNT];
	int local_count;
	Upvalue upvalues[UINT8_MAX];
	int scope_depth;
} Compiler;

typedef struct {
	token_t current;
	token_t previous;
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

function_t *compile(const char *source);
void mark_compiler_roots(void);

void init_compiler(Compiler *compiler, FunctionType type);

function_t *finish_compiling(void);
