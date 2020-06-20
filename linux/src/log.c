#include "stdafx.h"
#include "log.h"

typedef struct log_file {
	FILE *fp;
	LOG_LEVEL lv;
	int	flushrate;
	int counter;
	pthread_mutex_t mux;
} log_file_t;

static log_file_t *log_file = NULL;

int log_init(const char *filename, LOG_LEVEL lv, int flushrate)
{
	assert(filename != NULL);

	if ((log_file = (log_file_t *)calloc(1, sizeof(log_file_t))) == NULL)
		return 1;

	if ((log_file->fp = fopen(filename, "a")) == NULL)
		return 1;

	if (pthread_mutex_init(&(log_file->mux), NULL) != 0)
		return 1;

	log_file->flushrate = flushrate;
	log_file->counter = 0;
	log_file->lv = lv;

	return 0;
}

void log_destroy(void)
{
	assert(log_file != NULL);

	pthread_mutex_lock(&(log_file->mux));
	if (log_file->fp != NULL) {
		fflush(log_file->fp);
		fclose(log_file->fp);
		log_file->fp = NULL;
	}
	pthread_mutex_destroy(&(log_file->mux));

	free(log_file);
	log_file = NULL;
}

void log_write(LOG_LEVEL lv, const char *file, const char *func, int line, const wchar_t *format, ...)
{
	assert(log_file != NULL);
	assert(format != NULL);

	wchar_t buf[0xFF] = { 0 };
	wchar_t timestamp[0xFF] = { 0 };
	wchar_t *level = L"???";
	va_list args;
	
	if (lv >= log_file->lv) {
		pthread_mutex_lock(&(log_file->mux));
		time_t ct = 0;
		time(&ct);
		struct tm *timeinfo = localtime(&ct);

		wcsftime(timestamp, 0xFF, L"%Y/%m/%d:%X", timeinfo);

		switch (lv) {
		case DEBUG:
			level = L"DBG";
			break;
		case WARNING:
			level = L"WAR";
			break;
		case INFO:
			level = L"INF";
			break;
		case ERR:
			level = L"ERR";
			break;
		case CRITICAL:
			level = L"CRT";
			break;
		}

		va_start(args, format);
		vswprintf(buf, 0xFF, format, args);
		va_end(args);

		if (log_file->lv <= DEBUG)
			fwprintf(log_file->fp, L"%lu %S [%S] :: %s:%d: %S\n", pthread_self(), timestamp, level, func, line, buf);
		else
			fwprintf(log_file->fp, L"%S [%S] :: %s: %S\n", timestamp, level, func, buf);
		if ((++log_file->counter % log_file->flushrate) == 0)
			fflush(log_file->fp);

		pthread_mutex_unlock(&(log_file->mux));
	}
}
