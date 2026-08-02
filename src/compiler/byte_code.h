#pragma once

#include <stdint.h>

#include "value.h"

#define EMIT_BYTES(first_byte, second_byte) \
	emit_byte(first_byte);              \
	emit_byte(second_byte);

void emit_byte(uint8_t byte);

void emit_loop(size_t loop_start);

int emit_jump(uint8_t instruction);

void emit_return(void);

uint8_t make_constant(Value value);

void emit_constant(Value value);

void patch_jump(int offset);
