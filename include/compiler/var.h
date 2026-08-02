#pragma once

#include <stdint.h>

#include "compiler.h"

uint8_t parse_variable(const char *error_message);

void mark_initialized(void);

void define_variable(uint8_t global);

void var_declaration(void);

void declare_variable(void);

void named_variable(token_t name, bool can_assign);

void add_local(token_t name);

int resolve_local(Compiler *compiler, token_t *name);

int add_upvalue(Compiler *compiler, uint8_t index, bool is_local);

int resolve_upvalue(Compiler *compiler, token_t *name);
