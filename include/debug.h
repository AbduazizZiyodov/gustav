#pragma once

#include "chunk.h"

void Debug_DisassembleChunk(Chunk *chunk, const char *name);
int Debug_DisassembleInstruction(Chunk *chunk, int offset);
