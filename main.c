#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sysexits.h>

#include "utils/logging.h"
#include "utils/errors.h"

#define MAX_LINE 1000
#define REPL_PROMPT_SYMBOL "=>"

bool had_error = false;

void run_from_prompt();
void execute_source(char *source);
void run_from_source(char *file_name);

int main(int argc, char **argv)
{

    error(12, "fucking error");

    if (argc == 1)
    {
        log_debug("Running REPL (PRESS CTRL+D for exit)");
        run_from_prompt();
    }
    else if (argc > 2 || !strcmp(argv[1], "--help"))
    {
        log_info("Usage: none [script]");
        exit(EX_USAGE);
    }
    else
    {
        log_debug("Running from file");
        run_from_source(argv[1]);
    }

    return EXIT_SUCCESS;
}
void execute_source(char *source)
{
    log_debug("Executing source=%s", source);
}

void run_from_source(char *file_name)
{
    log_debug("Reading file=%s", file_name);
    FILE *file = fopen(file_name, "r");

    if (!file)
    {
        log_error("Could not open file, check whether it exists");
        exit(EX_OSFILE);
    }

    log_info("Reading ...");

    long file_length;
    char *buffer = 0;

    fseek(file, 0, SEEK_END);
    file_length = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = malloc(file_length);

    if (buffer)
    {
        fread(buffer, 1, file_length, file);
    }

    if (buffer)
    {
        log_info("%s", buffer);
        execute_source(buffer);

        if (had_error)
        {
            exit(EX_DATAERR);
        }
    }

    log_debug("Closing file=%s", file_name);
    fclose(file);
}

void run_from_prompt()
{
    char line[MAX_LINE];
    printf("%s ", REPL_PROMPT_SYMBOL);

    while (true)
    {
        fgets(line, MAX_LINE, stdin);
        if (feof(stdin))
        {
            log_info("Bye bye!");
            exit(EXIT_SUCCESS);
        }
        execute_source(line);
        had_error = false;
        printf("%s ", REPL_PROMPT_SYMBOL);
    }
}
