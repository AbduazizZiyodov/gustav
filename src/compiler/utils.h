#pragma once

#include "chunk.h"
#include "scanner.h"

Chunk *current_chunk(void);

void token_advance(void);

void token_consume(TokenType type, const char *message);

bool token_check(TokenType type);

bool token_match(TokenType type);
