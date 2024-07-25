#include <stdbool.h>
#include "logging.h"

extern bool had_error;

void report(int line, char *where, char *message)
{
    log_error("[line %d] Error %s: %s", line, where, message);
    had_error = true;
}

void error(int line, char *message)
{
    report(line, "", message);
}

