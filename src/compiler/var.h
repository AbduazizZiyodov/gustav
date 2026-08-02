#pragma once

#include <stdint.h>

#include "scanner.h"

uint8_t parse_variable(const char *error_message);

void define_variable(uint8_t global);

void var_declaration(void);

void declare_variable(void);

void named_variable(Token name, bool can_assign);
