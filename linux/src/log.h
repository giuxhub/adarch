#ifndef __INC_LOG_H__
#define __INC_LOG_H__

#include <wchar.h>

typedef enum log_level_e
{
	DEBUG = 0, INFO, WARNING, ERR, CRITICAL
} LOG_LEVEL;

int log_init(const char *filename, LOG_LEVEL lv, int flushrate);
void log_write(LOG_LEVEL lv, const char *file, const char *func, int line,
		const wchar_t *format, ...);
void log_destroy(void);

#define logging(lv, fmt, args...) log_write(lv, __FILE__, __FUNCTION__, __LINE__, fmt, ##args)

#endif
