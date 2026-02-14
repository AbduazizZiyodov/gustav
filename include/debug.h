#ifndef GUSTAV_DEBUG_H
#define GUSTAV_DEBUG_H

#include "chunk.h"

void disassemble_chunk(chunk_t *chunk, const char *name);
size_t disassemble_instruction(chunk_t *chunk, size_t offset);

#endif // GUSTAV_DEBUG_H
