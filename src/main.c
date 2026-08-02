#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cli.h"
#include "common.h"
#include "log.h"
#include "vm.h"

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

	show_gustav_info();

	VM_Init();

	int status = EXIT_SUCCESS;

	if (argc == 1) {
		LOG_TRACE("Running from repl()\n");
		repl();
	} else if (argc == 2) {
		LOG_TRACE("Running from run_file()\n");
		status = run_file(argv[1]);
	} else {
		gustav_error(64, "Usage: gustav [path]\n");
	}

	VM_Free();

	return status;
}
