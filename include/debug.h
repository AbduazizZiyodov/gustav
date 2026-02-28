#ifndef GUSTAV_DEBUG_H
#define GUSTAV_DEBUG_H

#include "chunk.h"

void disassemble_chunk(chunk_t *chunk, const char *name);
int disassemble_instruction(chunk_t *chunk, int offset);

#endif // GUSTAV_DEBUG_H
