#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "compiler/byte_code.h"
#include "compiler/function.h"
#include "compiler/parse.h"
#include "compiler/utils.h"
#include "log.h"
#include "memory.h"
#include "object.h"
#include "scanner.h"
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

function_t *finish_compiling(void)
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
