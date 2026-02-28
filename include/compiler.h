#ifndef GUSTAV_COMPILER_H
#define GUSTAV_COMPILER_H

#include "object.h"
#include <stdbool.h>

function_t *compile(const char *source);
void mark_compiler_roots(void);

#endif // GUSTAV_COMPILER_H
