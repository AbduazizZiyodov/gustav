#pragma once

#include "object.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

FunctionObject *Compiler_Compile(const char *source);
void Compiler_MarkRoots(void);
