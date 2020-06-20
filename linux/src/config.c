#include "utils.h"
#include "log.h"
#include "jsmn.h"
#include "config.h"

static config_t *config = NULL;

static config_entry_t *config_init_entry(void)
{
	config_entry_t *entry;

	utils_create(entry, config_entry_t, 1);
	utils_create(entry->ports, config_port_list_t, 1);
	utils_create(entry->file, char, CONFIG_MAX_LEN);
	utils_create(entry->_class, char, CONFIG_MAX_LEN);
	utils_create(entry->method, char, CONFIG_MAX_LEN);
	list_init(&(entry->ports->list));

	return entry;
}

static void config_free_entry_ports(config_port_list_t *node)
{
	utils_free(node);
}

static void config_free(config_t *config)
{
	assert(config != NULL);

	config_port_list_t *curr, *nn;
	list_destroy(curr, nn, config->entry->ports, list, config_free_entry_ports);

	utils_free(config->entry->file);
	utils_free(config->entry->_class);
	utils_free(config->entry->method);
	utils_free(config->entry);
	utils_free(config);
}

/*
 static const char *CONFIG_EXAMPLE =
 "{\"example\": {\n \"file\": \"py_file\",\n \"class\": \"Multiply\",\n \"method\": \"multiply\",\n "
 "\"ports\": [6020, 6021, 6022]\n}"
 "\"example2\": {\n \"file\": \"py_file\",\n \"class\": \"Multiply\",\n \"method\": \"multiply2\",\n "
 "\"ports\": [6020, 6021, 6022]\n}}";
 */

static int config_json_eq(const char *json, jsmntok_t *tok, const char *s)
{
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start
			&& strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

static config_entry_t *config_json_parse_obj(jsmntok_t *t, int *i,
		const char *str)
{
	u_int j = 0;
	jsmntok_t *g = NULL;
	config_entry_t *entry = NULL;
	config_port_list_t *tmp = NULL;

	if (t[*i + 1].type != JSMN_OBJECT)
		return NULL;

	if ((entry = config_init_entry()) == NULL)
		return NULL;

	g = &t[*i + 2];
	u_int to = t[*i + 1].size;
	for (j = 0; j < to; j++) {
		int idx = j * 2;
		if (config_json_eq(str, &g[idx], "file") == 0) {
			snprintf(entry->file, CONFIG_MAX_LEN, "%.*s", g[idx + 1].end - g[idx + 1].start,
					str + g[idx + 1].start);

			*i += 1;
		} else if (config_json_eq(str, &g[idx], "class") == 0) {
			snprintf(entry->_class, CONFIG_MAX_LEN, "%.*s",
					g[idx + 1].end - g[idx + 1].start, str + g[idx + 1].start);

			*i += 1;
		} else if (config_json_eq(str, &g[idx], "method") == 0) {
			snprintf(entry->method, CONFIG_MAX_LEN, "%.*s",
					g[idx + 1].end - g[idx + 1].start, str + g[idx + 1].start);

			*i += 1;
		} else if (config_json_eq(str, &g[idx], "ports") == 0) {
			int y;
			if (g[idx + 1].type != JSMN_ARRAY) {
				continue;
			}
			for (y = 0; y < g[idx + 1].size; y++) {
				jsmntok_t *gg = &g[idx + y + 2];
				utils_create(tmp, config_port_list_t, 1);

				char tmp_buf[CONFIG_MAX_LEN] = { 0 };
				snprintf(tmp_buf, CONFIG_MAX_LEN, "%.*s", gg->end - gg->start,
						str + gg->start);

				tmp->port = (uint16_t) strtol(tmp_buf, NULL, 10);
				list_add(&(tmp->list), &(entry->ports->list));
			}
			*i += g[idx + 1].size + 1;
		}
	}
	*i += to + 1;

	return entry;
}

static bool config_json_parse(config_t *config, const char *str)
{
	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[128]; /* We expect no more than 128 tokens */

	jsmn_init(&p);
	r = jsmn_parse(&p, str, strlen(str), t, sizeof(t) / sizeof(t[0]));
	if (r < 0) {
		logging(ERR, L"Failed to parse JSON: %d", r);
		return false;
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		logging(ERR, L"Object expected");
		return false;
	}

	config_t *tmp = NULL;

	for (i = 1; i < r; i++) {
		if (config_json_eq(str, &t[i], "tool_1") == 0) {
			utils_create(tmp, config_t, 1);
			tmp->entry = config_json_parse_obj(t, &i, str);
			list_add(&(tmp->list), &(config->list));
		} else if (config_json_eq(str, &t[i], "tool_2") == 0) {
			utils_create(tmp, config_t, 1);
			tmp->entry = config_json_parse_obj(t, &i, str);
			list_add(&(tmp->list), &(config->list));
		} else {
			logging(WARNING, L"Unexpected key: %.*s", t[i].end - t[i].start,
					str + t[i].start);
		}
	}

	return true;
}

void config_destroy(void)
{
	assert(config != NULL);

	config_t *curr, *nn;
	list_destroy(curr, nn, config, list, config_free);
}

list_t *config_get_list(void)
{
	assert(config != NULL);

	return &(config->list);
}
/*
 void config_print_test(config_t *config)
 {
 config_t *tmp;
 config_port_list_t *entry_tmp;
 list_t *cur, *n = NULL;
 list_for_each(cur, n, &(config->list)){
 tmp = list_entry(cur, config_t, list);
 printf("file %s\n", tmp->entry->file);
 printf("class %s\n", tmp->entry->file);
 printf("method %s\n", tmp->entry->file);
 list_t *curr, *nn = NULL;
 list_for_each(curr, nn, &(tmp->entry->ports->list)){
 entry_tmp = list_entry(curr, config_port_list_t, list);
 printf("port %d\n", entry_tmp->port);
 }
 }
 }
 */
bool config_load(const char *path)
{
	assert(path != NULL);

	utils_create(config, config_t, 1);
	list_init(&(config->list));


	FILE *fp = NULL;

	if ((fp = fopen(path, "r")) == NULL) {
		logging(CRITICAL, L"unable to find the config file");
		return false;
	}

	char in[8192] = { 0 };
	char tmp[8192] = { 0 };
	size_t r = 0;

	while ((r = utils_readline(fp, tmp, 512)) != -1) {
		strncat(in, tmp, r);
		buffer_clean(tmp);
	}

	fclose(fp);

	config_json_parse(config, in);
	return true;
}
