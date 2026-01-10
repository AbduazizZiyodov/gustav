#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "vm.h"

static void repl(void);
static char *read_file(const char *path);
static void run_file(const char *path);

static void gustav_shutdown(int sig_num)
{
	printf("\nCaught SIGINT(%d), exiting ...\n", sig_num);
	free_vm();
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	if (signal(SIGINT, gustav_shutdown) == SIG_ERR) {
		fprintf(stderr,
			"An error occurred while setting a signal handler.\n");
		return EXIT_FAILURE;
	}

	init_vm();

	if (argc == 1) {
		repl();
	} else if (argc == 2) {
		run_file(argv[1]);
	} else {
		fprintf(stderr, "Usage: gustav [path]\n");
		exit(64);
	}

	free_vm();
	return EXIT_SUCCESS;
}

static void repl(void)
{
	char line[LINE_LENGTH];
	unsigned long long command_num = 0;

	while (true) {
		printf("[%lld] => ", command_num++);
		fflush(stdout);

		if (!fgets(line, sizeof(line), stdin)) {
			printf("\n");
			break;
		}

		interpret(line);
	}
}

static char *read_file(const char *path)
{
	FILE *file = fopen(path, "rb");

	if (file == NULL) {
		fprintf(stderr, "Could not open file: %s\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);

	size_t file_size = (size_t)ftell(file);
	rewind(file);

	char *buffer = (char *)malloc(sizeof(char) * file_size + 1);

	if (buffer == NULL) {
		fprintf(stderr, "Not enough memory to allocate buffer for %s\n",
			path);
		exit(74);
	}

	size_t bytes_read = fread(buffer, sizeof(char), file_size, file);

	if (bytes_read < file_size) {
		fprintf(stderr, "Could not read the file %s\n", path);
		exit(74);
	}
	buffer[bytes_read] = '\0';

	fclose(file);

	return buffer;
}

static void run_file(const char *path)
{
	char *source = read_file(path);
	InterpretResult result = interpret(source);
	free(source);

	switch (result) {
	case INTERPRET_COMPILE_ERROR:
		exit(65);
	case INTERPRET_RUNTIME_ERROR:
		exit(64);
	default:
		break;
	}
}
