#ifndef __INC_CONFIG_H__
#define __INC_CONFIG_H__

#include "stdafx.h"
#include "list.h"

#define CONFIG_MAX_LEN 64

typedef struct config_port_list_t
{
	uint16_t port;
	list_t list;
} config_port_list_t;

typedef struct config_entry_t
{
	char *file;
	char *_class;
	char *method;
	config_port_list_t *ports;
} config_entry_t;

typedef struct config_t
{
	config_entry_t *entry;
	list_t list;
} config_t;

bool config_load(const char *path);
void config_destroy(void);
list_t *config_get_list(void);
//void config_print_test(config_t *config);

#endif
