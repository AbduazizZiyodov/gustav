#pragma once

#include "scanner.h"

void error_at(Token *token, const char *message);

void compiler_error(const char *message);

void error_at_current(const char *what);
