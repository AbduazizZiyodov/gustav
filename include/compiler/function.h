#pragma once

#include <stdint.h>

typedef enum {
	TYPE_FUNCTION,
	TYPE_SCRIPT,
	TYPE_METHOD,
	TYPE_INITIALIZER
} FunctionType;

void fun_declaration(void);
uint8_t argument_list(void);
void function(FunctionType type);
