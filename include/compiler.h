#ifndef GUSTAV_COMPILER_H
#define GUSTAV_COMPILER_H

#include <stdbool.h>

#include "chunk.h"
#include "object.h"

bool compile(const char *source, Chunk *chunk);

#endif
