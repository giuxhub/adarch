#include "stdafx.h"
#include "log.h"

typedef struct log_file {
	HANDLE fp;
	LOG_LEVEL lv;
	int	flushrate;
	int counter;
	HANDLE mux;
} log_file_t;

static log_file_t *log_file = NULL;

int log_init(const char *filename, LOG_LEVEL lv, int flushrate)
{
	assert(filename != NULL);

	if ((log_file = (log_file_t *)calloc(1, sizeof(log_file_t))) == NULL)
		return 1;

	if ((log_file->fp = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		return 1;

	log_file->mux = CreateMutex(NULL, FALSE, NULL);

	log_file->flushrate = flushrate;
	log_file->counter = 0;
	log_file->lv = lv;

	return 0;
}

void log_destroy(void)
{
	assert(log_file != NULL);

	WaitForSingleObject(log_file->mux, INFINITE);
	if (log_file->fp != NULL) {
		FlushFileBuffers(log_file->fp);
		CloseHandle(log_file->fp);
		log_file->fp = NULL;
	}
	CloseHandle(log_file->mux);

	free(log_file);
	log_file = NULL;
}

void log_write(LOG_LEVEL lv, const char *file, const char *func, int line, const wchar_t *format, ...)
{
	assert(log_file != NULL);
	assert(format != NULL);

	wchar_t buf[0xFF] = { 0 };
	wchar_t timestamp[0xFF] = { 0 };
	char to_write[0xFF * 3] = { 0 };
	int len = 0, ret = 0;

	wchar_t *level = L"???";
	va_list args;

	if (lv >= log_file->lv) {
		WaitForSingleObject(log_file->mux, INFINITE);
		time_t ct = 0;
		time(&ct);
		struct tm timeinfo;
		localtime_s(&timeinfo, &ct);

		wcsftime(timestamp, 0xFF, L"%Y/%m/%d:%X", &timeinfo);

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
		{
			_snprintf_s(to_write, (0xFF * 3), (0xFF * 3), "%lu %S [%S] :: %s:%d: %S\n", GetCurrentThreadId(), timestamp, level, func, line, buf);
			len = (int)strnlen_s(to_write, _TRUNCATE);
			WriteFile(log_file->fp, to_write, len, &ret, NULL);
		}
		else
		{
			_snprintf_s(to_write, 0xFF * 3, 0xFF * 3, "%S [%S] :: %s: %S\n", timestamp, level, func, buf);
			len = (int)strnlen_s(to_write, _TRUNCATE);
			WriteFile(log_file->fp, to_write, len, &ret, NULL);
		}

		if ((++log_file->counter % log_file->flushrate) == 0)
			FlushFileBuffers(log_file->fp);

		ReleaseMutex(log_file->mux);
	}
}
