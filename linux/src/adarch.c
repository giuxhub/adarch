#include "stdafx.h"
#include "utils.h"
#include "python_launcher.h"
#include "connection_pool.h"
#include "config.h"
#include "log.h"


static void main_destroy(int exit_status)
{
	logging(DEBUG, L"destroying connection pool");
	connection_pool_destroy();

	logging(DEBUG, L"destroying config");
	config_destroy();

	logging(DEBUG, L"destroying python interpreter");
	py_destroy();

	logging(DEBUG, L"destroying log");
	log_destroy();

	exit(exit_status);
}

static void handle_sigint(void)
{
	main_destroy(EXIT_SUCCESS);
	return;
}

static void handle_sigusr1(void)
{
	//humble mumble...
	return;
}

static void handle_sigusr2(void)
{
	//humble mumble...
	return;
}

static void handle_signal(int signal)
{
	sigset_t pending;

	switch (signal) {

	case SIGUSR1:
		logging(DEBUG, L"signal SIGUSR1 caught");
		handle_sigusr1();
		break;
	case SIGUSR2:
		logging(DEBUG, L"signal SIGUSR2 caught");
		handle_sigusr2();
		break;
	case SIGINT:
		logging(DEBUG, L"signal SIGINT caught");
		handle_sigint();
		break;
	default:
		logging(WARNING, L"Caught wrong signal number(%d)\n", signal);
		return;
	}

	sigpending(&pending);

	if (sigismember(&pending, SIGUSR1))
		handle_sigusr1();

	if (sigismember(&pending, SIGUSR2))
		handle_sigusr2();
}

int main(int argc, char *argv[])
{
	log_init("log.txt", WARNING, 1);

	const char *py_name = "pycore.adarch";
	size_t size = strlen(py_name);
	py_init(py_name, &size);

	struct sigaction sa;
	sa.sa_handler = &handle_signal;
	sa.sa_flags = SA_RESTART;

	sigfillset(&sa.sa_mask);

	if (sigaction(SIGUSR1, &sa, NULL) == -1)
		logging(ERR, L"cannot handle SIGUSR1");

	if (sigaction(SIGUSR2, &sa, NULL) == -1)
		logging(ERR, L"cannot handle SIGUSR2");

	if (sigaction(SIGINT, &sa, NULL) == -1)
		logging(ERR, L"cannot handle SIGINT");

	if (!config_load("adarch.conf"))
		main_destroy(EXIT_FAILURE);

	if (!connection_pool_init(config_get_list()))
		main_destroy(EXIT_FAILURE);

	struct timespec timeout = { 30, 0 };
	while (true)
		utils_sleep(&timeout);

	return 0;
}
