#ifndef __INC_CONNECTION_POOL_H_
#define __INC_CONNECTION_POOL_H_

#include "connection.h"
#include "list.h"

typedef struct connection_pool_t
{
	connection_t *conn;
	list_t list;
} connection_pool_t;

bool connection_pool_init(list_t *config);
void connection_pool_destroy(void);
connection_t *connection_pool_find(int fd);

#endif
