#pragma once

#include "object.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

FunctionObject *compile(const char *source);
void compiler_mark_roots(void);
