#include "stdafx.h"
#include "connection.h"
#include "connection_task.h"
#include "utils.h"
#include "log.h"

#define RECV_TIMEOUT 16
#define MAX_LEN 2048

static ssize_t connection_read_socket_by_line(connection_activelist_t *curr_a, char *buffer, size_t len, int flags, int to, connection_t *conn)
{
	fd_set readset;
	int fd = curr_a->fd;
	ssize_t result;
	struct timeval tv;

	if (connection_get_status(conn) || connection_active_getstatus(conn->conn_a, curr_a))
		return CONN_SHUTDOWN;

	FD_ZERO(&readset);
	FD_SET(fd, &readset);

	tv.tv_sec = 0;
	tv.tv_usec = to * 1000;

	if ((result = select(FD_SETSIZE, &readset, NULL, NULL, &tv)) < 0)
		return CONN_SELECT;
	else if (result > 0 && FD_ISSET(fd, &readset)) {
		u_long non_blocking = 1;
		ioctlsocket(fd, FIONBIO, &non_blocking);

		result = utils_socket_read(fd, buffer, len);

		non_blocking = 0;
		ioctlsocket(fd, FIONBIO, &non_blocking);

		return result;
	}
	return CONN_TIMEOUT;
}

static int connection_accept_socket(int fd, struct sockaddr *__restrict addr, socklen_t *__restrict len, int to, connection_t *conn)
{
	WaitForSingleObject(conn->accept_mux, INFINITE);

	if (connection_get_status(conn)) {
		ReleaseMutex(conn->accept_mux);
		return CONN_SHUTDOWN;
	}

	fd_set acceptset;
	int result;
	struct timeval tv;

	FD_ZERO(&acceptset);
	FD_SET(fd, &acceptset);

	tv.tv_sec = 0;
	tv.tv_usec = to * 1000;

	if ((result = select(FD_SETSIZE, &acceptset, NULL, NULL, &tv)) < 0) {
		ReleaseMutex(conn->accept_mux);
		return CONN_SELECT;
	}
	else if (result > 0 && FD_ISSET(fd, &acceptset)){
		u_long non_blocking = 1;
		ioctlsocket(fd, FIONBIO, &non_blocking);

		result = accept(fd, (struct sockaddr *)addr, len);

		non_blocking = 0;
		ioctlsocket(fd, FIONBIO, &non_blocking);

		ReleaseMutex(conn->accept_mux);

		return result;
	}

	ReleaseMutex(conn->accept_mux);
	return CONN_TIMEOUT;
}

static void *connection_handler(void *input)
{
	assert(input != NULL);

	connection_t *conn = (connection_t *)input;

	while (!connection_get_status(conn)) {
		struct sockaddr_in sin;
		socklen_t address_size = sizeof(sin);
		ssize_t r = 0;
		int x = 0;
		uint16_t port = 0;
		char client_ip[INET_ADDRSTRLEN] = { 0 };

		x = connection_accept_socket(conn->sock, (struct sockaddr *) &sin,
			&address_size, 250, conn);
		if (0 > x) {
			if (x == CONN_SHUTDOWN)
				break;
			else if (x == CONN_TIMEOUT)
				continue;
			else
				logging(ERROR, L"socket accept errno(%d) x(%d)", errno, x);
		}
		if (getsockname(x, (struct sockaddr *)&sin, &address_size) == 0)
			logging(ERROR, L"socket getsockopt errno(%d) x(%d)", WSAGetLastError(), x);

		inet_ntop(AF_INET, &sin.sin_addr, client_ip, INET_ADDRSTRLEN);
		port = ntohs(sin.sin_port);

		connection_activelist_t *curr_a = connection_active_add(conn->conn_a, x);
		connection_active_setinstance(conn->conn_a, curr_a,
			connection_task_build(conn->task, x, client_ip, port));

		char buffer[MAX_LEN];
		do {
			buffer_clean(buffer);
			r = connection_read_socket_by_line(curr_a, buffer, MAX_LEN, 0, 250,
				conn);
			if (0 > r) {
				if (errno == EINTR || r == CONN_TIMEOUT)
					continue;
				else if (errno == EINVAL || r == CONN_SHUTDOWN)
					break;
				else
					break;
			}

			PyObject *ret = connection_task_do(conn->task, curr_a->instance,
				buffer);
			py_release_reference_th_safe(ret);

		} while (!connection_active_getstatus(conn->conn_a, curr_a));

		connection_active_close(conn->conn_a, curr_a);
	}

	return NULL;
}

connection_t *connection_init(uint16_t port, int max_conn)
{
	int l = 0;
	connection_t *connection = NULL;

	int iResult;
	WSADATA wsaData;
	struct addrinfo hints, *res;
	char port_str[0xFF] = { 0 };

	if ((iResult = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0){
		logging(CRITICAL, L"WSAStartup failed: %d", iResult);
		return NULL;
	}

	do {
		utils_create(connection, connection_t, 1);

		if ((connection->conn_a = connection_active_init()) == NULL) {
			utils_free(connection->task);
			utils_free(connection);
			WSACleanup();
			logging(CRITICAL, L"active connection list init error");
			break;
		}

		ZeroMemory(&hints, sizeof(hints));

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		sprintf_s(port_str, 0xFF, "%d", port);
		if (getaddrinfo(NULL, port_str, &hints, &res) != 0) {
			logging(CRITICAL, L"socket getaddrinfo errno(%d)", WSAGetLastError());
			WSACleanup();
			break;
		}

		if ((connection->sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == INVALID_SOCKET) {
			utils_free(connection);
			logging(CRITICAL, L"socket init error(%d)", WSAGetLastError());
			WSACleanup();
			break;
		}

		if(bind(connection->sock, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR) {
			utils_free(connection);
			logging(CRITICAL, L"socket bind error(%d)", WSAGetLastError());
			WSACleanup();
			break;
		}

		if ((l = listen(connection->sock, max_conn)) == -1){
			utils_free(connection);
			logging(CRITICAL, L"socket listen error(%d)", WSAGetLastError());
			WSACleanup();
			break;
		}

		SECURITY_ATTRIBUTES security = { sizeof(security), NULL, true };

		if ((connection->accept_mux = CreateMutex(&security, FALSE, NULL)) == NULL) {
			utils_free(connection);
			WSACleanup();
			break;
		}

		if ((connection->shutdown_mux = CreateMutex(&security, FALSE, NULL)) == NULL) {
			utils_free(connection);
			WSACleanup();
			break;
		}
		if (!(connection->pool = threadpool_init(max_conn, max_conn))){
			utils_free(connection);
			WSACleanup();
			logging(CRITICAL, L"connection threadpool error");
			break;
		}
	} while (0);

	return connection;
}

void connection_add_task(connection_t *conn, connection_task_t *task)
{
	assert(conn != NULL && task != NULL && conn->task == NULL && !conn->_started);

	conn->task = task;
}

void connection_start_threadpool(connection_t *conn, u_int max_conn)
{
	assert(conn != NULL && conn->task != NULL && !conn->_started && max_conn > 0);

	conn->_started = true;
	u_int i;
	for (i = 0; i < max_conn; i++)
		threadpool_add_task(conn->pool, connection_handler, conn);
}

bool connection_get_status(connection_t *conn)
{
	assert(conn != NULL);

	WaitForSingleObject(conn->shutdown_mux, INFINITE);
	bool ret = conn->_shutdown;
	ReleaseMutex(conn->shutdown_mux);

	return ret;
}

void connection_destroy(connection_t *conn, void(*task_free)(connection_task_t *))
{
	assert(conn != NULL);

	int err = 0;

	WaitForSingleObject(conn->shutdown_mux, INFINITE);
	if (!conn->_shutdown)
		conn->_shutdown = true;

	ReleaseMutex(conn->shutdown_mux);

	if ((err = threadpool_destroy(conn->pool)) < 0)
		logging(ERROR, L"destroy pool error code (%d)", err);

	connection_active_destroy(conn->conn_a);

	WaitForSingleObject(conn->accept_mux, INFINITE);
	CloseHandle(conn->accept_mux);
	WaitForSingleObject(conn->shutdown_mux, INFINITE);
	CloseHandle(conn->shutdown_mux);

	if (conn->sock)
		closesocket(conn->sock);

	task_free(conn->task);
	utils_free(conn);
	WSACleanup();
}
