#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "log.h"
#include "vm.h"

static void repl(void);
static char *read_file(const char *path);
static void run_file(const char *path);

static volatile sig_atomic_t shutdown_requested = 0;

static void gustav_shutdown(int signum)
{
	(void)signum;
	shutdown_requested = 1;
}

int main(int argc, char **argv)
{
	if (signal(SIGINT, gustav_shutdown) == SIG_ERR) {
		gustav_error(EXIT_FAILURE, "Can't set signal handler");
	}

	init_vm();

	if (argc == 1) {
		repl();
	} else if (argc == 2) {
		run_file(argv[1]);
	} else {
		gustav_error(64, "Usage: gustav [path]\n");
	}

	free_vm();

	return EXIT_SUCCESS;
}

void repl(void)
{
#ifdef DEBUG
	char build_type[] = "debug";
#else
	char build_type[] = "release";
#endif

	printf("[compiled version at %s, build_type=%s]\n", __TIME__,
	       build_type);

	char line[LINE_LENGTH];
	for (;;) {
		if (shutdown_requested) {
			break; // = exit
		}

		printf("> ");
		(void)fflush(stdout);

		if (!fgets(line, sizeof(line), stdin)) {
			printf("\n");
			break; // = exit
		}

		interpret(line);
	}
}

// Bob hates me
static char *read_file(const char *path)
{
	FILE *file = fopen(path, "rb");

	if (file == NULL) {
		gustav_error(74, "Could not open file: %s\n", path);
	}

	if (fseek(file, 0L, SEEK_END) != 0) {
		gustav_error(74, "Failed to seek to end: %s\n", path);
	}

	long file_size = ftell(file);

	if (file_size == -1L) {
		gustav_error(74, "Failed to get file size: %s\n", path);
	}

	if (fseek(file, 0L, SEEK_SET) != 0) {
		gustav_error(74, "Failed to rewind file: %s\n", path);
	}

	char *buffer = (char *)malloc((sizeof(char) * (size_t)file_size) + 1);
	if (buffer == NULL) {
		gustav_error(74,
			     "Not enough memory to allocate buffer for %s\n",
			     path);
	}

	size_t bytes_read =
		fread(buffer, sizeof(char), (size_t)file_size, file);

	if (bytes_read < (size_t)file_size) {
		if (ferror(file)) {
			(void)fclose(file);
			free(buffer);
			gustav_error(74, "Could not read the file %s\n", path);
		}
	}

	buffer[bytes_read] = '\0';

	if (fclose(file) != 0) {
		free(buffer);
		gustav_error(74, "Failed to close file: %s\n", path);
	}

	return buffer;
}

static void run_file(const char *path)
{
	char *source = read_file(path);
	interpreter_result_t result = interpret(source);
	free(source);

	switch (result) {
	case INTERPRET_COMPILE_ERROR:
		_Exit(65);
	case INTERPRET_RUNTIME_ERROR:
		_Exit(64);
	default:
		break;
	}
}
