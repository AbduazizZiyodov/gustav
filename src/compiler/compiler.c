#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "byte_code.h"
#include "compiler.h"
#include "declaration.h"
#include "gc.h"
#include "internal.h"
#include "log.h"
#include "object.h"
#include "scanner.h"
#include "utils.h"
#include "value.h"

#ifdef DEBUG
#include "debug.h"
#endif

ParserState parser_state;
Compiler *current = NULL;
ClassCompiler *current_class = NULL;

void init_compiler(Compiler *compiler, FunctionType type)
{
	compiler->enclosing = current;

	compiler->function = NULL;
	compiler->type = type;

	compiler->local_count = 0;
	compiler->scope_depth = 0;

	compiler->function = Function_New();

	current = compiler;

	if (type != TYPE_SCRIPT) {
		current->function->name =
			String_FromChars(parser_state.previous.start,
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

FunctionObject *finish_compiling(void)
{
	emit_return();
	FunctionObject *function = current->function;

#ifdef DEBUG
	if (!parser_state.had_error) {
		Debug_DisassembleChunk(current_chunk(),
				       function->name != NULL ?
					       function->name->chars :
					       "<script>");
	}
#endif // DEBUG
	current = current->enclosing;

	return function;
}

FunctionObject *Compiler_Compile(const char *source)
{
	Scanner_Init(source);
	Compiler compiler;
	init_compiler(&compiler, TYPE_SCRIPT);

	parser_state.panic_mode = false;
	parser_state.had_error = false;

	LOG_INFO("Begin scanning\n");
	LOG_DEBUG("== [scanner] ==\n");

	token_advance();

	while (!token_match(TOKEN_EOF)) {
		declaration_parse();
	}
	LOG_DEBUG("== [/scanner] ==\n");

	FunctionObject *function = finish_compiling();

	if (parser_state.had_error) {
		return NULL;
	}
	return function;
}

void Compiler_MarkRoots(void)
{
	Compiler *compiler = current;

	while (compiler != NULL) {
		GC_MarkObject((Object *)compiler->function);
		compiler = compiler->enclosing;
	}
}
