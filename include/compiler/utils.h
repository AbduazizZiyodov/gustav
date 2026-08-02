#pragma once

#include "chunk.h"
#include "scanner.h"

chunk_t *current_chunk(void);

void advance(void);

void consume(TokenType type, const char *message);

bool check(TokenType type);

bool match(TokenType type);
