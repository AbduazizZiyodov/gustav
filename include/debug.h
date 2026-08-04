#pragma once

#include "chunk.h"

void debug_disassemble_chunk(Chunk *chunk, const char *name);
int debug_disassemble_instruction(Chunk *chunk, int offset);
