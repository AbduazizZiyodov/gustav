#ifndef GUSTAV_COMPILER_H
#define GUSTAV_COMPILER_H

#include <stdbool.h>

#include "chunk.h"

bool compile(const char *source, chunk_t *chunk);

#endif
