#include "stdafx.h"
#include "utils.h"
#include "python_launcher.h"
#include "connection_pool.h"
#include "config.h"
#include "log.h"


#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Python36.lib")

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

static void handle_signal(int signal)
{
	switch (signal) {
	case SIGINT:
		logging(DEBUG, L"signal SIGINT caught");
		handle_sigint();
		break;
	default:
		logging(WARNING, L"Caught wrong signal number(%d)\n", signal);
		return;
	}
}

int main(int argc, char *argv[])
{
	log_init("log.txt", DEBUG, 1);

	const char *py_name = "pycore.adarch";
	size_t size = strlen(py_name);
	py_init(py_name, &size);

	signal(SIGINT, handle_signal);

	if (!config_load("adarch.conf"))
		main_destroy(EXIT_FAILURE);

	if (!connection_pool_init(config_get_list()))
		main_destroy(EXIT_FAILURE);

	struct timeval timeout = { 30, 0 };
	while (true)
		utils_sleep(&timeout);

	return 0;
}
