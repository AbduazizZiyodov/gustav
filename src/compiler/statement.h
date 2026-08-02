#pragma once

#include <stdint.h>

#include "chunk.h"

void statement_parse(void);

void statement_block(void);
void statement_expression(void);
void statement_print(OpCode op);
void statement_if(void);
void statement_while(void);
void statement_for(void);
void statement_return(void);
